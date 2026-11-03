#include "decoders.hpp"
#include <cstdint>
enum class uart_states : uint8_t{
    IDLE,
    POT_START,
    READ_BYTE,
    PARITY,
    STOP
};

class handling_uart_member {
  private:
    uint64_t bond_clocking{};
    uint32_t byte_placeholder{};
    uint8_t byte_size_;
    uint8_t parity_; // 0 for none, 1 for odd, 2 for even
    uint8_t amm_stop_bits_;
    uint64_t baudrate_;
    uint8_t ammount_of_bits{};
    uint64_t clk_out{};
    uart_states state{uart_states::IDLE};
    bool sync{false};
    uint8_t last_bit{};
    uint8_t good_frames{};
    bool output_enabled{};
    
  public:
    handling_uart_member(uint64_t bondclocking, uint8_t byte_size, uint8_t parity, uint8_t amm_stop_bits);
    std::string update(uint8_t bit_now);
};


class UART_decoder : public Decoder {
  private:
    uint8_t TX_;
    handling_uart_member tx_;
    uint8_t RX_;
    handling_uart_member rx_;
  public:
    UART_decoder(uint8_t TX, uint8_t RX, uint8_t byte_size, uint8_t parity, uint8_t amm_stop_bits, uint64_t baudrate, uint64_t freq);
    void update(uint8_t byte, std::vector<Event> &ev) override;
    std::vector<std::string> track_names() override;
    std::string pin_roles(uint8_t pin) override;
};