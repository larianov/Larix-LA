add_executable(logic_analyzer_host main.cpp)
target_link_libraries(logic_analyzer_host PRIVATE
    fstparser
    decoders
    project_warnings
)
