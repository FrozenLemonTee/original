#include <algorithm>
#include <gtest/gtest.h>
#include "vector.h"
#include <vector>

// 对比函数，用于比较 original::vector 和 std::vector
void compareVectors(const original::vector<int>& originalVec, const std::vector<int>& stdVec) {
    ASSERT_EQ(originalVec.size(), stdVec.size());
    for (size_t i = 0; i < stdVec.size(); ++i) {
        ASSERT_EQ(originalVec.get(i), stdVec[i]);
    }
}

// 测试 original::vector 类
class VectorTest : public testing::Test {
protected:
    void SetUp() override {
        // 每个测试用例前都创建一个空的 original::vector 和 std::vector
        originalVec = original::vector<int>();
        stdVec = std::vector<int>();
    }

    original::vector<int> originalVec;
    std::vector<int> stdVec;
};

// 测试 push 和 pop 操作
TEST_F(VectorTest, PushPopTest) {
    // Testing pushEnd
    this->originalVec.pushEnd(1);
    this->stdVec.push_back(1);
    compareVectors(this->originalVec, this->stdVec);

    // Testing pushBegin
    this->originalVec.pushBegin(2);
    this->stdVec.insert(this->stdVec.begin(), 2);
    compareVectors(this->originalVec, this->stdVec);

    // Testing push at middle
    this->originalVec.push(1, 3);
    this->stdVec.insert(this->stdVec.begin() + 1, 3);
    compareVectors(this->originalVec, this->stdVec);

    // Testing popEnd
    int endPop = this->originalVec.popEnd();
    ASSERT_EQ(endPop, 1);
    this->stdVec.pop_back();
    compareVectors(this->originalVec, this->stdVec);

    // Testing popBegin
    int beginPop = this->originalVec.popBegin();
    ASSERT_EQ(beginPop, 2);
    this->stdVec.erase(this->stdVec.begin());
    compareVectors(this->originalVec, this->stdVec);

    // Testing pop at middle
    int middlePop = this->originalVec.pop(0);
    ASSERT_EQ(middlePop, 3);
    this->stdVec.erase(this->stdVec.begin() + 0);
    compareVectors(this->originalVec, this->stdVec);
}

// 测试索引和赋值操作
TEST_F(VectorTest, IndexAndSetTest) {
    this->originalVec.pushEnd(1);
    this->originalVec.pushEnd(2);
    this->stdVec.push_back(1);
    this->stdVec.push_back(2);

    // Testing operator[]
    ASSERT_EQ(this->originalVec[0], this->stdVec[0]);
    ASSERT_EQ(this->originalVec[1], this->stdVec[1]);

    // Testing set
    this->originalVec.set(0, 3);
    this->stdVec[0] = 3;
    compareVectors(this->originalVec, this->stdVec);

    this->originalVec[0] = 4;
    this->stdVec[0] = 4;
    compareVectors(this->originalVec, this->stdVec);
}

// 测试迭代器操作
TEST_F(VectorTest, IteratorTest) {
    this->originalVec.pushEnd(1);
    this->originalVec.pushEnd(2);
    this->originalVec.pushEnd(3);
    this->stdVec.push_back(1);
    this->stdVec.push_back(2);
    this->stdVec.push_back(3);

    // Testing Iterator (begin and end)
    auto* originalBegin = this->originalVec.begins();
    auto stdBegin = this->stdVec.begin();

    for (const auto it = originalBegin; it->isValid(); it->next(), ++stdBegin) {
        ASSERT_EQ(**it, *stdBegin);
    }

    delete originalBegin;
}

// 测试其他基本方法
TEST_F(VectorTest, BasicMethodsTest) {
    // Testing size
    ASSERT_EQ(this->originalVec.size(), this->stdVec.size());

    // Testing push and pop at begin and end
    this->originalVec.pushEnd(1);
    this->stdVec.push_back(1);
    this->originalVec.pushEnd(2);
    this->stdVec.push_back(2);
    compareVectors(this->originalVec, this->stdVec);

    // Testing indexOf
    ASSERT_EQ(this->originalVec.indexOf(1), this->stdVec.size() - 2);
    ASSERT_EQ(this->originalVec.indexOf(2), this->stdVec.size() - 1);

    // Testing equality operator
    const original::vector<int> otherVec = this->originalVec;
    ASSERT_TRUE(this->originalVec == otherVec);
}

// 测试构造函数
TEST_F(VectorTest, ConstructorTest) {
    // Testing with initializer list
    const original::vector vecFromList = {1, 2};
    const std::vector stdVecFromList = {1, 2};
    compareVectors(vecFromList, stdVecFromList);

    // Testing with array
    const original::array arr = {3, 4};
    const original::vector vecFromArray(arr);
    const std::vector stdVecFromArray = {3, 4};
    compareVectors(vecFromArray, stdVecFromArray);
}

