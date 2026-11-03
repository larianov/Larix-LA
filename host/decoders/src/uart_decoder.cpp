#include "uart_decoder.hpp"
#include <cstdint>
#include <format>
UART_decoder::UART_decoder(uint8_t TX, uint8_t RX, uint8_t byte_size, uint8_t parity, uint8_t amm_stop_bits, uint64_t baudrate, uint64_t freq) : TX_(TX), tx_((freq*256ULL)/baudrate, byte_size, parity, amm_stop_bits), RX_(RX), rx_((freq*256ULL)/baudrate, byte_size, parity, amm_stop_bits){};


void UART_decoder::update(uint8_t byte, std::vector<Event> &ev){
    std::pair<std::string, std::string>to_ret;
    to_ret.first = tx_.update((byte >> TX_)&1);
    to_ret.second = rx_.update((byte >> RX_)&1);
    if (!to_ret.first.empty()) {
        ev.emplace_back(0, to_ret.first);
    }
    if (!to_ret.second.empty()) {
        ev.emplace_back(1, to_ret.second);
    }
}

handling_uart_member::handling_uart_member(uint64_t bondclocking, uint8_t byte_size, uint8_t parity, uint8_t amm_stop_bits) : bond_clocking(bondclocking), byte_size_(byte_size), parity_(parity), amm_stop_bits_(amm_stop_bits){
}


std::string handling_uart_member::update(uint8_t bit_now){
    std::string to_ret;
    bool is_error{};
    clk_out+=256;
    if (!sync && bit_now == 1) sync = true;
    switch (state) {
        case uart_states::IDLE:
            clk_out = 0;
            byte_placeholder = 0;
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
                byte_placeholder |= (bit_now << ammount_of_bits);
                ammount_of_bits++;
                to_ret = {std::format("DATA: {:02X}/{:#04X}", byte_placeholder, byte_placeholder)};
                clk_out -= bond_clocking;
                if (ammount_of_bits == byte_size_) {
                    if (parity_ == 0) {
                        state = uart_states::STOP;
                        ammount_of_bits = 0;
                    }
                    else {
                        state = uart_states::PARITY;
                    }
                }
            }
            break;
        case uart_states::PARITY:
        {
            if (clk_out >= bond_clocking) {
                uint8_t acc{};
                acc^=bit_now;
                for(uint8_t i{}; i < byte_size_; i++){
                    acc ^= (byte_placeholder >> i) & 1;
                }
                if ((acc != 0 && parity_ == 2) || (acc != 1 && parity_ == 1)) {
                    to_ret = "PAIRITY ERR";
                    is_error = true;
                    state = uart_states::IDLE;
                }
                else {
                    to_ret = "PAIRITY OK";
                    state = uart_states::STOP;
                    ammount_of_bits = 0;
                }
                clk_out-=bond_clocking;
            }
            break;
        }
        case uart_states::STOP:
            if (clk_out >= bond_clocking) {
                if (bit_now == 1) {
                    if (++ammount_of_bits == amm_stop_bits_) {
                        to_ret = std::format("STOP, FIN DATA: {:02X}/{:#04X}", byte_placeholder, byte_placeholder);
                        if (good_frames < 16) {
                            good_frames++;
                        }                    
                        state = uart_states::IDLE;
                    }
                }
                else{
                    to_ret = std::format("FRAME_ER, ER DATA: {:02X}/{:#04X}", byte_placeholder, byte_placeholder);
                    is_error = true;
                    state = uart_states::IDLE;
                }
                clk_out -= bond_clocking;
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


std::vector<std::string> UART_decoder::track_names() {
    return std::vector<std::string>{"TX", "RX"};
}

std::string UART_decoder::pin_roles(uint8_t pin) {
    if (pin == TX_) 
        return "UART_TX";
    
    if (pin == RX_)
        return "UART_RX";
    return "";
}