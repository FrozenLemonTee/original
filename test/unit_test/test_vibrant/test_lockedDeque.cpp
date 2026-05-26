#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <vector>
#include "thread.h"
#include "lockedDeque.h"

using namespace original;

class LockedDequeTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

TEST_F(LockedDequeTest, DefaultConstruction) {
    lockedDeque<int> deque;

    EXPECT_TRUE(deque.empty());
    EXPECT_EQ(deque.size(), 0);
    EXPECT_FALSE(deque.head().hasValue());
    EXPECT_FALSE(deque.tail().hasValue());
    EXPECT_FALSE(deque.tryPopBegin().hasValue());
    EXPECT_FALSE(deque.tryPopEnd().hasValue());
}

TEST_F(LockedDequeTest, PushBeginAndSize) {
    lockedDeque<int> deque;

    deque.pushBegin(1);
    EXPECT_FALSE(deque.empty());
    EXPECT_EQ(deque.size(), 1);

    deque.pushBegin(2);
    EXPECT_EQ(deque.size(), 2);

    deque.pushBegin(3);
    EXPECT_EQ(deque.size(), 3);
}

TEST_F(LockedDequeTest, PushEndAndSize) {
    lockedDeque<int> deque;

    deque.pushEnd(1);
    EXPECT_FALSE(deque.empty());
    EXPECT_EQ(deque.size(), 1);

    deque.pushEnd(2);
    EXPECT_EQ(deque.size(), 2);

    deque.pushEnd(3);
    EXPECT_EQ(deque.size(), 3);
}

TEST_F(LockedDequeTest, PopBeginBasic) {
    lockedDeque<int> deque;

    deque.pushEnd(42);
    deque.pushEnd(100);

    const int value1 = deque.popBegin();
    EXPECT_EQ(value1, 42);
    EXPECT_EQ(deque.size(), 1);

    const int value2 = deque.popBegin();
    EXPECT_EQ(value2, 100);
    EXPECT_TRUE(deque.empty());
}

TEST_F(LockedDequeTest, PopEndBasic) {
    lockedDeque<int> deque;

    deque.pushEnd(42);
    deque.pushEnd(100);

    const int value1 = deque.popEnd();
    EXPECT_EQ(value1, 100);
    EXPECT_EQ(deque.size(), 1);

    const int value2 = deque.popEnd();
    EXPECT_EQ(value2, 42);
    EXPECT_TRUE(deque.empty());
}

TEST_F(LockedDequeTest, MixedEndOrder) {
    lockedDeque<int> deque;

    deque.pushEnd(1);
    deque.pushEnd(2);
    deque.pushBegin(0);
    deque.pushEnd(3);

    EXPECT_EQ(deque.popBegin(), 0);
    EXPECT_EQ(deque.popEnd(), 3);
    EXPECT_EQ(deque.popBegin(), 1);
    EXPECT_EQ(deque.popEnd(), 2);
    EXPECT_TRUE(deque.empty());
}

TEST_F(LockedDequeTest, TryPopBegin) {
    lockedDeque<int> deque;

    const auto result1 = deque.tryPopBegin();
    EXPECT_FALSE(result1.hasValue());

    deque.pushEnd(99);

    auto result2 = deque.tryPopBegin();
    EXPECT_TRUE(result2.hasValue());
    EXPECT_EQ(*result2, 99);
    EXPECT_TRUE(deque.empty());
}

TEST_F(LockedDequeTest, TryPopEnd) {
    lockedDeque<int> deque;

    const auto result1 = deque.tryPopEnd();
    EXPECT_FALSE(result1.hasValue());

    deque.pushEnd(99);

    auto result2 = deque.tryPopEnd();
    EXPECT_TRUE(result2.hasValue());
    EXPECT_EQ(*result2, 99);
    EXPECT_TRUE(deque.empty());
}

TEST_F(LockedDequeTest, HeadAndTail) {
    lockedDeque<int> deque;

    deque.pushEnd(10);
    deque.pushEnd(20);
    deque.pushEnd(30);

    auto head = deque.head();
    auto tail = deque.tail();

    EXPECT_TRUE(head.hasValue());
    EXPECT_TRUE(tail.hasValue());
    EXPECT_EQ(*head, 10);
    EXPECT_EQ(*tail, 30);
    EXPECT_EQ(deque.size(), 3);
}

TEST_F(LockedDequeTest, PopBeginForTimeout) {
    lockedDeque<int> deque;

    const auto start = std::chrono::steady_clock::now();
    const auto result = deque.popBeginFor(milliseconds(100));
    const auto end = std::chrono::steady_clock::now();

    EXPECT_FALSE(result.hasValue());
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_GE(duration.count(), 98);
}

TEST_F(LockedDequeTest, PopEndForTimeout) {
    lockedDeque<int> deque;

    const auto start = std::chrono::steady_clock::now();
    const auto result = deque.popEndFor(milliseconds(100));
    const auto end = std::chrono::steady_clock::now();

    EXPECT_FALSE(result.hasValue());
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_GE(duration.count(), 98);
}

