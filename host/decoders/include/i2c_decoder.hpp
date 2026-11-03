#pragma once
#include "decoders.hpp"

enum class i2c_states : uint8_t {
    START,
    READ_BYTE,
    CHECK_ACK,
};

class I2C_decoder : public Decoder {
  private:
    uint8_t SDA_;
    uint8_t SCL_;
    uint8_t byte_placeholder{};
    i2c_states state{i2c_states::START};
    uint8_t ammount_of_bytes{};
    uint8_t last_state_of_sda{1};
    uint8_t last_state_of_scl{1};
    bool address_defined{};

  public:
    I2C_decoder(uint8_t SDA, uint8_t SCL);
    void update(uint8_t byte, std::vector<Event> &ev) override;
    std::vector<std::string> track_names() override;
    std::string pin_roles(uint8_t pin) override;
};

