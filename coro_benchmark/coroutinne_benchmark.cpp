#include <benchmark/benchmark.h>
#include <iostream>
#include <string>
#include <assert.h>

#include "fcontext.hpp"

int value1 = 0;

void stackful( transfer_t t) {
    transfer_t tt {t.fctx, t.data};
    while(1) {
        benchmark::DoNotOptimize(++value1);
        tt = jump_fcontext( tt.fctx, tt.data);
    }
}

void BM_JumpFcontext(benchmark::State& state) {
    char stack[4096];
    fcontext_t ctx = make_fcontext(&stack[4095], 4096, stackful);

    transfer_t tt {ctx, nullptr};
    for(auto _ : state) {
        tt = jump_fcontext(tt.fctx, tt.data);
    }
}

BENCHMARK(BM_JumpFcontext)->Unit(benchmark::TimeUnit::kNanosecond);

#include "generator.h"

generator<int> cpp20() {
    while(1) {
        benchmark::DoNotOptimize(++value1);
        co_yield value1;
    }
}

void BM_CPP20Coroutie(benchmark::State& state) {
    auto g = cpp20();
    auto it = g.begin();

    for(auto _ : state) {
        benchmark::DoNotOptimize(++it);
    }
}

BENCHMARK(BM_CPP20Coroutie)->Unit(benchmark::TimeUnit::kNanosecond);

// cmake的时候注意加 -DCMAKE_BUILD_TYPE=Release

BENCHMARK_MAIN();
