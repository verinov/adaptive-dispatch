# Intro
`adaptive-dispatch` is a C++ library to optimize parameter choice
from a fixed set of options at runtime.

Suppose you have several functions which solve the same problem
but with different trade-offs and edge cases (e.g. different ways to sort).
Or you need to select a hyperparameter value out of a set of reasonable options
(e.g. use 1, 2 or 4 threads for a job).
`adaptive-dispatch` library lets you make this kind of decisions
based on runtime feedback.

More generally, this library tries to make an on average good choice
from a fixed set of options at runtime. "Goodness" here is judged by a number,
potentially known after each execution. "Goodness" is measured on a small
fraction of calls to reduce the overhead. 

At the moment, only the number of CPU cycles is implemented. 

This library is at PoC stage, and more sophisticated selection strategies
are yet to come.

# Usage
The implementation is in `src/*.h`. It has no external dependencies.

As is done in `src/test.cpp`, let's consider choosing a sorting algorithm
implementation out of a few options. Using this library, this might look like:
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

## Example results
We ran the following benchmark scenarios:
- Almost sorted data: [0, 1, ..., N-3, N-1, N-2]
  (sorted vector, but with the last two items swapped)
- Reversed data: [N-1, N-2, ..., 1, 0]
- Sorted data: [0, 1, ..., N-2, N-1]

They were executed for N in [8, 16, 64, 2^10, 2^15].

Each (scenario, N) pair was executed with each tested algorithm 5 times, and
normalized by the time of the fasted sample (hence relative time).

Geometric mean of relative times, grouping by (scenario, algorithm):
```
BM_AlmostSorted
        AdaptiveSort<int>{}            1.1144566731696144
        AnotherAdaptiveSort<int>{}     1.1186069476189522
        CheckedSort<StdSort<int>>{}    1.1977358462070637
        StdSort<int>{}                 1.072571258777149
        StdStableSort<int>{}           1.5578675199922665

BM_Reversed
        AdaptiveSort<int>{}            1.0540428636553096
        AnotherAdaptiveSort<int>{}     1.053041714829056
        CheckedSort<StdSort<int>>{}    1.0210169241463283
        StdSort<int>{}                 1.0132608758660413
        StdStableSort<int>{}           1.8229502967067002

BM_Sorted
        AdaptiveSort<int>{}            1.2239400893573669
        AnotherAdaptiveSort<int>{}     1.2219118315436746
        CheckedSort<StdSort<int>>{}    1.032098123866385
        StdSort<int>{}                 4.198302303266702
        StdStableSort<int>{}           6.169533565071652
```

Geomean of the geomeans above, now grouping by just algorithm:
```
Overall
        AdaptiveSort<int>{}            1.1286532602812622
        AnotherAdaptiveSort<int>{}     1.129070306399413
        CheckedSort<StdSort<int>>{}    1.0806996039760388
        StdSort<int>{}                 1.6585951029560353
        StdStableSort<int>{}           2.5972827267867578
```

Data before the aggregations:
![Sorted](/benchmark_results/BM_Sorted.png)
![Reversted](/benchmark_results/BM_Reversed.png)
![Almost sorted](/benchmark_results/BM_AlmostSorted.png)
