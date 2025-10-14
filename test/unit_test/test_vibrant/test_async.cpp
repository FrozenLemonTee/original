#include <algorithm>
#include <numeric>
#include <unordered_set>
#include <gtest/gtest.h>
#include <utility>
#include "async.h"
#include "zeit.h"

using namespace original;

class runTimeTestError final : std::exception {
    mutable std::string msg_{"runTimeTestError"};

public:
    runTimeTestError() = default;

    explicit runTimeTestError(std::string msg) : msg_(std::move(msg)) {}

    [[nodiscard]] const char* what() const noexcept override {
        return this->msg_.c_str();
    }
};

// 辅助函数：运行 promise 在单独线程中
template<typename Promise>
void runPromiseInThread(Promise p) {
    auto shared_p = makeStrongPtr<Promise>(std::move(p));
    thread t([shared_p]() mutable {
        shared_p->run();
    });
}

TEST(AsyncTest, SimpleAsyncReturnsValue) {
    auto p = async::makePromise([] {
        thread::sleep(milliseconds(100));
        return 42;
    });

    auto f = p.getFuture();
    runPromiseInThread(std::move(p));
    f.wait();
    EXPECT_EQ(f.result(), 42);
}

TEST(AsyncTest, MultipleWaitsAreSafe) {
    auto p = async::makePromise([] {
        thread::sleep(milliseconds(120));
        return 99;
    });

    auto f = p.getFuture();
    runPromiseInThread(std::move(p));

    std::cout << f.result() << std::endl;  // 第一次应该成功

    // 再次调用应该抛出异常
    EXPECT_THROW(f.result(), original::error);
}

TEST(AsyncTest, AsynchronousExecution) {
    auto p = async::makePromise([] {
        thread::sleep(milliseconds(200));
        return 2025;
    });

    const auto start = time::point::now();
    auto f = p.getFuture();
    runPromiseInThread(std::move(p));

    const int result = f.result();
    const auto end = time::point::now();

    const auto duration = end - start;
    EXPECT_GE(duration.value(), 190);  // 至少等待了200ms（允许误差）
    EXPECT_EQ(result, 2025);
}

TEST(AsyncTest, ExceptionHandledProperly) {
    auto p = async::makePromise([]() -> int {
        throw runTimeTestError("failure in thread");
    });

    auto f = p.getFuture();
    runPromiseInThread(std::move(p));
    EXPECT_THROW(f.result(), runTimeTestError);
}

// 测试 void 返回类型的异步任务
TEST(AsyncTest, VoidReturnType) {
    auto p = async::makePromise([] {
        thread::sleep(milliseconds(50));
        // do nothing, just return
    });

    auto f = p.getFuture();
    runPromiseInThread(std::move(p));
    EXPECT_NO_THROW(f.result());  // 应该能正常返回（void）
}

// 测试传参异步任务
TEST(AsyncTest, TaskWithArguments) {
    auto p = async::makePromise([](const int a, const int b) {
        return a + b;
    }, 10, 32);

    auto f = p.getFuture();
    runPromiseInThread(std::move(p));
    EXPECT_EQ(f.result(), 42);
}

// 测试连续创建多个 future，不互相干扰
TEST(AsyncTest, MultipleFuturesIndependence) {
    auto p1 = async::makePromise([] { return 1; });
    auto p2 = async::makePromise([] { return 2; });

    auto f1 = p1.getFuture();
    auto f2 = p2.getFuture();

    runPromiseInThread(std::move(p1));
    runPromiseInThread(std::move(p2));

    EXPECT_EQ(f1.result(), 1);
    EXPECT_EQ(f2.result(), 2);
}

// 测试异常任务在多次调用后仍能抛出原始异常
TEST(AsyncTest, ExceptionConsistency) {
    auto p = async::makePromise([]() -> int {
        throw runTimeTestError("boom");
    });

    auto f = p.getFuture();
    runPromiseInThread(std::move(p));

    EXPECT_THROW(f.result(), runTimeTestError);
    // 第二次调用仍然抛异常
    EXPECT_THROW(f.result(), runTimeTestError);
}

// 测试 get 函数的基本功能
TEST(AsyncTest, GetAsyncBasicFunctionality) {
    auto sf = async::get([] {
        thread::sleep(milliseconds(100));
        return 42;
    });

    EXPECT_TRUE(sf.valid());
    EXPECT_FALSE(sf.ready());  // 任务刚开始，应该还没完成

    // 等待任务完成
    sf.wait();

    EXPECT_TRUE(sf.ready());
    EXPECT_EQ(sf.result(), 42);
}

// 测试 get 函数的异常处理
TEST(AsyncTest, GetAsyncExceptionHandling) {
    auto sf = async::get([]() -> int {
        thread::sleep(milliseconds(50));
        throw runTimeTestError("getAsync error");
        return 42;
    });

    EXPECT_TRUE(sf.valid());

    // 等待任务完成
    sf.wait();

    EXPECT_TRUE(sf.ready());
    EXPECT_THROW(sf.result(), runTimeTestError);
}

// 测试 get 函数带参数
TEST(AsyncTest, GetAsyncWithArguments) {
    auto sf = async::get([](const int a, const int b, const int c) {
        thread::sleep(milliseconds(50));
        return a + b + c;
    }, 10, 20, 12);

    EXPECT_TRUE(sf.valid());

    // 等待任务完成
    sf.wait();

    EXPECT_TRUE(sf.ready());
    EXPECT_EQ(sf.result(), 42);
}

// 测试 get 的 void 返回类型
TEST(AsyncTest, GetAsyncVoidReturnType) {
    std::atomic completed{false};

    auto sf = async::get([&completed] {
        thread::sleep(milliseconds(50));
        completed.store(true);
    });

    EXPECT_TRUE(sf.valid());

    // 等待任务完成
    sf.wait();

    EXPECT_TRUE(sf.ready());
    EXPECT_NO_THROW(sf.result());
    EXPECT_TRUE(completed.load());
}

// 测试多个 get 任务并发执行
TEST(AsyncTest, GetAsyncMultipleConcurrentTasks) {
    constexpr int numTasks = 10;
    std::vector<async::sharedFuture<int>> futures;

    // 启动多个并发任务
    for (int i = 0; i < numTasks; i++) {
        futures.push_back(async::get([i] {
            thread::sleep(milliseconds(20));
            return i * i;
        }).share());
    }

    // 等待所有任务完成
    for (auto& sf : futures) {
        sf.wait();
        EXPECT_TRUE(sf.ready());
    }

    // 验证所有结果
    for (int i = 0; i < numTasks; i++) {
        EXPECT_EQ(futures[i].result(), i * i);
    }
}

// 测试 get 任务的自动分离线程
TEST(AsyncTest, GetAsyncAutoDetachThread) {
    // 这个测试验证线程确实会自动分离，不会造成资源泄漏
    const auto sf = async::get([] {
        thread::sleep(milliseconds(100));
        return 123;
    }).share();

    // 获取结果，应该能正常完成
    EXPECT_EQ(sf.result(), 123);

    // 多次调用应该都能正常工作（因为是 sharedFuture）
    EXPECT_EQ(sf.result(), 123);
    EXPECT_EQ(sf.result(), 123);
}

// 测试 ready() 方法
TEST(AsyncTest, ReadyMethodWorks) {
    auto p = async::makePromise([] {
        thread::sleep(milliseconds(100));
        return 42;
    });

    auto f = p.getFuture();
    EXPECT_FALSE(f.ready());  // 任务还没开始，应该没准备好

    runPromiseInThread(std::move(p));
    thread::sleep(milliseconds(150));  // 等待任务完成

    EXPECT_TRUE(f.ready());  // 现在应该准备好了
    EXPECT_EQ(f.result(), 42);
}

// 测试 wait() 方法
TEST(AsyncTest, WaitMethodWorks) {
    auto p = async::makePromise([] {
        thread::sleep(milliseconds(100));
        return 42;
    });

    auto f = p.getFuture();
    auto start = time::point::now();

    runPromiseInThread(std::move(p));
    f.wait();  // 等待任务完成

    const auto end = time::point::now();
    const auto duration = end - start;

    EXPECT_GE(duration.value(), 90);  // 应该等待了至少100ms
    EXPECT_TRUE(f.ready());
    EXPECT_EQ(f.result(), 42);
}

// 测试 sharedFuture 的基本功能（复制和多次访问）
TEST(AsyncTest, SharedFutureBasicFunctionality) {
    auto p = async::makePromise([] {
        thread::sleep(milliseconds(100));
        return 42;
    });

    auto f = p.getFuture();
    const auto sf = f.share();  // 转换为 sharedFuture

    runPromiseInThread(std::move(p));

    // 多个 sharedFuture 副本应该都能访问结果
    auto sf2 = sf;  // NOLINT: Copy test
    EXPECT_EQ(sf.result(), 42);
    EXPECT_EQ(sf2.result(), 42);

    // 原始 future 应该失效
    EXPECT_FALSE(f.valid());
}

