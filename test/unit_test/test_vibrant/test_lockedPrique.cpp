#include <gtest/gtest.h>
#include <algorithm>
#include "thread.h"
#include "lockedPrique.h"

using namespace original;

class LockedPriqueTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

// 测试默认构造和基本属性
TEST_F(LockedPriqueTest, DefaultConstruction) {
    lockedPrique<int> prique;
    
    EXPECT_TRUE(prique.empty());
    EXPECT_EQ(prique.size(), 0);
    EXPECT_FALSE(prique.top().hasValue());
    EXPECT_FALSE(prique.tryPop().hasValue());
}

// 测试 push 和 size
TEST_F(LockedPriqueTest, PushAndSize) {
    lockedPrique<int> prique;
    
    prique.push(1);
    EXPECT_FALSE(prique.empty());
    EXPECT_EQ(prique.size(), 1);
    
    prique.push(2);
    EXPECT_EQ(prique.size(), 2);
    
    prique.push(3);
    EXPECT_EQ(prique.size(), 3);
}

// 测试优先级顺序 - 默认小顶堆
TEST_F(LockedPriqueTest, PriorityOrderMinHeap) {
    lockedPrique<int> prique; // 默认使用 increaseComparator，小顶堆
    
    prique.push(30);
    prique.push(10);
    prique.push(20);
    
    // 应该按照升序弹出
    EXPECT_EQ(prique.pop(), 10);
    EXPECT_EQ(prique.pop(), 20);
    EXPECT_EQ(prique.pop(), 30);
    EXPECT_TRUE(prique.empty());
}

// 测试大顶堆优先级顺序
TEST_F(LockedPriqueTest, PriorityOrderMaxHeap) {
    lockedPrique<int, decreaseComparator> prique; // 使用 decreaseComparator，大顶堆
    
    prique.push(10);
    prique.push(30);
    prique.push(20);
    
    // 应该按照降序弹出
    EXPECT_EQ(prique.pop(), 30);
    EXPECT_EQ(prique.pop(), 20);
    EXPECT_EQ(prique.pop(), 10);
    EXPECT_TRUE(prique.empty());
}

// 测试 pop 基本功能
TEST_F(LockedPriqueTest, PopBasic) {
    lockedPrique<int> prique;
    
    prique.push(42);
    prique.push(100);

    const int value1 = prique.pop();
    EXPECT_EQ(value1, 42); // 小顶堆，最小值先出
    EXPECT_EQ(prique.size(), 1);

    const int value2 = prique.pop();
    EXPECT_EQ(value2, 100);
    EXPECT_TRUE(prique.empty());
}

// 测试 tryPop
TEST_F(LockedPriqueTest, TryPop) {
    lockedPrique<int> prique;
    
    // 空队列时 tryPop 应该返回空 alternative
    const auto result1 = prique.tryPop();
    EXPECT_FALSE(result1.hasValue());
    
    prique.push(99);
    
    auto result2 = prique.tryPop();
    EXPECT_TRUE(result2.hasValue());
    EXPECT_EQ(*result2, 99);
    EXPECT_TRUE(prique.empty());
}

// 测试 top 方法
TEST_F(LockedPriqueTest, Top) {
    lockedPrique<int> prique;
    
    // 空队列的 top
    auto top1 = prique.top();
    EXPECT_FALSE(top1.hasValue());
    
    prique.push(50);
    prique.push(30);
    prique.push(70);
    
    auto top2 = prique.top();
    EXPECT_TRUE(top2.hasValue());
    EXPECT_EQ(*top2, 30); // 小顶堆，top 应该是最小值
    
    // top 不应该影响队列大小
    EXPECT_EQ(prique.size(), 3);
    
    // 再次调用 top 应该返回相同值
    auto top3 = prique.top();
    EXPECT_TRUE(top3.hasValue());
    EXPECT_EQ(*top3, 30);
}

// 测试 popFor 超时
TEST_F(LockedPriqueTest, PopForTimeout) {
    lockedPrique<int> prique;

    const auto start = std::chrono::steady_clock::now();
    const auto result = prique.popFor(milliseconds(100));
    const auto end = std::chrono::steady_clock::now();
    
    EXPECT_FALSE(result.hasValue());
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_GE(duration.count(), 98);
}

// 测试 popFor 成功获取元素
TEST_F(LockedPriqueTest, PopForSuccess) {
    lockedPrique<int> prique;
    
    // 在另一个线程中延迟添加元素
    thread producer([&prique] {
        thread::sleep(milliseconds(50));
        prique.push(123);
    });
    
    auto result = prique.popFor(milliseconds(200));
    
    EXPECT_TRUE(result.hasValue());
    EXPECT_EQ(*result, 123);
    
    producer.join();
}

// 测试 clear
TEST_F(LockedPriqueTest, Clear) {
    lockedPrique<int> prique;
    
    prique.push(1);
    prique.push(2);
    prique.push(3);
    
    EXPECT_EQ(prique.size(), 3);
    
    prique.clear();
    
    EXPECT_TRUE(prique.empty());
    EXPECT_EQ(prique.size(), 0);
    EXPECT_FALSE(prique.tryPop().hasValue());
    EXPECT_FALSE(prique.top().hasValue());
}

// 测试多线程安全性
TEST_F(LockedPriqueTest, MultiThreadedPushPop) {
    lockedPrique<int> prique;
    constexpr int num_elements = 1000;
    std::atomic pop_count{0};
    std::atomic<int> max_value{0};
    
    // 生产者线程
    thread producer([&prique] {
        for (int i = 0; i < num_elements; ++i) {
            prique.push(i);
        }
    });
    
    // 消费者线程
    thread consumer([&prique, num_elements, &pop_count, &max_value] {
        for (int i = 0; i < num_elements; ++i) {
            int value = prique.pop();
            ++pop_count;
            // 验证值的范围
            EXPECT_GE(value, 0);
            EXPECT_LT(value, num_elements);
            if (value > max_value) {
                max_value = value;
            }
        }
    });
    
    producer.join();
    consumer.join();
    
    EXPECT_TRUE(prique.empty());
    EXPECT_EQ(pop_count.load(), num_elements);
    EXPECT_LT(max_value.load(), num_elements);
}

