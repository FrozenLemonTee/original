#include <gtest/gtest.h>
#include "coroutines.h"
#include "executors.h"
#include "awaitable.h"
#include <thread>
#include <chrono>
#include <stdexcept>
#include <vector>
#include <atomic>

#include "singleton.h"

using namespace original;


// 用于测试的简单任务
coroutine::task<int> simpleIntTask(int value) {
    co_return value;
}

coroutine::task<std::string> simpleStringTask(const std::string& str) {
    co_return str;
}

coroutine::task<void> simpleVoidTask() {
    co_return;
}

// ===================== Task基础功能测试 =====================

class TaskBasicTest : public ::testing::Test {
protected:
    void SetUp() override {
        singleton<taskDelegator>::reset();
        delegator = &singleton<taskDelegator>::instance();
        singleton<threadPoolExecutor>::reset(*delegator);
        executor = &singleton<threadPoolExecutor>::instance();
    }

    taskDelegator* delegator = nullptr;
    threadPoolExecutor* executor = nullptr;
};

TEST_F(TaskBasicTest, IntTaskCreationAndResult) {
    auto t = coroutine::makeTask(*executor, []() -> int {
        return 42;
    });
    
    ASSERT_TRUE(t.hasExecutor());
    ASSERT_FALSE(t.started());
    ASSERT_FALSE(t.finished());
    
    t.start();
    ASSERT_TRUE(t.started());
    
    // 等待任务完成
    while (!t.finished()) {
        std::this_thread::yield();
    }
    
    EXPECT_EQ(t.result(), 42);
}

TEST_F(TaskBasicTest, StringTaskCreationAndResult) {
    auto t = coroutine::makeTask(*executor, []() -> std::string {
        return "Hello, World!";
    });
    
    t.via(*executor).start();
    
    while (!t.finished()) {
        std::this_thread::yield();
    }
    
    EXPECT_EQ(t.result(), "Hello, World!");
}

TEST_F(TaskBasicTest, VoidTaskCreationAndResult) {
    std::atomic executed{false};
    
    auto t = coroutine::makeTask(*executor, [&executed]
    {
        executed = true;
    });
    
    EXPECT_TRUE(t.via(*executor).start());
    
    while (!t.finished()) {
        std::this_thread::yield();
    }
    
    EXPECT_NO_THROW(t.result());
    EXPECT_TRUE(executed);
}

TEST_F(TaskBasicTest, TaskMoveSemantics) {
    auto t1 = coroutine::makeTask(*executor, []() -> int {
        return 100;
    });
    
    coroutine::task<int> t2 = std::move(t1);
    
    EXPECT_FALSE(t1); // t1 should be empty after move
    EXPECT_TRUE(t2);  // t2 should be valid
    
    t2.via(*executor).start();
    
    while (!t2.finished()) {
        std::this_thread::yield();
    }
    
    EXPECT_EQ(t2.result(), 100);
}

TEST_F(TaskBasicTest, TaskExceptionPropagation) {
    auto t = coroutine::makeTask(*executor, []() -> int {
        throw std::runtime_error("Test exception");
        return 0; // NOLINT: Exception test
    });
    
    EXPECT_TRUE(t.via(*executor).start());
    
    while (!t.finished()) {
        std::this_thread::yield();
    }
    
    EXPECT_THROW(t.result(), std::runtime_error);
}

// ===================== 运算符重载测试 =====================

TEST_F(TaskBasicTest, PipeOperatorSequence) {
    std::vector<int> executionOrder;
    
    auto t1 = coroutine::makeTask(*executor, [&executionOrder]() -> int {
        executionOrder.push_back(1);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return 100;
    });
    
    auto t2 = [&executionOrder](const int input) -> std::string {
        executionOrder.push_back(2);
        return std::to_string(input + 50);
    };
    
    auto combined = t1 >> t2; // 管道操作符
    
    combined.start();
    
    while (!combined.finished()) {
        std::this_thread::yield();
    }
    
    EXPECT_EQ(combined.result(), "150");
    EXPECT_EQ(executionOrder.size(), 2);
    EXPECT_EQ(executionOrder[0], 1);
    EXPECT_EQ(executionOrder[1], 2);
}

TEST_F(TaskBasicTest, OrOperatorSequence) {
    std::vector<int> executionOrder;
    
    auto t1 = coroutine::makeTask(*executor, [&executionOrder]() -> int {
        executionOrder.push_back(1);
        return 10;
    });
    
    auto t2 = coroutine::makeTask(*executor, [&executionOrder]() -> int {
        executionOrder.push_back(2);
        return 20;
    });
    
    auto combined = t1 | std::move(t2); // 或操作符
    
    EXPECT_TRUE(combined.start());
    
    while (!combined.finished()) {
        std::this_thread::yield();
    }
    
    EXPECT_EQ(combined.result(), 20);
    EXPECT_EQ(executionOrder.size(), 2);
    EXPECT_EQ(executionOrder[0], 1);
    EXPECT_EQ(executionOrder[1], 2);
}

