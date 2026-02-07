cmake_minimum_required(VERSION 3.20)

project(coroutine)

set(CMAKE_CXX_STANDARD 20)

enable_language(ASM) # 编译汇编要加这行

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        main
)

FetchContent_Declare(googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG main) # need master for benchmark::benchmark

FetchContent_MakeAvailable(
    googletest
    googlebenchmark)

if (APPLE)
add_executable(coroutine_benchmark 
    coroutinne_benchmark.cpp
    jump_x86_64_sysv_macho_gas.S 
    make_x86_64_sysv_macho_gas.S)
else () # 下面是linux，windows的自己加
add_executable(coroutine_benchmark 
    coroutinne_benchmark.cpp
    jump_x86_64_sysv_elf_gas.S 
    make_x86_64_sysv_elf_gas.S)
endif (APPLE)

target_link_libraries(coroutine_benchmark benchmark::benchmark)