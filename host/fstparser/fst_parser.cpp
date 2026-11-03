#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <fcntl.h>
#include "fst_parser.hpp"
#include <cstdint>
#include <format>
#include <memory>
#include <span>
#include <string>
#include "decoders.hpp"
#include "i2c_decoder.hpp"
#include <string_view>
#include <utility>
#include <vector>
extern "C" {
    #include "fstapi.h"
}


void fst_parse(std::span<const uint8_t> to_parse, const uint64_t freq, const std::string& name_of_file, std::vector<std::unique_ptr<Decoder>> &decoders, std::vector<std::string> &namings, std::vector<uint8_t>& chans) {
    uint64_t step_fs = 1'000'000'000'000'000ULL / freq;
    auto *fst_object = fstWriterCreate(name_of_file.c_str(), 1);
    fstWriterSetTimescale(fst_object, -15);
    fstWriterSetScope(fst_object, FST_ST_VCD_MODULE, "WIRES", nullptr);
    std::vector<std::pair<uint8_t, fstHandle>> arr;
    std::vector<std::pair<std::unique_ptr<Decoder>, std::vector<fstHandle>>> var_dec;
    for (uint8_t i{}; i < chans.size(); i++) {
        auto temp = std::string("wire" + std::to_string(chans[i]));
        for (auto &v : decoders) {
            auto s = v->pin_roles(i);
            if (!s.empty()) {
                temp+=" " + s;
                break;
            }
        }
        arr.emplace_back(255, fstWriterCreateVar(fst_object, FST_VT_VCD_WIRE, FST_VD_INPUT, 1, temp.c_str(), 0));
    }
    
    for (std::size_t i{}; i < decoders.size(); i++) {
        if (decoders[i]->track_names().size() > 1) {
            var_dec.emplace_back(std::move(decoders[i]), std::vector<fstHandle>{fstWriterCreateVar(fst_object, FST_VT_GEN_STRING, FST_VD_IMPLICIT, 1, ((namings[i]+ "." + decoders[i]->track_names()[0]).c_str()),0), fstWriterCreateVar(fst_object, FST_VT_GEN_STRING, FST_VD_IMPLICIT, 1, ((namings[i] + "." + decoders[i]->track_names()[1]).c_str()),0)});
        }
        else {
            var_dec.emplace_back(std::move(decoders[i]), std::vector<fstHandle>{fstWriterCreateVar(fst_object, FST_VT_GEN_STRING, FST_VD_IMPLICIT, 1, ((namings[i]+ "." + decoders[i]->track_names()[0]).c_str()),0)});
        }
    }
    
    uint64_t current_time_fs = 0;
    fstWriterSetUpscope(fst_object);
    fstWriterEmitTimeChange(fst_object, 0);
    std::vector<Event> ev;
    for (auto v : to_parse) {
        fstWriterEmitTimeChange(fst_object, current_time_fs);
        for (uint8_t i{}; i < chans.size(); i++) {
            uint8_t bit = (v >> i) & 1;
            if (bit != arr[i].first) {
                if (bit == 0)
                    fstWriterEmitValueChange(fst_object, arr[i].second, "0");
                else
                    fstWriterEmitValueChange(fst_object, arr[i].second, "1");
                arr[i].first = bit;
            }
        }
        for (auto &d : var_dec) {
            d.first->update(v, ev);
            if (ev.empty()) continue;
            for (auto &evs: ev) {
                fstWriterEmitVariableLengthValueChange(fst_object, d.second[evs.num], evs.txt.c_str(), evs.txt.size());
            }
            ev.clear();
        }
        current_time_fs += step_fs;
    }
    fstWriterClose(fst_object);
}


