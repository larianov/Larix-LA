#pragma once
#include <optional>
#include "toml_config.hpp"
#include <string>
#include <toml++/impl/node.hpp>
#include <toml++/impl/parser.inl>
#include <toml++/toml.hpp>
#include <vector>
#include <algorithm>


toml::optional<std::string> parse_config(const std::string& path, user_config &conf);

template<typename T>
std::optional<std::string> error_handler(const std::string &field, const std::string &value, const std::vector<T> &expected_values, const toml::table &tbl, T& out,  bool range = false){
    auto noda = field == "decoder" ? tbl[value].node() : tbl[field][value].node();
    if(!noda){
        return "Not found key: " + value + " from field: " + field;
    }
    std::optional<T> temp = noda->value<T>();
    if (temp && expected_values.empty()) {
        ;
    }
    else if (!temp || (range && (*temp < expected_values[0] || *temp > expected_values[1])) || (!range && std::find(expected_values.begin(), expected_values.end(), *temp) == expected_values.end())) {
        std::string to_ret{"Error appeared on line : " + std::to_string(noda->source().begin.line) + "\nExpected field: " + field + "\nWith key: " + value};
        if (range) {
            if constexpr (std::is_arithmetic_v<T>) {
                to_ret += "\nWith range: " + std::to_string(expected_values[0])
                         + " - " + std::to_string(expected_values[1]) + "\n";
            }
        }
        else{
            to_ret += "\nAccepted values: ";
            bool first {};
            for (auto &v : expected_values) {
                if (!first) {
                    to_ret += v ;
                    first = true;
                }
                else {
                    to_ret += ", " + v;
                }
            }
        }     
        return to_ret;
    }
    out = *temp;
    return std::nullopt;
}