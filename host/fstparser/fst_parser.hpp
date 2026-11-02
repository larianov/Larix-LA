#pragma once
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>
#include "fstapi.h"

enum class c_decoders : uint8_t {
    ERROR,
    I2C,
    SPI,
    UART
};


struct logic_an_input {
    uint32_t hz{};      
    uint8_t channel{};  
    uint8_t amm{};     
};

struct t_decoders {
    c_decoders code;
    uint8_t first_ch;
    uint8_t second_ch;
    std::optional<uint8_t> third_ch;
    std::optional<uint8_t> fourth_ch;
    std::optional<uint32_t> baudrate{115200};
};


void fst_parse(std::span<const uint8_t> to_parse, const logic_an_input &config, const std::string &name_of_file,
               const std::vector<t_decoders> &ver);

enum class i2c_states : uint8_t {
    START,
    READ_BYTE,
    CHECK_ACK,
};

class handling_i2c {
  private:
    uint8_t bit_placeholder{};
    i2c_states state{i2c_states::START};
    uint8_t ammount_of_bytes{};
    static constexpr std::string start{"START"};
    static constexpr std::string stop{"STOP"};
    static constexpr std::string re_start{"REP_START"};
    static constexpr std::string ack{"ACK"};
    static constexpr std::string nack{"NACK"};
    uint8_t last_state_of_sda{1};
    uint8_t last_state_of_scl{1};
    bool address_defined{};
    uint8_t sda_idx;
    uint8_t scl_idx;
    bool first_iteration{true};
  public:
    handling_i2c(uint8_t sda_ix, uint8_t scl_ix);
    std::string update(std::vector<std::pair<uint8_t, fstHandle>> &arr);
};


enum class uart_states : uint8_t{
    IDLE,
    POT_START,
    READ_BYTE,
    STOP
};

class handling_uart_member {
  private:
    uint64_t bond_clocking{};
    uint8_t bit_placeholder{};
    uint8_t ammount_of_bits{};
    uint64_t clk_out{};
    uart_states state{uart_states::IDLE};
    bool sync{false};
    uint8_t last_bit{};
    uint8_t good_frames{};
    bool output_enabled{};
  public:
    explicit handling_uart_member(uint64_t bondclocking);
    std::string update(uint8_t bit_now);
};

class handling_uart {
  private:
    handling_uart_member tx_;
    uint8_t tx_idx;
    handling_uart_member rx_;
    uint8_t rx_idx;
  public:
    handling_uart(uint8_t rx, uint8_t tx, uint32_t baudrate, uint32_t freq);
    std::pair<std::string, std::string> update(std::vector<std::pair<uint8_t, fstHandle>> &arr);
};


enum class spi_states: uint8_t{
    IDLE,
    READ,
};

//mosi, miso
class handling_spi_member{
private:
    uint8_t byte{};
    uint8_t ammount_of_bits;
public:
    handling_spi_member() = default;
    std::string update(uint8_t bit_now);
};

class handling_spi{
private:
    uint8_t mosi_idx;
    handling_spi_member mosi_;
    uint8_t miso_idx;
    handling_spi_member miso_;
    uint8_t last_clock{};
    uint8_t clk_idx;
    uint8_t last_cs{1};
    uint8_t cs_idx;
public:
    handling_spi(uint8_t mosi, uint8_t miso, uint8_t clk, uint8_t cs);
    std::pair<std::string, std::string> update(std::vector<std::pair<uint8_t, fstHandle>> &arr);
};
