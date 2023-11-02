cmake_minimum_required(VERSION 3.27)

project(cout_benchmark)

enable_testing()

include(FetchContent)

## Project-wide setup
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

# Externally provided libraries
FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG main)

FetchContent_Declare(googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG main) # need main for benchmark::benchmark

FetchContent_MakeAvailable(
        googletest
        googlebenchmark)

add_executable(cout_benchmark cout_vs_printf.cpp)
target_link_libraries(cout_benchmark benchmark::benchmark)