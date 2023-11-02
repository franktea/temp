#include <benchmark/benchmark.h>
#include <stdio.h>
#include <array>
#include <iostream>

void test_printf(benchmark::State& state) {
    for(auto _: state) {
        printf("hello world\n");
    }
}

void test_cout(benchmark::State& state) {
    std::cout.sync_with_stdio(false);
    for(auto _: state) {
        std::cout<<"hello world\n";
    }
}

BENCHMARK(test_printf);
BENCHMARK(test_cout);
BENCHMARK_MAIN();