// 测试 void 类型的 sharedFuture
TEST(AsyncTest, SharedFutureVoidType) {
    auto p = async::makePromise([] {
        thread::sleep(milliseconds(50));
    });

    auto f = p.getFuture();
    const auto sf = f.share();

    runPromiseInThread(std::move(p));

    // 应该能正常调用（无返回值）
    EXPECT_NO_THROW(sf.result());

    // 可以复制
    auto sf2 = sf;  // NOLINT: Copy test
    EXPECT_NO_THROW(sf2.result());
}

// 测试 sharedFuture 的异常处理
TEST(AsyncTest, SharedFutureExceptionHandling) {
    auto p = async::makePromise([]() -> int {
        throw runTimeTestError("shared future error");
    });

    auto f = p.getFuture();
    auto sf = f.share();

    runPromiseInThread(std::move(p));

    // 应该抛出相同的异常
    EXPECT_THROW(sf.result(), runTimeTestError);

    // 副本也应该抛出相同的异常
    auto sf2 = sf;  // NOLINT: Copy test
    EXPECT_THROW(sf2.result(), runTimeTestError);
}

// 测试多线程同时访问 sharedFuture
TEST(AsyncTest, SharedFutureMultithreadedAccess) {
    auto p = async::makePromise([] {
        thread::sleep(milliseconds(200));
        return 100;
    });

    auto f = p.getFuture();
    auto sf = f.share();

    runPromiseInThread(std::move(p));

    std::vector<thread> threads;
    std::atomic successCount{0};

    // 启动多个线程同时访问 sharedFuture
    for (int i = 0; i < 5; i++) {
        threads.emplace_back([&sf, &successCount] {
            try {
                EXPECT_EQ(sf.result(), 100);
                ++successCount;
            } catch (...) {
                // 不应该有异常
                FAIL() << "Unexpected exception in worker thread";
            }
        });
    }

    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(successCount.load(), 5);
}

// 测试 futureBase 多态接口
TEST(AsyncTest, FutureBasePolymorphicInterface) {
    auto p = async::makePromise([] {
        thread::sleep(milliseconds(50));
        return 42;
    });

    auto f = p.getFuture();

    // 通过基类指针操作
    async::futureBase* basePtr = &f;

    runPromiseInThread(std::move(p));

    EXPECT_TRUE(basePtr->valid());
    EXPECT_NO_THROW(basePtr->wait());
    EXPECT_TRUE(basePtr->ready());

    // 异常应该为nullptr（无异常）
    EXPECT_EQ(basePtr->exception(), nullptr);
}

// 测试 sharedFuture 通过 futureBase 接口操作
TEST(AsyncTest, SharedFutureBaseInterface) {
    auto p = async::makePromise([] {
        thread::sleep(milliseconds(50));
        return 42;
    });

    auto f = p.getFuture();
    auto sf = f.share();

    // 通过基类指针操作 sharedFuture
    async::futureBase* basePtr = &sf;

    runPromiseInThread(std::move(p));

    EXPECT_TRUE(basePtr->valid());
    EXPECT_NO_THROW(basePtr->wait());
    EXPECT_TRUE(basePtr->ready());
    EXPECT_EQ(basePtr->exception(), nullptr);
}

// 测试异常情况下的 futureBase 接口
TEST(AsyncTest, FutureBaseExceptionCase) {
    auto p = async::makePromise([]() -> int {
        throw runTimeTestError("base interface error");
    });

    auto f = p.getFuture();
    async::futureBase* basePtr = &f;

    runPromiseInThread(std::move(p));

    EXPECT_TRUE(basePtr->valid());

    // 应该能检测到异常
    const auto exceptionPtr = basePtr->exception();
    EXPECT_NE(exceptionPtr, nullptr);

    // 通过异常指针重新抛出
    EXPECT_THROW(std::rethrow_exception(exceptionPtr), runTimeTestError);
}

// 测试无效的 future 通过 base 接口操作
TEST(AsyncTest, InvalidFutureBaseAccess) {
    async::future<int> f;  // 默认构造，无效的future

    async::futureBase* basePtr = &f;

    EXPECT_FALSE(basePtr->valid());
    EXPECT_THROW(basePtr->wait(), original::error);

    // 无效的future应该返回空指针
    auto exception_ptr = basePtr->exception();
    EXPECT_EQ(exception_ptr, nullptr);

    strongPtr<async::sharedFuture<void>> f2 = makeStrongPtr<async::sharedFuture<void>>();

    EXPECT_FALSE(f2->valid());
    EXPECT_THROW(f2->wait(), original::error);

    auto exception_ptr2 = f2->exception();
    EXPECT_EQ(exception_ptr2, nullptr);

    auto f3 = f2.dynamicCastTo<async::futureBase>();
    EXPECT_FALSE(f3->valid());
    EXPECT_THROW(f3->wait(), original::error);

    auto exception_ptr3 = f3->exception();
    EXPECT_EQ(exception_ptr3, nullptr);
}

// 测试 sharedFuture 的 ready() 方法
TEST(AsyncTest, SharedFutureReadyMethod) {
    auto p = async::makePromise([] {
        thread::sleep(milliseconds(100));
        return 42;
    });

    auto f = p.getFuture();
    auto sf = f.share();

    EXPECT_FALSE(sf.ready());  // 任务还没开始

    runPromiseInThread(std::move(p));
    thread::sleep(milliseconds(150));  // 等待任务完成

    EXPECT_TRUE(sf.ready());  // 现在应该准备好了
    EXPECT_EQ(sf.result(), 42);
}

// 测试 sharedFuture 的 wait() 方法
TEST(AsyncTest, SharedFutureWaitMethod) {
    auto p = async::makePromise([] {
        thread::sleep(milliseconds(100));
        return 42;
    });

    auto f = p.getFuture();
    auto sf = f.share();

    auto start = time::point::now();

    runPromiseInThread(std::move(p));
    sf.wait();  // 等待任务完成

    auto end = time::point::now();
    auto duration = end - start;

    EXPECT_GE(duration.value(), 90);  // 应该等待了至少100ms
    EXPECT_TRUE(sf.ready());
    EXPECT_EQ(sf.result(), 42);
}

// 测试 sharedFuture 的比较运算符
TEST(AsyncTest, SharedFutureComparisonOperators) {
    auto p1 = async::makePromise([] {
        return 42;
    });
    auto p2 = async::makePromise([] {
        return 100;
    });

    auto f1 = p1.getFuture();
    auto f2 = p2.getFuture();

    auto sf1 = f1.share();
    auto sf2 = f2.share();
    auto sf1_copy = sf1;  // 复制构造
    auto sf1_move = std::move(sf1);  // 移动构造

    // 相同底层对象的副本应该相等
    EXPECT_TRUE(sf1_copy == sf1_move);
    EXPECT_FALSE(sf1_copy != sf1_move);

    // 不同底层对象应该不相等
    EXPECT_FALSE(sf1_copy == sf2);
    EXPECT_TRUE(sf1_copy != sf2);

    // 无效的 sharedFuture 应该相等（都是nullptr）
    async::sharedFuture<int> invalid1, invalid2;
    EXPECT_TRUE(invalid1 == invalid2);
    EXPECT_FALSE(invalid1 != invalid2);

    // 有效的和无效的不相等
    EXPECT_FALSE(sf1_copy == invalid1);
    EXPECT_TRUE(sf1_copy != invalid1);

    runPromiseInThread(std::move(p1));
    runPromiseInThread(std::move(p2));

    // 比较结果不应该影响值访问
    EXPECT_EQ(sf1_copy.result(), 42);
    EXPECT_EQ(sf1_move.result(), 42);
    EXPECT_EQ(sf2.result(), 100);
}

// 测试 strongPointer() 方法
TEST(AsyncTest, SharedFutureStrongPointerMethod) {
    auto p = async::makePromise([] {
        thread::sleep(milliseconds(100));
        return std::vector{1, 2, 3, 4, 5};
    });

    auto f = p.getFuture();
    auto sf = f.share();

    runPromiseInThread(std::move(p));
    sf.wait();  // 等待任务完成

    // 使用 strongPointer() 避免拷贝
    const auto result_ptr = sf.strongPointer();
    ASSERT_NE(result_ptr, nullptr);
    EXPECT_EQ(result_ptr->size(), 5);
    EXPECT_EQ((*result_ptr)[0], 1);
    EXPECT_EQ((*result_ptr)[4], 5);

    // 原始数据不应被修改（因为是 const 指针）
    // (*result_ptr)[0] = 99;  // 这行应该编译错误

    // 拷贝后的结果可以随意修改
    auto result = *result_ptr;
    result.push_back(6);
    EXPECT_EQ(result.size(), 6);
}