// 测试多个生产者和消费者
TEST_F(LockedPriqueTest, MultipleProducersConsumers) {
    lockedPrique<int> prique;
    constexpr int num_producers = 4;
    constexpr int num_consumers = 4;
    constexpr int elements_per_producer = 250;
    constexpr int total_elements = num_producers * elements_per_producer;
    std::atomic consumed_count{0};
    std::vector<int> consumed_values;
    std::mutex consumed_mutex;
    
    std::vector<thread> producers;
    std::vector<thread> consumers;
    
    // 创建生产者线程
    for (int i = 0; i < num_producers; ++i) {
        producers.emplace_back([&prique, i] {
            for (int j = 0; j < elements_per_producer; ++j) {
                prique.push(i * elements_per_producer + j);
            }
        });
    }
    
    // 创建消费者线程
    for (int i = 0; i < num_consumers; ++i) {
        consumers.emplace_back([&prique, total_elements, &consumed_count, &consumed_values, &consumed_mutex] {
            for (int j = 0; j < total_elements / num_consumers; ++j) {
                int value = prique.pop();
                ++consumed_count;
                EXPECT_GE(value, 0);
                EXPECT_LT(value, total_elements);
                
                std::lock_guard lock(consumed_mutex);
                consumed_values.push_back(value);
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
    
    EXPECT_TRUE(prique.empty());
    EXPECT_EQ(consumed_count.load(), total_elements);
    EXPECT_EQ(consumed_values.size(), total_elements);
    
    // 验证所有值都被正确处理（可选）
    std::ranges::sort(consumed_values);
    for (int i = 0; i < total_elements; ++i) {
        EXPECT_EQ(consumed_values[i], i);
    }
}

// 测试复杂类型和自定义比较器
struct Task {
    int priority;
    std::string name;
    
    bool operator<(const Task& other) const {
        return priority < other.priority;
    }
};

TEST_F(LockedPriqueTest, CustomTypeAndComparator) {
    lockedPrique<Task> prique;
    
    prique.push(Task{3, "Low priority"});
    prique.push(Task{1, "High priority"});
    prique.push(Task{2, "Medium priority"});
    
    // 应该按照优先级升序弹出
    auto task1 = prique.pop();
    EXPECT_EQ(task1.priority, 1);
    EXPECT_EQ(task1.name, "High priority");
    
    auto task2 = prique.pop();
    EXPECT_EQ(task2.priority, 2);
    EXPECT_EQ(task2.name, "Medium priority");
    
    auto task3 = prique.pop();
    EXPECT_EQ(task3.priority, 3);
    EXPECT_EQ(task3.name, "Low priority");
}

namespace
{
    template<typename PTR>
    struct ptrComparator {
        bool operator()(const PTR& lhs, const PTR& rhs) const {
            return *lhs < *rhs;
        }
    };
}

// 测试移动语义
TEST_F(LockedPriqueTest, MoveSemantics) {
    lockedPrique<strongPtr<int>, ptrComparator> p1;
    
    auto ptr1 = makeStrongPtr<int>(42);
    auto ptr2 = makeStrongPtr<int>(24);

    EXPECT_EQ(*ptr1, 42);
    EXPECT_EQ(*ptr2, 24);
    
    p1.push(std::move(ptr1));
    p1.push(std::move(ptr2));
    
    // ptr1 和 ptr2 现在应该为空
    EXPECT_EQ(ptr1, nullptr);
    EXPECT_EQ(ptr2, nullptr);

    // 应该按照值的大小顺序弹出
    const auto result1 = std::move(p1.pop());
    EXPECT_NE(result1, nullptr);
    EXPECT_EQ(*result1, 24); // 小顶堆，小值先出
    
    const auto result2 = std::move(p1.pop());
    EXPECT_NE(result2, nullptr);
    EXPECT_EQ(*result2, 42);

    lockedPrique<const int*, ptrComparator> p2;

    p2.push(result1.get());
    p2.push(result2.get());

    const auto result3 = p2.pop();
    EXPECT_NE(result3, nullptr);
    EXPECT_EQ(*result3, 24);

    const auto result4 = p2.pop();
    EXPECT_NE(result4, nullptr);
    EXPECT_EQ(*result4, 42);
}

// 测试异常安全性
TEST_F(LockedPriqueTest, ExceptionSafety) {
    lockedPrique<int> prique;
    
    // 正常操作不应该抛出异常
    EXPECT_NO_THROW(prique.push(1));
    EXPECT_NO_THROW(prique.pop());
    EXPECT_NO_THROW(prique.top());
    EXPECT_NO_THROW(prique.tryPop());

    // 测试超时操作不会抛出异常
    thread consumer([&prique] {
        EXPECT_NO_THROW({
            auto result = prique.popFor(milliseconds(10));
        });
    });
    
    consumer.join();
}

// 测试优先级队列的特性：相同优先级的元素
TEST_F(LockedPriqueTest, SamePriorityElements) {
    lockedPrique<int> prique;
    
    // 添加多个相同值的元素
    for (int i = 0; i < 5; ++i) {
        prique.push(10);
    }
    
    EXPECT_EQ(prique.size(), 5);
    
    // 所有弹出的值都应该是10
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(prique.pop(), 10);
    }
    
    EXPECT_TRUE(prique.empty());
}