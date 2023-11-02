#include <benchmark/benchmark.h>
#include <stdio.h>
#include <array>
#include <iostream>
#include <format>
#include <print>

void test_printf_string(benchmark::State& state) {
    for(auto _: state) {
        printf("hello world\n");
    }
}

void test_cout_string(benchmark::State& state) {
    std::cout.sync_with_stdio(false);
    for(auto _: state) {
        std::cout<<"hello world\n";
    }
}

void test_print_string(benchmark::State& state) {
    std::cout.sync_with_stdio(false);
    for(auto _: state) {
        std::println("hello world");
    }
}

void test_printf_int(benchmark::State& state) {
    int i = 0;
    for(auto _: state) {
        printf("%d\n", ++i);
    }
}

void test_cout_int(benchmark::State& state) {
    std::cout.sync_with_stdio(false);
    int i = 0;
    for(auto _: state) {
        std::cout<<++i<<"\n";
    }
}

void test_print_int(benchmark::State& state) {
    int i = 0;
    for(auto _: state) {
        std::println("{}", ++i);
    }
}

BENCHMARK(test_printf_string);
BENCHMARK(test_cout_string);
BENCHMARK(test_print_string);
BENCHMARK(test_printf_int);
BENCHMARK(test_cout_int);
BENCHMARK(test_print_int);

BENCHMARK_MAIN();