// 测试 sharedFuture 的哈希功能
TEST(AsyncTest, SharedFutureHashFunctionality) {
    auto p1 = async::makePromise([] {
        return 42;
    });
    auto p2 = async::makePromise([] {
        return 100;
    });

    auto f1 = p1.getFuture();
    auto f2 = p2.getFuture();

    auto sf1 = f1.share();
    auto sf2 = f2.share();
    auto sf1_copy = sf1;  // 复制构造
    auto sf1_move = std::move(sf1);  // 移动构造

    runPromiseInThread(std::move(p1));
    runPromiseInThread(std::move(p2));

    // 相同底层对象的副本应该有相同的哈希值
    EXPECT_EQ(sf1_copy.toHash(), sf1_move.toHash());
    EXPECT_TRUE(sf1_copy.equals(sf1_move));

    // 不同底层对象应该有不同的哈希值（大概率）
    EXPECT_NE(sf1_copy.toHash(), sf2.toHash());
    EXPECT_FALSE(sf1_copy.equals(sf2));

    // 验证哈希值的一致性（多次调用应该返回相同值）
    auto hash1 = sf1_copy.toHash();
    auto hash2 = sf1_copy.toHash();
    EXPECT_EQ(hash1, hash2);

    // 无效的 sharedFuture 应该有合理的哈希值（通常是0）
    async::sharedFuture<int> invalid1, invalid2;
    EXPECT_EQ(invalid1.toHash(), invalid2.toHash());
    EXPECT_TRUE(invalid1.equals(invalid2));

    // 有效的和无效的不应该相等
    EXPECT_FALSE(sf1_copy.equals(invalid1));
    EXPECT_NE(sf1_copy.toHash(), invalid1.toHash());
}

// 测试 void 类型的 sharedFuture 哈希
TEST(AsyncTest, SharedFutureVoidHashFunctionality) {
    auto p1 = async::makePromise([] {
        thread::sleep(milliseconds(50));
    });
    auto p2 = async::makePromise([] {
        thread::sleep(milliseconds(50));
    });

    auto f1 = p1.getFuture();
    auto f2 = p2.getFuture();

    auto sf1 = f1.share();
    auto sf2 = f2.share();
    auto sf1_copy = sf1;

    runPromiseInThread(std::move(p1));
    runPromiseInThread(std::move(p2));

    // 等待任务完成
    sf1.wait();
    sf2.wait();

    // 相同底层对象应该有相同的哈希值
    EXPECT_EQ(sf1.toHash(), sf1_copy.toHash());
    EXPECT_TRUE(sf1.equals(sf1_copy));

    // 不同底层对象应该有不同的哈希值
    EXPECT_NE(sf1.toHash(), sf2.toHash());
    EXPECT_FALSE(sf1.equals(sf2));

    // 验证哈希一致性
    EXPECT_EQ(sf1.toHash(), sf1.toHash());
}

// 测试 sharedFuture 在 unordered_set 中的使用
TEST(AsyncTest, SharedFutureInUnorderedSet) {
    std::unordered_set<async::sharedFuture<int>> future_set;

    auto p1 = async::makePromise([] { return 1; });
    auto p2 = async::makePromise([] { return 2; });
    auto p3 = async::makePromise([] { return 3; });

    auto sf1 = p1.getFuture().share();
    auto sf2 = p2.getFuture().share();
    auto sf3 = p3.getFuture().share();
    auto sf1_copy = sf1;  // 副本

    runPromiseInThread(std::move(p1));
    runPromiseInThread(std::move(p2));
    runPromiseInThread(std::move(p3));

    // 插入到 unordered_set
    future_set.insert(sf1);
    future_set.insert(sf2);
    future_set.insert(sf3);
    future_set.insert(sf1_copy);  // 应该不会插入，因为与 sf1 相同

    // 应该有3个元素（sf1_copy 是重复的）
    EXPECT_EQ(future_set.size(), 3);

    // 检查元素是否存在
    EXPECT_NE(future_set.find(sf1), future_set.end());
    EXPECT_NE(future_set.find(sf2), future_set.end());
    EXPECT_NE(future_set.find(sf3), future_set.end());
    EXPECT_NE(future_set.find(sf1_copy), future_set.end());  // 应该找到 sf1

    // 检查不存在的元素
    auto p4 = async::makePromise([] { return 4; });
    auto sf4 = p4.getFuture().share();
    EXPECT_EQ(future_set.find(sf4), future_set.end());
}

// 测试 sharedFuture 在 unordered_map 中的使用
TEST(AsyncTest, SharedFutureInUnorderedMap) {
    std::unordered_map<async::sharedFuture<int>, std::string> future_map;

    auto p1 = async::makePromise([] { return 1; });
    auto p2 = async::makePromise([] { return 2; });

    auto sf1 = p1.getFuture().share();
    auto sf2 = p2.getFuture().share();
    auto sf1_copy = sf1;  // 副本

    runPromiseInThread(std::move(p1));
    runPromiseInThread(std::move(p2));

    // 插入到 unordered_map
    future_map[sf1] = "first";
    future_map[sf2] = "second";
    future_map[sf1_copy] = "first_copy";  // 应该覆盖之前的值

    // 应该有2个元素
    EXPECT_EQ(future_map.size(), 2);

    // 检查值
    EXPECT_EQ(future_map[sf1], "first_copy");
    EXPECT_EQ(future_map[sf2], "second");
    EXPECT_EQ(future_map[sf1_copy], "first_copy");

    // 使用 find 方法
    auto it1 = future_map.find(sf1);
    ASSERT_NE(it1, future_map.end());
    EXPECT_EQ(it1->second, "first_copy");

    auto it2 = future_map.find(sf2);
    ASSERT_NE(it2, future_map.end());
    EXPECT_EQ(it2->second, "second");
}

// 测试哈希值的稳定性（多次运行应该相同）
TEST(AsyncTest, SharedFutureHashStability) {
    auto p = async::makePromise([] {
        return 42;
    });

    auto sf = p.getFuture().share();
    runPromiseInThread(std::move(p));

    // 获取哈希值多次，应该都相同
    const auto hash1 = sf.toHash();
    const auto hash2 = sf.toHash();
    const auto hash3 = sf.toHash();

    EXPECT_EQ(hash1, hash2);
    EXPECT_EQ(hash2, hash3);
    EXPECT_EQ(hash1, hash3);

    // 即使等待任务完成，哈希值也应该不变
    sf.wait();
    const auto hash_after_wait = sf.toHash();
    EXPECT_EQ(hash1, hash_after_wait);

    // 获取结果后，哈希值也应该不变
    const int result = sf.result();
    EXPECT_EQ(result, 42);
    const auto hash_after_result = sf.toHash();
    EXPECT_EQ(hash1, hash_after_result);
}

// 测试异常情况下的哈希行为
TEST(AsyncTest, SharedFutureHashWithException) {
    auto p = async::makePromise([]() -> int {
        throw runTimeTestError("hash test error");
    });

    const auto sf = p.getFuture().share();
    runPromiseInThread(std::move(p));

    // 即使有异常，哈希值也应该可用
    EXPECT_NO_THROW({
        std::cout << sf.toHash() << std::endl;
    });
    const auto hash = sf.toHash();

    // 多次调用应该返回相同的哈希值
    EXPECT_EQ(sf.toHash(), hash);
    EXPECT_EQ(sf.toHash(), hash);

    // 尝试获取结果应该抛出异常，但哈希值不变
    EXPECT_THROW(sf.result(), runTimeTestError);
    EXPECT_EQ(sf.toHash(), hash);
}

// 测试移动语义对哈希的影响
TEST(AsyncTest, SharedFutureHashMoveSemantics) {
    auto p = async::makePromise([] {
        return 42;
    });

    auto sf_original = p.getFuture().share();
    runPromiseInThread(std::move(p));

    const auto original_hash = sf_original.toHash();

    // 移动构造
    async::sharedFuture sf_moved(std::move(sf_original));
    EXPECT_EQ(sf_moved.toHash(), original_hash);

    // 移动后原始对象应该无效
    EXPECT_FALSE(sf_original.valid());
    EXPECT_EQ(sf_original.toHash(), 0);  // 无效对象的哈希值应该是0

    // 移动赋值
    const async::sharedFuture<int> sf_target = std::move(sf_moved);
    EXPECT_EQ(sf_target.toHash(), original_hash);
    EXPECT_FALSE(sf_moved.valid());
    EXPECT_EQ(sf_moved.toHash(), 0);
}


// 测试 future 的 waitFor 方法（成功情况）
TEST(AsyncTest, FutureWaitForSuccess) {
    auto f = async::get([] {
        thread::sleep(milliseconds(100));
        return 42;
    });

    // 等待足够长的时间，应该成功
    const bool completed = f.waitFor(milliseconds(200));
    EXPECT_TRUE(completed);
    EXPECT_TRUE(f.ready());
    EXPECT_EQ(f.result(), 42);
}

// 测试 future 的 waitFor 方法（超时情况）
TEST(AsyncTest, FutureWaitForTimeout) {
    auto f = async::get([] {
        thread::sleep(milliseconds(200));  // 需要200ms
        return 42;
    });

    // 立即检查，应该还没完成
    EXPECT_FALSE(f.ready());

    // 等待时间不够，应该超时
    const bool completed = f.waitFor(milliseconds(50));
    EXPECT_FALSE(completed);
    EXPECT_FALSE(f.ready());  // 应该还没完成

    // 现在等待足够的时间
    f.wait();
    EXPECT_TRUE(f.ready());
    EXPECT_EQ(f.result(), 42);
}

