cmake_minimum_required(VERSION 3.24)
project(logic_analyzer_host C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(gtkwave
    GIT_REPOSITORY https://github.com/gtkwave/gtkwave
    GIT_TAG v3.3.116
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(gtkwave)
set(FST ${gtkwave_SOURCE_DIR}/gtkwave3-gtk3/src/helpers/fst)

find_package(ZLIB REQUIRED)

add_library(fstapi STATIC ${FST}/fstapi.c ${FST}/fastlz.c ${FST}/lz4.c)
target_include_directories(fstapi PUBLIC ${FST})
target_link_libraries(fstapi PUBLIC ZLIB::ZLIB)

add_executable(logic_analyzer_host src/main.cpp fstparser/fst_parser.cpp)
target_include_directories(logic_analyzer_host PRIVATE fstparser)
target_link_libraries(logic_analyzer_host PRIVATE fstapi)
target_compile_options(logic_analyzer_host PRIVATE -Wall -Wextra -Wshadow -Wpedantic)
