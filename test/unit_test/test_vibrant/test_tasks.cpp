#include <gtest/gtest.h>
#include "tasks.h"
#include "thread.h"

using namespace original;

// 基础测试函数
int add_func(const int a, const int b) {
    thread::sleep(seconds(1));
    return a + b;
}

int sub_func(const int a, const int b) {
    thread::sleep(seconds(1));
    return a - b;
}

TEST(TaskDelegatorTest, SubmitNormalTasks) {
    taskDelegator delegator(4);

    auto f1 = delegator.submit(add_func, 2, 3);
    auto f2 = delegator.submit(sub_func, 10, 4);

    thread::sleep(seconds(1));

    EXPECT_EQ(f1.result(), 5);
    EXPECT_EQ(f2.result(), 6);
}

TEST(TaskDelegatorTest, SubmitHighPriority) {
    taskDelegator delegator(2);

    auto f1 = delegator.submit(taskDelegator::NORMAL, add_func, 1, 1);
    auto f2 = delegator.submit(taskDelegator::HIGH, add_func, 2, 2);

    EXPECT_EQ(f1.result(), 2);
    EXPECT_EQ(f2.result(), 4);
}

TEST(TaskDelegatorTest, StopPreventsNewSubmits) {
    taskDelegator delegator(2);

    auto f1 = delegator.submit(add_func, 3, 4);
    EXPECT_EQ(f1.result(), 7);

    delegator.stop();

    EXPECT_THROW({
        delegator.submit(add_func, 5, 6);
    }, sysError);
}

// 测试立即任务在没有空闲线程时提交会抛出异常
TEST(TaskDelegatorTest, SubmitImmediateWithoutIdleThreadThrows) {
    taskDelegator delegator{4};


    for (int i = 0; i < 10; ++i)
    {
        // 提交长时间运行的任务，使线程繁忙
        auto long_task = delegator.submit([]{
            thread::sleep(milliseconds(1500));
            return 42;
        });
    }

    // 尝试提交立即任务，应抛出异常
    EXPECT_THROW({
        delegator.submit(taskDelegator::IMMEDIATE, []{ return 0; });
    }, sysError);
}

// 测试未知优先级抛出异常
TEST(TaskDelegatorTest, SubmitWithUnknownPriorityThrows) {
    taskDelegator delegator(2);

    // 使用无效的优先级值
    constexpr auto invalid_priority = static_cast<taskDelegator::priority>(999);

    EXPECT_THROW({
        delegator.submit(invalid_priority, []{ return 0; });
    }, sysError);
}

// 测试析构函数自动停止
TEST(TaskDelegatorTest, DestructorAutoStops) {
    // 在内部作用域创建delegator，验证析构时自动停止
    {
        taskDelegator delegator(2);
        auto f = delegator.submit([]{ return 1; });
        EXPECT_EQ(f.result(), 1);
        // 不手动调用stop，让析构函数处理
    }
    // 如果析构函数正常工作，此处不应有问题
    SUCCEED();
}

// 测试活动线程和空闲线程计数
TEST(TaskDelegatorTest, ActiveAndIdleThreadCounts) {
    taskDelegator delegator(2);

    // 初始状态，所有线程空闲
    EXPECT_EQ(delegator.activeThreads(), 0);

    // 提交一个任务，应有一个活动线程
    auto f1 = delegator.submit([]{
        thread::sleep(milliseconds(100));
        return 1;
    });

    // 短暂等待让任务开始执行
    thread::sleep(milliseconds(10));

    EXPECT_EQ(delegator.activeThreads(), 1);
    EXPECT_EQ(delegator.idleThreads(), 1);

    // 提交第二个任务
    auto f2 = delegator.submit([]{
        thread::sleep(milliseconds(100));
        return 2;
    });

    thread::sleep(milliseconds(10));

    EXPECT_EQ(delegator.activeThreads(), 2);
    EXPECT_EQ(delegator.idleThreads(), 0);

    // 等待任务完成
    f1.result();
    f2.result();

    delegator.stop();

    // 短暂等待让工作线程退出
    thread::sleep(milliseconds(10));

    // 任务完成后，线程应回到空闲状态
    EXPECT_EQ(delegator.activeThreads(), 0);
    EXPECT_EQ(delegator.idleThreads(), 0);
}

