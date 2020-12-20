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

template <typename Sort, typename T>
void BM_Sorted(benchmark::State &state, T zero, Sort sort) {
  for (auto _ : state) {
    int n = state.range(0);
    std::vector<T> v(n);
    std::iota(std::begin(v), std::end(v), zero);

    sort(v);

    benchmark::DoNotOptimize(v.data());
    benchmark::ClobberMemory();
  }
}

template <typename Sort, typename T>
void BM_Reversed(benchmark::State &state, T zero, Sort sort) {
  for (auto _ : state) {
    int n = state.range(0);
    std::vector<T> v(n);
    std::iota(std::rbegin(v), std::rend(v), zero);

    sort(v);

    benchmark::DoNotOptimize(v.data());
    benchmark::ClobberMemory();
  }
}

static void CustomArguments(benchmark::internal::Benchmark *b) {
  b->Arg(2)->Arg(10)->Arg(50)->Arg(1000)->Arg(1 << 15);
}

#define SORT_BENCHMARK(BM_fn, sort)                                            \
  BENCHMARK_CAPTURE(BM_fn, sort, int(0), (sort))->Apply(CustomArguments)

StdSort<int> std_sort;
StdStableSort<int> std_stable_sort;
CheckedSort<StdSort<int>> checked_std_sort;

SORT_BENCHMARK(BM_Sorted, std_sort);
SORT_BENCHMARK(BM_Sorted, std_stable_sort);
SORT_BENCHMARK(BM_Sorted, checked_std_sort);
SORT_BENCHMARK(BM_Sorted, Dispatch<std::vector<int> &>(std::make_tuple(
                              checked_std_sort, std_stable_sort, std_sort)));

SORT_BENCHMARK(BM_Reversed, std_sort);
SORT_BENCHMARK(BM_Reversed, std_stable_sort);
SORT_BENCHMARK(BM_Reversed, checked_std_sort);
SORT_BENCHMARK(BM_Reversed, Dispatch<std::vector<int> &>(std::make_tuple(
                                checked_std_sort, std_stable_sort, std_sort)));

BENCHMARK_MAIN();
