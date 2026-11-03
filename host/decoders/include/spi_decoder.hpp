#include "decoders.hpp"
#include <cstdint>



enum class spi_states : uint8_t {
    IDLE,
    READ,
};


class handling_spi_member{
private:
    uint32_t byte{};
    uint8_t ammount_of_bits{};
    uint8_t byte_size_;
    bool msb_on_;
public:
    handling_spi_member(uint8_t byte_size, uint8_t msb_on_);
    std::string update(uint8_t bit_now);
};

class SPI_decoder : public Decoder {
  private:
    uint8_t cs_;
    uint8_t MOSI_;
    handling_spi_member mosi_;
    uint8_t MISO_;
    handling_spi_member miso_;
    uint8_t last_clock{};
    uint8_t last_cs{1};
    std::pair<std::string, std::string> last_to_ret;
    uint8_t SCK_;
    uint8_t spi_mode_;
  public:
    SPI_decoder(uint8_t cs, uint8_t MOSI, uint8_t MISO, uint8_t SCK, uint8_t byte_size, uint8_t spi_mode, bool msb_on);
    void update(uint8_t byte, std::vector<Event> &ev) override;
    std::vector<std::string> track_names() override;
    std::string pin_roles(uint8_t pin) override;
};
