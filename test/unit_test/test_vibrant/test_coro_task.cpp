#include <gtest/gtest.h>
#include "executors.h"
#include "coroutines.h"
#include "tasks.h"
#include <atomic>
#include <thread>
#include <chrono>

using namespace original;
using namespace  std::literals;

class CoroutineTaskTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

// Test basic task creation and execution
TEST_F(CoroutineTaskTest, BasicTaskCreation) {
    auto simpleTask = []() -> coroutine::task<int> {
        co_return 42;
    }();

    // Lazy task should not be ready initially
    EXPECT_FALSE(simpleTask.ready());

    // Start the task
    simpleTask.start();
    EXPECT_TRUE(simpleTask.ready());
    EXPECT_EQ(simpleTask.result(), 42);
}

// Test task with computation
TEST_F(CoroutineTaskTest, TaskWithComputation) {
    auto computeTask = []() -> coroutine::task<int> {
        int result = 0;
        for (int i = 1; i <= 5; ++i) {
            result += i;
        }
        co_return result;
    }();

    EXPECT_FALSE(computeTask.ready());
    computeTask.start();
    EXPECT_TRUE(computeTask.ready());
    EXPECT_EQ(computeTask.result(), 15); // 1+2+3+4+5 = 15
}

// Test task exception handling
TEST_F(CoroutineTaskTest, TaskExceptionHandling) {
    auto throwingTask = []() -> coroutine::task<int> {
        throw std::runtime_error("Test exception");
        co_return 42; // NOLINT: Unreachable code test
    }();

    EXPECT_FALSE(throwingTask.ready());
    throwingTask.start();
    EXPECT_TRUE(throwingTask.ready());
    EXPECT_THROW(throwingTask.result(), std::runtime_error);
}

// Test task with co_await
TEST_F(CoroutineTaskTest, TaskWithCoAwait) {
    auto nestedTask = []() -> coroutine::task<int> {
        co_return 100;
    };

    auto mainTask = [nestedTask = std::move(nestedTask)]() mutable -> coroutine::task<int> {
        const auto value = co_await nestedTask();
        co_return value + 50;
    }();

    EXPECT_FALSE(mainTask.ready());
    mainTask.start();
    EXPECT_TRUE(mainTask.ready());
    EXPECT_EQ(mainTask.result(), 150);
}

// Test syncExecutor with basic task
TEST_F(CoroutineTaskTest, SyncExecutorBasic) {
    syncExecutor executor;

    auto task = [](syncExecutor& exec) -> coroutine::task<int> {
        co_await exec;
        co_return 123;
    }(executor);

    // Use wait to execute the task - this should start and run the task
    int result = executor.wait(std::move(task));
    EXPECT_EQ(result, 123);
}

// Test syncExecutor with multiple tasks
TEST_F(CoroutineTaskTest, SyncExecutorMultipleTasks) {
    syncExecutor executor;

    std::atomic<int> counter{0};

    auto task1 = [&](syncExecutor& exec) -> coroutine::task<int> {
        co_await exec;
        counter.fetch_add(1, std::memory_order_relaxed);
        co_return 1;
    }(executor);

    auto task2 = [&](syncExecutor& exec) -> coroutine::task<int> {
        co_await exec;
        counter.fetch_add(2, std::memory_order_relaxed);
        co_return 2;
    }(executor);

    int result1 = executor.wait(std::move(task1));
    int result2 = executor.wait(std::move(task2));

    EXPECT_EQ(result1, 1);
    EXPECT_EQ(result2, 2);
    EXPECT_EQ(counter.load(), 3);
}

// Test syncExecutor spinWait
TEST_F(CoroutineTaskTest, SyncExecutorSpinWait) {
    syncExecutor executor;

    auto delayedTask = [](syncExecutor& exec) -> coroutine::task<int> {
        co_await exec;
        // Simulate some work
        int result = 0;
        for (int i = 0; i < 1000; ++i) {
            result += i % 10;
        }
        co_return result;
    }(executor);

    const int result = executor.spinWait(std::move(delayedTask));
    EXPECT_GT(result, 0);
}

// Test threadPoolExecutor
TEST_F(CoroutineTaskTest, ThreadPoolExecutor) {
    taskDelegator delegator;
    threadPoolExecutor executor(delegator);

    auto task = [](threadPoolExecutor& exec) -> coroutine::task<int> {
        co_await exec;
        co_return 999;
    }(executor);

    // Since MockTaskDelegator executes immediately, task should be ready after start
    EXPECT_FALSE(task.ready());
    task.start();
    thread::sleep(milliseconds(10));
    EXPECT_TRUE(task.ready());
    EXPECT_EQ(task.result(), 999);
}

