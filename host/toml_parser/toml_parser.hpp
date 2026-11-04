#pragma once
#include <optional>
#include "toml_config.hpp"
#include <string>
#include <toml++/impl/node.hpp>
#include <toml++/impl/parser.inl>
#include <toml++/toml.hpp>
#include <vector>
#include <utility>


toml::optional<std::string> parse_config(const std::string& path, user_config &conf);

template<typename T>
std::optional<std::string> error_handler(const std::string &field, const std::string &value, const std::vector<T> &expected_values, const toml::table &tbl, T& out,  bool range = false){
    auto noda = tbl[field][value].node();
    if(!noda){
        return "Not found value: " + value + " in field: " + field;
    }
    T temp = noda->value<T>();
    if (temp && expected_values.empty()) {
        ;
    }
    else if (!temp || (range && (temp < expected_values[0] || temp > expected_values[1])) || std::find(expected_values.begin(), expected_values.end(), temp) == expected_values.end()) {
        return "Error appeared on line : " + std::to_string(noda->source().begin.line) + "\n";
    }
    out = temp;
    return std::nullopt;
}