First install https://github.com/google/benchmark.
Then `mkdir build && cd build && cmake .. && make`.
Then
```
./src/Test --benchmark_repetitions=5 --benchmark_report_aggregates_only=true
```