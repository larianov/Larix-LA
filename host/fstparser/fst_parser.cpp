#include <cstdlib>
#include <fcntl.h>
#include "fst_parser.hpp"
#include <cstdint>
#include <format>
#include <span>
#include <string>
#include <sys/types.h>
#include <utility>
#include <vector>
extern "C" {
    #include "fstapi.h"
}



handling_uart::handling_uart(uint8_t rx, uint8_t tx, uint32_t baudrate, uint32_t freq) : tx_((freq*256ULL)/baudrate), tx_idx(tx), rx_((freq*256ULL)/baudrate), rx_idx(rx) {
}

std::pair<std::string, std::string> handling_uart::update(std::vector<std::pair<uint8_t, fstHandle>> &arr){
    std::pair<std::string, std::string>to_ret;
    to_ret.first = tx_.update(arr[tx_idx].first);
    to_ret.second = rx_.update(arr[rx_idx].first);
    return to_ret;
}

handling_uart_member::handling_uart_member(uint64_t bondclocking) : bond_clocking(bondclocking){
}


std::string handling_uart_member::update(uint8_t bit_now){
    std::string to_ret;
    bool is_error{};
    clk_out+=256;
    if (!sync && bit_now == 1) sync = true;
    switch (state) {
        case uart_states::IDLE:
            clk_out = 0;
            bit_placeholder = 0;
            ammount_of_bits = 0;
            if (bit_now == 0 && sync && last_bit == 1) {
                state = uart_states::POT_START;
            }
            break;
        case uart_states::POT_START:
            if (clk_out >= bond_clocking/2) {
                if (bit_now == 0) {
                    state = uart_states::READ_BYTE;
                    clk_out -= bond_clocking/2;
                    to_ret = "START";
                    if (good_frames >= 12) 
                        output_enabled = true;
                    break;
                }
                else state = uart_states::IDLE;
            }
            break;
        case uart_states::READ_BYTE:
            if (clk_out >= bond_clocking) {
                bit_placeholder |= (bit_now << ammount_of_bits);
                ammount_of_bits++;
                to_ret = {std::format("DATA: {:02X}/{:#04X}", bit_placeholder, bit_placeholder)};
                clk_out -= bond_clocking;
                if (ammount_of_bits == 8) {
                    state = uart_states::STOP;
                }
            }
            break;
        case uart_states::STOP:
            if (clk_out >= bond_clocking) {
                if (bit_now == 1) {
                    to_ret = std::format("STOP, FIN DATA: {:02X}/{:#04X}", bit_placeholder, bit_placeholder);
                    if (good_frames < 16) {
                        good_frames++;
                    }
                }
                else{
                    to_ret = std::format("FRAME_ER, ER DATA: {:02X}/{:#04X}", bit_placeholder, bit_placeholder);
                    is_error = true;
                }
                state = uart_states::IDLE;
            }       
            break;
    }
    last_bit = bit_now;
    if (is_error) {
        good_frames = 0;
        output_enabled = false;
    }
    else if (!output_enabled) {
        to_ret = "";
    }
    return to_ret;
}


handling_spi::handling_spi(uint8_t mosi, uint8_t miso, uint8_t clk, uint8_t cs) : mosi_idx(mosi), miso_idx(miso), clk_idx(clk), cs_idx(cs){
}

std::pair<std::string, std::string>handling_spi::update(std::vector<std::pair<uint8_t, fstHandle>> &arr){
    std::pair<std::string, std::string> to_ret;
    if (arr[cs_idx].first == 0) {
        if (last_clock == 0 && arr[clk_idx].first == 1) {
            to_ret.first = mosi_.update(arr[mosi_idx].first);
            to_ret.second = miso_.update(arr[miso_idx].first);
        }
        else if (last_cs == 1) {
            to_ret.first = "WAITING FOR A CLK";
            to_ret.second = "WAITING FOR A CLK";
        }
    }
    else if (arr[cs_idx].first == 1 && last_cs == 0) {
        to_ret.first = mosi_.update(2);
        to_ret.second = miso_.update(2);
        if (to_ret.first.empty()) {
            to_ret.first = "CS_WAITING";
        }
        if (to_ret.second.empty()) {
            to_ret.second = "CS_WAITING";
        }
    }
    else {
        to_ret.first = "CS_WAITING";
        to_ret.second = "CS_WAITING";
    }
    last_clock = arr[clk_idx].first;
    last_cs = arr[cs_idx].first;
    return to_ret;
}