// 测试逐个运行延迟任务
TEST(TaskDelegatorTest, RunDeferredOneByOne) {
    taskDelegator delegator(2);

    std::atomic<int> counter{0};

    // 提交多个延迟任务
    for (int i = 0; i < 3; ++i) {
        delegator.submit(taskDelegator::DEFERRED, [&counter]{
            ++counter;
            return counter.load();
        });
    }

    // 初始时延迟任务不应执行
    EXPECT_EQ(counter.load(), 0);

    // 逐个运行延迟任务
    for (int i = 0; i < 3; ++i) {
        delegator.runDeferred();
        // 等待任务执行
        thread::sleep(milliseconds(50));
    }

    // 所有延迟任务应已执行
    EXPECT_EQ(counter.load(), 3);
}

// 测试空任务提交
TEST(TaskDelegatorTest, SubmitEmptyTask) {
    taskDelegator delegator(1);

    // 提交不执行任何操作的任务
    auto f = delegator.submit([]{});

    // 应正常完成而不抛出异常
    EXPECT_NO_THROW(f.result());
}

// 测试异常传播
TEST(TaskDelegatorTest, ExceptionPropagation) {
    taskDelegator delegator(1);

    auto f = delegator.submit([]{
        throw std::runtime_error("Test exception");
    });

    EXPECT_THROW(f.result(), std::runtime_error);
}

// 测试停止后无法提交任何优先级任务
TEST(TaskDelegatorTest, StopPreventsAllPrioritySubmits) {
    taskDelegator delegator(1);
    delegator.stop();

    // 所有优先级都应抛出异常
    EXPECT_THROW({
        delegator.submit(taskDelegator::IMMEDIATE, []{});
    }, sysError);

    EXPECT_THROW({
        delegator.submit(taskDelegator::HIGH, []{});
    }, sysError);

    EXPECT_THROW({
        delegator.submit(taskDelegator::NORMAL, []{});
    }, sysError);

    EXPECT_THROW({
        delegator.submit(taskDelegator::LOW, []{});
    }, sysError);

    EXPECT_THROW({
        delegator.submit(taskDelegator::DEFERRED, []{});
    }, sysError);
}

// 测试延迟任务计数功能
TEST(TaskDelegatorTest, DeferredTaskCount) {
    taskDelegator delegator(2);

    // 初始时没有延迟任务
    EXPECT_EQ(delegator.deferredCnt(), 0);

    // 提交一些延迟任务
    constexpr int deferred_count = 5;
    for (int i = 0; i < deferred_count; ++i) {
        delegator.submit(taskDelegator::DEFERRED, [i]{ return i; });
    }

    // 验证延迟任务计数
    EXPECT_EQ(delegator.deferredCnt(), deferred_count);

    // 运行所有延迟任务
    delegator.runAllDeferred();

    // 延迟任务计数应归零
    EXPECT_EQ(delegator.deferredCnt(), 0);
}

// 测试混合优先级任务的延迟计数
TEST(TaskDelegatorTest, MixedPriorityDeferredCount) {
    taskDelegator delegator(2);

    // 提交不同优先级的任务
    delegator.submit(taskDelegator::NORMAL, []{ return 1; });
    delegator.submit(taskDelegator::HIGH, []{ return 2; });
    delegator.submit(taskDelegator::LOW, []{ return 3; });

    // 非延迟任务不应影响延迟计数
    EXPECT_EQ(delegator.deferredCnt(), 0);

    // 提交延迟任务
    constexpr int deferred_count = 3;
    for (int i = 0; i < deferred_count; ++i) {
        delegator.submit(taskDelegator::DEFERRED, [i]{ return i + 10; });
    }

    // 验证只有延迟任务被计数
    EXPECT_EQ(delegator.deferredCnt(), deferred_count);

    // 运行所有延迟任务
    delegator.runAllDeferred();

    // 延迟任务计数应归零
    EXPECT_EQ(delegator.deferredCnt(), 0);
}

