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
./src/Test --benchmark_repetitions=5 --benchmark_report_aggregates_only=false --benchmark_out_format=csv --benchmark_out=bm_results.csv
python ../benchmark_results.py
```
