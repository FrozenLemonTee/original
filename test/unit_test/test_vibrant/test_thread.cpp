#include <gtest/gtest.h>
#include "thread.h"
#include <atomic>
#include "zeit.h"
#include <chrono>

using namespace original;
using namespace original::literals;
using namespace std::literals;

namespace {
    using thread_type =
#if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG
        pThread;
#elif ORIGINAL_COMPILER_MSVC
        wThread;
#endif
    // Test class with member functions
    class Worker {
    public:
        int result = 0;

        void compute(const int a, const int b) {
            result = a + b;
        }

        static void setTrue(std::atomic<bool>& flag) {
            flag = true;
        }
    };

    // Test fixture for thread tests
    class ThreadTest : public ::testing::Test {
    protected:
        void SetUp() override {
            // Setup code if needed
        }

        void TearDown() override {
            // Cleanup code if needed
        }
    };

    // Simple function for testing
    void simpleFunction(int& value) {
        value = 42;
    }

    // Function that throws an error
    void throwingFunction(std::exception_ptr& ptr) {
        try {
            throw sysError();
        }
        catch (const sysError&) {
            ptr = std::current_exception();
        }
    }

    // Function with delay for join/detach tests
    void delayedFunction(std::atomic<bool>& flag) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        flag = true;
    }
}

// Test basic thread creation with function
TEST_F(ThreadTest, BasicFunctionThread) {
    int value = 0;
    {
        thread t1(simpleFunction, std::ref(value));
    }
    ASSERT_EQ(value, 42);

    const thread t2([] {});
    const ul_integer id1 = t2.id();
    ASSERT_NE(id1, 0);  // ID should not be zero for a valid thread

    const thread t3;
    ASSERT_EQ(t3.id(), 0);  // Default-constructed thread should have ID 0
}

// Test non-const member function as thread entry
TEST_F(ThreadTest, MemberFunctionThread) {
    Worker w;
    {
        thread t(&Worker::compute, &w, 5, 7);
    }
    ASSERT_EQ(w.result, 12);
}

// Test const member function as thread entry
TEST_F(ThreadTest, ConstMemberFunctionThread) {
    std::atomic flag(false);
    {
        thread t(&Worker::setTrue, std::ref(flag));
    }
    ASSERT_TRUE(flag);
}

// Test member function via lambda wrapping
TEST_F(ThreadTest, LambdaWrapMemberFunction) {
    Worker w;
    std::atomic flag(false);
    {
        thread t([&]
        {
            w.compute(3, 9);
            flag = true;
        });
    }
    ASSERT_EQ(w.result, 12);
    ASSERT_TRUE(flag);
}

// Test thread creation with lambda
TEST_F(ThreadTest, LambdaThread) {
    int value = 0;
    {
        thread t([&value] { value = 100; });
    }
    ASSERT_EQ(value, 100);
}

// Test move constructor
TEST_F(ThreadTest, MoveConstructor) {
    std::atomic flag(false);
    {
        thread t1(delayedFunction, std::ref(flag));
        thread t2(std::move(t1));
    }
    ASSERT_TRUE(flag);
}

// Test move assignment
TEST_F(ThreadTest, MoveAssignment) {
    std::atomic flag1(false);
    {
        std::atomic flag2(false);
        thread t1(delayedFunction, std::ref(flag1));
        thread t2(delayedFunction, std::ref(flag2));
        ASSERT_FALSE(flag2); // t2's original thread should be detached
    }

    ASSERT_TRUE(flag1);
}

// Test joinable
TEST_F(ThreadTest, Joinable) {
    thread t([] {});
    ASSERT_TRUE(t.joinable());
    t.join();
    ASSERT_FALSE(t.joinable());
}

// Test detach
TEST_F(ThreadTest, Detach) {
    std::atomic flag(false);
    {
    thread t(delayedFunction, std::ref(flag));
    t.detach();
    ASSERT_FALSE(t);
}
    // Give some time for the thread to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    ASSERT_TRUE(flag);
}

// Test automatic join in destructor
TEST_F(ThreadTest, DestructorJoin) {
    std::atomic flag(false);
    {
    thread t(delayedFunction, std::ref(flag));
    // t will be joined when it goes out of scope
    }
    ASSERT_TRUE(flag);
}

// Test automatic detach in destructor when will_join is false
TEST_F(ThreadTest, DestructorDetach) {
    std::atomic flag(false);
    {
    thread t(delayedFunction, std::ref(flag));
    // t will be detached when it goes out of scope
    }
    // Give some time for the thread to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    ASSERT_TRUE(flag);
}

// Test thread with multiple arguments
TEST_F(ThreadTest, MultipleArguments) {
    int result = 0;
    {
        auto func = [](const int a, const int b, int& r) { r = a + b; };
        thread t(func, 10, 20, std::ref(result));
    }
    ASSERT_EQ(result, 30);
}

