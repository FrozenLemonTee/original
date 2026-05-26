#include <gtest/gtest.h>
#include "tasks.h"
#include "thread.h"

using namespace original;


int add_func_v2(const int a, const int b) {
    thread::sleep(milliseconds(200));
    return a + b;
}

int sub_func_v2(const int a, const int b) {
    thread::sleep(milliseconds(200));
    return a - b;
}

TEST(TaskDelegatorV2Test, SubmitNormalTasks) {
    taskDelegatorV2 delegator(4);

    auto f1 = delegator.submit(add_func_v2, 2, 3);
    auto f2 = delegator.submit(sub_func_v2, 10, 4);

    thread::sleep(milliseconds(10));

    EXPECT_EQ(f1.result(), 5);
    EXPECT_EQ(f2.result(), 6);
}

TEST(TaskDelegatorV2Test, SubmitHighPriority) {
    taskDelegatorV2 delegator(2);

    auto f1 = delegator.submit(taskDelegatorV2::NORMAL, add_func_v2, 1, 1);
    auto f2 = delegator.submit(taskDelegatorV2::HIGH, add_func_v2, 2, 2);

    EXPECT_EQ(f1.result(), 2);
    EXPECT_EQ(f2.result(), 4);
}

TEST(TaskDelegatorV2Test, StopPreventsNewSubmits) {
    taskDelegatorV2 delegator(2);

    auto f1 = delegator.submit(add_func_v2, 3, 4);
    EXPECT_EQ(f1.result(), 7);

    delegator.stop();

    EXPECT_THROW({
        delegator.submit(add_func_v2, 5, 6);
    }, sysError);
}


TEST(TaskDelegatorV2Test, SubmitImmediateWithoutIdleThreadThrows) {
    taskDelegatorV2 delegator{2};


    for (int i = 0; i < 4; ++i)
    {

        auto long_task = delegator.submit([]{
            thread::sleep(milliseconds(700));
            return 42;
        });
    }


    EXPECT_THROW({
        delegator.submit(taskDelegatorV2::IMMEDIATE, []{ return 0; });
    }, sysError);
}


TEST(TaskDelegatorV2Test, SubmitWithUnknownPriorityThrows) {
    taskDelegatorV2 delegator(2);


    constexpr auto invalid_priority = static_cast<taskDelegatorV2::priority>(999);

    EXPECT_THROW({
        delegator.submit(invalid_priority, []{ return 0; });
    }, sysError);
}


TEST(TaskDelegatorV2Test, DestructorAutoStops) {

    {
        taskDelegatorV2 delegator(2);
        auto f = delegator.submit([]{ return 1; });
        EXPECT_EQ(f.result(), 1);

    }

    SUCCEED();
}


TEST(TaskDelegatorV2Test, ActiveAndIdleThreadCounts) {
    taskDelegatorV2 delegator(2);


    EXPECT_EQ(delegator.activeThreads(), 0);


    auto f1 = delegator.submit([]{
        thread::sleep(milliseconds(100));
        return 1;
    });


    thread::sleep(milliseconds(10));

    EXPECT_EQ(delegator.activeThreads(), 1);
    EXPECT_EQ(delegator.idleThreads(), 1);


    auto f2 = delegator.submit([]{
        thread::sleep(milliseconds(100));
        return 2;
    });

    thread::sleep(milliseconds(10));

    EXPECT_EQ(delegator.activeThreads(), 2);
    EXPECT_EQ(delegator.idleThreads(), 0);


    f1.result();
    f2.result();

    delegator.stop();


    thread::sleep(milliseconds(10));


    EXPECT_EQ(delegator.activeThreads(), 0);
    EXPECT_EQ(delegator.idleThreads(), 0);
}


TEST(TaskDelegatorV2Test, RunDeferredOneByOne) {
    taskDelegatorV2 delegator(2);

    std::atomic<int> counter{0};


    for (int i = 0; i < 3; ++i) {
        delegator.submit(taskDelegatorV2::DEFERRED, [&counter]{
            ++counter;
            return counter.load();
        });
    }


    EXPECT_EQ(counter.load(), 0);


    for (int i = 0; i < 3; ++i) {
        delegator.runDeferred();

        thread::sleep(milliseconds(50));
    }


    EXPECT_EQ(counter.load(), 3);
}


TEST(TaskDelegatorV2Test, SubmitEmptyTask) {
    taskDelegatorV2 delegator(1);


    auto f = delegator.submit([]{});


    EXPECT_NO_THROW(f.result());
}


