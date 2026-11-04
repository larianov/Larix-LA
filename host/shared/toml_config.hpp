#pragma once
#include "decoders.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

uint32_t inline constexpr MAX_M10K_SAMPLES = 650'000;
uint32_t inline constexpr MAX_DDR3_SAMPLES = 524'288'000;
uint8_t inline constexpr CHANNEL_RANGE = 35;
uint32_t inline constexpr MAX_FREQ = 1'000'000'000;
uint32_t inline const MAX_STREAM_FREQ = 30'000'000;

enum transport{
    serial,
    tcp
};

struct device{
    transport tr_;
    std::string port_;
    uint32_t baudrate_;
};

enum mode{
    m10k,
    ddr3,
    stream
};

struct capture{
    mode mode_t;
    uint64_t sampling_freq_;
    uint32_t samples_;
    uint8_t starting_channel;
};

enum trig_type{
    edge,
    command
};

enum edge_type{
    rising,
    falling
};

struct trigger{
    trig_type type_t;
    edge_type edge_t;
    uint8_t channel;
};


struct user_config{
    device device_t;
    capture capture_t;
    trigger trig_t;
    std::string file_output;
    std::vector<std::unique_ptr<Decoder>> decoders_;
};