// 测试大数据量
TEST_F(VectorTest, LargeDataTest) {
    constexpr size_t dataSize = 1000000;  // 测试 100万数据
    for (size_t i = 0; i < dataSize; ++i) {
        this->originalVec.pushEnd(static_cast<int>(i));
        this->stdVec.push_back(static_cast<int>(i));
    }

    compareVectors(this->originalVec, this->stdVec);
}

// 测试空容器
TEST_F(VectorTest, EmptyContainerTest) {
    ASSERT_EQ(this->originalVec.size(), 0);
    ASSERT_EQ(this->stdVec.size(), 0);

    // Testing pop on empty vector (should throw an exception)
    ASSERT_THROW(this->originalVec.popEnd(), original::noElementError);

    // Testing index on empty vector (should throw an exception)
    ASSERT_THROW(this->originalVec[0], original::outOfBoundError);
}

// 测试单一元素
TEST_F(VectorTest, SingleElementTest) {
    this->originalVec.pushEnd(1);
    this->stdVec.push_back(1);

    // Test if the single element is correct
    ASSERT_EQ(this->originalVec[0], this->stdVec[0]);

    // Test popping the only element
    int poppedValue = this->originalVec.popEnd();
    ASSERT_EQ(poppedValue, 1);
    this->stdVec.pop_back();
    compareVectors(this->originalVec, this->stdVec);
}

// 测试在容器中间插入和弹出
TEST_F(VectorTest, InsertPopMiddleTest) {
    this->originalVec.pushEnd(1);
    this->originalVec.pushEnd(2);
    this->originalVec.pushEnd(3);
    this->stdVec.push_back(1);
    this->stdVec.push_back(2);
    this->stdVec.push_back(3);

    // Insert in the middle
    this->originalVec.push(1, 10);
    this->stdVec.insert(this->stdVec.begin() + 1, 10);
    compareVectors(this->originalVec, this->stdVec);

    // Pop from the middle
    int middlePop = this->originalVec.pop(1);
    ASSERT_EQ(middlePop, 10);
    this->stdVec.erase(this->stdVec.begin() + 1);
    compareVectors(this->originalVec, this->stdVec);
}

// 测试重复元素
TEST_F(VectorTest, DuplicateElementTest) {
    this->originalVec.pushEnd(1);
    this->originalVec.pushEnd(1);
    this->stdVec.push_back(1);
    this->stdVec.push_back(1);

    // Test if the repeated elements are correctly handled
    ASSERT_EQ(this->originalVec[0], this->stdVec[0]);
    ASSERT_EQ(this->originalVec[1], this->stdVec[1]);

    // Test removing duplicate elements
    this->originalVec.popEnd();
    this->stdVec.pop_back();
    compareVectors(this->originalVec, this->stdVec);
}

// 测试容器大小
TEST_F(VectorTest, SizeTest) {
    // Initially both vectors should be empty
    ASSERT_EQ(this->originalVec.size(), 0);
    ASSERT_EQ(this->stdVec.size(), 0);

    // Push elements and check size
    this->originalVec.pushEnd(1);
    this->stdVec.push_back(1);
    ASSERT_EQ(this->originalVec.size(), 1);
    ASSERT_EQ(this->stdVec.size(), 1);

    this->originalVec.pushEnd(2);
    this->stdVec.push_back(2);
    ASSERT_EQ(this->originalVec.size(), 2);
    ASSERT_EQ(this->stdVec.size(), 2);

    // Pop elements and check size
    this->originalVec.popEnd();
    this->stdVec.pop_back();
    ASSERT_EQ(this->originalVec.size(), 1);
    ASSERT_EQ(this->stdVec.size(), 1);

    this->originalVec.popEnd();
    this->stdVec.pop_back();
    ASSERT_EQ(this->originalVec.size(), 0);
    ASSERT_EQ(this->stdVec.size(), 0);
}

// 测试边界访问
TEST_F(VectorTest, BoundaryAccessTest) {
    this->originalVec.pushEnd(10);
    this->stdVec.push_back(10);

    // Testing accessing the last element
    ASSERT_EQ(this->originalVec[0], this->stdVec[0]);
    ASSERT_EQ(this->originalVec.get(0), this->stdVec[0]);

    // Testing out-of-bounds access
    ASSERT_THROW(this->originalVec[1], original::outOfBoundError);
}

