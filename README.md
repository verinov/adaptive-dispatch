# Usage
The implementation is in `src/*.h` and has no external dependencies.

As is done in `src/test.cpp`, let's consider choosing a sorting algorithm implementation out of a few options. Using this library, this might look like:
```c++
#include <algorithm>
#include <vector>

#include "bandit.h"

template <typename T> struct AdaptiveSort {
  static auto MakeFunc() {
    // Dispatch also accepts functors and function pointers
    return Dispatch<std::vector<T> &>(std::make_tuple(
        [](auto &x) { std::sort(std::begin(x), std::end(x)); },
        [](auto &x) { std::stable_sort(std::begin(x), std::end(x)); },
        [](auto &x) {
            if (!std::is_sorted(std::begin(x), std::end(x))) {
                std::stable_sort(std::begin(x), std::end(x));
            }
        }));
  }

  decltype(MakeFunc()) func = MakeFunc();
  void operator()(std::vector<T> &x) { return func(x); }
};
```

The same, but using low-level syntax:
```c++
// alternative implementation of AdaptiveSort
template <typename T> struct AnotherAdaptiveSort {
  TimingSelector<3> selector{};

  void operator()(std::vector<T> &x) {
    // To skip the timing, use `switch (selector.GetOption()) {` instead
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
```

# Running benchmarks
First install https://github.com/google/benchmark.
Then build:
```bash
mkdir build
cd build
cmake ..
make
```

Then run benchmarks (from `build` directory):
```bash
# Text-only option
./src/Test --benchmark_repetitions=5 --benchmark_report_aggregates_only=true

# The option with images
mkdir benchmark_results
./src/Test --benchmark_repetitions=5 --benchmark_report_aggregates_only=false \
    --benchmark_out_format=csv --benchmark_out=bm_results.csv \
&& python ../benchmark_results.py
```