TEST(TaskDelegatorV2Test, ExceptionPropagation) {
    taskDelegatorV2 delegator(1);

    auto f = delegator.submit([]{
        throw std::runtime_error("Test exception");
    });

    EXPECT_THROW(f.result(), std::runtime_error);
}


TEST(TaskDelegatorV2Test, StopPreventsAllPrioritySubmits) {
    taskDelegatorV2 delegator(1);
    delegator.stop();


    EXPECT_THROW({
        delegator.submit(taskDelegatorV2::IMMEDIATE, []{});
    }, sysError);

    EXPECT_THROW({
        delegator.submit(taskDelegatorV2::HIGH, []{});
    }, sysError);

    EXPECT_THROW({
        delegator.submit(taskDelegatorV2::NORMAL, []{});
    }, sysError);

    EXPECT_THROW({
        delegator.submit(taskDelegatorV2::LOW, []{});
    }, sysError);

    EXPECT_THROW({
        delegator.submit(taskDelegatorV2::DEFERRED, []{});
    }, sysError);
}


TEST(TaskDelegatorV2Test, DeferredTaskCount) {
    taskDelegatorV2 delegator(2);


    EXPECT_EQ(delegator.deferredCnt(), 0);


    constexpr int deferred_count = 5;
    for (int i = 0; i < deferred_count; ++i) {
        delegator.submit(taskDelegatorV2::DEFERRED, [i]{ return i; });
    }


    EXPECT_EQ(delegator.deferredCnt(), deferred_count);


    delegator.runAllDeferred();


    EXPECT_EQ(delegator.deferredCnt(), 0);
}


TEST(TaskDelegatorV2Test, MixedPriorityDeferredCount) {
    taskDelegatorV2 delegator(2);


    delegator.submit(taskDelegatorV2::NORMAL, []{ return 1; });
    delegator.submit(taskDelegatorV2::HIGH, []{ return 2; });
    delegator.submit(taskDelegatorV2::LOW, []{ return 3; });


    EXPECT_EQ(delegator.deferredCnt(), 0);


    constexpr int deferred_count = 3;
    for (int i = 0; i < deferred_count; ++i) {
        delegator.submit(taskDelegatorV2::DEFERRED, [i]{ return i + 10; });
    }


    EXPECT_EQ(delegator.deferredCnt(), deferred_count);


    delegator.runAllDeferred();


    EXPECT_EQ(delegator.deferredCnt(), 0);
}


TEST(TaskDelegatorV2Test, RunDeferredAffectsCount) {
    taskDelegatorV2 delegator(2);


    constexpr int total_deferred = 4;
    for (int i = 0; i < total_deferred; ++i) {
        delegator.submit(taskDelegatorV2::DEFERRED, [i]{ return i; });
    }

    EXPECT_EQ(delegator.deferredCnt(), total_deferred);


    for (int i = 0; i < total_deferred; ++i) {
        delegator.runDeferred();
        thread::sleep(milliseconds(10));
        EXPECT_EQ(delegator.deferredCnt(), total_deferred - i - 1);
    }

    EXPECT_EQ(delegator.deferredCnt(), 0);
}


TEST(TaskDelegatorV2Test, RunAllDeferredAffectsCount) {
    taskDelegatorV2 delegator(2);


    constexpr int deferred_count = 5;
    for (int i = 0; i < deferred_count; ++i) {
        delegator.submit(taskDelegatorV2::DEFERRED, [i]{ return i; });
    }

    EXPECT_EQ(delegator.deferredCnt(), deferred_count);


    delegator.runAllDeferred();


    EXPECT_EQ(delegator.deferredCnt(), 0);
}


TEST(TaskDelegatorV2Test, EmptyDeferredQueueCount) {
    taskDelegatorV2 delegator(2);


    EXPECT_EQ(delegator.deferredCnt(), 0);


    delegator.runDeferred();
    EXPECT_EQ(delegator.deferredCnt(), 0);

    delegator.runAllDeferred();
    EXPECT_EQ(delegator.deferredCnt(), 0);
}


TEST(TaskDelegatorV2Test, StopModeDiscardDeferred) {
    taskDelegatorV2 delegator(2);

    std::atomic executed_count{0};


    for (int i = 0; i < 3; ++i) {
        delegator.submit(taskDelegatorV2::DEFERRED, [&executed_count, i]{
            ++executed_count;
            return i;
        });
    }

    EXPECT_EQ(delegator.deferredCnt(), 3);


    delegator.stop(taskDelegatorV2::DISCARD_DEFERRED);


    EXPECT_EQ(executed_count.load(), 0);
    EXPECT_EQ(delegator.deferredCnt(), 0);
}


