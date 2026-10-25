cmake_minimum_required(VERSION 3.16)
project(EPP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(epp src/main.cpp)

enable_testing()
add_executable(epp_tests tests/test_compiler.cpp)
target_compile_definitions(epp_tests PRIVATE EPP_NO_MAIN)
add_test(NAME epp_tests COMMAND epp_tests)
