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






