#include "uart_decoder.hpp"
#include <cstdint>
UART_decoder::UART_decoder(uint8_t TX, uint8_t RX, uint8_t byte_size, uint8_t parity, uint8_t amm_stop_bits, uint64_t baudrate, uint64_t freq) : TX_(TX), tx_((freq*256ULL)/baudrate, byte_size, parity, amm_stop_bits), RX_(RX), rx_((freq*256ULL)/baudrate, byte_size, parity, amm_stop_bits){};


void UART_decoder::update(uint8_t byte, std::vector<Event> &ev){
    std::pair<std::string, std::string>to_ret;
    to_ret.first = tx_.update(byte >> );
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
