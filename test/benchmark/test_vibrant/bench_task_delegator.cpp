#include <benchmark/benchmark.h>
#include <atomic>
#include <cstdint>
#include <utility>
#include <vector>

#include "tasks.h"

namespace {

constexpr int WORK_ITERATIONS = 128;

std::uint32_t cpuWork(const int seed)
{
    auto value = static_cast<std::uint32_t>(seed);
    for (int i = 0; i < WORK_ITERATIONS; ++i)
    {
        value = value * 1664525u + 1013904223u;
    }
    return value;
}

template <typename Delegator>
void runNormalTaskThroughput(benchmark::State& state)
{
    const auto thread_count = static_cast<original::u_integer>(state.range(0));
    const auto task_count = static_cast<int>(state.range(1));
    using Future = decltype(std::declval<Delegator&>().submit(cpuWork, 0));

    for (auto _ : state)
    {
        Delegator delegator(thread_count);
        std::vector<Future> futures;
        futures.reserve(task_count);

        for (int i = 0; i < task_count; ++i)
        {
            futures.push_back(delegator.submit(cpuWork, i));
        }

        std::uint32_t result = 0;
        for (auto& future : futures)
        {
            result += future.result();
        }

        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations() * task_count);
}

template <typename Delegator>
void runMixedPriorityThroughput(benchmark::State& state)
{
    const auto thread_count = static_cast<original::u_integer>(state.range(0));
    const auto task_count = static_cast<int>(state.range(1));
    using Future = decltype(std::declval<Delegator&>().submit(cpuWork, 0));
    std::atomic<std::uint32_t> sink{0};

    for (auto _ : state)
    {
        Delegator delegator(thread_count);
        std::vector<Future> futures;
        futures.reserve(task_count);

        for (int i = 0; i < task_count; ++i)
        {
            const auto priority = i % 3 == 0 ? Delegator::HIGH
                                : i % 3 == 1 ? Delegator::NORMAL
                                             : Delegator::LOW;
            futures.push_back(delegator.submit(priority, cpuWork, i));
        }

        std::uint32_t result = 0;
        for (auto& future : futures)
        {
            result += future.result();
        }

        sink.fetch_add(result, std::memory_order_relaxed);
        benchmark::DoNotOptimize(sink.load(std::memory_order_relaxed));
    }

    state.SetItemsProcessed(state.iterations() * task_count);
}

void BM_TaskDelegator_NormalTaskThroughput(benchmark::State& state)
{
    runNormalTaskThroughput<original::taskDelegator>(state);
}

void BM_TaskDelegatorV2_NormalTaskThroughput(benchmark::State& state)
{
    runNormalTaskThroughput<original::taskDelegatorV2>(state);
}

void BM_TaskDelegator_MixedPriorityThroughput(benchmark::State& state)
{
    runMixedPriorityThroughput<original::taskDelegator>(state);
}

void BM_TaskDelegatorV2_MixedPriorityThroughput(benchmark::State& state)
{
    runMixedPriorityThroughput<original::taskDelegatorV2>(state);
}

} // namespace

BENCHMARK(BM_TaskDelegator_NormalTaskThroughput)
        ->Args({1, 1024})
        ->Args({2, 1024})
        ->Args({4, 4096})
        ->Args({8, 4096})
        ->Unit(benchmark::kMillisecond);

BENCHMARK(BM_TaskDelegatorV2_NormalTaskThroughput)
        ->Args({1, 1024})
        ->Args({2, 1024})
        ->Args({4, 4096})
        ->Args({8, 4096})
        ->Unit(benchmark::kMillisecond);

BENCHMARK(BM_TaskDelegator_MixedPriorityThroughput)
        ->Args({1, 1024})
        ->Args({2, 1024})
        ->Args({4, 4096})
        ->Args({8, 4096})
        ->Unit(benchmark::kMillisecond);

BENCHMARK(BM_TaskDelegatorV2_MixedPriorityThroughput)
        ->Args({1, 1024})
        ->Args({2, 1024})
        ->Args({4, 4096})
        ->Args({8, 4096})
        ->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
