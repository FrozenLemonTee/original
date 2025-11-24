#include <gtest/gtest.h>
#include "vector.h"
#include "queue.h"
#include "zeit.h"
#include "mutex.h"
#include "condition.h"
#include "thread.h"

using namespace original;
using namespace original::literals;

class pConditionTest : public testing::Test {
protected:
    mutex mutex_;
    condition cond_;
    bool ready = false;
};

class ProducerConsumerTest : public testing::Test {
protected:
    queue<int> buffer;
    static constexpr std::size_t MAX_SIZE = 5;
    mutex mutex_;
    condition cond_full;  // 消费者等待“非空”
    condition cond_empty; // 生产者等待“非满”
    bool done = false;
};

// 基本通知机制：等待一个条件，然后 notify 唤醒
TEST_F(pConditionTest, BasicNotifyTest) {
    thread t([&] {
        uniqueLock lock(mutex_);
        while (!ready) {
            cond_.wait(mutex_);
        }
    }, thread::AUTO_JOIN);

    thread::sleep(50_ms); // 让线程进入 wait 状态

    {
        uniqueLock lock(mutex_);
        ready = true;
        cond_.notify();
    }

    t.join();
    SUCCEED();
}

// 测试 waitFor 的超时行为
TEST_F(pConditionTest, TimedWaitTimeoutTest) {
    uniqueLock lock(mutex_);
    const bool result = cond_.waitFor(mutex_, 200_ms);
    EXPECT_FALSE(result);  // 应超时
}

// 测试 waitFor 在超时前被唤醒
TEST_F(pConditionTest, TimedWaitNotTimeoutTest) {
    thread t([&] {
        thread::sleep(50_ms); // 等待 50ms 再唤醒
        uniqueLock lock(mutex_);
        ready = true;
        cond_.notify();
    }, thread::AUTO_JOIN);

    bool result;
    {
        uniqueLock lock(mutex_);
        result = cond_.waitFor(mutex_, 200_ms);
    }

    EXPECT_TRUE(result);
}

// 测试 notifyAll 能唤醒多个等待线程
TEST_F(pConditionTest, NotifyAllWakesAllWaiters) {
    int wake_count = 0;
    ready = false;

    thread t1([&] {
        uniqueLock lock(mutex_);
        while (!ready) cond_.wait(mutex_);
        wake_count++;
    });

    thread t2([&] {
        uniqueLock lock(mutex_);
        while (!ready) cond_.wait(mutex_);
        wake_count++;
    });

    thread::sleep(50_ms); // 等待线程进入阻塞状态

    {
        uniqueLock lock(mutex_);
        ready = true;
        cond_.notifyAll();
    }

    t1.join();
    t2.join();

    EXPECT_EQ(wake_count, 2);
}

TEST_F(ProducerConsumerTest, ProducerConsumerWorkCorrectly) {
    constexpr std::size_t total_count = 100;
    vector<int> consumed;

    // 消费者线程
    thread consumer([&] {
        while (true) {
            uniqueLock lock(mutex_);
            cond_full.wait(mutex_, [&]{
                return !buffer.empty() || done;
            });
            if (done && buffer.empty())
                break;

            // 消费
            int value = buffer.head();
            buffer.pop();
            consumed.pushEnd(value);

            cond_empty.notify(); // 通知生产者：空位释放
        }
    }, thread::AUTO_JOIN);

    // 生产者线程
    thread producer([&] {
        for (int i = 1; i <= total_count; ++i) {
            uniqueLock lock(mutex_);
            cond_empty.wait(mutex_, [&] {
                return buffer.size() < MAX_SIZE;
            });
            buffer.push(i); // 生产
            cond_full.notify();  // 通知消费者：有货了
        }

        // 设置完成标志并广播
        {
            uniqueLock lock(mutex_);
            done = true;
            cond_full.notifyAll();
        }
    }, thread::AUTO_JOIN);

    consumer.join();
    producer.join();

    // 检查是否所有数据都被消费
    ASSERT_EQ(consumed.size(), total_count);
    for (int i = 0; i < total_count; ++i) {
        EXPECT_EQ(consumed[i], i + 1);
    }
}