// Test task via method
TEST_F(CoroutineTaskTest, TaskViaExecutor) {
    syncExecutor executor;

    auto task = []() -> coroutine::task<std::string> {
        co_return "Hello via executor";
    }();

    // Task shouldn't have executor initially
    EXPECT_FALSE(task.hasExecutor());
    EXPECT_FALSE(task.ready());

    // Set executor via via() method - this should schedule the task
    task.via(executor);
    EXPECT_TRUE(task.hasExecutor());

    std::string result = executor.wait(std::move(task));
    EXPECT_EQ(result, "Hello via executor");
}

// Test coroutine::run function
TEST_F(CoroutineTaskTest, CoroutineRunFunction) {
    syncExecutor executor;

    auto addFunction = [](int a, int b) -> int {
        return a + b;
    };

    auto runTask = coroutine::makeTask(executor, addFunction, 10, 20);

    // run should schedule the task on executor
    EXPECT_FALSE(runTask.hasExecutor());
    const int result = executor.wait(std::move(runTask));
    EXPECT_EQ(result, 30);
}

namespace {
    auto fibonacci(const int n) -> coroutine::task<int> {
        if (n <= 1)
            co_return n;

        auto task1 = fibonacci(n - 1);
        auto task2 = fibonacci(n - 2);

        int result = co_await task1 + co_await task2;
        co_return result;
    }
}

// Test task with complex computation
TEST_F(CoroutineTaskTest, ComplexComputationTask) {
    syncExecutor executor;
    auto task = fibonacci(6); // fib(6) = 8
    task.via(executor);

    int result = executor.wait(std::move(task));
    EXPECT_EQ(result, 8);
}

// Test task destruction before completion
TEST_F(CoroutineTaskTest, TaskDestruction) {
    std::atomic destroyed{false};

    {
        auto task = [&]() -> coroutine::task<int> {
            struct DestructionDetector {
                std::atomic<bool>& destroyed_;
                ~DestructionDetector() { destroyed_ = true; }
            };

            DestructionDetector detector{destroyed};

            co_return 42;
        }();

        // Task should not be destroyed yet
        EXPECT_FALSE(destroyed.load());
        // Task should not be ready until started
        EXPECT_FALSE(task.ready());
        task.start();
    }
    // Task should be destroyed now
    EXPECT_TRUE(destroyed.load());
}

// Test executor co_await operator
TEST_F(CoroutineTaskTest, ExecutorCoAwaitOperator) {
    syncExecutor executor;

    auto task = [&]() -> coroutine::task<bool> {
        // Use co_await on executor directly
        co_await executor;
        co_return true;
    }();

    const bool result = executor.wait(std::move(task));
    EXPECT_TRUE(result);
}

// Test multiple co_await in single task
TEST_F(CoroutineTaskTest, MultipleCoAwait) {
    syncExecutor executor;

    auto task = [&]() -> coroutine::task<int> {
        co_await executor;
        int value = 0;

        co_await executor;
        value += 10;

        co_await executor;
        value += 20;

        co_return value;
    }();

    const int result = executor.wait(std::move(task));
    EXPECT_EQ(result, 30);
}

// Test that task without executor needs manual start
TEST_F(CoroutineTaskTest, ManualStartRequired) {
    auto task = []() -> coroutine::task<int> {
        co_return 777;
    }();

    // Task should not be ready without executor or manual start
    EXPECT_FALSE(task.ready());
    EXPECT_FALSE(task.hasExecutor());

    // Manual start should make it ready
    task.start();
    EXPECT_TRUE(task.ready());
    EXPECT_EQ(task.result(), 777);
}

// Test task with executor doesn't need manual start
TEST_F(CoroutineTaskTest, ExecutorAutoStart) {
    syncExecutor executor;

    auto task = [](syncExecutor& exec) -> coroutine::task<int> {
        co_await exec;
        co_return 888;
    }(executor);

    // Task with executor should be scheduled automatically when awaited
    const int result = executor.wait(std::move(task));
    EXPECT_EQ(result, 888);
}

// Test co_await on task
TEST_F(CoroutineTaskTest, CoAwaitOnTask) {
    auto innerTask = []() -> coroutine::task<int> {
        co_return 333;
    };

    auto outerTask = [&]() -> coroutine::task<int> {
        const auto value1 = co_await innerTask();
        const auto value2 = co_await innerTask();
        co_return value1 + value2;
    }();

    outerTask.start();
    EXPECT_TRUE(outerTask.ready());
    EXPECT_EQ(outerTask.result(), 666);
}