// 测试runDeferred对计数的影响
TEST(TaskDelegatorTest, RunDeferredAffectsCount) {
    taskDelegator delegator(2);

    // 提交多个延迟任务
    constexpr int total_deferred = 4;
    for (int i = 0; i < total_deferred; ++i) {
        delegator.submit(taskDelegator::DEFERRED, [i]{ return i; });
    }

    EXPECT_EQ(delegator.deferredCnt(), total_deferred);

    // 逐个运行延迟任务并验证计数
    for (int i = 0; i < total_deferred; ++i) {
        delegator.runDeferred();
        thread::sleep(milliseconds(10)); // 短暂等待
        EXPECT_EQ(delegator.deferredCnt(), total_deferred - i - 1);
    }

    EXPECT_EQ(delegator.deferredCnt(), 0);
}

// 测试runAllDeferred对计数的影响
TEST(TaskDelegatorTest, RunAllDeferredAffectsCount) {
    taskDelegator delegator(2);

    // 提交多个延迟任务
    constexpr int deferred_count = 5;
    for (int i = 0; i < deferred_count; ++i) {
        delegator.submit(taskDelegator::DEFERRED, [i]{ return i; });
    }

    EXPECT_EQ(delegator.deferredCnt(), deferred_count);

    // 运行所有延迟任务
    delegator.runAllDeferred();

    // 延迟计数应立即归零
    EXPECT_EQ(delegator.deferredCnt(), 0);
}

// 测试空延迟任务队列的计数
TEST(TaskDelegatorTest, EmptyDeferredQueueCount) {
    taskDelegator delegator(2);

    // 空队列时应返回0
    EXPECT_EQ(delegator.deferredCnt(), 0);

    // 运行空延迟任务队列不应改变计数
    delegator.runDeferred();
    EXPECT_EQ(delegator.deferredCnt(), 0);

    delegator.runAllDeferred();
    EXPECT_EQ(delegator.deferredCnt(), 0);
}

// 测试停止模式：丢弃延迟任务
TEST(TaskDelegatorTest, StopModeDiscardDeferred) {
    taskDelegator delegator(2);

    std::atomic executed_count{0};

    // 提交一些延迟任务
    for (int i = 0; i < 3; ++i) {
        delegator.submit(taskDelegator::DEFERRED, [&executed_count, i]{
            ++executed_count;
            return i;
        });
    }

    EXPECT_EQ(delegator.deferredCnt(), 3);

    // 使用 DISCARD_DEFERRED 模式停止
    delegator.stop(taskDelegator::DISCARD_DEFERRED);

    // 延迟任务应该被丢弃，没有执行
    EXPECT_EQ(executed_count.load(), 0);
    EXPECT_EQ(delegator.deferredCnt(), 0);
}

// 测试停止模式：保持延迟任务
TEST(TaskDelegatorTest, StopModeKeepDeferred) {
    taskDelegator delegator(2);

    auto executed_count = std::make_shared<std::atomic<int>>(0);

    // 提交一些延迟任务
    for (int i = 0; i < 3; ++i) {
        delegator.submit(taskDelegator::DEFERRED, [executed_count, i]{
            ++*executed_count;
            return i;
        });
    }

    EXPECT_EQ(delegator.deferredCnt(), 3);

    // 使用 KEEP_DEFERRED 模式停止
    delegator.stop(taskDelegator::KEEP_DEFERRED);

    // 延迟任务应该保持，没有执行
    EXPECT_EQ(executed_count->load(), 0);
    EXPECT_EQ(delegator.deferredCnt(), 3);
}