// 测试 future<void> 的 waitFor 方法
TEST(AsyncTest, FutureVoidWaitFor) {
    std::atomic completed{false};

    auto f = async::get([&completed] {
        thread::sleep(milliseconds(100));
        completed = true;
    });

    // 等待足够长的时间
    const bool success = f.waitFor(milliseconds(150));
    EXPECT_TRUE(success);
    EXPECT_TRUE(f.ready());
    EXPECT_NO_THROW(f.result());
    EXPECT_TRUE(completed);
}

// 测试 sharedFuture 的 waitFor 方法
TEST(AsyncTest, SharedFutureWaitFor) {
    const auto sf = async::get([] {
        thread::sleep(milliseconds(100));
        return 42;
    }).share();

    // 等待足够长的时间
    const bool completed = sf.waitFor(milliseconds(150));
    EXPECT_TRUE(completed);
    EXPECT_TRUE(sf.ready());
    EXPECT_EQ(sf.result(), 42);

    // 副本也应该有相同的行为
    const auto sf_copy = sf;
    EXPECT_TRUE(sf_copy.ready());
    EXPECT_EQ(sf_copy.result(), 42);
}

// 测试 sharedFuture<void> 的 waitFor 方法
TEST(AsyncTest, SharedFutureVoidWaitFor) {
    auto executed = std::make_shared<std::atomic<bool>>(false);

    const auto sf = async::get([executed] {
        thread::sleep(milliseconds(100));
        executed->store(true);
    }).share();

    // 等待足够长的时间
    const bool success = sf.waitFor(milliseconds(150));
    EXPECT_TRUE(success);
    EXPECT_TRUE(sf.ready());

    // 在检查result之前确保任务已完成
    if (sf.ready()) {
        EXPECT_NO_THROW(sf.result());
    }

    EXPECT_TRUE(executed->load());

    sf.wait();
}

// 测试 futureBase 接口的 waitFor 方法
TEST(AsyncTest, FutureBaseWaitFor) {
    auto f = async::get([] {
        thread::sleep(milliseconds(100));
        return 42;
    });

    async::futureBase* base_ptr = &f;

    // 通过基类接口调用 waitFor
    const bool completed = base_ptr->waitFor(milliseconds(150));
    EXPECT_TRUE(completed);
    EXPECT_TRUE(base_ptr->ready());
    EXPECT_EQ(base_ptr->exception(), nullptr);
}

// 测试 sharedFuture 通过 futureBase 接口的 waitFor
TEST(AsyncTest, SharedFutureBaseWaitFor) {
    const auto sf = async::get([] {
        thread::sleep(milliseconds(100));
        return 42;
    }).share();

    const async::futureBase* base_ptr = &sf;

    // 通过基类接口调用 waitFor
    const bool completed = base_ptr->waitFor(milliseconds(150));
    EXPECT_TRUE(completed);
    EXPECT_TRUE(base_ptr->ready());
    EXPECT_EQ(base_ptr->exception(), nullptr);
}

// 测试 waitFor 在异常情况下的行为
TEST(AsyncTest, WaitForWithException) {
    auto f = async::get([]() -> int {
        thread::sleep(milliseconds(50));
        throw runTimeTestError("waitFor test error");
    });

    // 等待足够长的时间
    const bool completed = f.waitFor(milliseconds(100));
    EXPECT_TRUE(completed);
    EXPECT_TRUE(f.ready());

    // 应该能检测到异常
    const auto exception_ptr = f.exception();
    EXPECT_NE(exception_ptr, nullptr);
    EXPECT_THROW(std::rethrow_exception(exception_ptr), runTimeTestError);
    EXPECT_THROW(f.result(), runTimeTestError);
}

// 测试 waitFor 的多次调用
TEST(AsyncTest, WaitForMultipleCalls) {
    auto f = async::get([] {
        thread::sleep(milliseconds(100));
        return 42;
    });

    // 第一次调用，可能还没完成
    const bool first_call = f.waitFor(milliseconds(50));
    std::cout << printable::formatString(first_call) << std::endl;

    // 第二次调用，等待更长时间
    const bool second_call = f.waitFor(milliseconds(100));

    // 第三次调用，应该已经完成了
    const bool third_call = f.waitFor(milliseconds(50));

    EXPECT_TRUE(second_call || third_call);  // 至少有一次应该成功
    EXPECT_TRUE(f.ready());
    EXPECT_EQ(f.result(), 42);
}

// 测试 waitFor 零超时的情况
TEST(AsyncTest, WaitForZeroTimeout) {
    auto f = async::get([] {
        thread::sleep(milliseconds(100));
        return 42;
    });

    // 零超时，应该立即返回
    if (f.waitFor(milliseconds(0))) {
        EXPECT_TRUE(f.ready());
        EXPECT_EQ(f.result(), 42);
    } else {
        EXPECT_FALSE(f.ready());
        // 稍等一会儿再检查
        thread::sleep(milliseconds(150));
        EXPECT_TRUE(f.ready());
        EXPECT_EQ(f.result(), 42);
    }
}

// 测试 waitFor 在任务立即完成的情况
TEST(AsyncTest, WaitForImmediateCompletion) {
    auto f = async::get([] {
        return 42;  // 立即返回
    });

    // 即使是很短的超时，也应该立即完成
    const bool completed = f.waitFor(milliseconds(10));
    EXPECT_TRUE(completed);
    EXPECT_TRUE(f.ready());
    EXPECT_EQ(f.result(), 42);
}

// 测试 waitFor 与 wait 的组合使用
TEST(AsyncTest, WaitForAndWaitCombination) {
    auto f = async::get([] {
        thread::sleep(milliseconds(200));
        return 42;
    });

    // 先用 waitFor 尝试短时间等待
    const bool short_wait = f.waitFor(milliseconds(50));
    EXPECT_FALSE(short_wait);
    EXPECT_FALSE(f.ready());

    // 然后用 wait 无限等待
    f.wait();
    EXPECT_TRUE(f.ready());
    EXPECT_EQ(f.result(), 42);

    // 现在 waitFor 应该立即返回 true
    const bool final_wait = f.waitFor(milliseconds(0));
    EXPECT_TRUE(final_wait);
}

// 测试 waitFor 在多个共享 future 上的行为
TEST(AsyncTest, WaitForMultipleSharedFutures) {
    const auto sf1 = async::get([] {
        thread::sleep(milliseconds(100));
        return 42;
    }).share();

    const auto sf2 = sf1;  // 副本

    // 两个共享 future 都应该能正确响应 waitFor
    const bool completed1 = sf1.waitFor(milliseconds(150));
    const bool completed2 = sf2.waitFor(milliseconds(150));

    EXPECT_TRUE(completed1);
    EXPECT_TRUE(completed2);
    EXPECT_TRUE(sf1.ready());
    EXPECT_TRUE(sf2.ready());
    EXPECT_EQ(sf1.result(), 42);
    EXPECT_EQ(sf2.result(), 42);
}

// 测试 future 的管道操作符
TEST(AsyncTest, FuturePipeOperator) {
    auto f = async::get([] {
        return 21;
    }) | [](const int x) {
        return x * 2;
    };

    EXPECT_EQ(f.result(), 42);
}

// 测试 sharedFuture 的管道操作符
TEST(AsyncTest, SharedFuturePipeOperator) {
    const auto sf = async::get([] {
        return 21;
    }).share() | [](int x) {
        return x * 2;
    };

    EXPECT_EQ(sf.result(), 42);
}

// 测试多级管道操作
TEST(AsyncTest, MultiplePipeOperations) {
    auto f = async::get([] {
        return 10;
    }) | [](int x) {
        return x * 2;  // 20
    } | [](int x) {
        return x + 22;  // 42
    } | [](int x) {
        return std::to_string(x);  // "42"
    };

    EXPECT_EQ(f.result(), "42");
}

// 测试管道操作中的类型转换
TEST(AsyncTest, PipeOperatorTypeConversion) {
    auto f = async::get([] {
        return 42;
    }) | [](const int x) {
        return std::vector<int>{x, x * 2, x * 3};
    };

    const auto result = f.result();

    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 42);
    EXPECT_EQ(result[1], 84);
    EXPECT_EQ(result[2], 126);
}

// 测试管道操作的异步性
TEST(AsyncTest, PipeOperatorAsynchronous) {
    const auto start = time::point::now();

    auto f = async::get([] {
        thread::sleep(milliseconds(100));
        return 1;
    }) | [](const int x) {
        thread::sleep(milliseconds(100));
        return x + 1;
    };

    EXPECT_EQ(f.result(), 2);
    const auto end = time::point::now();
    const auto duration = end - start;

    EXPECT_GE(duration.value(), 190);  // 至少200ms
}

// 测试管道操作中的异常处理
TEST(AsyncTest, PipeOperatorExceptionHandling) {
    auto future = async::get([]() -> int {
        throw runTimeTestError("source error");
    }) | [](const int x) {
        return x + 1;  // 不会执行
    };

    EXPECT_THROW(future.result(), runTimeTestError);
}

// 测试管道操作中回调抛出异常
TEST(AsyncTest, PipeOperatorCallbackException) {
    auto future = async::get([] {
        return 42;
    }) | [](int x) -> int {
        throw runTimeTestError("callback error");
    };

    EXPECT_THROW(future.result(), runTimeTestError);
}

