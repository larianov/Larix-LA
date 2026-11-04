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
    auto t = parse_config(str, conf);
    if (t) {
        std::cout << *t << std::endl;
        return 1;
    }
    
}