TEST_F(LockedDequeTest, PopBeginForSuccess) {
    lockedDeque<int> deque;

    thread producer([&deque] {
        thread::sleep(milliseconds(50));
        deque.pushEnd(123);
    });

    auto result = deque.popBeginFor(milliseconds(200));

    EXPECT_TRUE(result.hasValue());
    EXPECT_EQ(*result, 123);

    producer.join();
}

TEST_F(LockedDequeTest, PopEndForSuccess) {
    lockedDeque<int> deque;

    thread producer([&deque] {
        thread::sleep(milliseconds(50));
        deque.pushEnd(123);
    });

    auto result = deque.popEndFor(milliseconds(200));

    EXPECT_TRUE(result.hasValue());
    EXPECT_EQ(*result, 123);

    producer.join();
}

TEST_F(LockedDequeTest, Clear) {
    lockedDeque<int> deque;

    deque.pushEnd(1);
    deque.pushEnd(2);
    deque.pushEnd(3);

    EXPECT_EQ(deque.size(), 3);

    deque.clear();

    EXPECT_TRUE(deque.empty());
    EXPECT_EQ(deque.size(), 0);
    EXPECT_FALSE(deque.tryPopBegin().hasValue());
    EXPECT_FALSE(deque.tryPopEnd().hasValue());
    EXPECT_FALSE(deque.head().hasValue());
    EXPECT_FALSE(deque.tail().hasValue());
}

TEST_F(LockedDequeTest, WorkStealingAccessPattern) {
    lockedDeque<int> deque;

    deque.pushEnd(1);
    deque.pushEnd(2);
    deque.pushEnd(3);

    EXPECT_EQ(deque.popEnd(), 3);
    EXPECT_EQ(deque.popBegin(), 1);
    EXPECT_EQ(deque.popEnd(), 2);
    EXPECT_TRUE(deque.empty());
}

TEST_F(LockedDequeTest, MultiThreadedPushEndPopBegin) {
    lockedDeque<int> deque;
    constexpr int num_elements = 1000;
    std::atomic pop_count{0};

    thread producer([&deque] {
        for (int i = 0; i < num_elements; ++i) {
            deque.pushEnd(i);
        }
    });

    thread consumer([&deque, &pop_count, num_elements] {
        for (int i = 0; i < num_elements; ++i) {
            const int value = deque.popBegin();
            ++pop_count;
            EXPECT_GE(value, 0);
            EXPECT_LT(value, num_elements);
        }
    });

    producer.join();
    consumer.join();

    EXPECT_TRUE(deque.empty());
    EXPECT_EQ(pop_count.load(), num_elements);
}

TEST_F(LockedDequeTest, MultiThreadedPushEndPopEnd) {
    lockedDeque<int> deque;
    constexpr int num_elements = 1000;
    std::atomic pop_count{0};

    thread producer([&deque] {
        for (int i = 0; i < num_elements; ++i) {
            deque.pushEnd(i);
        }
    });

    thread consumer([&deque, &pop_count, num_elements] {
        for (int i = 0; i < num_elements; ++i) {
            const int value = deque.popEnd();
            ++pop_count;
            EXPECT_GE(value, 0);
            EXPECT_LT(value, num_elements);
        }
    });

    producer.join();
    consumer.join();

    EXPECT_TRUE(deque.empty());
    EXPECT_EQ(pop_count.load(), num_elements);
}

TEST_F(LockedDequeTest, MultipleProducersConsumers) {
    lockedDeque<int> deque;
    constexpr int num_producers = 4;
    constexpr int num_consumers = 4;
    constexpr int elements_per_producer = 250;
    constexpr int total_elements = num_producers * elements_per_producer;
    std::atomic consumed_count{0};

    std::vector<thread> producers;
    std::vector<thread> consumers;

    for (int i = 0; i < num_producers; ++i) {
        producers.emplace_back([&deque, i] {
            for (int j = 0; j < elements_per_producer; ++j) {
                deque.pushEnd(i * elements_per_producer + j);
            }
        });
    }

    for (int i = 0; i < num_consumers; ++i) {
        consumers.emplace_back([&deque, total_elements, &consumed_count] {
            for (int j = 0; j < total_elements / num_consumers; ++j) {
                const int value = deque.popBegin();
                ++consumed_count;
                EXPECT_GE(value, 0);
                EXPECT_LT(value, total_elements);
            }
        });
    }

    for (auto& producer : producers) {
        producer.join();
    }

    for (auto& consumer : consumers) {
        consumer.join();
    }

    EXPECT_TRUE(deque.empty());
    EXPECT_EQ(consumed_count.load(), total_elements);
}

TEST_F(LockedDequeTest, ExceptionSafety) {
    lockedDeque<int> deque;

    EXPECT_NO_THROW(deque.pushBegin(1));
    EXPECT_NO_THROW(deque.pushEnd(2));
    EXPECT_NO_THROW(deque.head());
    EXPECT_NO_THROW(deque.tail());
    EXPECT_NO_THROW(deque.popBegin());
    EXPECT_NO_THROW(deque.popEnd());

    thread consumer([&deque] {
        EXPECT_NO_THROW({
            auto result1 = deque.popBeginFor(milliseconds(10));
            auto result2 = deque.popEndFor(milliseconds(10));
        });
    });

    consumer.join();
}