// 测试 void future 的管道操作
TEST(AsyncTest, VoidFuturePipeOperator) {
    std::atomic executed{false};

    auto future = async::get([] {
        thread::sleep(milliseconds(50));
    }) | [&executed]{
        executed = true;
    };

    future.result();
    EXPECT_TRUE(executed);
}

// 测试 void 到非 void 的管道转换
TEST(AsyncTest, VoidToNonVoidPipe) {
    auto f = async::get([] {
        thread::sleep(milliseconds(50));
    }) | []{
        return 42;
    };

    EXPECT_EQ(f.result(), 42);
}

// 测试管道操作的链式调用与共享
TEST(AsyncTest, PipeOperatorChainingAndSharing) {
    const auto shared_future = async::get([] {
        return 21;
    }).share();

    // 从同一个共享 future 创建多个管道
    const auto chain1 = shared_future | [](const int x) { return x * 2; };
    const auto chain2 = shared_future | [](const int x) { return x + 21; };
    const auto chain3 = shared_future | [](const int x) { return x - 1; };

    EXPECT_EQ(chain1.result(), 42);
    EXPECT_EQ(chain2.result(), 42);
    EXPECT_EQ(chain3.result(), 20);
}

// 测试管道操作与标准算法结合
TEST(AsyncTest, PipeOperatorWithAlgorithms) {
    auto f = async::get([] {
        return std::vector{1, 2, 3, 4, 5};
    }) | [](const std::vector<int>& vec) {
        std::vector<int> transformed;
        std::ranges::transform(vec, std::back_inserter(transformed),
                               [](const int x) { return x * x; });
        return transformed;
    };

    EXPECT_EQ(f.result(), std::vector({1, 4, 9, 16, 25}));
}

// 测试管道操作中的移动语义
TEST(AsyncTest, PipeOperatorMoveSemantics) {
    auto f = async::get([] {
        return std::make_unique<int>(42);
    }) | [](const std::unique_ptr<int>& ptr) {
        return *ptr;  // 移动语义，ptr 被消耗
    };

    EXPECT_EQ(f.result(), 42);
}

// 测试管道操作与异常传播
TEST(AsyncTest, PipeOperatorExceptionPropagation) {
    try {
        auto f = async::get([]() -> int {
            throw runTimeTestError("first error");
        }) | [](const int x) {
            return x + 1;  // 不会执行
        } | [](const int x) {
            return x * 2;  // 不会执行
        };
        f.result();

        FAIL() << "Should have thrown an exception";
    } catch (const runTimeTestError& e) {
        EXPECT_STREQ(e.what(), "first error");
    }
}

// 测试复杂的管道组合
TEST(AsyncTest, ComplexPipeComposition) {
    auto process_data = [](const std::string& input) {
        return input + " processed";
    };

    auto convert_to_upper = [](std::string str) {
        std::ranges::transform(str, str.begin(), ::toupper);
        return str;
    };

    auto add_suffix = [](const std::string& str) {
        return str + "!";
    };

    auto f = async::get([] {
        return "hello";
    }) | process_data | convert_to_upper | add_suffix;

    EXPECT_EQ(f.result(), "HELLO PROCESSED!");
}

// 测试管道操作与条件逻辑
TEST(AsyncTest, PipeOperatorWithConditionalLogic) {
    auto f = async::get([] {
        return 42;
    }) | [](const int x) {
    if (x > 0) {
        return x * 2;
    }
    return x;
    };

    EXPECT_EQ(f.result(), 84);
}

// 测试 promise 的 valid() 方法
TEST(AsyncTest, PromiseValidMethod) {
    // 默认构造的 promise 应该是无效的
    async::promise<int, std::function<int()>> p1;
    EXPECT_FALSE(p1.valid());

    // 通过 makePromise 创建的 promise 应该是有效的
    auto p2 = async::makePromise([] { return 42; });
    EXPECT_TRUE(p2.valid());

    // 获取 future 后，promise 应该仍然有效
    auto f = p2.getFuture();
    EXPECT_TRUE(p2.valid());

    // 运行 promise 后，应该变为无效
    p2.run();
    EXPECT_FALSE(p2.valid());
}

// 测试 promise 的 function() 方法
TEST(AsyncTest, PromiseFunctionMethod) {
    auto p = async::makePromise([] { return 42; });

    // 获取函数对象
    const auto func = p.function();
    EXPECT_FALSE(p.valid());  // 获取函数后 promise 应该变为无效

    // 执行函数应该返回正确的结果
    EXPECT_EQ(func(), 42);

    // 再次获取函数应该抛出异常
    EXPECT_THROW(p.function(), original::error);
}

// 测试 promise 的 function() 方法在无效时的行为
TEST(AsyncTest, PromiseFunctionMethodWhenInvalid) {
    async::promise<int, std::function<int()>> p;  // 默认构造，无效

    // 无效的 promise 获取函数应该抛出异常
    EXPECT_THROW(p.function(), original::error);
}

// 测试 promise 的 run() 方法在无效时的行为
TEST(AsyncTest, PromiseRunMethodWhenInvalid) {
    async::promise<int, std::function<int()>> p;  // 默认构造，无效

    // 无效的 promise 运行应该抛出异常
    EXPECT_THROW(p.run(), original::error);
}

// 测试 promise 的多次运行尝试
TEST(AsyncTest, PromiseMultipleRunAttempts) {
    auto p = async::makePromise([] { return 42; });

    // 第一次运行应该成功
    p.run();
    EXPECT_FALSE(p.valid());

    // 第二次运行应该抛出异常
    EXPECT_THROW(p.run(), original::error);
}

// 测试 promise 的多次获取函数尝试
TEST(AsyncTest, PromiseMultipleFunctionAttempts) {
    auto p = async::makePromise([] { return 42; });

    // 第一次获取函数应该成功
    auto func = p.function();
    EXPECT_FALSE(p.valid());

    // 第二次获取函数应该抛出异常
    EXPECT_THROW(p.function(), original::error);
}

// 测试 promise 的获取函数和运行互斥
TEST(AsyncTest, PromiseFunctionAndRunMutualExclusion) {
    auto p = async::makePromise([] { return 42; });

    // 先获取函数
    const auto func = p.function();
    EXPECT_FALSE(p.valid());

    // 然后尝试运行应该失败
    EXPECT_THROW(p.run(), original::error);

    // 但函数应该仍然可以执行
    EXPECT_EQ(func(), 42);
}

// 测试 promise 的先运行后获取函数
TEST(AsyncTest, PromiseRunThenFunction) {
    auto p = async::makePromise([] { return 42; });

    // 先运行
    p.run();
    EXPECT_FALSE(p.valid());

    // 然后尝试获取函数应该失败
    EXPECT_THROW(p.function(), original::error);
}

// 测试 void 类型的 promise 的 valid() 方法
TEST(AsyncTest, PromiseVoidValidMethod) {
    // 默认构造的 void promise 应该是无效的
    async::promise<void, std::function<void()>> p1;
    EXPECT_FALSE(p1.valid());

    // 通过 makePromise 创建的 void promise 应该是有效的
    auto p2 = async::makePromise([] {});
    EXPECT_TRUE(p2.valid());

    // 运行后应该变为无效
    p2.run();
    EXPECT_FALSE(p2.valid());
}

// 测试 void 类型的 promise 的 function() 方法
TEST(AsyncTest, PromiseVoidFunctionMethod) {
    std::atomic executed{false};

    auto p = async::makePromise([&executed] { executed = true; });

    // 获取函数对象
    const auto func = p.function();
    EXPECT_FALSE(p.valid());  // 获取函数后 promise 应该变为无效

    // 执行函数
    func();
    EXPECT_TRUE(executed.load());
}

// 测试 promise 的有效性状态在异常情况下的行为
TEST(AsyncTest, PromiseValidityWithException) {
    auto p = async::makePromise([]() -> int {
        throw runTimeTestError("test error");
    });

    EXPECT_TRUE(p.valid());

    // 运行应该捕获异常，但 promise 仍然会变为无效
    EXPECT_NO_THROW(p.run());
    EXPECT_FALSE(p.valid());
}

// 测试通过 function() 获取的函数抛出异常
TEST(AsyncTest, PromiseFunctionThrowsException) {
    auto p = async::makePromise([]() -> int {
        throw runTimeTestError("function error");
    });

    const auto func = p.function();
    EXPECT_FALSE(p.valid());

    // 执行函数应该抛出异常
    EXPECT_THROW(func(), runTimeTestError);
}

// 测试 promise 的有效性状态在多线程环境下的行为
TEST(AsyncTest, PromiseValidityMultithreaded) {
    auto p = async::makePromise([] {
        thread::sleep(milliseconds(100));
        return 42;
    });

    std::atomic valid_count{0};
    std::atomic invalid_count{0};

    // 启动多个线程检查 promise 的有效性
    std::vector<thread> threads;
    for (int i = 0; i < 5; i++) {
        threads.emplace_back([&p, &valid_count, &invalid_count] {
            if (p.valid()) {
                ++valid_count;
            } else {
                ++invalid_count;
            }
        });
    }

    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }

    // 在运行前，所有线程应该看到 promise 有效
    EXPECT_EQ(valid_count.load(), 5);
    EXPECT_EQ(invalid_count.load(), 0);

    // 运行 promise
    p.run();

    // 再次检查，现在应该无效
    EXPECT_FALSE(p.valid());
}

