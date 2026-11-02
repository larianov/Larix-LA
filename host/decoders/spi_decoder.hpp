#include "decoders.hpp"



enum class spi_states : uint8_t {
    IDLE,
    READ,
};


class handling_spi_member{
private:
    uint8_t byte{};
    uint8_t ammount_of_bits{};
    uint8_t byte_size_;
    uint8_t spi_mode;
    bool msb_on;
public:
    handling_spi_member() = default;
    std::string update(uint8_t bit_now);
};

class SPI_decoder : Decoder {
  private:
    uint8_t cs_;
    uint8_t MOSI_;
    handling_spi_member mosi_;
    uint8_t MISO_;
    handling_spi_member miso_;
    uint8_t SCK_;
    uint8_t byte_size_;
    uint8_t spi_mode;
    bool msb_on;
  public:
    SPI_decoder(uint8_t cs, uint8_t MOSI, uint8_t MISO, uint8_t SCK, uint8_t byte_size, uint8_t spi_mode, bool msb_on);
    void update(uint8_t byte, std::vector<Event> &ev) override;
    std::vector<std::string> track_names() override;
    std::string pin_roles(uint8_t pin) override;
};
