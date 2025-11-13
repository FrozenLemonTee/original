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

TEST_F(VectorTest, ConstructFromArrayView) {
    // 测试从arrayView构造vector
    int rawData[6] = {1, 2, 3, 4, 5, 6};
    original::arrayView<int> view(rawData);
    original::vector<int> vec(view);

    EXPECT_EQ(vec.size(), 6);
    for (int i = 0; i < 6; ++i) {
        EXPECT_EQ(vec[i], rawData[i]);
    }

    // 验证修改原始数据不影响vector（应该是独立拷贝）
    rawData[0] = 100;
    EXPECT_EQ(vec[0], 1); // vector应该保持原始值
}

TEST_F(VectorTest, ConstructFromArrayViewWithAllocator) {
    // 测试从arrayView构造vector并指定分配器
    std::string strings[3] = {"hello", "world", "test"};
    const original::arrayView view(strings, 3);
    original::vector<std::string, original::objPoolAllocator<std::string>> vec(view);

    EXPECT_EQ(vec.size(), 3);
    for (int i = 0; i < 3; ++i) {
        EXPECT_EQ(vec[i], strings[i]);
    }

    // 验证可以修改vector中的元素
    vec[1] = "modified";
    EXPECT_EQ(vec[1], "modified");
    EXPECT_EQ(strings[1], "world"); // 原始arrayView数据保持不变
}

TEST_F(VectorTest, ConstructFromEmptyArrayView) {
    // 测试从空arrayView构造vector
    original::arrayView<int> emptyView(nullptr, 0);
    original::vector<int> vec(emptyView);

    EXPECT_EQ(vec.size(), 0);
    EXPECT_TRUE(vec.empty());

    // 验证不能访问任何元素
    EXPECT_THROW(vec[0], original::outOfBoundError);
}

TEST_F(VectorTest, ViewMethod) {
    // 测试view()方法返回arrayView
    original::vector<int> vec = {10, 20, 30, 40, 50};
    original::arrayView<int> view = vec.view();

    EXPECT_EQ(view.count(), vec.size());

    // 验证view的内容与vector一致
    for (original::u_integer i = 0; i < vec.size(); ++i) {
        EXPECT_EQ(view[i], vec[i]);
    }

    // 通过view修改vector中的元素
    view[2] = 300;
    EXPECT_EQ(vec[2], 300);

    // 验证view的边界检查
    EXPECT_THROW(view.get(vec.size()), original::outOfBoundError);
}

TEST_F(VectorTest, ViewMethodOnEmptyVector) {
    // 测试空vector的view()方法
    original::vector<int> emptyVec;
    original::arrayView<int> view = emptyVec.view();

    EXPECT_EQ(view.count(), 0);
    EXPECT_TRUE(view.empty());
    EXPECT_EQ(view.data(), &emptyVec.data());
}

TEST_F(VectorTest, SliceMethod) {
    // 测试slice方法
    original::vector<int> vec = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    // 获取中间切片
    original::arrayView<int> slice1 = vec.slice(2, 7);
    EXPECT_EQ(slice1.count(), 5);
    EXPECT_EQ(slice1[0], 2);
    EXPECT_EQ(slice1[4], 6);

    // 获取开头切片
    original::arrayView<int> slice2 = vec.slice(0, 3);
    EXPECT_EQ(slice2.count(), 3);
    EXPECT_EQ(slice2[0], 0);
    EXPECT_EQ(slice2[2], 2);

    // 获取结尾切片
    original::arrayView<int> slice3 = vec.slice(7, 10);
    EXPECT_EQ(slice3.count(), 3);
    EXPECT_EQ(slice3[0], 7);
    EXPECT_EQ(slice3[2], 9);
}

TEST_F(VectorTest, SliceMethodEmptyRange) {
    // 测试空范围的slice
    original::vector<int> vec = {1, 2, 3, 4, 5};

    // start == end 应该返回空view
    original::arrayView<int> emptySlice = vec.slice(2, 2);
    EXPECT_EQ(emptySlice.count(), 0);
    EXPECT_TRUE(emptySlice.empty());

    // start > end 应该返回空view
    original::arrayView<int> reverseSlice = vec.slice(3, 2);
    EXPECT_EQ(reverseSlice.count(), 0);
    EXPECT_TRUE(reverseSlice.empty());
}

TEST_F(VectorTest, SliceMethodOutOfBounds) {
    // 测试slice方法的边界检查
    original::vector<int> vec = {1, 2, 3, 4, 5};

    // start越界
    EXPECT_THROW(vec.slice(10, 12), original::outOfBoundError);

    // end越界
    EXPECT_THROW(vec.slice(2, 10), original::outOfBoundError);

    // start和end都越界
    EXPECT_THROW(vec.slice(10, 15), original::outOfBoundError);
}

TEST_F(VectorTest, SliceMethodModification) {
    // 测试通过slice修改原vector
    original::vector<int> vec = {1, 2, 3, 4, 5, 6};
    original::arrayView<int> slice = vec.slice(1, 4);  // 元素: 2, 3, 4

    // 通过slice修改元素
    slice[0] = 20;
    slice[1] = 30;
    slice[2] = 40;

    // 验证原vector被修改
    EXPECT_EQ(vec[1], 20);
    EXPECT_EQ(vec[2], 30);
    EXPECT_EQ(vec[3], 40);

    // 原vector其他元素保持不变
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[4], 5);
    EXPECT_EQ(vec[5], 6);
}

