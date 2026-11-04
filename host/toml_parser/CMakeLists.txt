add_library(toml_parser STATIC toml_parser.cpp)
target_include_directories(toml_parser PUBLIC . ${PROJECT_SOURCE_DIR}/shared)
target_link_libraries(toml_parser
    PUBLIC  decoders tomlplusplus::tomlplusplus  
    PRIVATE project_warnings
)
