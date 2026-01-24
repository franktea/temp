cmake_minimum_required(VERSION 3.14)
project(VirtualFunctionBenchmark CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF)

# Enable profiling with perf if available
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -g")

# Google Benchmark via FetchContent
include(FetchContent)
FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.8.3
)
FetchContent_MakeAvailable(benchmark)

# Add benchmark executable
add_executable(virtual_benchmark src/benchmark.cc)
target_link_libraries(virtual_benchmark benchmark::benchmark benchmark::benchmark_main)

# Install clang-format check target
find_program(CLANG_FORMAT_EXE NAMES clang-format)
if(CLANG_FORMAT_EXE)
  add_custom_target(
    format-check
    COMMAND ${CLANG_FORMAT_EXE} --style=Google --dry-run --Werror src/benchmark.cc
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Checking Google C++ style compliance"
  )
endif()

# Perf target for easy profiling
add_custom_target(
  perf-record
  COMMAND perf record -g ./virtual_benchmark --benchmark_min_time=0.5
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  DEPENDS virtual_benchmark
  COMMENT "Running perf record on benchmark"
)

add_custom_target(
  perf-report
  COMMAND perf report -g graph --no-children
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  DEPENDS perf-record
  COMMENT "Generating perf report"
)

add_custom_target(
  flamegraph
  COMMAND perf script | stackcollapse-perf.pl | flamegraph.pl > flamegraph.svg
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  DEPENDS perf-record
  COMMENT "Generating flamegraph (requires stackcollapse-perf.pl and flamegraph.pl)"
)