// Test error handling in thread
TEST_F(ThreadTest, ThreadThrowsError) {
    bool exception_caught = false;
    std::exception_ptr ptr;
    thread t{ throwingFunction, std::ref(ptr) };
    t.join();
    try {
        std::rethrow_exception(ptr);
    }
    catch (const sysError&) {
        exception_caught = true;
    }

    EXPECT_TRUE(exception_caught);
}

// Test bool operator
TEST_F(ThreadTest, BoolOperator) {
    const thread t1;
    ASSERT_FALSE(t1);

    thread t2([] {});
    ASSERT_TRUE(t2);
    t2.join();
    ASSERT_FALSE(t2);
}

// Test not operator
TEST_F(ThreadTest, NotOperator) {
    const thread t1;
    ASSERT_TRUE(!t1);

    thread t2([] {});
    ASSERT_FALSE(!t2);
    t2.join();
    ASSERT_TRUE(!t2);
}

// Test will_join flag
TEST_F(ThreadTest, WillJoinFlag) {
    std::atomic flag(false);
    {
    thread t(delayedFunction, std::ref(flag));
    // Will be detached on destruction
    }
    // Give some time for the thread to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    ASSERT_TRUE(flag);
}

// Test thread ID uniqueness
TEST_F(ThreadTest, ThreadIdUniqueness) {
    thread t1([] { std::this_thread::sleep_for(std::chrono::milliseconds(100)); });
    thread t2([] { std::this_thread::sleep_for(std::chrono::milliseconds(100)); });

    const ul_integer id1 = t1.id();
    const ul_integer id2 = t2.id();

    ASSERT_NE(id1, 0);
    ASSERT_NE(id2, 0);
    ASSERT_NE(id1, id2);  // Different threads should have different IDs

    t1.join();
    t2.join();

    // After joining, the IDs should be 0 because handles are reset
    ASSERT_EQ(t1.id(), 0);
    ASSERT_EQ(t2.id(), 0);
}

// Test thread ID after move
TEST_F(ThreadTest, ThreadIdAfterMove) {
    thread t1([](){});
    const ul_integer original_id = t1.id();

    thread t2(std::move(t1));
    ASSERT_EQ(t2.id(), original_id);  // ID should be preserved after move
    ASSERT_EQ(t1.id(), 0);            // Moved-from thread should have ID 0

    t2.join();
    ASSERT_EQ(t2.id(), 0);            // After joining, ID should be 0
}

// Test platform thread ID functionality
TEST_F(ThreadTest, PlatformThreadId) {
    thread_type pt([] {});
    const ul_integer id1 = pt.id();
    ASSERT_NE(id1, 0);

    thread_type pt2(std::move(pt));
    ASSERT_EQ(pt2.id(), id1);  // ID should be preserved after move
    ASSERT_EQ(pt.id(), 0);     // Moved-from pThread should have ID 0

    pt2.join();
}

TEST_F(ThreadTest, NowFunctionTimeSpendTest) {
    constexpr integer count = 1e6;
    for (integer i = 0; i < count; ++i) {
        time::point::now();
    }
}

// Test sleep_for functionality
TEST_F(ThreadTest, SleepForDuration) {
    using namespace original::literals;  // For duration literals

    // Test basic sleep functionality
    auto start = std::chrono::steady_clock::now();
    thread::sleep(500_ms);  // Sleep for 500 milliseconds
    auto end = std::chrono::steady_clock::now();

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    ASSERT_GE(elapsed.count(), 450);  // Should sleep at least 450ms
    ASSERT_LE(elapsed.count(), 550);  // With some tolerance for scheduling

    // Test with different time units
    start = std::chrono::steady_clock::now();
    thread::sleep(1_s + 200_ms);  // Sleep for 1.2 seconds
    end = std::chrono::steady_clock::now();

    elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    ASSERT_GE(elapsed.count(), 1150);
    ASSERT_LE(elapsed.count(), 1250);

    // Test short sleep
    const auto p1 = time::point::now();
    thread::sleep(50_ms);
    const auto p2 = time::point::now();
    const auto elapsed2 = p2 - p1;
    ASSERT_GE(elapsed2.value(time::MICROSECOND), 30000);
    ASSERT_LE(elapsed2.value(time::MICROSECOND), 70000);
}

TEST_F(ThreadTest, SleepCompletion) {
    std::atomic sleep_completed(false);

    thread t([&] {
        thread::sleep(100_ms);
        sleep_completed = true;
    });

    t.join();
    ASSERT_TRUE(sleep_completed);
}

