add_library(decoders STATIC
    src/i2c_decoder.cpp
    # src/spi_decoder.cpp
    # src/uart_decoder.cpp
)
target_include_directories(decoders PUBLIC include)
target_link_libraries(decoders PRIVATE project_warnings)
