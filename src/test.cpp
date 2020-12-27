#include "bandit.h"

#include <algorithm>
#include <numeric>
#include <random>
#include <vector>

#include <benchmark/benchmark.h>

template <typename T> struct StdSort {
  using value_type = T;
  void operator()(std::vector<T> &x) { std::sort(std::begin(x), std::end(x)); }
};

template <typename T> struct StdStableSort {
  using value_type = T;
  void operator()(std::vector<T> &x) {
    std::stable_sort(std::begin(x), std::end(x));
  }
};

template <typename Sort> struct CheckedSort : public Sort {
  void operator()(std::vector<typename Sort::value_type> &x) {
    if (!std::is_sorted(std::begin(x), std::end(x))) {
      Sort::operator()(x);
    }
  }
};

template <typename T> struct AdaptiveSort {
  static auto MakeFunc() {
    return Dispatch<std::vector<T> &>(std::make_tuple(
        [](auto &x) { std::sort(std::begin(x), std::end(x)); },
        [](auto &x) { std::stable_sort(std::begin(x), std::end(x)); },
        CheckedSort<StdSort<T>>{}));
  }

  decltype(MakeFunc()) func = MakeFunc();
  void operator()(std::vector<T> &x) { return func(x); }
};

// alternative implementation of AdaptiveSort
template <typename T> struct AnotherAdaptiveSort {
  TimingSelector<3> selector{};

  void operator()(std::vector<T> &x) {
    switch (auto timer = selector.GetOptionTimed()) {
    case 0:
      std::sort(std::begin(x), std::end(x));
      break;
    case 1:
      std::stable_sort(std::begin(x), std::end(x));
      break;
    case 2:
      if (!std::is_sorted(std::begin(x), std::end(x))) {
        std::sort(std::begin(x), std::end(x));
      }
      break;
    } // switch
  }
};

template <typename Sort> void BM_Sorted(benchmark::State &state, Sort sort) {
  for (auto _ : state) {
    int n = state.range(0);
    std::vector<int> v(n);
    std::iota(std::begin(v), std::end(v), 0);

    sort(v);

    benchmark::DoNotOptimize(v.data());
    benchmark::ClobberMemory();
  }
}

template <typename Sort> void BM_Reversed(benchmark::State &state, Sort sort) {
  for (auto _ : state) {
    int n = state.range(0);
    std::vector<int> v(n);
    std::iota(std::rbegin(v), std::rend(v), 0);

    sort(v);

    benchmark::DoNotOptimize(v.data());
    benchmark::ClobberMemory();
  }
}

template <typename Sort>
void BM_AlmostSorted(benchmark::State &state, Sort sort) {
  for (auto _ : state) {
    int n = state.range(0);
    std::vector<int> v(n);
    std::iota(std::begin(v), std::end(v), 0);
    std::swap(v[n - 1], v[n - 2]);

    sort(v);

    benchmark::DoNotOptimize(v.data());
    benchmark::ClobberMemory();
  }
}

static void CustomArguments(benchmark::internal::Benchmark *b) {
  b->Arg(8)->Arg(16)->Arg(64)->Arg(1024)->Arg(1 << 15);
}

#define SORT_BENCHMARK(BM_fn, sort)                                            \
  BENCHMARK_CAPTURE(BM_fn, sort, (sort))->Apply(CustomArguments)

#define SORT_BENCHMARKS(BM_fn)                                                 \
  SORT_BENCHMARK(BM_fn, StdSort<int>{});                                       \
  SORT_BENCHMARK(BM_fn, StdStableSort<int>{});                                 \
  SORT_BENCHMARK(BM_fn, CheckedSort<StdSort<int>>{});                          \
  SORT_BENCHMARK(BM_fn, AnotherAdaptiveSort<int>{});                           \
  SORT_BENCHMARK(BM_fn, AdaptiveSort<int>{})

SORT_BENCHMARKS(BM_Sorted);
SORT_BENCHMARKS(BM_Reversed);
SORT_BENCHMARKS(BM_AlmostSorted);

BENCHMARK_MAIN();
