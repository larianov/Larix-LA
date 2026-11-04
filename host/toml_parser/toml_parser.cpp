#include "toml_parser.hpp"
#include "decoders.hpp"
#include "i2c_decoder.hpp"
#include "toml_config.hpp"
#include <cstddef>
#include <cstdint>
#include <fcntl.h>
#include <memory>
#include <optional>
#include <string>
#include <toml++/impl/table.hpp>
#include <toml++/toml.hpp>
#include <vector>


toml::optional<std::string> parse_config(const std::string& path, user_config &conf, std::vector<std::string> &namings){
    toml::table tbl;
    try{
        tbl = toml::parse_file(path);
    }
    catch(...){
        return "Toml file parsing failed";
    }
    
    conf = user_config{};
    namings = {};
    std::string temp;
    auto to_ret = error_handler("device", "transport", {"serial", "tcp"}, tbl, temp);
    if (to_ret) {
        return *to_ret;
    }
    
    if (temp == "serial"){
        conf.device_t.tr_ = serial;
        to_ret = error_handler("device", "port", {}, tbl, conf.device_t.port_);
        if (to_ret) return *to_ret;
        to_ret = error_handler("device", "baud", {1, 20'000'000}, tbl, conf.device_t.baudrate_, true);
        if (to_ret) return *to_ret;
    }
    else {
        conf.device_t.tr_ = tcp;
    }

    to_ret = error_handler("capture", "mode", {"m10k", "ddr3", "stream"}, tbl, temp);
    if(to_ret) return *to_ret;
    if (temp == "stream") {
        conf.capture_t.mode_t = stream;
        to_ret = error_handler("capture", "rate_hz", {1, MAX_STREAM_FREQ}, tbl, conf.capture_t.sampling_freq_, true);
        if(to_ret) return *to_ret;  
    }
    else {
        if (temp == "m10k"){
            conf.capture_t.mode_t = m10k;
            to_ret = error_handler("capture", "samples", {1, MAX_M10K_SAMPLES+1}, tbl, conf.capture_t.samples_, true);
            if(to_ret) return *to_ret;
        }
        else{
            conf.capture_t.mode_t = ddr3;
            to_ret = error_handler("capture", "samples", {1, MAX_DDR3_SAMPLES}, tbl, conf.capture_t.samples_, true);
            if(to_ret) return *to_ret;
        }
        to_ret = error_handler("capture", "rate_hz", {1, MAX_FREQ}, tbl, conf.capture_t.sampling_freq_, true);
        if(to_ret) return *to_ret;
    }
    to_ret = error_handler("capture", "starting_channel", {0, CHANNEL_RANGE-7}, tbl, conf.capture_t.starting_channel, true);
    if(to_ret) return *to_ret;

    to_ret = error_handler("trigger", "type", {"edge", "command"}, tbl, temp);
    if(to_ret) return *to_ret;
    if (temp == "command") {
        conf.trig_t.type_t = command;
    }
    else {
        conf.trig_t.type_t = edge;
        to_ret = error_handler("trigger", "channel", {conf.capture_t.starting_channel, static_cast<uint8_t>(conf.capture_t.starting_channel + 7)}, tbl, conf.trig_t.channel, true);
        if(to_ret) return *to_ret;
        to_ret = error_handler("trigger", "edge", {"rising", "falling"}, tbl, temp);
        if(to_ret) return *to_ret;
        if (temp == "rising") 
            conf.trig_t.edge_t = rising;
        else
            conf.trig_t.edge_t = falling;
    }
    to_ret = error_handler("output", "file", {}, tbl, conf.file_output);
    if(to_ret) return *to_ret;


    const toml::array* arr = tbl["decoders"].as_array();
    if (!arr) {
        return std::nullopt;
    }
    for (std::size_t i{}; i < arr->size(); i++) {
        to_ret = error_handler("decoders", "name", {}, *(*arr)[i].as_table(), temp);
        if(to_ret) return *to_ret;
        namings.push_back(temp);
        to_ret = error_handler("decoders", "type", {"i2c", "uart", "spi"}, *(*arr)[i].as_table(), temp);
        if(to_ret) return *to_ret;
        if (temp == "i2c") {
        
        }
        else if(temp == "spi") {
        
        }
        else if (temp == "uart") {
        
        }
    }
}