// 测试停止模式：运行延迟任务
TEST(TaskDelegatorTest, StopModeRunDeferred) {
    taskDelegator delegator(2);

    std::atomic executed_count{0};
    vector<async::future<int>> futures;

    // 提交一些延迟任务
    for (int i = 0; i < 3; ++i) {
        futures.pushEnd(delegator.submit(taskDelegator::DEFERRED,
            [&executed_count, i]{
                ++executed_count;
                return i;
        }));
    }

    EXPECT_EQ(delegator.deferredCnt(), 3);

    // 使用 RUN_DEFERRED 模式停止
    delegator.stop(taskDelegator::RUN_DEFERRED);

    // 阻塞主线程，等待工作线程完成任务
    for (auto& future : futures) {
        future.result();
    }

    // 延迟任务应该全部执行
    EXPECT_EQ(executed_count.load(), 3);
    EXPECT_EQ(delegator.deferredCnt(), 0);
}

// 测试析构函数自动运行延迟任务
TEST(TaskDelegatorTest, DestructorRunsDeferredTasks) {
    std::atomic executed_count{0};

    {
        taskDelegator delegator(2);

        // 提交一些延迟任务
        for (int i = 0; i < 3; ++i) {
            delegator.submit(taskDelegator::DEFERRED, [&executed_count, i]{
                ++executed_count;
                return i;
            });
        }

        EXPECT_EQ(delegator.deferredCnt(), 3);
        // 不手动调用stop，让析构函数处理（默认使用RUN_DEFERRED模式）
    }

    // 析构函数应该运行所有延迟任务
    EXPECT_EQ(executed_count.load(), 3);
}

// 测试未知停止模式抛出异常
TEST(TaskDelegatorTest, UnknownStopModeThrows) {
    taskDelegator delegator(2);

    // 使用无效的停止模式值
    constexpr auto invalid_stop_mode = static_cast<taskDelegator::stopMode>(999);

    EXPECT_THROW({
        delegator.stop(invalid_stop_mode);
    }, sysError);
}

// 测试混合停止模式场景
TEST(TaskDelegatorTest, MixedStopModeScenarios) {
    // 测试1: 空延迟任务队列的各种停止模式
    {
        taskDelegator delegator(2);

        // 提交一些正常任务
        auto f1 = delegator.submit([]{ return 1; });
        auto f2 = delegator.submit([]{ return 2; });

        EXPECT_EQ(f1.result(), 1);
        EXPECT_EQ(f2.result(), 2);

        // 各种停止模式都应该正常工作（没有延迟任务）
        EXPECT_NO_THROW(delegator.stop(taskDelegator::DISCARD_DEFERRED));
    }

    // 测试2: 混合优先级任务的停止
    {
        taskDelegator delegator(2);

        std::atomic normal_executed{0};
        std::atomic deferred_executed{0};

        vector<async::future<int>> futures;

        // 提交混合优先级任务
        futures.pushEnd(delegator.submit(taskDelegator::NORMAL,
            [&normal_executed]{
                ++normal_executed;
                return 1;
        }));

        for (int i = 0; i < 2; ++i) {
            futures.pushEnd(delegator.submit(taskDelegator::DEFERRED,
                [&deferred_executed, i]{
                    ++deferred_executed;
                    return i;
            }));
        }

        // 使用RUN_DEFERRED模式停止
        delegator.stop(taskDelegator::RUN_DEFERRED);

        // 阻塞主线程，等待工作线程完成任务
        for (auto& future : futures) {
            future.result();
        }

        EXPECT_EQ(normal_executed.load(), 1);
        EXPECT_EQ(deferred_executed.load(), 2);
    }
}

// 测试停止后再次停止的行为
TEST(TaskDelegatorTest, StopAfterStop) {
    taskDelegator delegator(2);

    // 第一次停止
    delegator.stop(taskDelegator::KEEP_DEFERRED);

    // 再次停止应该不会抛出异常
    EXPECT_NO_THROW(delegator.stop(taskDelegator::DISCARD_DEFERRED));
    EXPECT_NO_THROW(delegator.stop(taskDelegator::RUN_DEFERRED));
}