// ===================== 流式延迟测试 =====================

TEST_F(TaskBasicTest, StreamDelayBasic) {
    
    auto t = coroutine::makeTask(*executor, [] -> int {
        return 1;
    });
    
    // 添加延迟
    auto delayed = t | coDelay(milliseconds(50));

    const time::point start = time::point::now();
    delayed.start();
    
    while (!delayed.finished()) {
        std::this_thread::yield();
    }

    const time::point end = time::point::now();
    const time::duration elapsed = end - start;
    EXPECT_GT(elapsed.value(time::MILLISECOND), 45); // 应该延迟了
}

TEST_F(TaskBasicTest, MultipleDelays) {
    auto t = coroutine::makeTask(*executor, []() -> int {
        return 1;
    });
    
    // 链式延迟
    auto delayed = t 
        | coDelay(milliseconds(50))
        | coDelay(milliseconds(50));

    const time::point start = time::point::now();
    delayed.start();
    
    while (!delayed.finished()) {
        std::this_thread::yield();
    }

    const time::point end = time::point::now();
    const time::duration elapsed = end - start;
    EXPECT_GT(elapsed.value(time::MILLISECOND), 95); // 总共应该延迟约100ms
}

// ===================== 流式错误捕获测试 =====================

TEST_F(TaskBasicTest, ErrorCatchBasic) {
    auto throwingTask = coroutine::makeTask(*executor, []() -> int {
        throw std::runtime_error("Error from task");
        return 0; // NOLINT: Exception test
    });
    
    bool catchHandlerCalled = false;
    std::string caughtMessage;
    
    auto handler = [&](const std::runtime_error& e) -> int {
        catchHandlerCalled = true;
        caughtMessage = e.what();
        return -1; // 错误时返回-1
    };
    
    auto withCatch = throwingTask | coCatch<std::runtime_error>(handler);
    
    withCatch.start();
    
    while (!withCatch.finished()) {
        std::this_thread::yield();
    }
    
    EXPECT_NO_THROW({
        const int result = withCatch.result();
        EXPECT_EQ(result, -1);
    });
    
    EXPECT_TRUE(catchHandlerCalled);
    EXPECT_EQ(caughtMessage, "Error from task");
}

TEST_F(TaskBasicTest, ErrorCatchSpecificException) {
    auto throwingTask = coroutine::makeTask(*executor, []() -> int {
        throw std::logic_error("Logic error");
        return 0; // NOLINT: Exception test
    });
    
    bool runtimeHandlerCalled = false;
    bool logicHandlerCalled = false;
    
    auto runtimeHandler = [&](const std::runtime_error&) -> int {
        runtimeHandlerCalled = true;
        return 1;
    };
    
    auto logicHandler = [&](const std::logic_error&) -> int {
        logicHandlerCalled = true;
        return 2;
    };
    
    // 应该被logicHandler捕获
    auto withCatch = throwingTask 
        | coCatch<std::runtime_error>(runtimeHandler)
        | coCatch<std::logic_error>(logicHandler);
    
    withCatch.start();
    
    while (!withCatch.finished()) {
        std::this_thread::yield();
    }
    
    EXPECT_NO_THROW({
        const int result = withCatch.result();
        EXPECT_EQ(result, 2);
    });
    
    EXPECT_FALSE(runtimeHandlerCalled);
    EXPECT_TRUE(logicHandlerCalled);
}

TEST_F(TaskBasicTest, ErrorCatchNotTriggeredOnSuccess) {
    auto successfulTask = coroutine::makeTask(*executor, []() -> int {
        return 42;
    });
    
    bool catchHandlerCalled = false;
    
    auto handler = [&](const std::runtime_error&) -> int {
        catchHandlerCalled = true;
        return -1;
    };
    
    auto withCatch = successfulTask | coCatch<std::runtime_error>(handler);
    
    withCatch.start();
    
    while (!withCatch.finished()) {
        std::this_thread::yield();
    }
    
    EXPECT_NO_THROW({
        const int result = withCatch.result();
        EXPECT_EQ(result, 42); // 应该返回原始结果
    });
    
    EXPECT_FALSE(catchHandlerCalled);
}

TEST_F(TaskBasicTest, ErrorCatchWithVoidTask) {
    auto throwingTask = coroutine::makeTask(*executor, []
    {
        throw std::runtime_error("Void task error");
    });
    
    bool catchHandlerCalled = false;
    
    auto handler = [&](const std::runtime_error&) {
        catchHandlerCalled = true;
    };

    const auto withCatch = throwingTask | coCatch<std::runtime_error>(handler);
    
    EXPECT_TRUE(withCatch.start());
    
    while (!withCatch.finished()) {
        std::this_thread::yield();
    }
    
    EXPECT_NO_THROW(withCatch.result());
    EXPECT_TRUE(catchHandlerCalled);
}

