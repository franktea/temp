cmake_minimum_required(VERSION 3.27)
project(cout_benchmark)
enable_testing()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

include(FetchContent)

FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG main)

FetchContent_Declare(googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG main) # need main for benchmark::benchmark

FetchContent_Declare(fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG master)      

FetchContent_MakeAvailable(
        googletest
        googlebenchmark
        fmt)

add_executable(cout_benchmark cout_vs_printf.cpp)
target_link_libraries(cout_benchmark PRIVATE benchmark::benchmark fmt::fmt)