// 测试停止模式默认参数
TEST(TaskDelegatorTest, StopModeDefaultParameter) {
    taskDelegator delegator(2);

    auto executed_count = std::make_shared<std::atomic<int>>(0);

    // 提交一些延迟任务
    for (int i = 0; i < 2; ++i) {
        delegator.submit(taskDelegator::DEFERRED, [executed_count, i]{
            ++*executed_count;
            return i;
        });
    }

    // 使用默认参数停止（KEEP_DEFERRED）
    delegator.stop();

    // 延迟任务应该保持，没有执行
    EXPECT_EQ(executed_count->load(), 0);
    EXPECT_EQ(delegator.deferredCnt(), 2);

    // 线程池自动析构应无崩溃触发或异常抛出
}

// 测试等待任务计数
TEST(TaskDelegatorTest, WaitingTaskCount) {
    taskDelegator delegator(2);

    // 初始时没有等待任务
    EXPECT_EQ(delegator.waitingCnt(), 0);
    std::vector<async::future<int>> futures;

    // 提交一些正常优先级任务
    for (int i = 0; i < 3; ++i) {
        futures.emplace_back(delegator.submit(taskDelegator::NORMAL, []{
            thread::sleep(milliseconds(100));
            return 1;
        }));
    }

    // 等待任务完成
    for (auto& future : futures) {
        future.wait();
    }
    EXPECT_EQ(delegator.waitingCnt(), 0);
}

// 测试立即任务计数
TEST(TaskDelegatorTest, ImmediateTaskCount) {
    taskDelegator delegator(2);

    // 初始时没有立即任务
    EXPECT_EQ(delegator.immediateCnt(), 0);

    // 提交一个立即任务
    try {
        delegator.submit(taskDelegator::IMMEDIATE, []{ return 1; });
        // 应该有1个立即任务
        EXPECT_EQ(delegator.immediateCnt(), 1);
    } catch (const sysError&) {
        // 如果没有空闲线程，立即任务提交会失败
        // 这是正常情况，跳过测试
        SUCCEED();
    }
}

// 测试丢弃单个延迟任务
TEST(TaskDelegatorTest, DiscardSingleDeferredTask) {
    taskDelegator delegator(2);

    // 提交3个延迟任务
    for (int i = 0; i < 3; ++i) {
        delegator.submit(taskDelegator::DEFERRED, [i]{ return i; });
    }

    EXPECT_EQ(delegator.deferredCnt(), 3);

    // 丢弃一个延迟任务
    EXPECT_EQ(delegator.discardDeferred(), 2);

    // 再丢弃一个
    EXPECT_EQ(delegator.discardDeferred(), 1);

    // 丢弃最后一个
    EXPECT_EQ(delegator.discardDeferred(), 0);
}

// 测试丢弃所有延迟任务
TEST(TaskDelegatorTest, DiscardAllDeferredTasks) {
    taskDelegator delegator(2);

    std::atomic executed_count{0};

    // 提交多个延迟任务
    for (int i = 0; i < 5; ++i) {
        delegator.submit(taskDelegator::DEFERRED, [&executed_count, i]{
            ++executed_count;
            return i;
        });
    }

    EXPECT_EQ(delegator.deferredCnt(), 5);

    // 丢弃所有延迟任务
    delegator.discardAllDeferred();
    EXPECT_EQ(delegator.deferredCnt(), 0);
    EXPECT_EQ(executed_count.load(), 0);
}

// 测试带超时的提交接口 - 成功情况
TEST(TaskDelegatorTest, SubmitWithTimeoutSuccess) {
    taskDelegator delegator(2);

    // 短暂阻塞主线程，等待线程池的工作线程准备好
    thread::sleep(milliseconds(10));

    // 确保有空闲线程
    EXPECT_GT(delegator.idleThreads(), 0);

    // 提交带超时的任务
    auto future = delegator.submit(milliseconds(100), []{
        return 42;
    });

    // 应该成功完成
    EXPECT_EQ(future.result(), 42);
}

// 测试带超时的提交接口 - 超时情况
TEST(TaskDelegatorTest, SubmitWithTimeoutFailure) {
    taskDelegator delegator(1);

    // 提交一个长时间运行的任务占用线程
    auto long_task = delegator.submit([]{
        thread::sleep(seconds(1));
        return 100;
    });

    // 短暂等待确保线程被占用
    thread::sleep(milliseconds(10));

    // 尝试提交带超时的任务，应该超时
    EXPECT_THROW({
        delegator.submit(milliseconds(50), []{
            return 42;
        });
    }, sysError);
}

