#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include "executors.h"

using namespace original;
using namespace original::literals;

// 辅助宏：允许小误差的定时器检查（±15ms 通常可接受）
#define EXPECT_DURATION_NEAR(actual_ms, expected_ms) \
    EXPECT_NEAR(actual_ms, expected_ms, 15)

TEST(EventsLoopExecutorTest, ImmediateTask_ExecutedInRunLoop)
{
    eventsLoopExecutor loop;

    std::atomic counter{0};
    loop.schedule([&counter] { counter.fetch_add(1); });

    // 在另一个线程运行事件循环
    thread loop_thread([&loop] { loop.run(); });

    // 等待任务执行
    while (counter.load() == 0) thread::sleep(1_ms);

    loop.stop();

    EXPECT_EQ(counter.load(), 1);
}

TEST(EventsLoopExecutorTest, DelayedTask_ExecutedAfterDelay)
{
    eventsLoopExecutor loop;
    std::atomic executed{false};
    std::atomic<std::chrono::steady_clock::time_point> execute_time{};

    thread loop_thread([&loop] { loop.run(); });

    std::atomic loop_ready{false};
    loop.schedule([&loop_ready] { loop_ready = true; });
    while (!loop_ready.load()) {
        thread::yield();
    }

    const auto scheduled_time = std::chrono::steady_clock::now();

    auto fn = [&executed, &execute_time] {
        execute_time = std::chrono::steady_clock::now();
        executed = true;
    };

    loop.schedule(200_ms, fn);

    while (!executed.load()) {
        thread::yield();
    }

    const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        execute_time.load() - scheduled_time).count();

    loop.stop();
    EXPECT_DURATION_NEAR(elapsed_ms, 200);
}

TEST(EventsLoopExecutorTest, MultipleTimers_ExecuteInOrder)
{
    eventsLoopExecutor loop;

    std::vector<int> order;
    std::mutex mtx;

    loop.schedule(100_ms, [&] {
        std::lock_guard lk(mtx);
        order.push_back(1);
    });
    loop.schedule(50_ms, [&] {
        std::lock_guard lk(mtx);
        order.push_back(2);
    });
    loop.schedule(150_ms, [&] {
        std::lock_guard lk(mtx);
        order.push_back(3);
    });

    thread loop_thread([&loop] { loop.run(); });
    thread::sleep(300_ms);

    loop.stop();

    ASSERT_EQ(order.size(), 3);
    EXPECT_EQ(order[0], 2);  // 50ms 先到
    EXPECT_EQ(order[1], 1);  // 100ms 次之
    EXPECT_EQ(order[2], 3);  // 150ms 最后
}

TEST(EventsLoopExecutorTest, RunOnce_ExecutesOneTask)
{
    eventsLoopExecutor loop;

    std::atomic<int> counter{0};
    loop.schedule([&] { ++counter; });
    loop.schedule([&] { counter += 10; });

    loop.runOnce();
    loop.runOnce();

    // 两个任务都应执行
    EXPECT_EQ(counter.load(), 11);

    // 队列应为空
    loop.runOnce();
    EXPECT_EQ(counter.load(), 11);  // 不再增加
}

TEST(EventsLoopExecutorTest, Stop_PreventsNewTasks)
{
    eventsLoopExecutor loop;
    loop.stop();

    std::atomic<bool> executed{false};
    loop.schedule([&executed] { executed = true; });
    loop.schedule(100_ms, [&executed] { executed = true; });

    // 给定时器线程一点时间
    thread::sleep(50_ms);

    EXPECT_FALSE(executed);
    EXPECT_TRUE(loop.hasStopped());
}

TEST(EventsLoopExecutorTest, Destructor_JoinsTimerThread)
{
    {
        std::atomic thread_started{false};
        eventsLoopExecutor loop;
        thread loop_thread{[&loop]{ loop.run(); }};
        loop.schedule(10_ms, [&thread_started] { thread_started = true; });

        thread::sleep(50_ms);
        loop.stop();
        EXPECT_TRUE(thread_started.load());
    }

    SUCCEED() << "Destructor correctly joined timer thread";
}

TEST(EventsLoopExecutorTest, ScheduleAfterStop_IsIgnored)
{
    eventsLoopExecutor loop;
    std::thread loop_thread([&loop] { loop.run(); });

    thread::sleep(50_ms);  // 确保 run() 已启动
    loop.stop();

    std::atomic<int> counter{0};
    loop.schedule([&counter] { ++counter; });  // 应被忽略
    loop.schedule(10_ms, [&counter] { ++counter; });  // 应被忽略

    loop_thread.join();
    EXPECT_EQ(counter.load(), 0);
}

TEST(EventsLoopExecutorTest, PeriodicTimer_ManualImplementation)
{
    eventsLoopExecutor loop;

    std::atomic count{0};
    auto periodic = [&](auto&& self) -> void {
        ++count;
        if (count < 5)
            loop.schedule(50_ms, [self] { self(self); });
    };

    loop.schedule(50_ms, [periodic] { periodic(periodic); });

    thread loop_thread([&loop] { loop.run(); });
    thread::sleep(400_ms);  // 5 次 × 50ms = 250ms + 余量

    loop.stop();
    EXPECT_EQ(count.load(), 5);
}