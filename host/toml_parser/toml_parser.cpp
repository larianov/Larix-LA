#include "toml_parser.hpp"
#include "toml_config.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <optional>
#include <string>
#include <format>
#include <toml++/impl/table.hpp>
#include <toml++/toml.hpp>


toml::optional<std::string> parse_config(const std::string& path, user_config &conf){
    toml::table tbl = toml::parse_file(path);
    conf = user_config{};
    std::string temp;
    auto to_ret = error_handler("device", "transport", {"serial", "tcp"}, tbl, temp);
    if (to_ret) {
        return *to_ret;
    }
    
    if (temp == "serial"){
        conf.device_t.tr_ = serial;
        to_ret = error_handler("device", "port", {}, tbl, conf.device_t.port_);
        if (to_ret) return *to_ret;
        to_ret = error_handler("device", "baud", {0, 20'000'000}, tbl, conf.device_t.baudrate_, true);
        if (to_ret) return *to_ret;
    }
    else {
        conf.device_t.tr_ = tcp;
    }

    to_ret = error_handler("capture", "mode", {"m10k", "ddr3", "stream"}, tbl, temp);
    if(to_ret) return *to_ret;
    if (temp == "stream") {
        conf.capture_t.mode_t = stream;
        to_ret = error_handler("capture", "rate_hz", {0, MAX_STREAM_FREQ+1}, tbl, conf.capture_t.sampling_freq_);
        if(to_ret) return *to_ret;  
    }
    else {
        if (temp == "m10k"){
            conf.capture_t.mode_t = m10k;
            to_ret = error_handler("capture", "samples", {0, MAX_M10K_SAMPLES+1}, tbl, conf.capture_t.samples_);
            if(to_ret) return *to_ret;
        }
        else{
            conf.capture_t.mode_t = ddr3;
            to_ret = error_handler("capture", "samples", {0, MAX_DDR3_SAMPLES+1}, tbl, conf.capture_t.samples_);
            if(to_ret) return *to_ret;
        }
        to_ret = error_handler("capture", "rate_hz", {0, MAX_FREQ+1}, tbl, conf.capture_t.sampling_freq_);
        if(to_ret) return *to_ret;
    }
    to_ret = error_handler("capture", "starting_channel", {0, CHANNEL_RANGE-7}, tbl, conf.capture_t.starting_channel);
    if(to_ret) return *to_ret;

    to_ret = error_handler("trigger", "type", {"edge", "command"}, tbl, temp);
    if(to_ret) return *to_ret;
    if (temp == "command") {
        conf.trig_t.type_t = command;
    }
    else {
        conf.trig_t.type_t = edge;
        to_ret = error_handler("trigger", "channel", {conf.capture_t.starting_channel, conf.capture_t.starting_channel + 8}, tbl, conf.trig_t.channel);
        if(to_ret) return *to_ret;
    }
    
}