// 测试 promise 的 function() 方法返回的函数对象可以安全存储和执行
TEST(AsyncTest, PromiseFunctionStorageAndExecution) {
    auto p = async::makePromise([] {
        return std::vector{1, 2, 3};
    });

    // 获取函数并存储
    const auto stored_func = p.function();

    // 稍后执行
    thread::sleep(milliseconds(50));
    const auto result = stored_func();

    EXPECT_EQ(result, std::vector({1, 2, 3}));
}

// 测试 promise 的 function() 方法与移动语义
TEST(AsyncTest, PromiseFunctionMoveSemantics) {
    auto p = async::makePromise([] {
        return std::make_unique<int>(42);
    });

    // 获取函数
    auto func = p.function();

    // 移动函数对象
    const auto moved_func = std::move(func);

    // 执行移动后的函数
    const auto result = moved_func();
    EXPECT_EQ(*result, 42);
}

// 测试惰性管道操作符的基本功能
TEST(AsyncTest, LazyPipeOperatorBasic) {
    auto p = async::makePromise([] {
        return 21;
    }) | [](const int x) {
        return x * 2;
    };

    EXPECT_TRUE(p.valid());

    auto f = p.getFuture();
    p.run();

    EXPECT_EQ(f.result(), 42);
    EXPECT_FALSE(p.valid()); // promise 执行后应该变为无效
}

// 测试惰性管道操作符的多级组合
TEST(AsyncTest, LazyPipeOperatorMultipleStages) {
    auto p = async::makePromise([] {
        return 10;
    }) | [](const int x) {
        return x + 5;  // 15
    } | [](const int x) {
        return x * 3;  // 45
    } | [](const int x) {
        return x - 3;  // 42
    };

    auto f = p.getFuture();
    p.run();

    EXPECT_EQ(f.result(), 42);
}

// 测试惰性管道操作符的类型转换
TEST(AsyncTest, LazyPipeOperatorTypeConversion) {
    auto p = async::makePromise([] {
        return 42;
    }) | [](const int x) {
        return std::to_string(x) + " answer";
    };

    auto f = p.getFuture();
    p.run();

    EXPECT_EQ(f.result(), "42 answer");
}

// 测试惰性管道操作符的异常处理
TEST(AsyncTest, LazyPipeOperatorExceptionHandling) {
    auto p = async::makePromise([]() -> int {
        throw runTimeTestError("source error");
    }) | [](const int x) {
        return x + 1;  // 不会执行
    };

    auto f = p.getFuture();
    p.run();

    EXPECT_THROW(f.result(), runTimeTestError);
}

// 测试惰性管道操作符中回调抛出异常
TEST(AsyncTest, LazyPipeOperatorCallbackException) {
    auto p = async::makePromise([] {
        return 42;
    }) | [](int) -> int {
        throw runTimeTestError("callback error");
    };

    auto f = p.getFuture();
    p.run();

    EXPECT_THROW(f.result(), runTimeTestError);
}

// 测试 void 类型的惰性管道操作符
TEST(AsyncTest, LazyPipeOperatorVoidType) {
    std::atomic counter{0};

    auto p = async::makePromise([&counter] {
        counter = 100;
    }) | [&counter] {
        counter += 42;
    };

    EXPECT_EQ(counter.load(), 0); // 惰性执行，尚未修改

    const auto f = p.getFuture();
    p.run();

    f.wait(); // 等待执行完成
    EXPECT_EQ(counter.load(), 142);
}

// 测试 void 到非 void 的惰性管道转换
TEST(AsyncTest, LazyPipeOperatorVoidToNonVoid) {
    std::atomic initValue{0};

    auto p = async::makePromise([&initValue] {
        initValue = 21;
    }) | [&initValue] {
        return initValue.load() * 2;
    };

    auto f = p.getFuture();
    p.run();

    EXPECT_EQ(f.result(), 42);
    EXPECT_EQ(initValue.load(), 21);
}

// 测试惰性管道操作符的移动语义
TEST(AsyncTest, LazyPipeOperatorMoveSemantics) {
    auto p = async::makePromise([] {
        return std::make_unique<int>(21);
    }) | [](std::unique_ptr<int> ptr) {
        *ptr *= 2;
        return ptr;
    };

    auto f = p.getFuture();
    p.run();

    const auto result = f.result();
    EXPECT_EQ(*result, 42);
}

// 测试惰性管道操作符与标准算法结合
TEST(AsyncTest, LazyPipeOperatorWithAlgorithms) {
    auto p = async::makePromise([] {
        return std::vector{1, 2, 3, 4, 5};
    }) | [](const std::vector<int>& vec) {
        std::vector<int> transformed;
        std::ranges::transform(vec, std::back_inserter(transformed),
                             [](const int x) { return x * x; });
        return transformed;
    };

    auto f = p.getFuture();
    p.run();

    EXPECT_EQ(f.result(), std::vector({1, 4, 9, 16, 25}));
}

// 测试惰性管道操作符的多次使用
TEST(AsyncTest, LazyPipeOperatorMultipleUses) {
    auto func = [] {
        return 10;
    };

    // 创建多个不同的管道
    auto pipe1 = async::makePromise(func) | [](int x) { return x + 32; };
    auto pipe2 = async::makePromise(func) | [](int x) { return x * 4; };
    auto pipe3 = async::makePromise(func) | [](int x) { return x - 2; };

    // 执行各个管道
    auto f1 = pipe1.getFuture();
    pipe1.run();
    EXPECT_EQ(f1.result(), 42);

    auto f2 = pipe2.getFuture();
    pipe2.run();
    EXPECT_EQ(f2.result(), 40);

    auto f3 = pipe3.getFuture();
    pipe3.run();
    EXPECT_EQ(f3.result(), 8);
}

// 测试惰性管道操作符的延迟执行特性
TEST(AsyncTest, LazyPipeOperatorDelayedExecution) {
    std::atomic executionCount{0};

    auto p = async::makePromise([&executionCount] {
        ++executionCount;
        return 21;
    }) | [&executionCount](int x) {
        ++executionCount;
        return x * 2;
    };

    // 尚未执行，计数器应为0
    EXPECT_EQ(executionCount.load(), 0);

    // 执行管道
    auto f = p.getFuture();
    p.run();

    EXPECT_EQ(f.result(), 42);
    EXPECT_EQ(executionCount.load(), 2); // 两个阶段都执行了
}

// 测试惰性管道操作符的复杂组合
TEST(AsyncTest, LazyPipeOperatorComplexComposition) {
    auto process_data = [](const std::string& input) {
        return input + " processed";
    };

    auto convert_to_upper = [](std::string str) {
        std::ranges::transform(str, str.begin(), ::toupper);
        return str;
    };

    auto add_suffix = [](const std::string& str) {
        return str + "!";
    };

    auto p = async::makePromise([] {
        return "hello";
    }) | process_data | convert_to_upper | add_suffix;

    auto f = p.getFuture();
    p.run();

    EXPECT_EQ(f.result(), "HELLO PROCESSED!");
}

// 测试惰性管道操作符的条件逻辑
TEST(AsyncTest, LazyPipeOperatorWithConditionalLogic) {
    auto p = async::makePromise([] {
        return 42;
    }) | [](const int x) {
        if (x > 0) {
            return x * 2;
        }
        return x;
    };

    auto f = p.getFuture();
    p.run();

    EXPECT_EQ(f.result(), 84);
}

// 测试惰性管道操作符与异常传播
TEST(AsyncTest, LazyPipeOperatorExceptionPropagation) {
    try {
        auto p = async::makePromise([]() -> int {
            throw runTimeTestError("first error");
        }) | [](const int x) {
            return x + 1;  // 不会执行
        } | [](const int x) {
            return x * 2;  // 不会执行
        };

        auto f = p.getFuture();
        p.run();
        f.result();

        FAIL() << "Should have thrown an exception";
    } catch (const runTimeTestError& e) {
        EXPECT_STREQ(e.what(), "first error");
    }
}

// 测试惰性管道操作符在无效 promise 上的行为
TEST(AsyncTest, LazyPipeOperatorWithInvalidPromise) {
    async::promise<int, std::function<int()>> invalid_promise;

    EXPECT_THROW({
        auto p = std::move(invalid_promise) | [](const int x) { return x + 1; };
    }, original::error);
}

// 测试惰性管道操作符的函数提取
TEST(AsyncTest, LazyPipeOperatorFunctionExtraction) {
    auto p = async::makePromise([] {
        return 21;
    }) | [](const int x) {
        return x * 2;
    };

    // 提取函数而不是运行
    const auto func = p.function();
    EXPECT_FALSE(p.valid());

    // 手动执行函数
    const int result = func();
    EXPECT_EQ(result, 42);
}

