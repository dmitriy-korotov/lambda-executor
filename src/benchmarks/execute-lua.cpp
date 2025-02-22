#include <miet/lambda-executor/handlers/execute-lambda.hpp>

#include <cstdint>
#include <iterator>
#include <string_view>

#include <benchmark/benchmark.h>
#include <userver/engine/run_standalone.hpp>

/**
 * @todo Write a benchmark for Lua execution */
void ExecuteLuaBenchmark(benchmark::State& state) {
  userver::engine::RunStandalone([&] {
    constexpr std::string_view kNames[] = {"userver", "is", "awesome", "!"};
    std::uint64_t i = 0;

    for (auto _ : state) {
      const auto name = kNames[i++ % std::size(kNames)];
      auto result = std::string(name) + "_stub";
      benchmark::DoNotOptimize(result);
    }
  });
}

BENCHMARK(ExecuteLuaBenchmark);
