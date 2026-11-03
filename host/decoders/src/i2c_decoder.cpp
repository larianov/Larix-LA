#include "i2c_decoder.hpp"
#include <format>
#include <string>
#include <utility>
#include <vector>
I2C_decoder::I2C_decoder(uint8_t SDA, uint8_t SCL) : SDA_(SDA), SCL_(SCL){};

void I2C_decoder::update(uint8_t byte, std::vector<Event> &ev) {
    auto status_sda = (byte >> SDA_) & 1;
    auto status_scl = (byte >> SCL_) & 1;
    std::string to_ret;
    auto parallel_testinng = [&]() -> bool {
        if (status_scl == 1 && last_state_of_scl == 1) {
            if (last_state_of_sda == 0 && status_sda == 1) {
                to_ret = "STOP";
                address_defined = false;
                state = i2c_states::START;
                return true;
            } else if (last_state_of_sda == 1 && status_sda == 0) {
                state = i2c_states::READ_BYTE;
                address_defined = false;
                byte_placeholder = 0;
                ammount_of_bytes = 0;
                to_ret = "REPEATED_START";
                return true;
            }
        }
        return false;
    };
    switch (state) {
    case i2c_states::START:
        if (status_scl == 1 && status_sda == 0 && last_state_of_sda == 1) {
            state = i2c_states::READ_BYTE;
            ammount_of_bytes = 0;
            byte_placeholder = 0;
            to_ret = "START";
        }
        break;
    case i2c_states::READ_BYTE:
        if (parallel_testinng())
            break;
        if (last_state_of_scl == 0 && status_scl == 1) {
            byte_placeholder = (byte_placeholder << 1) | status_sda;
            ammount_of_bytes++;
        }
        if (ammount_of_bytes == 8) {
            if (address_defined) {
                to_ret = {std::format("DATA: {:02X}", byte_placeholder, byte_placeholder)};
            } else {
                to_ret = {
                    std::format("ADDR: {:#04X}[{}]", byte_placeholder >> 1, (byte_placeholder & 1) ? "READ" : "WRITE")};
                address_defined = true;
            }

            state = i2c_states::CHECK_ACK;
        }
        break;
    case i2c_states::CHECK_ACK:
        if (parallel_testinng())
            break;
        if (last_state_of_scl == 0 && status_scl == 1) {
            state = i2c_states::READ_BYTE;
            ammount_of_bytes = 0;
            byte_placeholder = 0;
            if (status_sda == 0) {
                to_ret = "ACK";
            } else {
                to_ret = "NACK";
            }
        }
    }
    last_state_of_scl = status_scl;
    last_state_of_sda = status_sda;
    if (!to_ret.empty()) {
        ev.emplace_back(0, std::move(to_ret));
    }
}

std::vector<std::string> I2C_decoder::track_names() {
    return std::vector<std::string>{"I2C"};
}

std::string I2C_decoder::pin_roles(uint8_t pin) {
    if (pin == SDA_) return "I2C_SDA";
    if (pin == SCL_) return "I2C_SCL";
    return "";
}