std::string handling_spi_member::update(uint8_t bit_now){
    std::string to_ret{};
    if (bit_now == 2) {
        if(ammount_of_bits == 8)
         to_ret = std::format("FIN DATA: {:02X}/{:#04X}", byte, byte);
        else if (ammount_of_bits > 0) 
         to_ret = std::format("ERROR: by early cs: {}", byte);
        else
         to_ret = "";
        byte = 0;
        ammount_of_bits = 0;
    }
    else {
        byte = (byte << 1) | bit_now;
        to_ret = std::format("BIT: {}", ammount_of_bits+1);
        ammount_of_bits++;
        if (ammount_of_bits == 8) {
            to_ret = std::format("FIN DATA: {:02X}/{:#04X}", byte, byte);
            byte = 0;
            ammount_of_bits = 0;
        }
    }
    return to_ret;
}

void fst_parse(std::span<const uint8_t> to_parse, const logic_an_input &config, const std::string &name_of_file,
               const std::vector<t_decoders> &ver) {
    uint64_t step_fs = 1'000'000'000'000'000ULL / config.hz;
    auto *fst_object = fstWriterCreate(name_of_file.c_str(), 1);
    fstWriterSetTimescale(fst_object, -15);
    fstWriterSetScope(fst_object, FST_ST_VCD_MODULE, "GPIO", nullptr);
    std::vector<std::pair<uint8_t, fstHandle>> arr;
    std::vector<std::pair<handling_i2c, fstHandle>> i2c_handlers;
    std::vector<std::pair<handling_uart, std::pair<fstHandle, fstHandle>>> uart_handlers;
    std::vector<std::pair<handling_spi, std::pair<fstHandle, fstHandle>>> spi_handlers;

    for (uint8_t i{}; i < config.amm + 1; i++) {
        auto temp = std::string("gpio" + std::to_string(config.channel + i));
        for (const auto &v : ver) {
            if (v.first_ch == config.channel + i) {
                if (v.code == c_decoders::I2C)
                    temp += " (I2C_SDA)";
                if (v.code == c_decoders::SPI)
                    temp += " (SPI_MOSI)";
                if (v.code == c_decoders::UART)
                    temp += " (UART_RX)";
                break;
            } else if (v.second_ch == config.channel + i) {
                if (v.code == c_decoders::I2C)
                    temp += " (I2C_SCL)";
                if (v.code == c_decoders::SPI)
                    temp += " (SPI_MISO)";
                if (v.code == c_decoders::UART)
                    temp += " (UART_TX)";
                break;
            } else if (v.third_ch.has_value() && v.third_ch.value() == config.channel + i) {
                temp += " (SPI_SCK)";
                break;
            }
            else if (v.fourth_ch.has_value() && v.fourth_ch.value() == config.channel + i) {
                temp += " (SPI_CS)";
                break;
            }
        }
        arr.emplace_back(
            std::pair{255, fstWriterCreateVar(fst_object, FST_VT_VCD_WIRE, FST_VD_INPUT, 1, temp.c_str(), 0)});
    }
    for (const auto &v : ver) {
        if (v.code == c_decoders::I2C) {
            handling_i2c to_save{static_cast<uint8_t>(v.first_ch - config.channel),
                                 static_cast<uint8_t>(v.second_ch - config.channel)};
            i2c_handlers.emplace_back(
                to_save, fstWriterCreateVar(fst_object, FST_VT_GEN_STRING, FST_VD_IMPLICIT, 1,
                                            std::string(std::format("I2C[{}]{{{}}}-{{{}}}", i2c_handlers.size(),
                                                                    v.first_ch, v.second_ch))
                                                .c_str(),
                                            0));
        }
        if (v.code == c_decoders::UART) {
            handling_uart to_save{static_cast<uint8_t>(v.first_ch - config.channel),
                                  static_cast<uint8_t>(v.second_ch - config.channel), v.baudrate.value(), config.hz};
            uart_handlers.emplace_back(
                to_save,
                std::pair(fstWriterCreateVar(
                     fst_object, FST_VT_GEN_STRING, FST_VD_IMPLICIT, 1,
                     std::string(std::format("UART_TX[{}]{{{}}}", uart_handlers.size(), v.second_ch)).c_str(), 0),                 fstWriterCreateVar(
                     fst_object, FST_VT_GEN_STRING, FST_VD_IMPLICIT, 1,
                     std::string(std::format("UART_RX[{}]{{{}}}", uart_handlers.size(), v.first_ch)).c_str(), 0)));
        }
        if (v.code == c_decoders::SPI) {
            handling_spi to_save{static_cast<uint8_t>(v.first_ch - config.channel), static_cast<uint8_t>(v.second_ch - config.channel), static_cast<uint8_t>(v.third_ch.value() - config.channel), static_cast<uint8_t>(v.fourth_ch.value() - config.channel)};
            auto _to_save = std::pair(fstWriterCreateVar(
                 fst_object, FST_VT_GEN_STRING, FST_VD_IMPLICIT, 1,
                 std::string(std::format("SPI_MOSI[{}]{{{}}}", spi_handlers.size(), v.first_ch)).c_str(), 0),                
                 fstWriterCreateVar(
                 fst_object, FST_VT_GEN_STRING, FST_VD_IMPLICIT, 1,
                 std::string(std::format("SPI_MISO[{}]{{{}}}", spi_handlers.size(), v.second_ch)).c_str(), 0));
            spi_handlers.emplace_back(to_save, _to_save);
        }
    }
    uint64_t current_time_fs = 0;
    fstWriterSetUpscope(fst_object);
    fstWriterEmitTimeChange(fst_object, 0);

    for (auto v : to_parse) {
        fstWriterEmitTimeChange(fst_object, current_time_fs);
        for (uint8_t i{}; i < config.amm + 1; i++) {
            uint8_t bit = (v >> i) & 1;
            if (bit != arr[i].first) {
                if (bit == 0)
                    fstWriterEmitValueChange(fst_object, arr[i].second, "0");
                else
                    fstWriterEmitValueChange(fst_object, arr[i].second, "1");
                arr[i].first = bit;
            }
        }
        for (auto &handler : i2c_handlers) {
            auto temp = handler.first.update(arr);
            if (!temp.empty())
                fstWriterEmitVariableLengthValueChange(fst_object, handler.second, temp.c_str(), temp.size());
        }
        for (auto &handler: uart_handlers) {
            auto temp = handler.first.update(arr);
            if (!temp.first.empty()) {
                fstWriterEmitVariableLengthValueChange(fst_object, handler.second.first, temp.first.c_str(), temp.first.size());
            }
            if (!temp.second.empty()) {
                fstWriterEmitVariableLengthValueChange(fst_object, handler.second.second, temp.second.c_str(), temp.second.size());
            }
        }
        for(auto &handler : spi_handlers){
            auto temp = handler.first.update(arr);
            if (!temp.first.empty()) {
                fstWriterEmitVariableLengthValueChange(fst_object, handler.second.first, temp.first.c_str(), temp.first.size());
            }
            if (!temp.second.empty()) {
                fstWriterEmitVariableLengthValueChange(fst_object, handler.second.second, temp.second.c_str(), temp.second.size());
            }
        }
        current_time_fs += step_fs;
    }
    fstWriterClose(fst_object);
}


