#include <gtest/gtest.h>
#include "thread.h"
#include "lockedQueue.h"

using namespace original;

class LockedQueueTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

// 测试默认构造和基本属性
TEST_F(LockedQueueTest, DefaultConstruction) {
    lockedQueue<int> queue;
    
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
    EXPECT_FALSE(queue.head().hasValue());
    EXPECT_FALSE(queue.tail().hasValue());
    EXPECT_FALSE(queue.tryPop().hasValue());
}

// 测试 push 和 size
TEST_F(LockedQueueTest, PushAndSize) {
    lockedQueue<int> queue;
    
    queue.push(1);
    EXPECT_FALSE(queue.empty());
    EXPECT_EQ(queue.size(), 1);
    
    queue.push(2);
    EXPECT_EQ(queue.size(), 2);
    
    queue.push(3);
    EXPECT_EQ(queue.size(), 3);
}

// 测试 pop 基本功能
TEST_F(LockedQueueTest, PopBasic) {
    lockedQueue<int> queue;
    
    queue.push(42);
    queue.push(100);

    const int value1 = queue.pop();
    EXPECT_EQ(value1, 42);
    EXPECT_EQ(queue.size(), 1);

    const int value2 = queue.pop();
    EXPECT_EQ(value2, 100);
    EXPECT_TRUE(queue.empty());
}

// 测试 tryPop
TEST_F(LockedQueueTest, TryPop) {
    lockedQueue<int> queue;
    
    // 空队列时 tryPop 应该返回空 alternative
    const auto result1 = queue.tryPop();
    EXPECT_FALSE(result1.hasValue());
    
    queue.push(99);
    
    auto result2 = queue.tryPop();
    EXPECT_TRUE(result2.hasValue());
    EXPECT_EQ(*result2, 99);
    EXPECT_TRUE(queue.empty());
}

// 测试 head 和 tail
TEST_F(LockedQueueTest, HeadAndTail) {
    lockedQueue<int> queue;
    
    queue.push(10);
    queue.push(20);
    queue.push(30);
    
    auto head = queue.head();
    auto tail = queue.tail();
    
    EXPECT_TRUE(head.hasValue());
    EXPECT_TRUE(tail.hasValue());
    EXPECT_EQ(*head, 10);
    EXPECT_EQ(*tail, 30);
    
    // head 和 tail 不应该影响队列大小
    EXPECT_EQ(queue.size(), 3);
}

// 测试 popFor 超时
TEST_F(LockedQueueTest, PopForTimeout) {
    lockedQueue<int> queue;

    const auto start = std::chrono::steady_clock::now();
    const auto result = queue.popFor(milliseconds(100));
    const auto end = std::chrono::steady_clock::now();
    
    EXPECT_FALSE(result.hasValue());
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_GE(duration.count(), 100);
}

// 测试 popFor 成功获取元素
TEST_F(LockedQueueTest, PopForSuccess) {
    lockedQueue<int> queue;
    
    // 在另一个线程中延迟添加元素
    thread producer([&queue] {
        thread::sleep(milliseconds(50));
        queue.push(123);
    });
    
    auto result = queue.popFor(milliseconds(200));
    
    EXPECT_TRUE(result.hasValue());
    EXPECT_EQ(*result, 123);
    
    producer.join();
}

// 测试 clear
TEST_F(LockedQueueTest, Clear) {
    lockedQueue<int> queue;
    
    queue.push(1);
    queue.push(2);
    queue.push(3);
    
    EXPECT_EQ(queue.size(), 3);
    
    queue.clear();
    
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
    EXPECT_FALSE(queue.tryPop().hasValue());
}

// 测试多线程安全性
TEST_F(LockedQueueTest, MultiThreadedPushPop) {
    lockedQueue<int> queue;
    constexpr int num_elements = 1000;
    std::atomic pop_count{0};
    
    // 生产者线程
    thread producer([&queue]
    {
        for (int i = 0; i < num_elements; ++i) {
            queue.push(i);
        }
    });
    
    // 消费者线程
    thread consumer([&queue, num_elements, &pop_count]
    {
        for (int i = 0; i < num_elements; ++i) {
            int value = queue.pop();
            ++pop_count;
            // 验证值的范围
            EXPECT_GE(value, 0);
            EXPECT_LT(value, num_elements);
        }
    });
    
    producer.join();
    consumer.join();
    
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(pop_count.load(), num_elements);
}

// 测试多个生产者和消费者
TEST_F(LockedQueueTest, MultipleProducersConsumers) {
    lockedQueue<int> queue;
    constexpr int num_producers = 4;
    constexpr int num_consumers = 4;
    constexpr int elements_per_producer = 250;
    constexpr int total_elements = num_producers * elements_per_producer;
    std::atomic consumed_count{0};
    
    std::vector<thread> producers;
    std::vector<thread> consumers;
    
    // 创建生产者线程
    for (int i = 0; i < num_producers; ++i) {
        producers.emplace_back([&queue, i]
        {
            for (int j = 0; j < elements_per_producer; ++j) {
                queue.push(i * elements_per_producer + j);
            }
        });
    }
    
    // 创建消费者线程
    for (int i = 0; i < num_consumers; ++i) {
        consumers.emplace_back([&queue, total_elements, &consumed_count]
        {
            for (int j = 0; j < total_elements / num_consumers; ++j) {
                int value = queue.pop();
                ++consumed_count;
                EXPECT_GE(value, 0);
                EXPECT_LT(value, total_elements);
            }
        });
    }
    
    // 等待所有生产者完成
    for (auto& producer : producers) {
        producer.join();
    }
    
    // 等待所有消费者完成
    for (auto& consumer : consumers) {
        consumer.join();
    }
    
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(consumed_count.load(), total_elements);
}

// 测试移动语义（如果 TYPE 支持）
TEST_F(LockedQueueTest, MoveSemantics) {
    lockedQueue<ownerPtr<int>> queue;
    
    auto ptr = makeOwnerPtr<int>(42);
    queue.push(std::move(ptr));
    
    // ptr 现在应该为空
    EXPECT_EQ(ptr, nullptr); // NOLINT: Null check

    const auto result = std::move(queue.pop());
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(*result, 42);
}

// 测试异常安全性（如果适用）
TEST_F(LockedQueueTest, ExceptionSafety) {
    lockedQueue<int> queue;
    
    // 正常操作不应该抛出异常
    EXPECT_NO_THROW(queue.push(1));
    EXPECT_NO_THROW(queue.pop());

    thread consumer([&queue] {
        EXPECT_NO_THROW({
            auto result = queue.popFor(milliseconds(10));
        });
    });
    
    consumer.join();
}