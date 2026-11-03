add_library(fstparser STATIC fst_parser.cpp)
target_include_directories(fstparser PUBLIC .)
target_link_libraries(fstparser
    PUBLIC  fstapi          # fst_parser.hpp includes fstapi.h -> must be PUBLIC
    PRIVATE decoders project_warnings
)
