#include "spi_decoder.hpp"
#include "decoders.hpp"
#include <cstdint>
#include <format>
#include <utility>
#include <vector>
SPI_decoder::SPI_decoder(uint8_t cs, uint8_t MOSI, uint8_t MISO, uint8_t SCK, uint8_t byte_size, uint8_t spi_mode, bool msb_on) : cs_(cs), MOSI_(MOSI), mosi_(byte_size, msb_on), MISO_(MISO), miso_(byte_size, msb_on), SCK_(SCK), spi_mode_(spi_mode){};

void SPI_decoder::update(uint8_t byte, std::vector<Event>&ev){
    std::pair<std::string, std::string> to_ret;
    if (((byte >> cs_) & 1) == 0) {
        if (last_clock == !(spi_mode_ % 3 == 0 ? 1 : 0) && ((byte >> SCK_) & 1) == (spi_mode_ % 3 == 0 ? 1 : 0)) {
            to_ret.first = mosi_.update((byte >> MOSI_)&1);
            to_ret.second = miso_.update((byte >> MISO_)&1);
        }
        else if (last_cs == 1) {
            to_ret.first = "WAITING FOR A CLK";
            to_ret.second = "WAITING FOR A CLK";
        }
    }
    else if (((byte >> cs_)&1) == 1 && last_cs == 0) {
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
    last_clock = ((byte >> SCK_) & 1);
    last_cs = ((byte >> cs_) & 1);
    if (!to_ret.first.empty() && last_to_ret.first != to_ret.first) {
        ev.emplace_back(0, to_ret.first);
    }
    if (!to_ret.second.empty() && last_to_ret.second != to_ret.second) {
        ev.emplace_back(1, to_ret.second);
    }
    last_to_ret = to_ret;
}


std::string handling_spi_member::update(uint8_t bit_now){
    std::string to_ret{};
    if (bit_now == 2) {
        if(ammount_of_bits == byte_size_)
         to_ret = std::format("FIN DATA: {:02X}/{:#04X}", byte, byte);
        else if (ammount_of_bits > 0) 
         to_ret = std::format("ERROR: by early cs: {}", byte);
        else
         to_ret = "";
        byte = 0;
        ammount_of_bits = 0;
    } else {
        if (msb_on_) {
            byte = (byte << 1) | bit_now;
        }
        else {
            byte |= (bit_now << ammount_of_bits);
        }
        to_ret = std::format("BIT: {}", ammount_of_bits+1);
        ammount_of_bits++;
        if (ammount_of_bits == byte_size_) {
            to_ret = std::format("FIN DATA: {:02X}/{:#04X}", byte, byte);
            byte = 0;
            ammount_of_bits = 0;
        }
    }
    return to_ret;
}
