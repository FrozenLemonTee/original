#include <algorithm>
#include <gtest/gtest.h>
#include <thread>
#include "mutex.h"
#include "thread.h"

using namespace original;

TEST(MutexTest, LockUnlockDoesNotThrow) {
    mutex m;
    EXPECT_NO_THROW(m.lock());
    EXPECT_NO_THROW(m.unlock());
}

TEST(MutexTest, TryLockSuccess) {
    mutex m;
    EXPECT_NO_THROW(m.tryLock());
    EXPECT_NO_THROW(m.unlock());
}

TEST(MutexTest, TryLockContested) {
    mutex m;
    std::thread t(
        [&m]
        {
            m.lock();
            thread::sleep(milliseconds(300));
            m.unlock();
        }
    );

    // 确保 t 抢先上锁
    thread::sleep(milliseconds(100));

    EXPECT_FALSE(m.tryLock());  // 此时 tryLock 应该失败
    t.join();
}

TEST(MutexTest, PreventsDataRaceWithMultipleThreads) {
    constexpr int thread_count = 10;
    constexpr int iterations = 10000;
    int counter = 0;
    mutex m;

    auto increment = [&]
    {
        for (int i = 0; i < iterations; ++i) {
            m.lock();
            ++counter;
            m.unlock();
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back(increment);
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(counter, thread_count * iterations);
}

TEST(MutexTest, LockInConstructorAndUnlockInDestructor) {
    {
        mutex pm;
        const uniqueLock m(pm);  // 默认锁定
        EXPECT_TRUE(m.isLocked());
    }
    SUCCEED();
}

TEST(MutexTest, TryLockConstructor) {
    {
        mutex pm;
        const uniqueLock m(pm, uniqueLock::TRY_LOCK);  // 使用 tryLock
        EXPECT_TRUE(m.isLocked());
    }
    SUCCEED();
}

TEST(MutexTest, MutexIsNonCopyable) {
    EXPECT_FALSE(std::is_copy_constructible_v<mutex>);
    EXPECT_FALSE(std::is_copy_assignable_v<mutex>);
    EXPECT_FALSE(std::is_copy_constructible_v<uniqueLock>);
    EXPECT_FALSE(std::is_copy_assignable_v<uniqueLock>);
}

TEST(MutexTest, MutexIsNonMovable) {
    EXPECT_FALSE(std::is_move_constructible_v<mutex>);
    EXPECT_FALSE(std::is_move_assignable_v<mutex>);
    EXPECT_FALSE(std::is_move_constructible_v<uniqueLock>);
    EXPECT_FALSE(std::is_move_assignable_v<uniqueLock>);
}

// TryLock失败时，uniqueLock::isLocked() 应为 false
TEST(MutexTest, TryLockFailsIsLockedFalse) {
    mutex pm;
    pm.lock();  // 主线程先锁住

    const uniqueLock s(pm, uniqueLock::TRY_LOCK);  // 尝试获取失败
    EXPECT_FALSE(s.isLocked());

    pm.unlock();  // 主线程释放锁
}

// scopeLock析构后，mutex可以再次上锁
TEST(MutexTest, RAIIUnlocksCorrectly) {
    mutex pm;
    {
        uniqueLock s(pm);
        EXPECT_TRUE(s.isLocked());
        // 测试期间尝试加锁应该失败
        EXPECT_FALSE(pm.tryLock());
    }
    // 现在应该能加锁
    EXPECT_TRUE(pm.tryLock());
    pm.unlock();
}

// 多线程验证uniqueLock作用域内有效
TEST(MutexTest, UniqueLockProtectsCriticalSection) {
    constexpr int thread_count = 10;
    constexpr int iterations = 10000;
    int counter = 0;
    mutex m;

    auto increment = [&]
    {
        for (int i = 0; i < iterations; ++i) {
            uniqueLock lock(m);
            ++counter;
        }
    };

    std::vector<thread> threads;
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back(increment);
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(counter, thread_count * iterations);
}

// TryLock获取失败时不会 unlock
TEST(MutexTest, TryLockFailDoesNotUnlock) {
    mutex pm;
    pm.lock();

    {
        const uniqueLock s(pm, uniqueLock::TRY_LOCK);
        EXPECT_FALSE(s.isLocked());
    }

    // pm 依旧被主线程持有，解锁后测试结束
    EXPECT_NO_THROW(pm.unlock());
}

// TryLock获取成功后，锁应被释放
TEST(MutexTest, TryLockSuccessUnlocksOnDestruction) {
    mutex pm;
    {
        if (const uniqueLock s(pm, uniqueLock::TRY_LOCK); s.isLocked()) {
            SUCCEED();  // 成功加锁
        }
    }

    // 若未正确释放，此处将阻塞
    EXPECT_NO_THROW(pm.lock());
    pm.unlock();
}

// 测试解锁后可以再次加锁
TEST(MutexTest, CanRelockAfterUnlock) {
    mutex m;
    m.lock();
    m.unlock();
    EXPECT_NO_THROW(m.lock());  // 再次加锁不应抛出异常
    m.unlock();
}

// 测试uniqueLock解锁后可以再次加锁
TEST(MutexTest, UniqueLockCanRelockAfterUnlock) {
    mutex pm;
    {
        uniqueLock lock(pm);
        lock.unlock();
        EXPECT_NO_THROW(lock.lock());  // 再次加锁
    }
    // 析构时应自动解锁
    EXPECT_NO_THROW(pm.lock());  // 验证锁已释放
    pm.unlock();
}

// 测试多次加锁解锁循环
TEST(MutexTest, MultipleLockUnlockCycles) {
    mutex m;
    for (int i = 0; i < 100; ++i) {
        EXPECT_NO_THROW(m.lock());
        EXPECT_NO_THROW(m.unlock());
    }
}

// 测试uniqueLock手动锁策略
TEST(MutexTest, ManualLockPolicy) {
    mutex pm;
    {
        uniqueLock lock(pm, uniqueLock::MANUAL_LOCK);
        EXPECT_FALSE(lock.isLocked());
        EXPECT_NO_THROW(lock.lock());
        EXPECT_TRUE(lock.isLocked());
        EXPECT_NO_THROW(lock.unlock());
        EXPECT_FALSE(lock.isLocked());
    }
    // 验证锁已释放
    EXPECT_NO_THROW(pm.lock());
    pm.unlock();
}

// 测试跨线程的加锁解锁顺序
TEST(MutexTest, CrossThreadLockUnlockSequence) {
    mutex m;
    std::atomic thread_locked{false};
    std::atomic main_proceed{false};

    std::thread t([&] {
        m.lock();
        thread_locked = true;
        while (!main_proceed) {}  // 等待主线程
        m.unlock();
    });

    // 等待线程加锁
    while (!thread_locked) {}

    EXPECT_FALSE(m.tryLock());  // 应该失败，因为线程持有锁

    main_proceed = true;  // 让线程继续
    t.join();

    EXPECT_TRUE(m.tryLock());  // 现在应该能加锁
    m.unlock();
}

TEST(MultiLockTest, MultiLockLocksAndUnlocksInReverseOrder) {
    std::vector<ul_integer> lock_order;
    std::vector<ul_integer> unlock_order;

    class Trackable {
        mutex mutex_;
        std::vector<ul_integer>& lock_record_;
        std::vector<ul_integer>& unlock_record_;
    public:
        explicit Trackable(std::vector<ul_integer>& lock_rec, std::vector<ul_integer>& unlock_rec)
                : lock_record_(lock_rec), unlock_record_(unlock_rec) {}

        void lock() {
            mutex_.lock();
            lock_record_.push_back(mutex_.id());
        }

        bool tryLock(){
            const bool success = mutex_.tryLock();
            if (success)
                lock_record_.push_back(mutex_.id());
            return success;
        }

        void unlock() {
            unlock_record_.push_back(mutex_.id());
            mutex_.unlock();
        }

        [[nodiscard]] ul_integer id() const {
            return mutex_.id();
        }
    };

    Trackable t1(lock_order, unlock_order);
    Trackable t2(lock_order, unlock_order);
    Trackable t3(lock_order, unlock_order);

    {
        multiLock lock(t1, t2, t3);
        EXPECT_EQ(lock_order.size(), 3);
        EXPECT_NE(std::ranges::find(lock_order, t1.id()), lock_order.end());
        EXPECT_NE(std::ranges::find(lock_order, t2.id()), lock_order.end());
        EXPECT_NE(std::ranges::find(lock_order, t3.id()), lock_order.end());
    }

    EXPECT_EQ(unlock_order.size(), 3);
    EXPECT_EQ(std::vector(unlock_order.rbegin(), unlock_order.rend()), lock_order);
}

TEST(MultiLockTest, MultiLockProtectsMultipleResources) {
    constexpr int iterations = 1000;
    int x = 0, y = 0;
    mutex mx, my;

    auto worker = [&]() {
        for (int i = 0; i < iterations; ++i) {
            multiLock lock(mx, my);
            ++x;
            ++y;
        }
    };

    thread t1(worker), t2(worker), t3(worker);
    t1.join();
    t2.join();
    t3.join();

    EXPECT_EQ(x, iterations * 3);
    EXPECT_EQ(y, iterations * 3);
}

TEST(MutexTest, AdoptLockPolicyAssumesLocked) {
    mutex m;
    m.lock();  // 手动加锁

    {
        const uniqueLock lock(m, uniqueLock::ADOPT_LOCK);
        EXPECT_TRUE(lock.isLocked());
        EXPECT_FALSE(m.tryLock());  // 锁未被释放
    }

    // 析构后应自动释放
    EXPECT_TRUE(m.tryLock());
    m.unlock();
}

TEST(MultiLockTest, AdoptLockSkipsLocking) {
    mutex m1, m2;
    m1.lock();
    m2.lock();

    {
        const multiLock lock(lockGuard::ADOPT_LOCK, m1, m2);
        EXPECT_TRUE(lock.isLocked());
        EXPECT_FALSE(m1.tryLock());
        EXPECT_FALSE(m2.tryLock());
    }

    EXPECT_TRUE(m1.tryLock());
    EXPECT_TRUE(m2.tryLock());
    m1.unlock();
    m2.unlock();
}

TEST(MultiLockTest, TryLockFailsWhenAnyMutexUnavailable) {
    mutex m1, m2;
    m1.lock();  // 抢占一个锁

    const multiLock lock(lockGuard::TRY_LOCK, m1, m2);
    EXPECT_FALSE(lock.isLocked());

    m1.unlock();
}