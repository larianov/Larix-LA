#include "toml_config.hpp"
#include "toml_parser.hpp"
#include <iostream>
int main(int argc, char **argv) {
    if (argc < 2) {
        std::cout << "Wrong Conf\n";
        return 1;
    }
    std::string str = argv[1];
    user_config conf;
    parse_config(str, conf);
}