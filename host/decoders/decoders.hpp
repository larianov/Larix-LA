#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct Event {
    uint8_t num;
    std::string txt;
};

struct Decoder {
    virtual void update(uint8_t byte, std::vector<Event> &ev) = 0;
    virtual std::vector<std::string> track_names() = 0;
    virtual std::string pin_roles(uint8_t pin) = 0;
    virtual ~Decoder() = default;
};