// Test sleep_for with zero or negative duration
TEST_F(ThreadTest, SleepZeroOrNegative) {
    auto start = std::chrono::steady_clock::now();
    thread::sleep(0_s);  // Zero duration
    auto end = std::chrono::steady_clock::now();

    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    ASSERT_LT(elapsed.count(), 100000);  // Should return almost immediately

    start = std::chrono::steady_clock::now();
    thread::sleep(-100_ms);  // Negative duration
    end = std::chrono::steady_clock::now();

    elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    ASSERT_LT(elapsed.count(), 100000);  // Should return almost immediately
}

// Test sleep_for in multiple threads
TEST_F(ThreadTest, SleepInMultipleThreads) {
    constexpr int num_threads = 5;
    std::vector<int> completion_order;
    std::mutex mtx;

    auto worker = [&](const int id, const time::duration& sleep_time) {
        thread::sleep(sleep_time);
        std::lock_guard lock(mtx);
        completion_order.push_back(id);
    };

    std::vector<thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        // Each thread sleeps for (i+1)*100 milliseconds
        threads.emplace_back(worker, i, (i+1)*100_ms);
    }

    // Wait for all threads to complete
    for (auto& t : threads) {
        t.join();
    }

    // Verify threads completed in expected order
    ASSERT_EQ(completion_order.size(), num_threads);
    for (int i = 0; i < num_threads; ++i) {
        ASSERT_EQ(completion_order[i], i);
    }
}

// Test comparable interface
TEST_F(ThreadTest, ComparableInterface) {
    thread t1([] {});
    thread t2([] {});

    // Test comparison operators
    ASSERT_NE(t1.id(), t2.id());  // Different threads should have different IDs
    ASSERT_NE(t1, t2);
    ASSERT_TRUE(t1 < t2 || t1 > t2);  // One should be less than the other

    // Test move and compare
    thread t3(std::move(t1));
    ASSERT_EQ(t1.id(), 0);  // Moved-from thread should be invalid
    ASSERT_NE(t3.id(), 0);  // Moved-to thread should be valid
    ASSERT_NE(t3, t2);

    t2.join();
    t3.join();

    // After joining, both should be equal (invalid)
    ASSERT_EQ(t2, t3);
}

// Test hashable interface
TEST_F(ThreadTest, HashableInterface) {
    thread t1([] {});
    thread t2([] {});

    // Test hash values
    const u_integer hash1 = t1.toHash();
    const u_integer hash2 = t2.toHash();

    ASSERT_NE(hash1, 0);
    ASSERT_NE(hash2, 0);
    ASSERT_NE(hash1, hash2);  // Different threads should have different hashes

    // Test equals
    ASSERT_FALSE(t1.equals(t2));

    // Test move and hash
    thread t3(std::move(t1));
    ASSERT_EQ(t1.toHash(), 0);  // Moved-from thread should have hash 0
    ASSERT_NE(t3.toHash(), 0);  // Moved-to thread should have valid hash

    t2.join();
    t3.join();

    // After joining, both should have hash 0
    ASSERT_EQ(t2.toHash(), 0);
    ASSERT_EQ(t3.toHash(), 0);
    ASSERT_TRUE(t2.equals(t3));
}

// Test printable interface
TEST_F(ThreadTest, PrintableInterface) {
    thread t1([] {});
    thread t2;

    // Test className
    ASSERT_EQ(t1.className(), "thread");

    // Test toString
    const std::string str1 = t1.toString(false);
    const std::string str2 = t2.toString(false);

    ASSERT_TRUE(str1.find("thread") != std::string::npos);
    ASSERT_TRUE(str1.find(std::to_string(t1.id())) != std::string::npos);
    ASSERT_TRUE(str2.find("thread") != std::string::npos);

    // Test operator<<
    std::ostringstream oss;
    oss << t1;
    ASSERT_TRUE(oss.str().find("thread") != std::string::npos);

    // Test after move
    thread t3(std::move(t1));
    const std::string str3 = t1.toString(false);
    ASSERT_TRUE(str3.find("thread") != std::string::npos);

    t3.join();
    t2.join();
}

// Test platform thread printable interface
TEST_F(ThreadTest, PlatformThreadPrintableInterface) {
    thread_type pt1([] {});
    thread_type pt2;

    const std::string platform_thread_name = ON_WINDOWS() ? "wThread": "pThread";

    // Test className
    ASSERT_EQ(pt1.className(), platform_thread_name);

    // Test toString
    const std::string str1 = pt1.toString(false);
    const std::string str2 = pt2.toString(false);

    ASSERT_TRUE(str1.find(platform_thread_name) != std::string::npos);
    ASSERT_TRUE(str1.find(std::to_string(pt1.id())) != std::string::npos);
    ASSERT_TRUE(str2.find(platform_thread_name) != std::string::npos);

    pt1.join();
}