// 测试惰性管道操作符的多线程安全性
TEST(AsyncTest, LazyPipeOperatorMultithreaded) {
    auto p = async::makePromise([] {
        thread::sleep(milliseconds(50));
        return 21;
    }) | [](int x) {
        thread::sleep(milliseconds(50));
        return x * 2;
    };

    auto f = p.getFuture();

    // 在单独线程中运行管道
    thread t([p = std::move(p)]() mutable {
        p.run();
    });

    // 等待结果
    const int result = f.result();
    EXPECT_EQ(result, 42);

    t.join();
}

// 测试惰性管道操作符与共享 future 的组合使用
TEST(AsyncTest, LazyPipeOperatorWithSharedFuture) {
    auto sourcePromise = async::makePromise([] {
        return 21;
    });

    auto sourceFuture = sourcePromise.getFuture().share();

    // 先运行源 promise
    sourcePromise.run();

    // 使用共享 future 的结果创建惰性管道
    auto p = async::makePromise([sourceFuture] {
        return sourceFuture.result();
    }) | [](int x) {
        return x * 2;
    };

    auto f = p.getFuture();
    p.run();

    EXPECT_EQ(f.result(), 42);
}

// 测试惰性管道操作符的性能特性（延迟执行验证）
TEST(AsyncTest, LazyPipeOperatorPerformance) {
    const auto start = time::point::now();

    // 创建管道但不立即执行
    auto p = async::makePromise([] {
        thread::sleep(milliseconds(100));
        return 1;
    }) | [](int x) {
        thread::sleep(milliseconds(100));
        return x + 1;
    };

    // 管道创建应该很快
    const auto createTime = time::point::now() - start;
    EXPECT_LT(createTime.value(), 50); // 创建时间应该很短

    // 执行管道
    auto f = p.getFuture();
    p.run();

    const int result = f.result();
    const auto totalTime = time::point::now() - start;

    EXPECT_EQ(result, 2);
    EXPECT_GE(totalTime.value(), 190); // 总执行时间应该至少200ms
}

// 测试 promise 的移动构造函数
TEST(AsyncTest, PromiseMoveConstructor) {
    auto p1 = async::makePromise([] { return 42; });
    EXPECT_TRUE(p1.valid());

    // 移动构造
    auto p2(std::move(p1));
    EXPECT_TRUE(p2.valid());
    EXPECT_FALSE(p1.valid());  // 移动后原对象应该无效

    // 运行移动后的 promise
    auto f = p2.getFuture();
    p2.run();
    EXPECT_EQ(f.result(), 42);
}

// 测试 promise 的移动赋值运算符
TEST(AsyncTest, PromiseMoveAssignment) {
    auto p1 = async::makePromise([] { return 42; });
    EXPECT_TRUE(p1.valid());

    auto p2 = std::move(p1);
    EXPECT_TRUE(p2.valid());
    p2.run();
    EXPECT_FALSE(p2.valid());
    EXPECT_FALSE(p1.valid());  // 移动后原对象应该无效

    // 运行移动后的 promise
    auto f = p2.getFuture();
    EXPECT_THROW(p2.run(), original::sysError);
}

// 测试移动到自身的移动赋值
TEST(AsyncTest, PromiseMoveAssignmentToSelf) {
    auto p = async::makePromise([] { return 42; });
    EXPECT_TRUE(p.valid());

    // 移动到自身应该保持有效
    p = std::move(p);
    EXPECT_TRUE(p.valid());  // 应该仍然有效

    // 运行 promise
    auto f = p.getFuture();
    p.run();
    EXPECT_EQ(f.result(), 42);
}

// 测试从无效 promise 移动
TEST(AsyncTest, PromiseMoveFromInvalid) {
    async::promise<int, std::function<int()>> p1;  // 默认构造，无效
    EXPECT_FALSE(p1.valid());

    // 从无效 promise 移动构造
    async::promise<int, std::function<int()>> p2(std::move(p1));
    EXPECT_FALSE(p2.valid());  // 移动后的对象也应该无效
    EXPECT_FALSE(p1.valid());  // 原对象仍然无效

    // 从无效 promise 移动赋值
    const async::promise<int, std::function<int()>> p3 = std::move(p1);
    EXPECT_FALSE(p3.valid());  // 移动后的对象也应该无效
}

// 测试移动语义与 function() 方法的交互
TEST(AsyncTest, PromiseMoveThenFunction) {
    auto p1 = async::makePromise([] { return 42; });
    EXPECT_TRUE(p1.valid());

    // 移动构造
    auto p2(std::move(p1));
    EXPECT_TRUE(p2.valid());
    EXPECT_FALSE(p1.valid());

    // 从移动后的对象获取函数
    const auto func = p2.function();
    EXPECT_FALSE(p2.valid());  // 获取函数后应该无效

    // 执行函数
    EXPECT_EQ(func(), 42);
}

// 测试移动语义与 run() 方法的交互
TEST(AsyncTest, PromiseMoveThenRun) {
    auto p1 = async::makePromise([] { return 42; });
    EXPECT_TRUE(p1.valid());

    // 移动构造
    auto p2(std::move(p1));
    EXPECT_TRUE(p2.valid());
    EXPECT_FALSE(p1.valid());

    // 运行移动后的 promise
    auto f = p2.getFuture();
    p2.run();
    EXPECT_FALSE(p2.valid());  // 运行后应该无效

    EXPECT_EQ(f.result(), 42);
}

// 测试移动语义与惰性管道操作符的结合
TEST(AsyncTest, PromiseMoveWithLazyPipeOperator) {
    auto source_promise = async::makePromise([] {
        return 10;
    });
    EXPECT_TRUE(source_promise.valid());

    // 移动源 promise 到管道
    auto pipe_promise = std::move(source_promise) | [](int x) { return x + 32; };
    EXPECT_TRUE(pipe_promise.valid());
    EXPECT_FALSE(source_promise.valid());  // 移动后源 promise 应该无效

    // 运行管道 promise
    auto f = pipe_promise.getFuture();
    pipe_promise.run();
    EXPECT_EQ(f.result(), 42);
}

// 测试多次移动
TEST(AsyncTest, PromiseMultipleMoves) {
    auto p1 = async::makePromise([] { return 42; });
    EXPECT_TRUE(p1.valid());

    // 第一次移动
    auto p2(std::move(p1));
    EXPECT_TRUE(p2.valid());
    EXPECT_FALSE(p1.valid());

    // 第二次移动
    auto p3(std::move(p2));
    EXPECT_TRUE(p3.valid());
    EXPECT_FALSE(p2.valid());

    // 第三次移动
    auto p4 = std::move(p3);
    EXPECT_TRUE(p4.valid());
    EXPECT_FALSE(p3.valid());

    // 运行最终 promise
    auto f = p4.getFuture();
    p4.run();
    EXPECT_EQ(f.result(), 42);
}

// 测试移动语义在多线程环境下的行为
TEST(AsyncTest, PromiseMoveMultithreaded) {
    auto p = async::makePromise([] {
        thread::sleep(milliseconds(100));
        return 42;
    });
    EXPECT_TRUE(p.valid());

    std::atomic moved{false};
    std::atomic valid_after_move{false};

    // 在一个线程中移动 promise
    thread move_thread([&p, &moved, &valid_after_move] {
        auto moved_promise(std::move(p));
        moved = true;
        valid_after_move = moved_promise.valid();

        // 运行移动后的 promise
        auto f = moved_promise.getFuture();
        moved_promise.run();
        EXPECT_EQ(f.result(), 42);
    });

    // 在主线程中等待移动完成
    while (!moved) {
        thread::sleep(milliseconds(10));
    }

    // 移动后原 promise 应该无效
    EXPECT_FALSE(p.valid());

    move_thread.join();
    EXPECT_TRUE(valid_after_move);
}

// 测试移动语义与异常处理的结合
TEST(AsyncTest, PromiseMoveWithException) {
    auto p1 = async::makePromise([]() -> int {
        throw runTimeTestError("test error");
    });
    EXPECT_TRUE(p1.valid());

    // 移动构造
    auto p2(std::move(p1));
    EXPECT_TRUE(p2.valid());
    EXPECT_FALSE(p1.valid());

    // 运行移动后的 promise
    auto f = p2.getFuture();
    p2.run();
    EXPECT_FALSE(p2.valid());

    // 应该抛出异常
    EXPECT_THROW(f.result(), runTimeTestError);
}

// 测试 void 类型 promise 的移动语义
TEST(AsyncTest, PromiseVoidMoveSemantics) {
    auto p1 = async::makePromise([] {});
    EXPECT_TRUE(p1.valid());

    // 移动构造
    auto p2(std::move(p1));
    EXPECT_TRUE(p2.valid());
    EXPECT_FALSE(p1.valid());

    // 运行移动后的 promise
    auto f = p2.getFuture();
    p2.run();
    EXPECT_FALSE(p2.valid());
    EXPECT_NO_THROW(f.result());
}

// 测试用例1: 非空 → 非空 (T → U)
TEST(AsyncTest, DynamicPipeNonVoidToNonVoid) {
    auto future = async::get([] {
        thread::sleep(milliseconds(100));
        return 21;
    }) | [](int x) -> int {
        return x * 2;  // 返回非空
    };

    EXPECT_EQ(future.result(), 42);
}

