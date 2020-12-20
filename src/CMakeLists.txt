find_package(benchmark REQUIRED)

add_executable (Test test.cpp)
target_link_libraries(
    Test
    benchmark::benchmark
)
