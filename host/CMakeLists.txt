cmake_minimum_required(VERSION 3.24)
project(logic_analyzer_host C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE -Wall -Wextra -Wshadow -Wpedantic)

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
target_compile_definitions(fstapi PRIVATE FST_CONFIG_INCLUDE="fstapi.h")
target_link_libraries(fstapi PUBLIC ZLIB::ZLIB)

add_subdirectory(decoders)
add_subdirectory(fstparser)
add_subdirectory(src)