// 测试用例2: 非空 → 空 (T → void)
TEST(AsyncTest, DynamicPipeNonVoidToVoid) {
    std::atomic<int> consumed_value{0};
    std::atomic<bool> executed{false};

    auto future = async::get([] {
        thread::sleep(milliseconds(100));
        return 42;
    }) | [&consumed_value, &executed](int value) {
        consumed_value = value;  // 消费值
        executed = true;
        // 隐式返回void
    };

    future.wait();  // 等待管道完成
    EXPECT_EQ(consumed_value.load(), 42);
    EXPECT_TRUE(executed.load());
}

// 测试用例3: 空 → 空 (void → void)
TEST(AsyncTest, DynamicPipeVoidToVoid) {
    std::atomic<int> step_counter{0};

    auto future = async::get([&step_counter] {
        thread::sleep(milliseconds(100));
        step_counter = 1;
    }) | [&step_counter] {
        step_counter = 2;  // 继续处理
        // 隐式返回void
    };

    future.wait();
    EXPECT_EQ(step_counter.load(), 2);
}

// 测试用例4: 空 → 非空 (void → U)
TEST(AsyncTest, DynamicPipeVoidToNonVoid) {
    auto future = async::get([] {
        thread::sleep(milliseconds(100));
        // 无返回值
    }) | []() -> std::string {
        return "completed";  // 产生非空结果
    };

    EXPECT_EQ(future.result(), "completed");
}

// 测试用例5: 多级管道组合
TEST(AsyncTest, DynamicPipeMultiStageCombination) {
    auto future = async::get([] {
        return 10;
    }) | [](int x) -> int {          // 非空 → 非空
        return x + 5;
    } | [](int x) {                 // 非空 → 空 (消费中间结果)
        // 消费但不返回值
    } | []() -> std::string {       // 空 → 非空
        return "final result";
    };

    EXPECT_EQ(future.result(), "final result");
}

// 测试用例6: 异常传播
TEST(AsyncTest, DynamicPipeExceptionPropagation) {
    // 非空 → 非空 异常
    auto future1 = async::get([]() -> int {
        throw runTimeTestError("source error");
    }) | [](int x) -> int {
        ADD_FAILURE() << "Should not be executed";
        return x + 1;
    };
    EXPECT_THROW(future1.result(), runTimeTestError);

    // 非空 → 空 异常
    auto future2 = async::get([]() -> int {
        throw runTimeTestError("source error");
    }) | [](int x) {
        FAIL() << "Should not be executed";
    };
    EXPECT_THROW(future2.result(), runTimeTestError);
}

// 测试用例1: 非空 → 非空 (T → U)
TEST(AsyncTest, LazyPipeNonVoidToNonVoid) {
    auto promise = async::makePromise([] {
        return 21;
    }) | [](int x) -> int {
        return x * 2;  // 返回非空
    };

    auto future = promise.getFuture();
    promise.run();  // 惰性执行
    EXPECT_EQ(future.result(), 42);
    EXPECT_FALSE(promise.valid());  // 执行后变为无效
}

// 测试用例2: 非空 → 空 (T → void)
TEST(AsyncTest, LazyPipeNonVoidToVoid) {
    std::atomic<int> consumed_value{0};
    std::atomic<bool> executed{false};

    auto promise = async::makePromise([] {
        return 42;
    }) | [&consumed_value, &executed](int value) {
        consumed_value = value;  // 消费值
        executed = true;
        // 隐式返回void
    };

    auto future = promise.getFuture();
    promise.run();  // 惰性执行

    future.wait();
    EXPECT_EQ(consumed_value.load(), 42);
    EXPECT_TRUE(executed.load());
}

// 测试用例3: 空 → 空 (void → void)
TEST(AsyncTest, LazyPipeVoidToVoid) {
    std::atomic<int> step_counter{0};

    auto promise = async::makePromise([&step_counter] {
        step_counter = 1;
    }) | [&step_counter] {
        step_counter = 2;  // 继续处理
        // 隐式返回void
    };

    auto future = promise.getFuture();
    promise.run();  // 惰性执行

    future.wait();
    EXPECT_EQ(step_counter.load(), 2);
}

// 测试用例4: 空 → 非空 (void → U)
TEST(AsyncTest, LazyPipeVoidToNonVoid) {
    auto promise = async::makePromise([] {
        // 无返回值
    }) | []() -> std::string {
        return "completed";  // 产生非空结果
    };

    auto future = promise.getFuture();
    promise.run();  // 惰性执行
    EXPECT_EQ(future.result(), "completed");
}

// 测试用例5: 多级惰性管道
TEST(AsyncTest, LazyPipeMultiStageLazyPipeline) {
    auto promise = async::makePromise([] {
        return 10;
    }) | [](int x) -> int {          // 非空 → 非空
        return x + 32;
    } | [](int x) {                 // 非空 → 空
        // 消费中间结果
    } | []() -> std::string {       // 空 → 非空
        return "pipeline completed";
    };

    auto future = promise.getFuture();

    // 验证惰性特性：在运行前不会执行
    std::atomic<bool> executed{false};
    auto lazy_promise = async::makePromise([&executed] {
        executed = true;
        return 100;
    });
    EXPECT_FALSE(executed.load());  // 尚未执行

    lazy_promise.run();  // 显式执行
    EXPECT_TRUE(executed.load());

    promise.run();  // 执行主管道
    EXPECT_EQ(future.result(), "pipeline completed");
}

// 测试用例6: 惰性管道异常处理
TEST(AsyncTest, LazyPipeExceptionHandling) {
    // 非空 → 非空 异常
    auto promise1 = async::makePromise([]() -> int {
        throw runTimeTestError("source error");
    }) | [](int x) -> int {
        ADD_FAILURE() << "Should not be executed";
        return x + 1;
    };

    auto future1 = promise1.getFuture();
    promise1.run();
    EXPECT_THROW(future1.result(), runTimeTestError);

    // 非空 → 空 异常
    auto promise2 = async::makePromise([]() -> int {
        throw runTimeTestError("source error");
    }) | [](int x) {
        FAIL() << "Should not be executed";
    };

    auto future2 = promise2.getFuture();
    promise2.run();
    EXPECT_THROW(future2.result(), runTimeTestError);
}

// 测试用例7: 惰性管道函数提取
TEST(AsyncTest, LazyPipeFunctionExtraction) {
    std::atomic<int> counter{0};

    auto promise = async::makePromise([&counter] {
        counter = 10;
        return 5;
    }) | [&counter](int x) -> int {
        counter += x;
        return counter.load();
    };

    // 提取函数而不是立即执行
    auto func = promise.function();
    EXPECT_FALSE(promise.valid());  // 提取后无效

    // 手动执行提取的函数
    int result = func();
    EXPECT_EQ(result, 15);
    EXPECT_EQ(counter.load(), 15);
}

// 测试用例8: 复杂类型转换
TEST(AsyncTest, LazyPipeComplexTypeTransformations) {
    auto promise = async::makePromise([] {
        return std::vector{1, 2, 3};
    }) | [](const std::vector<int>& vec) -> int {
        return std::accumulate(vec.begin(), vec.end(), 0);  // 容器 → 标量
    } | [](int sum) -> std::string {
        return "sum: " + std::to_string(sum);  // 标量 → 字符串
    };

    auto future = promise.getFuture();
    promise.run();
    EXPECT_EQ(future.result(), "sum: 6");
}

// 测试用例9: 移动语义支持
TEST(AsyncTest, LazyPipeMoveSemantics) {
    auto promise = async::makePromise([] {
        return std::make_unique<int>(21);
    }) | [](std::unique_ptr<int> ptr) -> std::unique_ptr<int> {
        *ptr *= 2;
        return ptr;  // 移动语义
    };

    auto future = promise.getFuture();
    promise.run();

    auto result = future.result();
    EXPECT_EQ(*result, 42);
}

// 测试用例10: 惰性验证（性能特性）
TEST(AsyncTest, LazyPipeLazyEvaluationVerification) {
    const auto start = time::point::now();

    std::atomic<bool> first_stage_executed{false};
    std::atomic<bool> second_stage_executed{false};

    auto promise = async::makePromise([&first_stage_executed] {
        thread::sleep(milliseconds(100));
        first_stage_executed = true;
        return 1;
    }) | [&second_stage_executed](int x) {
        thread::sleep(milliseconds(100));
        second_stage_executed = true;
        return x + 1;
    };

    // 管道创建后立即检查，应该都未执行
    const auto create_time = time::point::now() - start;
    EXPECT_LT(create_time.value(), 50);  // 创建应该很快
    EXPECT_FALSE(first_stage_executed.load());
    EXPECT_FALSE(second_stage_executed.load());

    // 执行管道
    auto future = promise.getFuture();
    promise.run();

    const int result = future.result();
    const auto total_time = time::point::now() - start;

    EXPECT_EQ(result, 2);
    EXPECT_TRUE(first_stage_executed.load());
    EXPECT_TRUE(second_stage_executed.load());
    EXPECT_GE(total_time.value(), 190);  // 总执行时间至少200ms
}