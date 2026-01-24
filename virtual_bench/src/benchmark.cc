#include <benchmark/benchmark.h>

#include <cstdint>
#include <random>

// Base class with three virtual functions
class Base {
 public:
  virtual ~Base() = default;
  virtual int64_t f1(int64_t x) const = 0;
  virtual int64_t f2(int64_t x) const = 0;
  virtual int64_t f3(int64_t x) const = 0;
};

// First derived level
class Derived1 : public Base {
 public:
  int64_t f1(int64_t x) const override { return x * 2 + 1; }
  int64_t f2(int64_t x) const override { return x * 3 + 2; }
  int64_t f3(int64_t x) const override { return x * 4 + 3; }
};

// Second derived level
class Derived2 : public Derived1 {
 public:
  int64_t f1(int64_t x) const override { return x * 5 + 4; }
  int64_t f2(int64_t x) const override { return x * 6 + 5; }
  int64_t f3(int64_t x) const override { return x * 7 + 6; }
};

// Third derived level
class Derived3 : public Derived2 {
 public:
  int64_t f1(int64_t x) const override { return x * 8 + 7; }
  int64_t f2(int64_t x) const override { return x * 9 + 8; }
  int64_t f3(int64_t x) const override { return x * 10 + 9; }
};

// Non‑virtual equivalent with three inline functions
class NonVirtualThree {
 public:
  int64_t f1(int64_t x) const { return x * 2 + 1; }
  int64_t f2(int64_t x) const { return x * 3 + 2; }
  int64_t f3(int64_t x) const { return x * 4 + 3; }
};

// Global volatile sink to prevent optimization
volatile int64_t global_sink = 0;

// ----------------------------------------------------------------------------
// Benchmark: single virtual function call through Base pointer (f1)
// ----------------------------------------------------------------------------
static void BM_VirtualSingle(benchmark::State& state) {
  Derived3 obj;
  const Base* ptr = &obj;
  int64_t sum = 0;
  for (auto _ : state) {
    benchmark::DoNotOptimize(sum += ptr->f1(state.iterations()));
  }
  global_sink = sum;
}
BENCHMARK(BM_VirtualSingle);

// ----------------------------------------------------------------------------
// Benchmark: single non‑virtual call (inline)
// ----------------------------------------------------------------------------
static void BM_NonVirtualSingle(benchmark::State& state) {
  NonVirtualThree obj;
  int64_t sum = 0;
  for (auto _ : state) {
    benchmark::DoNotOptimize(sum += obj.f1(state.iterations()));
  }
  global_sink = sum;
}
BENCHMARK(BM_NonVirtualSingle);

// ----------------------------------------------------------------------------
// Benchmark: three virtual function calls sequentially
// ----------------------------------------------------------------------------
static void BM_VirtualThree(benchmark::State& state) {
  Derived3 obj;
  const Base* ptr = &obj;
  int64_t sum = 0;
  for (auto _ : state) {
    benchmark::DoNotOptimize(sum += ptr->f1(state.iterations()));
    benchmark::DoNotOptimize(sum += ptr->f2(state.iterations()));
    benchmark::DoNotOptimize(sum += ptr->f3(state.iterations()));
  }
  global_sink = sum;
}
BENCHMARK(BM_VirtualThree);


// ----------------------------------------------------------------------------
// Benchmark: three non‑virtual calls (inline)
// ----------------------------------------------------------------------------
static void BM_NonVirtualThree(benchmark::State& state) {
  NonVirtualThree obj;
  int64_t sum = 0;
  for (auto _ : state) {
    benchmark::DoNotOptimize(sum += obj.f1(state.iterations()));
    benchmark::DoNotOptimize(sum += obj.f2(state.iterations()));
    benchmark::DoNotOptimize(sum += obj.f3(state.iterations()));
  }
  global_sink = sum;
}
BENCHMARK(BM_NonVirtualThree);

// ----------------------------------------------------------------------------
// Benchmark: polymorphic virtual calls across three derived types
// ----------------------------------------------------------------------------
static void BM_VirtualPolymorphic(benchmark::State& state) {
  Derived1 d1;
  Derived2 d2;
  Derived3 d3;
  const Base* ptrs[] = {&d1, &d2, &d3};
  const size_t n = sizeof(ptrs) / sizeof(ptrs[0]);
  int64_t sum = 0;
  for (auto _ : state) {
    size_t idx = state.iterations() % n;
    benchmark::DoNotOptimize(sum += ptrs[idx]->f1(state.iterations()));
  }
  global_sink = sum;
}
BENCHMARK(BM_VirtualPolymorphic);

// ----------------------------------------------------------------------------
// Benchmark: virtual calls through three‑level inheritance chain
// (call f1 on Derived3 through Base*, but with intermediate vtables)
// ----------------------------------------------------------------------------
static void BM_VirtualThreeLevel(benchmark::State& state) {
  Derived3 obj;
  const Base* ptr = &obj;
  int64_t sum = 0;
  for (auto _ : state) {
    // This traverses three vtables (Base, Derived1, Derived2) to reach Derived3's f1
    benchmark::DoNotOptimize(sum += ptr->f1(state.iterations()));
  }
  global_sink = sum;
}
BENCHMARK(BM_VirtualThreeLevel);

// ----------------------------------------------------------------------------
// Benchmark: non‑virtual calls in a loop (baseline)
// ----------------------------------------------------------------------------
static void BM_NonVirtualLoop(benchmark::State& state) {
  NonVirtualThree objs[8];
  int64_t sum = 0;
  for (auto _ : state) {
    for (size_t i = 0; i < 8; ++i) {
      benchmark::DoNotOptimize(sum += objs[i].f1(i));
    }
  }
  global_sink = sum;
}
BENCHMARK(BM_NonVirtualLoop);

// ----------------------------------------------------------------------------
// Benchmark: virtual calls in a loop with mixed types (harder to devirtualize)
// ----------------------------------------------------------------------------
static void BM_VirtualLoop(benchmark::State& state) {
  Derived1 d1;
  Derived2 d2;
  Derived3 d3;
  const Base* array[] = {&d1, &d2, &d3, &d1, &d2, &d3, &d1, &d2};
  const size_t len = sizeof(array) / sizeof(array[0]);
  int64_t sum = 0;
  for (auto _ : state) {
    for (size_t i = 0; i < len; ++i) {
      benchmark::DoNotOptimize(sum += array[i]->f1(i));
    }
  }
  global_sink = sum;
}
BENCHMARK(BM_VirtualLoop);

BENCHMARK_MAIN();