// 测试带大小的构造函数
TEST_F(VectorTest, SizeConstructorTest) {
    constexpr size_t testSize = 100;
    original::vector<int> sizedVec(testSize, original::allocator<int>{}, 0);
    std::vector<int> sizedStdVec(testSize);

    ASSERT_EQ(sizedVec.size(), sizedStdVec.size());
    for (size_t i = 0; i < testSize; ++i) {
        ASSERT_EQ(sizedVec[i], sizedStdVec[i]); // 默认构造的元素应该相同
    }
}

// 测试带大小和参数的构造函数
TEST_F(VectorTest, SizeAndArgsConstructorTest) {
    constexpr size_t testSize = 100;
    constexpr int initValue = 42;

    original::vector<int> sizedVec(testSize, original::allocator<int>{}, initValue);
    std::vector<int> sizedStdVec(testSize, initValue);

    ASSERT_EQ(sizedVec.size(), sizedStdVec.size());
    for (size_t i = 0; i < testSize; ++i) {
        ASSERT_EQ(sizedVec[i], sizedStdVec[i]);
    }
}

// 测试data()方法
TEST_F(VectorTest, DataMethodTest) {
    this->originalVec.pushEnd(1);
    this->originalVec.pushEnd(2);
    this->stdVec.push_back(1);
    this->stdVec.push_back(2);

    // data()应该返回第一个元素的引用
    ASSERT_EQ(this->originalVec.data(), this->originalVec[0]);
    ASSERT_EQ(this->originalVec.data(), this->stdVec.front());

    // 修改data()返回的引用应该修改容器中的元素
    this->originalVec.data() = 10;
    this->stdVec.front() = 10;
    compareVectors(this->originalVec, this->stdVec);
}

// 测试makeVector工厂函数
TEST_F(VectorTest, MakeVectorTest) {
    constexpr size_t testSize = 100;
    constexpr int initValue = 42;

    auto madeVec = original::makeVector<int>(testSize, initValue);
    std::vector<int> stdVec(testSize, initValue);

    ASSERT_EQ(madeVec.size(), stdVec.size());
    for (size_t i = 0; i < testSize; ++i) {
        ASSERT_EQ(madeVec[i], stdVec[i]);
    }

    // 测试空vector
    auto emptyVec = original::makeVector<int>(0);
    ASSERT_EQ(emptyVec.size(), 0);
}

// 测试带大小构造函数的边界情况
TEST_F(VectorTest, SizeConstructorEdgeCases) {
    // 测试大小为0
    original::vector<int> zeroVec(0, original::allocator<int>{}, 0);
    ASSERT_EQ(zeroVec.size(), 0);

    // 测试大尺寸
    constexpr size_t largeSize = 1000000;
    original::vector<int> largeVec(largeSize, original::allocator<int>{}, 0);
    ASSERT_EQ(largeVec.size(), largeSize);

    // 测试访问元素
    for (size_t i = 0; i < largeSize; i += largeSize/10) {
        ASSERT_EQ(largeVec[i], int{}); // 默认初始化值
    }
}

// 测试 pushBegin
TEST_F(VectorTest, PushBeginTest) {
    constexpr size_t dataSize = 100000;  // 大数据量测试
    for (size_t i = dataSize; i > 0; --i) {
        this->originalVec.pushBegin(static_cast<int>(i));
        this->stdVec.insert(this->stdVec.begin(), static_cast<int>(i));
    }

    compareVectors(this->originalVec, this->stdVec);
}

// 测试 popEnd
TEST_F(VectorTest, PopEndTest) {
    constexpr size_t dataSize = 100000;  // 大数据量测试
    for (size_t i = 0; i < dataSize; ++i) {
        this->originalVec.pushEnd(static_cast<int>(i));
        this->stdVec.push_back(static_cast<int>(i));
    }

    // 弹出元素并验证
    for (size_t i = dataSize - 1; i > 0; --i) {
        int popValueOriginal = this->originalVec.popEnd();
        int popValueStd = this->stdVec.back();
        ASSERT_EQ(popValueOriginal, popValueStd);
        this->stdVec.pop_back();
    }

    compareVectors(this->originalVec, this->stdVec);
}

// 测试 popBegin
TEST_F(VectorTest, PopBeginTest) {
    constexpr size_t dataSize = 100000;  // 大数据量测试
    for (size_t i = 0; i < dataSize; ++i) {
        this->originalVec.pushEnd(static_cast<int>(i));
        this->stdVec.push_back(static_cast<int>(i));
    }

    // 弹出元素并验证
    for (size_t i = 0; i < dataSize; ++i) {
        int popValueOriginal = this->originalVec.popBegin();
        int popValueStd = this->stdVec.front();
        ASSERT_EQ(popValueOriginal, popValueStd);
        this->stdVec.erase(this->stdVec.begin());
    }

    compareVectors(this->originalVec, this->stdVec);
}

