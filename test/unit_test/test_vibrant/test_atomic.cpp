#include "atomic.h"
#include <gtest/gtest.h>
#include <string>
#include <thread>

using namespace original;

// ========== 基础测试 ==========
TEST(AtomicTest, IsLockFreeTest) {
    auto a = makeAtomic(0);
    auto b = makeAtomic(std::string{});
    EXPECT_TRUE(a.isLockFree());
    EXPECT_FALSE(b.isLockFree());
}

TEST(AtomicTest, StoreAndLoadInt) {
    auto a = makeAtomic(0);
    EXPECT_EQ(a.load(), 0);

    a.store(42);
    EXPECT_EQ(a.load(), 42);
}

TEST(AtomicTest, FetchAddInt) {
    auto a = makeAtomic(10);
    const int old = a.fetchAdd(5);
    EXPECT_EQ(old, 10);
    EXPECT_EQ(a.load(), 15);
}

TEST(AtomicTest, FetchSubInt) {
    auto a = makeAtomic(10);
    const int old = a.fetchSub(3);
    EXPECT_EQ(old, 10);
    EXPECT_EQ(a.load(), 7);
}

TEST(AtomicTest, ExchangeInt) {
    auto a = makeAtomic(1);
    const int old = a.exchange(99);
    EXPECT_EQ(old, 1);
    EXPECT_EQ(a.load(), 99);
}

TEST(AtomicTest, StoreAndLoadBool) {
    auto a = makeAtomic(false);
    EXPECT_FALSE(a.load());

    a.store(true);
    EXPECT_TRUE(a.load());
}

// ========== CAS 测试 ==========
TEST(AtomicTest, CompareExchangeInt_Success) {
    auto a = makeAtomic(10);
    int expected = 10;
    constexpr int desired = 20;
    EXPECT_TRUE(a.exchangeCmp(expected, desired));
    EXPECT_EQ(a.load(), 20);
    EXPECT_EQ(expected, 10); // 成功时 expected 不会被改动
}

TEST(AtomicTest, CompareExchangeInt_Fail) {
    auto a = makeAtomic(10);
    int expected = 5; // 错误预期
    constexpr int desired = 20;
    EXPECT_FALSE(a.exchangeCmp(expected, desired));
    EXPECT_EQ(a.load(), 10);
    EXPECT_EQ(expected, 10); // 失败时 expected 被写回真实值
}

// ========== 非平凡类型测试（走锁模式） ==========
TEST(AtomicTest, StoreAndLoadString) {
    auto a = makeAtomic(std::string("hello"));
    EXPECT_EQ(a.load(), "hello");

    a.store(std::string("world"));
    EXPECT_EQ(a.load(), "world");
}

TEST(AtomicTest, ExchangeString) {
    auto a = makeAtomic(std::string("first"));
    const auto old = a.exchange(std::string("second"));
    EXPECT_EQ(old, "first");
    EXPECT_EQ(a.load(), "second");
}

TEST(AtomicTest, CompareExchangeString_Success) {
    auto a = makeAtomic(std::string("apple"));
    std::string expected = "apple";
    const std::string desired = "banana";
    EXPECT_TRUE(a.exchangeCmp(expected, desired));
    EXPECT_EQ(a.load(), "banana");
    EXPECT_EQ(expected, "apple");
}

TEST(AtomicTest, CompareExchangeString_Fail) {
    auto a = makeAtomic(std::string("apple"));
    std::string expected = "orange";
    const std::string desired = "banana";
    EXPECT_FALSE(a.exchangeCmp(expected, desired));
    EXPECT_EQ(a.load(), "apple");
    EXPECT_EQ(expected, "apple"); // 写回真实值
}

// ========== 多线程并发测试（CAS 保证正确性） ==========
TEST(AtomicTest, ConcurrentIncrementCAS) {
    auto counter = makeAtomic(0);

    auto task = [&counter] {
        for (int i = 0; i < 1000; ++i) {
            int expected = counter.load();
            int desired;
            do {
                desired = expected + 1;
            } while (!counter.exchangeCmp(expected, desired));
        }
    };

    std::thread t1(task);
    std::thread t2(task);
    t1.join();
    t2.join();

    EXPECT_EQ(counter.load(), 2000);
}