TEST_F(VectorTest, ArrayViewIterationFromVector) {
    // 测试通过vector的view进行迭代
    original::vector<int> vec = {0, 10, 20, 30, 40, 50};
    original::arrayView<int> view = vec.view();

    // 测试迭代器遍历
    int sum = 0;
    for (auto it = view.begin(); it != view.end(); ++it) { // NOLINT: traverse test
        sum += *it;
    }
    EXPECT_EQ(sum, 150);  // 0+10+20+30+40+50=150

    // 测试范围for循环
    sum = 0;
    for (int value : view) {
        sum += value;
    }
    EXPECT_EQ(sum, 150);
}

TEST_F(VectorTest, SliceIterationFromVector) {
    // 测试通过vector的slice进行迭代
    original::vector<int> vec = {0, 10, 20, 30, 40, 50};
    original::arrayView<int> slice = vec.slice(1, 5);  // 元素: 10, 20, 30, 40

    // 测试迭代器遍历
    int sum = 0;
    for (auto it = slice.begin(); it != slice.end(); ++it) { // NOLINT: traverse test
        sum += *it;
    }
    EXPECT_EQ(sum, 100);  // 10+20+30+40=100

    // 测试范围for循环
    sum = 0;
    for (const int& value : slice) {
        sum += value;
    }
    EXPECT_EQ(sum, 100);
}

TEST_F(VectorTest, VectorAndArrayViewIntegration) {
    // 综合测试vector和arrayView的互操作性
    int rawData[8] = {1, 2, 3, 4, 5, 6, 7, 8};

    // 从arrayView构造vector
    original::arrayView<int> view1(rawData, 8);
    original::vector<int> vec1(view1);

    // 从vector获取view
    original::arrayView<int> view2 = vec1.view();

    // 从view构造新的vector
    original::vector<int> vec2(view2);

    // 验证三个容器内容一致
    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(vec1[i], rawData[i]);
        EXPECT_EQ(view2[i], rawData[i]);
        EXPECT_EQ(vec2[i], rawData[i]);
    }

    // 验证它们是独立的拷贝
    vec1[0] = 100;
    EXPECT_EQ(view2[0], 100); // view2看到vec1的修改
    EXPECT_EQ(vec2[0], 1);    // vec2保持原始值
}

TEST_F(VectorTest, MoveSemanticsWithArrayView) {
    // 测试移动语义与arrayView的交互
    original::vector<int> vec1 = {10, 20, 30};
    original::arrayView<int> view = vec1.view();

    // 移动构造
    original::vector<int> vec2 = std::move(vec1);

    // view应该仍然有效，指向vec2的数据
    EXPECT_EQ(view.count(), 3);
    EXPECT_EQ(view[0], 10);
    EXPECT_EQ(view[1], 20);
    EXPECT_EQ(view[2], 30);

    // 通过view修改vec2
    view[1] = 200;
    EXPECT_EQ(vec2[1], 200);
}

TEST_F(VectorTest, ComplexTypeWithArrayView) {
    // 测试复杂类型的arrayView互转
    struct Person {
        std::string name;
        int age;
        bool operator==(const Person& other) const {
            return name == other.name && age == other.age;
        }
    };

    Person people[3] = {
        {"Alice", 25},
        {"Bob", 30},
        {"Charlie", 35}
    };

    original::arrayView<Person> view(people, 3);
    original::vector<Person> vec(view);

    EXPECT_EQ(vec.size(), 3);
    EXPECT_TRUE((vec[0] == Person{"Alice", 25}));
    EXPECT_TRUE((vec[1] == Person{"Bob", 30}));
    EXPECT_TRUE((vec[2] == Person{"Charlie", 35}));

    // 通过view修改原数据
    view[1] = Person{"Robert", 31};
    EXPECT_TRUE((people[1] == Person{"Robert", 31}));
    EXPECT_TRUE((vec[1] == Person{"Bob", 30})); // vector应该保持原始值
}

TEST_F(VectorTest, PerformanceTestWithArrayView) {
    // 性能测试：使用arrayView进行批量操作
    constexpr original::u_integer dataSize = 100000;
    original::vector<int> vec;

    // 填充数据
    for (original::u_integer i = 0; i < dataSize; ++i) {
        vec.pushEnd(static_cast<int>(i));
    }

    // 使用view进行批量操作
    original::arrayView<int> view = vec.view();

    // 批量修改
    for (original::u_integer i = 0; i < view.count(); ++i) {
        view[i] = view[i] * 2;
    }

    // 验证修改
    for (original::u_integer i = 0; i < dataSize; ++i) {
        EXPECT_EQ(vec[i], static_cast<int>(i * 2));
    }
}

// ============ 新增测试用例：与array的互操作性 ============

TEST_F(VectorTest, ConstructFromArray) {
    // 测试从array构造vector
    original::array<int> arr = {1, 2, 3, 4, 5};
    original::vector<int> vec(arr);

    EXPECT_EQ(vec.size(), 5);
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(vec[i], arr[i]);
    }

    // 验证修改array不影响vector
    arr[0] = 100;
    EXPECT_EQ(vec[0], 1); // vector应该保持原始值
}

TEST_F(VectorTest, VectorToArrayConversion) {
    // 测试vector转换为array（通过中间步骤）
    original::vector<int> vec = {10, 20, 30, 40, 50};

    // 通过arrayView转换为array
    original::arrayView<int> view = vec.view();
    original::array<int> arr(view);

    EXPECT_EQ(arr.size(), vec.size());
    for (original::u_integer i = 0; i < vec.size(); ++i) {
        EXPECT_EQ(arr[i], vec[i]);
    }
}
