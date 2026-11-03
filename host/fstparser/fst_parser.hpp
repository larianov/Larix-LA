#pragma once
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>
#include "decoders.hpp"
#include <memory>
#include "fstapi.h"

enum class c_decoders : uint8_t {
    ERROR,
    I2C,
    SPI,
    UART
};




void fst_parse(std::span<const uint8_t> to_parse, const uint64_t freq, const std::string& name_of_file, std::vector<std::unique_ptr<Decoder>> &decoders, std::vector<std::string> &namings, std::vector<uint8_t>&chans);