TEST(TaskDelegatorV2Test, StopModeKeepDeferred) {
    taskDelegatorV2 delegator(2);

    auto executed_count = std::make_shared<std::atomic<int>>(0);


    for (int i = 0; i < 3; ++i) {
        delegator.submit(taskDelegatorV2::DEFERRED, [executed_count, i]{
            ++*executed_count;
            return i;
        });
    }

    EXPECT_EQ(delegator.deferredCnt(), 3);


    delegator.stop(taskDelegatorV2::KEEP_DEFERRED);


    EXPECT_EQ(executed_count->load(), 0);
    EXPECT_EQ(delegator.deferredCnt(), 3);
}


TEST(TaskDelegatorV2Test, StopModeRunDeferred) {
    taskDelegatorV2 delegator(2);

    std::atomic executed_count{0};
    vector<async::future<int>> futures;


    for (int i = 0; i < 3; ++i) {
        futures.pushEnd(delegator.submit(taskDelegatorV2::DEFERRED,
            [&executed_count, i]{
                ++executed_count;
                return i;
        }));
    }

    EXPECT_EQ(delegator.deferredCnt(), 3);


    delegator.stop(taskDelegatorV2::RUN_DEFERRED);


    for (auto& future : futures) {
        future.result();
    }


    EXPECT_EQ(executed_count.load(), 3);
    EXPECT_EQ(delegator.deferredCnt(), 0);
}


TEST(TaskDelegatorV2Test, DestructorRunsDeferredTasks) {
    std::atomic executed_count{0};

    {
        taskDelegatorV2 delegator(2);


        for (int i = 0; i < 3; ++i) {
            delegator.submit(taskDelegatorV2::DEFERRED, [&executed_count, i]{
                ++executed_count;
                return i;
            });
        }

        EXPECT_EQ(delegator.deferredCnt(), 3);

    }


    EXPECT_EQ(executed_count.load(), 3);
}


TEST(TaskDelegatorV2Test, UnknownStopModeThrows) {
    taskDelegatorV2 delegator(2);


    constexpr auto invalid_stop_mode = static_cast<taskDelegatorV2::stopMode>(999);

    EXPECT_THROW({
        delegator.stop(invalid_stop_mode);
    }, sysError);
}


TEST(TaskDelegatorV2Test, MixedStopModeScenarios) {

    {
        taskDelegatorV2 delegator(2);


        auto f1 = delegator.submit([]{ return 1; });
        auto f2 = delegator.submit([]{ return 2; });

        EXPECT_EQ(f1.result(), 1);
        EXPECT_EQ(f2.result(), 2);


        EXPECT_NO_THROW(delegator.stop(taskDelegatorV2::DISCARD_DEFERRED));
    }


    {
        taskDelegatorV2 delegator(2);

        std::atomic normal_executed{0};
        std::atomic deferred_executed{0};

        vector<async::future<int>> futures;


        futures.pushEnd(delegator.submit(taskDelegatorV2::NORMAL,
            [&normal_executed]{
                ++normal_executed;
                return 1;
        }));

        for (int i = 0; i < 2; ++i) {
            futures.pushEnd(delegator.submit(taskDelegatorV2::DEFERRED,
                [&deferred_executed, i]{
                    ++deferred_executed;
                    return i;
            }));
        }


        delegator.stop(taskDelegatorV2::RUN_DEFERRED);


        for (auto& future : futures) {
            future.result();
        }

        EXPECT_EQ(normal_executed.load(), 1);
        EXPECT_EQ(deferred_executed.load(), 2);
    }
}


TEST(TaskDelegatorV2Test, StopAfterStop) {
    taskDelegatorV2 delegator(2);


    delegator.stop(taskDelegatorV2::KEEP_DEFERRED);


    EXPECT_NO_THROW(delegator.stop(taskDelegatorV2::DISCARD_DEFERRED));
    EXPECT_NO_THROW(delegator.stop(taskDelegatorV2::RUN_DEFERRED));
}


TEST(TaskDelegatorV2Test, StopModeDefaultParameter) {
    taskDelegatorV2 delegator(2);

    auto executed_count = std::make_shared<std::atomic<int>>(0);


    for (int i = 0; i < 2; ++i) {
        delegator.submit(taskDelegatorV2::DEFERRED, [executed_count, i]{
            ++*executed_count;
            return i;
        });
    }


    delegator.stop();


    EXPECT_EQ(executed_count->load(), 0);
    EXPECT_EQ(delegator.deferredCnt(), 2);


}