// Test coroutine::makeTask with syncExecutor - basic functionality
TEST_F(CoroutineTaskTest, MakeTaskWithSyncExecutorBasic) {
    syncExecutor executor;

    auto simpleFunction = [](const int a, const int b) -> int {
        return a * b;
    };

    auto task = coroutine::makeTask(executor, simpleFunction, 7, 8);

    // Task is lazy
    EXPECT_FALSE(task.ready());
    EXPECT_FALSE(task.hasExecutor());

    // Execute task
    const int result = executor.wait(std::move(task));
    EXPECT_EQ(result, 56);
}

// Test coroutine::makeTask with syncExecutor - void return type
TEST_F(CoroutineTaskTest, MakeTaskWithSyncExecutorVoidReturn) {
    syncExecutor executor;

    std::atomic counter{0};
    auto voidFunction = [&counter](const int increment) { counter += increment; };

    auto task = coroutine::makeTask(executor, voidFunction, 5);

    EXPECT_FALSE(task.ready());
    EXPECT_FALSE(task.hasExecutor());

    executor.wait(std::move(task));
    EXPECT_EQ(counter.load(), 5);
}

// Test coroutine::makeTask with syncExecutor - exception propagation
TEST_F(CoroutineTaskTest, MakeTaskWithSyncExecutorException) {
    syncExecutor executor;

    auto throwingFunction = [](const std::string& msg) -> std::string {
        throw std::runtime_error(msg);
    };

    auto task = coroutine::makeTask(executor, throwingFunction, "error");

    EXPECT_FALSE(task.ready());
    EXPECT_THROW(executor.wait(std::move(task)), std::runtime_error);
}

// Test coroutine::makeTask with syncExecutor - multiple sequential tasks
TEST_F(CoroutineTaskTest, MakeTaskWithSyncExecutorMultipleSequential) {
    syncExecutor executor;

    auto accumulateFunction = [](int start, const std::vector<int>& values) -> int {
        int sum = start;
        for (int v : values) sum += v;
        return sum;
    };

    auto task1 = coroutine::makeTask(executor, accumulateFunction, 10, std::vector{1,2,3});
    auto task2 = coroutine::makeTask(executor, accumulateFunction, 20, std::vector{4,5,6});

    const int result1 = executor.wait(std::move(task1));
    const int result2 = executor.wait(std::move(task2));

    EXPECT_EQ(result1, 16);
    EXPECT_EQ(result2, 35);
}

// Test mixed usage of threadPoolExecutor and syncExecutor
TEST_F(CoroutineTaskTest, MixedThreadPoolAndSyncExecutor) {
    // Shared taskDelegator for threadPoolExecutor
    taskDelegator delegator;
    threadPoolExecutor threadPoolExec(delegator);
    syncExecutor syncExec;

    std::atomic counter{0};

    // Task submitted to threadPoolExecutor
    auto poolTask = [&](threadPoolExecutor& exec) -> coroutine::task<int> {
        co_await exec;
        counter.fetch_add(10, std::memory_order_relaxed);
        co_return 10;
    }(threadPoolExec);

    // Task submitted to syncExecutor
    auto syncTask = [&](syncExecutor& exec) -> coroutine::task<int> {
        co_await exec;
        counter.fetch_add(1, std::memory_order_relaxed);
        co_return 1;
    }(syncExec);

    // Execute thread pool task manually
    poolTask.start();
    // Give some time for thread pool task to run
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Execute sync executor task
    const int syncResult = syncExec.wait(std::move(syncTask));

    // Wait for thread pool task to finish
    int poolResult = poolTask.result();

    EXPECT_EQ(poolResult, 10);
    EXPECT_EQ(syncResult, 1);
    EXPECT_EQ(counter.load(), 11);
}

// Test mixed usage of threadPoolExecutor and syncExecutor using makeTask
TEST_F(CoroutineTaskTest, MixedExecutorsWithMakeTask) {
    taskDelegator delegator;
    threadPoolExecutor threadPoolExec(delegator);
    syncExecutor syncExec;

    std::atomic counter{0};

    // Task for threadPoolExecutor via makeTask
    auto poolTask = coroutine::makeTask(threadPoolExec, [&counter]() -> int {
        counter.fetch_add(10, std::memory_order_relaxed);
        return 10;
    });

    // Task for syncExecutor via makeTask
    auto syncTask = coroutine::makeTask(syncExec, [&counter]() -> int {
        counter.fetch_add(1, std::memory_order_relaxed);
        return 1;
    });

    // Start the threadPoolExecutor task manually
    poolTask.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(50)); // give thread pool time to run

    // Execute the syncExecutor task
    const int syncResult = syncExec.wait(std::move(syncTask));

    // Wait for the threadPoolExecutor task to complete
    const int poolResult = poolTask.result();

    EXPECT_EQ(poolResult, 10);
    EXPECT_EQ(syncResult, 1);
    EXPECT_EQ(counter.load(), 11);
}