// ===================== 复杂组合测试 =====================

TEST_F(TaskBasicTest, ComplexPipelineWithDelayAndCatch) {
    std::vector<std::string> executionLog;
    
    auto task1 = coroutine::makeTask(*executor, [&executionLog]() -> std::string {
        executionLog.push_back("Task1 executed");
        return "Hello";
    });
    
    auto task2 = [&executionLog](const std::string& input) -> std::string {
        executionLog.push_back("Task2 executed with: " + input);
        return input + " World";
    };
    
    auto task3 = [&executionLog](const std::string& input) -> std::string {
        executionLog.push_back("Task3 executed with: " + input);
        if (input == "Hello World") {
            throw std::runtime_error("Intentional error");
        }
        return input + "!";
    };
    
    auto errorHandler = [&executionLog](const std::runtime_error& e) -> std::string {
        executionLog.push_back("Error handler called: " + std::string(e.what()));
        return "Recovered";
    };
    
    // 构建复杂管道：task1 -> task2 -> delay -> task3 -> catch
    auto pipeline = task1
        >> task2
        >> coDelay(milliseconds(10))
        >> task3
        | coCatch<std::runtime_error>(errorHandler);

    const time::point start = time::point::now();
    EXPECT_TRUE(pipeline.start());
    
    while (!pipeline.finished()) {
        std::this_thread::yield();
    }

    const std::string result = pipeline.result();
    
    // 验证结果
    EXPECT_EQ(result, "Recovered");
    
    // 验证执行顺序
    EXPECT_GE(executionLog.size(), 4);
    EXPECT_EQ(executionLog[0], "Task1 executed");
    EXPECT_EQ(executionLog[1], "Task2 executed with: Hello");
    EXPECT_EQ(executionLog[2], "Task3 executed with: Hello World");
    EXPECT_TRUE(executionLog[3].find("Error handler called") != std::string::npos);
    
    // 验证延迟
    time::duration elapsed = time::point::now() - start;
    EXPECT_GT(elapsed.value(time::MILLISECOND), 5); // 应该有一定的延迟
}

// ===================== whenAll 测试 =====================

TEST_F(TaskBasicTest, WhenAllBasic) {
    auto t1 = coroutine::makeTask(*executor, []() -> int {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        return 1;
    });

    auto t2 = coroutine::makeTask(*executor, []() -> std::string {
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        return "done";
    });

    auto t3 = coroutine::makeTask(*executor, []() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    });

    const auto results = coroutine::whenAll(std::move(t1), std::move(t2), std::move(t3));
    
    EXPECT_EQ(std::get<0>(results), 1);
    EXPECT_EQ(std::get<1>(results), "done");
    EXPECT_TRUE(std::get<2>(results));
}

TEST_F(TaskBasicTest, SpinWhenAll) {
    auto t1 = coroutine::makeTask(*executor, []() -> int {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        return 100;
    });
    
    auto t2 = coroutine::makeTask(*executor, []() -> int {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return 200;
    });

    const auto results = coroutine::spinWhenAll(std::move(t1), std::move(t2));
    
    EXPECT_EQ(std::get<0>(results), 100);
    EXPECT_EQ(std::get<1>(results), 200);
}

// ===================== 协程交互测试 =====================

TEST_F(TaskBasicTest, CoroutineCoAwait) {
    auto innerTask = coroutine::makeTask(*executor, []() -> int {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return 42;
    });
    
    auto outerTask = [](const int x){
        return x * 2;
    };

    auto chain = innerTask >> std::move(outerTask);
    
    EXPECT_TRUE(chain.via(*executor).start());
    
    while (!chain.finished()) {
        std::this_thread::yield();
    }
    
    EXPECT_EQ(chain.result(), 84);
}

// ===================== 边缘情况测试 =====================

TEST_F(TaskBasicTest, EmptyTaskOperations) {
    coroutine::task<int> emptyTask;
    
    EXPECT_TRUE(emptyTask.empty());
    EXPECT_FALSE(emptyTask);
    EXPECT_FALSE(emptyTask.hasExecutor());
    EXPECT_FALSE(emptyTask.started());
    EXPECT_FALSE(emptyTask.finished());
    
    EXPECT_THROW(emptyTask.result(), valueError);
    EXPECT_FALSE(emptyTask.start());
}

TEST_F(TaskBasicTest, TaskWithoutExecutor) {
    auto t = coroutine::makeTask([]() -> int {
        return 5;
    });
    
    EXPECT_FALSE(t.hasExecutor());
    EXPECT_FALSE(t.start());
    EXPECT_THROW(t | simpleIntTask(10), sysError); // 没有executor不能组合
}