TEST(TaskDelegatorV2Test, WaitingTaskCount) {
    taskDelegatorV2 delegator(2);


    EXPECT_EQ(delegator.waitingCnt(), 0);
    std::vector<async::future<int>> futures;


    for (int i = 0; i < 3; ++i) {
        futures.emplace_back(delegator.submit(taskDelegatorV2::NORMAL, []{
            thread::sleep(milliseconds(100));
            return 1;
        }));
    }


    for (auto& future : futures) {
        future.wait();
    }
    EXPECT_EQ(delegator.waitingCnt(), 0);
}


TEST(TaskDelegatorV2Test, ImmediateTaskCount) {
    taskDelegatorV2 delegator(2);


    EXPECT_EQ(delegator.immediateCnt(), 0);


    try {
        delegator.submit(taskDelegatorV2::IMMEDIATE, []{ return 1; });

        EXPECT_EQ(delegator.immediateCnt(), 1);
    } catch (const sysError&) {


        SUCCEED();
    }
}


TEST(TaskDelegatorV2Test, DiscardSingleDeferredTask) {
    taskDelegatorV2 delegator(2);


    for (int i = 0; i < 3; ++i) {
        delegator.submit(taskDelegatorV2::DEFERRED, [i]{ return i; });
    }

    EXPECT_EQ(delegator.deferredCnt(), 3);


    EXPECT_TRUE(delegator.discardDeferred());


    EXPECT_TRUE(delegator.discardDeferred());


    EXPECT_TRUE(delegator.discardDeferred());
}


TEST(TaskDelegatorV2Test, DiscardAllDeferredTasks) {
    taskDelegatorV2 delegator(2);

    std::atomic executed_count{0};


    for (int i = 0; i < 5; ++i) {
        delegator.submit(taskDelegatorV2::DEFERRED, [&executed_count, i]{
            ++executed_count;
            return i;
        });
    }

    EXPECT_EQ(delegator.deferredCnt(), 5);


    delegator.discardAllDeferred();
    EXPECT_EQ(delegator.deferredCnt(), 0);
    EXPECT_EQ(executed_count.load(), 0);
}


TEST(TaskDelegatorV2Test, SubmitWithTimeoutSuccess) {
    taskDelegatorV2 delegator(2);


    thread::sleep(milliseconds(10));


    EXPECT_GT(delegator.idleThreads(), 0);


    auto future = delegator.submit(milliseconds(100), []{
        return 42;
    });


    EXPECT_EQ(future.result(), 42);
}


TEST(TaskDelegatorV2Test, SubmitWithTimeoutFailure) {
    taskDelegatorV2 delegator(1);


    auto long_task = delegator.submit([]{
        thread::sleep(milliseconds(300));
        return 100;
    });


    thread::sleep(milliseconds(10));


    EXPECT_THROW({
        delegator.submit(milliseconds(25), []{
            return 42;
        });
    }, sysError);
}


TEST(TaskDelegatorV2Test, SubmitWithTimeoutWhenStopped) {
    taskDelegatorV2 delegator(2);
    delegator.stop();


    EXPECT_THROW({
        delegator.submit(milliseconds(100), []{
            return 42;
        });
    }, sysError);
}


TEST(TaskDelegatorV2Test, StressTestMixedTasks) {
    constexpr int thread_count = 8;
    constexpr int normal_tasks = 50;
    constexpr int high_tasks = 30;
    constexpr int low_tasks = 15;
    constexpr int deferred_tasks = 25;

    bool immediate_task_submitted = false;

    taskDelegatorV2 delegator(thread_count);

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


    try {
        constexpr int immediate_task = 1;
        futures.push_back(delegator.submit(taskDelegatorV2::IMMEDIATE, immediate_func, immediate_task));
        immediate_task_submitted = true;
    }
    catch (const sysError&) {
        immediate_task_submitted = false;
    }


    for (int j = 1; j <= low_tasks; ++j) {
        futures.push_back(delegator.submit(taskDelegatorV2::LOW, low_func, j));
    }


    for (int j = 1; j <= normal_tasks; ++j) {
        futures.push_back(delegator.submit(taskDelegatorV2::NORMAL, normal_func, j));
    }


    for (int j = 1; j <= high_tasks; ++j) {
        futures.push_back(delegator.submit(taskDelegatorV2::HIGH, high_func, j));
    }


    for (int j = 1; j <= deferred_tasks; ++j) {
        futures.push_back(delegator.submit(taskDelegatorV2::DEFERRED, deferred_func, j));
    }

    delegator.runAllDeferred();


    for (auto &fut : futures) {
        fut.result();
    }


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
