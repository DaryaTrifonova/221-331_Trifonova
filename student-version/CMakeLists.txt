cmake_minimum_required(VERSION 3.16)

project(MiniDex
    VERSION 1.0
    DESCRIPTION "Deliberately vulnerable educational C++ parser lab"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(minidex_core
    src/Cli.cpp
    src/FileReader.cpp
    src/Parser.cpp
    src/Pipeline.cpp
    src/Reporter.cpp)

target_include_directories(minidex_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(MSVC)
    target_compile_options(minidex_core PRIVATE /W4)
else()
    target_compile_options(minidex_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(minidex src/main.cpp)
target_link_libraries(minidex PRIVATE minidex_core)

enable_testing()

add_executable(minidex_tests tests/test_parser_validation.cpp)
target_link_libraries(minidex_tests PRIVATE minidex_core)
add_test(NAME minidex_tests COMMAND minidex_tests)

add_executable(fuzz_parse_index fuzz/fuzz_parse_index.cpp)
target_link_libraries(fuzz_parse_index PRIVATE minidex_core)

add_executable(fuzz_decode_record fuzz/fuzz_decode_record.cpp)
target_link_libraries(fuzz_decode_record PRIVATE minidex_core)