// 测试 push（插入到容器中间）
TEST_F(VectorTest, PushTestMiddle) {
    constexpr size_t dataSize = 10000;  // 大数据量测试
    for (size_t i = 0; i < dataSize; ++i) {
        this->originalVec.pushEnd(static_cast<int>(i));
        this->stdVec.push_back(static_cast<int>(i));
    }

    // 在中间插入元素
    for (size_t i = 0; i < dataSize / 2; ++i) {
        this->originalVec.push(dataSize / 2, static_cast<int>(i));
        this->stdVec.insert(this->stdVec.begin() + dataSize / 2, static_cast<int>(i));
    }

    compareVectors(this->originalVec, this->stdVec);
}

// 测试插入和弹出混合操作
TEST_F(VectorTest, MixedPushPopTest) {
    constexpr size_t dataSize = 10000;  // 大数据量测试

    // 在末尾插入数据
    for (size_t i = 0; i < dataSize; ++i) {
        this->originalVec.pushEnd(static_cast<int>(i));
        this->stdVec.push_back(static_cast<int>(i));
    }

    // 在开始插入数据
    for (size_t i = dataSize; i > 0; --i) {
        this->originalVec.pushBegin(static_cast<int>(i));
        this->stdVec.insert(this->stdVec.begin(), static_cast<int>(i));
    }

    // 弹出数据
    for (size_t i = 0; i < dataSize; ++i) {
        int popValueOriginalEnd = this->originalVec.popEnd();
        int popValueStdEnd = this->stdVec.back();
        ASSERT_EQ(popValueOriginalEnd, popValueStdEnd);
        this->stdVec.pop_back();

        int popValueOriginalBegin = this->originalVec.popBegin();
        int popValueStdBegin = this->stdVec.front();
        ASSERT_EQ(popValueOriginalBegin, popValueStdBegin);
        this->stdVec.erase(this->stdVec.begin());
    }

    compareVectors(this->originalVec, this->stdVec);
}

TEST_F(VectorTest, MoveAndCopyTest) {
    constexpr int dataSize = 100;
    auto vec = original::vector<int>();
    for (int i = 0; i < dataSize; ++i) {
        vec.pushEnd(i);
    }
    for (original::u_integer i = 0; i < dataSize; ++i) {
        EXPECT_EQ(vec[i], i);
    }
    this->originalVec = vec;
    EXPECT_EQ(this->originalVec, vec);
    const original::vector<int> src = std::move(this->originalVec);
    EXPECT_EQ(this->originalVec.size(), 0);
    EXPECT_EQ(this->originalVec, original::vector<int>{});
    EXPECT_EQ(vec, src);
}

// 自定义 move-only 类型
struct MoveOnly {
    int value;
    explicit MoveOnly(const int v = 0) : value(v) {}
    MoveOnly(const MoveOnly&) = delete;            // 禁止拷贝
    MoveOnly& operator=(const MoveOnly&) = delete; // 禁止拷贝赋值
    MoveOnly(MoveOnly&& other) noexcept : value(other.value) { other.value = 0; }
    MoveOnly& operator=(MoveOnly&& other) noexcept {
        if (this != &other) { value = other.value; other.value = 0; }
        return *this;
    }
    bool operator==(const MoveOnly& other) const { return value == other.value; }
    [[nodiscard]] int val() const noexcept { return value; }
};

TEST(VectorMoveOnlyTest, PushBackAndAccess) {
    original::vector<MoveOnly> vec;
    vec.pushEnd(MoveOnly(10));
    vec.pushEnd(MoveOnly(20));

    ASSERT_EQ(vec[0].value, 10);
    ASSERT_EQ(vec[1].value, 20);
}

TEST(VectorMoveOnlyTest, MoveVector) {
    original::vector<MoveOnly> vec1;
    vec1.pushEnd(MoveOnly(5));
    vec1.pushEnd(MoveOnly(15));

    original::vector<MoveOnly> vec2 = std::move(vec1);

    ASSERT_EQ(vec2[0].value, 5);
    ASSERT_EQ(vec2[1].value, 15);
    ASSERT_EQ(vec1.size(), 0);  // vec1被移动后size应为0 // NOLINT: Test move
}

TEST(VectorMoveOnlyTest, ResizeSimulation) {
    // 测试pushEnd多次触发扩容
    original::vector<MoveOnly> vec;
    for (int i = 0; i < 10; ++i) {
        vec.pushEnd(MoveOnly(i));
    }

    for (int i = 0; i < 10; ++i) {
        ASSERT_EQ(vec[i].value, i);
    }
}
