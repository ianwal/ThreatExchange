#include <benchmark/benchmark.h>
#include <vpdq/cpp/hashing/vpdqHashType.h>
#include <vpdq/cpp/hashing/filehasher.h>
#include <stdexcept>

static void BM_hashVideoFile(benchmark::State& state) {
    {
    std::vector<facebook::vpdq::hashing::vpdqFeature> features;
  if(!facebook::vpdq::hashing::hashVideoFile("../../../tmk/sample-videos/chair-22-sd-grey-bar.mp4", features))
  {
    throw std::runtime_error("error");
  }
    }
  for (auto _ : state) {
    std::vector<facebook::vpdq::hashing::vpdqFeature> features;
    facebook::vpdq::hashing::hashVideoFile("../../../tmk/sample-videos/chair-22-sd-grey-bar.mp4",features);
  }
}
BENCHMARK(BM_hashVideoFile)->Unit(benchmark::kMillisecond);;

// Define another benchmark
static void BM_StringCopy(benchmark::State& state) {
  std::string x = "hello";
  for (auto _ : state)
    std::string copy(x);
}
BENCHMARK(BM_StringCopy);

BENCHMARK_MAIN();