// 测试带超时的提交接口 - 停止状态
TEST(TaskDelegatorTest, SubmitWithTimeoutWhenStopped) {
    taskDelegator delegator(2);
    delegator.stop();

    // 在停止状态下提交带超时的任务应该立即抛出异常
    EXPECT_THROW({
        delegator.submit(milliseconds(100), []{
            return 42;
        });
    }, sysError);
}

// 高并发下的综合压力测试
TEST(TaskDelegatorTest, StressTestMixedTasks) {
    constexpr int thread_count = 8;
    constexpr int normal_tasks = 50;
    constexpr int high_tasks = 30;
    constexpr int low_tasks = 15;
    constexpr int deferred_tasks = 25;

    bool immediate_task_submitted = false;

    taskDelegator delegator(thread_count);

    std::atomic normal_sum{0};
    std::atomic high_sum{0};
    std::atomic low_sum{0};
    std::atomic deferred_sum{0};
    std::atomic immediate_sum{0};

    auto low_func = [&low_sum](const int val){
        thread::sleep(milliseconds(10));
        low_sum += val;
        return val;
    };

    auto normal_func = [&normal_sum](const int val){
        thread::sleep(milliseconds(10));
        normal_sum += val;
        return val;
    };

    auto high_func = [&high_sum](const int val){
        thread::sleep(milliseconds(10));
        high_sum += val;
        return val;
    };

    auto deferred_func = [&deferred_sum](const int val) {
        thread::sleep(milliseconds(10));
        deferred_sum += val;
        return val;
    };

    auto immediate_func = [&immediate_sum](const int val) {
        thread::sleep(milliseconds(10));
        immediate_sum += val;
        return val;
    };

    std::vector<async::future<int>> futures;

    // 提交 IMMEDIATE 任务
    try {
        constexpr int immediate_task = 1;
        futures.push_back(delegator.submit(taskDelegator::IMMEDIATE, immediate_func, immediate_task));
        immediate_task_submitted = true;
    }
    catch (const sysError&) {
        immediate_task_submitted = false;
    }

    // 提交 LOW 任务
    for (int j = 1; j <= low_tasks; ++j) {
        futures.push_back(delegator.submit(taskDelegator::LOW, low_func, j));
    }

    // 提交 NORMAL 任务
    for (int j = 1; j <= normal_tasks; ++j) {
        futures.push_back(delegator.submit(taskDelegator::NORMAL, normal_func, j));
    }

    // 提交 HIGH 任务
    for (int j = 1; j <= high_tasks; ++j) {
        futures.push_back(delegator.submit(taskDelegator::HIGH, high_func, j));
    }

    // 提交 DEFERRED 任务
    for (int j = 1; j <= deferred_tasks; ++j) {
        futures.push_back(delegator.submit(taskDelegator::DEFERRED, deferred_func, j));
    }

    delegator.runAllDeferred();

    // 阻塞主线程，等待工作线程完成任务
    for (auto &fut : futures) {
        fut.result();
    }

    // 验证计数和求和结果
    constexpr int expected_normal_sum = normal_tasks * (normal_tasks + 1) / 2;
    constexpr int expected_high_sum = high_tasks * (high_tasks + 1) / 2;
    constexpr int expected_low_sum = low_tasks * (low_tasks + 1) / 2;
    constexpr int expected_deferred_sum = deferred_tasks * (deferred_tasks + 1) / 2;
    constexpr int expected_immediate_sum = 1;

    EXPECT_EQ(normal_sum.load(), expected_normal_sum);
    EXPECT_EQ(high_sum.load(), expected_high_sum);
    EXPECT_EQ(low_sum.load(), expected_low_sum);
    EXPECT_EQ(deferred_sum.load(), expected_deferred_sum);
    EXPECT_EQ(immediate_sum.load(), immediate_task_submitted ? expected_immediate_sum : 0);
}
