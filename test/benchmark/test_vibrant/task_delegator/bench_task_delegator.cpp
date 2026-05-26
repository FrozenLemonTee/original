#include <benchmark/benchmark.h>
#include <atomic>
#include <cstdint>
#include <utility>
#include <vector>

#include "tasks.h"
#include "task_delegator_legacy.h"

namespace {

constexpr int WORK_ITERATIONS = 128;
constexpr int LONG_WORK_ITERATIONS = 8192;

std::uint32_t cpuWorkFor(const int seed, const int iterations)
{
    auto value = static_cast<std::uint32_t>(seed);
    for (int i = 0; i < iterations; ++i)
    {
        value = value * 1664525u + 1013904223u;
    }
    return value;
}

std::uint32_t cpuWork(const int seed)
{
    return cpuWorkFor(seed, WORK_ITERATIONS);
}

std::uint32_t longCpuWork(const int seed)
{
    return cpuWorkFor(seed, LONG_WORK_ITERATIONS);
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
void runLongTaskThroughput(benchmark::State& state)
{
    const auto thread_count = static_cast<original::u_integer>(state.range(0));
    const auto task_count = static_cast<int>(state.range(1));
    using Future = decltype(std::declval<Delegator&>().submit(longCpuWork, 0));

    for (auto _ : state)
    {
        Delegator delegator(thread_count);
        std::vector<Future> futures;
        futures.reserve(task_count);

        for (int i = 0; i < task_count; ++i)
        {
            futures.push_back(delegator.submit(longCpuWork, i));
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

template <typename Delegator>
void runLongMixedPriorityThroughput(benchmark::State& state)
{
    const auto thread_count = static_cast<original::u_integer>(state.range(0));
    const auto task_count = static_cast<int>(state.range(1));
    using Future = decltype(std::declval<Delegator&>().submit(longCpuWork, 0));
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
            futures.push_back(delegator.submit(priority, longCpuWork, i));
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

void BM_TaskDelegatorLegacy_NormalTaskThroughput(benchmark::State& state)
{
    runNormalTaskThroughput<original::taskDelegatorLegacy>(state);
}

void BM_TaskDelegator_LongTaskThroughput(benchmark::State& state)
{
    runLongTaskThroughput<original::taskDelegator>(state);
}

void BM_TaskDelegatorLegacy_LongTaskThroughput(benchmark::State& state)
{
    runLongTaskThroughput<original::taskDelegatorLegacy>(state);
}

void BM_TaskDelegator_MixedPriorityThroughput(benchmark::State& state)
{
    runMixedPriorityThroughput<original::taskDelegator>(state);
}

void BM_TaskDelegatorLegacy_MixedPriorityThroughput(benchmark::State& state)
{
    runMixedPriorityThroughput<original::taskDelegatorLegacy>(state);
}

void BM_TaskDelegator_LongMixedPriorityThroughput(benchmark::State& state)
{
    runLongMixedPriorityThroughput<original::taskDelegator>(state);
}

void BM_TaskDelegatorLegacy_LongMixedPriorityThroughput(benchmark::State& state)
{
    runLongMixedPriorityThroughput<original::taskDelegatorLegacy>(state);
}

} // namespace

BENCHMARK(BM_TaskDelegator_NormalTaskThroughput)
        ->Args({1, 1024})
        ->Args({2, 1024})
        ->Args({4, 4096})
        ->Args({8, 4096})
        ->Unit(benchmark::kMillisecond);

BENCHMARK(BM_TaskDelegatorLegacy_NormalTaskThroughput)
        ->Args({1, 1024})
        ->Args({2, 1024})
        ->Args({4, 4096})
        ->Args({8, 4096})
        ->Unit(benchmark::kMillisecond);

BENCHMARK(BM_TaskDelegator_LongTaskThroughput)
        ->Args({1, 1024})
        ->Args({2, 1024})
        ->Args({4, 4096})
        ->Args({8, 4096})
        ->Unit(benchmark::kMillisecond);

BENCHMARK(BM_TaskDelegatorLegacy_LongTaskThroughput)
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

BENCHMARK(BM_TaskDelegatorLegacy_MixedPriorityThroughput)
        ->Args({1, 1024})
        ->Args({2, 1024})
        ->Args({4, 4096})
        ->Args({8, 4096})
        ->Unit(benchmark::kMillisecond);

BENCHMARK(BM_TaskDelegator_LongMixedPriorityThroughput)
        ->Args({1, 1024})
        ->Args({2, 1024})
        ->Args({4, 4096})
        ->Args({8, 4096})
        ->Unit(benchmark::kMillisecond);

BENCHMARK(BM_TaskDelegatorLegacy_LongMixedPriorityThroughput)
        ->Args({1, 1024})
        ->Args({2, 1024})
        ->Args({4, 4096})
        ->Args({8, 4096})
        ->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();


