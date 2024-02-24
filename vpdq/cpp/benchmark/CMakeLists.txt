cmake_minimum_required (VERSION 3.17)

# Include Google Benchmark
include(./GoogleBenchmark.cmake)

# Add source to this project's executable.
add_executable (vpdqbenchmark benchmark_vpdq.cpp)

# Link Google Benchmark to the project
target_link_libraries(vpdqbenchmark benchmark::benchmark)

# Link Shlwapi to the project
if ("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
    target_link_libraries(vpdqbenchmark Shlwapi)
endif()

# TODO: Add tests and install targets if needed.
