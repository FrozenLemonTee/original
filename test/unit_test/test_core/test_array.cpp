#include <gtest/gtest.h>
#include <array>  // std::array
#include "array.h"  // original::array
#include "error.h"

namespace original {
    template <typename TYPE, std::size_t N>
    bool compareArrays(const array<TYPE>& a, const std::array<TYPE, N>& b) {
        if (a.size() != N) return false;
        for (std::size_t i = 0; i < N; ++i) {
            if (a.get(i) != b[i]) return false;
        }
        return true;
    }

class ArrayTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initial setup before each test
    }

    void TearDown() override {
        // Cleanup after each test
    }
};

TEST(ArrayTest, ConstructorAndSize) {
    const array<int> arr1(10);  // size = 10
    constexpr std::array<int, 10> stdArr1{};
    EXPECT_EQ(arr1.size(), stdArr1.size());

    const array arr2 = {1, 2, 3, 4, 5};
    const std::array stdArr2 = {1, 2, 3, 4, 5};
    EXPECT_TRUE(original::compareArrays(arr2, stdArr2));

    const array<int> arr3;
    constexpr std::array<int, 0> stdArr3{};
    EXPECT_EQ(arr3.size(), stdArr3.size());
}

TEST(ArrayTest, IndexingAndDataAccess) {
    array arr = {1, 2, 3, 4, 5};
    std::array stdArr = {1, 2, 3, 4, 5};

    EXPECT_EQ(arr[0], stdArr[0]);
    EXPECT_EQ(arr[4], stdArr[4]);

    arr.set(2, 100);
    stdArr[2] = 100;
    EXPECT_EQ(arr[2], stdArr[2]);
    EXPECT_TRUE(original::compareArrays(arr, stdArr));

    EXPECT_THROW(arr.get(10), outOfBoundError);
    EXPECT_THROW(arr[10], outOfBoundError);
}

TEST(ArrayTest, CopyConstructorAndAssignmentOperator) {
    const array arr1 = {1, 2, 3, 4, 5};
    array<int> arr2 = arr1;
    arr2[2] += 1;
    const std::array stdArr = {1, 2, 3, 4, 5};
    const std::array stdArr2 = {1, 2, 4, 4, 5};

    EXPECT_TRUE(original::compareArrays(arr1, stdArr));
    EXPECT_TRUE(original::compareArrays(arr2, stdArr2));

    const array<int>& arr3 = arr1;  // Assignment operator
    EXPECT_TRUE(original::compareArrays(arr3, stdArr));
}

TEST(ArrayTest, EqualityOperator) {
    const array arr1 = {1, 2, 3, 4, 5};
    const array arr2 = {1, 2, 3, 4, 5};
    const array arr3 = {5, 4, 3, 2, 1};

    EXPECT_TRUE(arr1 == arr2);
    EXPECT_FALSE(arr1 == arr3);
}

TEST(ArrayTest, IndexOf) {
    const array arr = {10, 20, 30, 40, 50};

    EXPECT_EQ(arr.indexOf(30), 2);
    EXPECT_EQ(arr.indexOf(100), arr.size());
}

TEST(ArrayTest, Iterator) {
    array arr = {10, 20, 30, 40, 50};

    const auto beginIt = arr.begins();
    EXPECT_EQ(beginIt->className(), "array::Iterator");
    const auto endIt = arr.ends();
    EXPECT_EQ(endIt->className(), "array::Iterator");
    EXPECT_GE(*endIt - *beginIt, 0);
    EXPECT_LE(*beginIt - *endIt, 0);


    delete beginIt;
    delete endIt;
}

TEST(ArrayTest, CloneIterator) {
    const array arr = {10, 20, 30, 40, 50};

    const auto it = arr.begins();
    const auto clonedIt = it->clone();
    const auto endIt = arr.ends();
    EXPECT_EQ(clonedIt->className(), "array::Iterator");
    EXPECT_EQ(it->atNext(endIt), clonedIt->atNext(endIt));

    delete endIt;
    delete clonedIt;
    delete it;
}

TEST(ArrayTest, ClassName) {
    const array<int> arr;
    EXPECT_EQ(arr.className(), "array");
}

TEST(ArrayTest, Destruction) {
    // Test if the array is destructed properly
    {
        array arr = {1, 2, 3};
    } // arr goes out of scope here
    EXPECT_TRUE(true);  // No crash or memory leak
}

TEST(ArrayTest, CopyAndMoveSemantics) {
    array arr1 = {1, 2, 3, 4, 5};
    auto arr2 = std::move(arr1);  // Move constructor
    EXPECT_EQ(arr2.size(), 5);
    EXPECT_EQ(arr1.size(), 0); // NOLINT: Test move
    EXPECT_THROW(arr1[0], outOfBoundError);  // arr1 should be in an invalid state

    array arr3 = {10, 20, 30};
    arr3 = std::move(arr2);  // Move assignment
    EXPECT_EQ(arr3.size(), 5);
    EXPECT_THROW(arr2[0], outOfBoundError);  // arr2 should be in an invalid state // NOLINT: Test move
    EXPECT_EQ(arr3[0], 1);
    EXPECT_EQ(arr3[1], 2);
}

TEST(ArrayTest, IndexOutOfBound) {
    array arr = {1, 2, 3, 4, 5};

    // Test indexOutOfBound method (or similar boundary check)
    EXPECT_NO_THROW(arr.get(0));   // Valid index
    EXPECT_NO_THROW(arr.get(4));   // Valid index (last element)
    EXPECT_THROW(arr.get(5), outOfBoundError);  // Invalid index
}

TEST(ArrayTest, IndexOfMethodWithBounds) {
    array<int> arr = {10, 20, 30, 40, 50};

    // Test indexOf method for valid index
    EXPECT_EQ(arr.indexOf(30), 2);   // 30 is at index 2
    EXPECT_EQ(arr.indexOf(50), 4);   // 50 is at index 4

    // Test indexOf method for non-existent element
    EXPECT_EQ(arr.indexOf(100), arr.size()); // 100 not in array, should return size
}

TEST(ArrayTest, MoveConstructorAndAssignment)
{
    array arr1 = {1, 2, 3, 4, 5};

    // Test move constructor
    array arr2 = std::move(arr1);
    EXPECT_EQ(arr2.size(), 5);
    EXPECT_EQ(arr1.size(), 0);  // arr1 should be empty after move  // NOLINT: Test move
    EXPECT_THROW(arr1[0], outOfBoundError);  // arr1 should be in an invalid state

    // Test move assignment
    array arr3 = {10, 20, 30};
    arr3 = std::move(arr2);
    EXPECT_EQ(arr3.size(), 5);
    EXPECT_THROW(arr2[0], outOfBoundError);  // NOLINT: Test move
}

TEST(ArrayTest, ToString) {
    const array arr = {1, 2, 3, 4, 5};

    // toString返回的格式是 "array(1, 2, 3, 4, 5)"
    const std::string expected = "array(1, 2, 3, 4, 5)";

    // 检查toString的输出
    EXPECT_EQ(arr.toString(false), expected);
}

    TEST(ArrayTest, ForEachTest) {
    array<int> array(5);
    std::array<int, 5> stdArr{};
    int i = 0;
    for (auto &e : array)
    {
        e = i;
        i += 1;
    }
    int j = 0;
    for (auto &e : stdArr)
    {
        e = j;
        j += 1;
    }

    int sum_arr = 0;
    array.forEach([&sum_arr](const auto& value) {
        sum_arr += value;
    });

    int sum_stdArr = 0;
    for (const auto &e : stdArr)
    {
        sum_stdArr += e;
    }

    // 检查 forEach 是否正确遍历并累加元素
    EXPECT_EQ(sum_arr, sum_stdArr);
    EXPECT_TRUE(compareArrays(array, stdArr));
}

    // 测试拷贝构造函数和拷贝赋值运算符
    TEST(ArrayTest, CopyConstructorAndAssignmentWithPointer) {
    array<int*> arr1(3);
    int a = 1, b = 2, c = 3;
    arr1.set(0, &a);
    arr1.set(1, &b);
    arr1.set(2, &c);

    // 拷贝构造
    array<int*> arr2 = arr1;
    EXPECT_EQ(*arr2[0], 1);
    EXPECT_EQ(*arr2[1], 2);
    EXPECT_EQ(*arr2[2], 3);

    // 修改 arr2 中的元素
    int d = 4;
    arr2[1] = &d;
    EXPECT_EQ(*arr2[1], 4);  // arr2[1] 指向 d
    EXPECT_EQ(*arr1[1], 2);   // arr1[1] 不应受影响

    // 拷贝赋值运算符
    array<int*> arr3 = arr1;
    EXPECT_EQ(*arr3[0], 1);
    EXPECT_EQ(*arr3[1], 2);
    EXPECT_EQ(*arr3[2], 3);
}

    // 测试移动构造函数和移动赋值运算符
    TEST(ArrayTest, MoveConstructorAndAssignmentWithPointer) {
    array<int*> arr1(3);
    int a = 1, b = 2, c = 3;
    arr1.set(0, &a);
    arr1.set(1, &b);
    arr1.set(2, &c);

    // 移动构造
    array<int*> arr2 = std::move(arr1);
    EXPECT_EQ(*arr2[0], 1);
    EXPECT_EQ(*arr2[1], 2);
    EXPECT_EQ(*arr2[2], 3);

    // arr1 变为空，不能访问它的元素
    EXPECT_THROW(arr1[0], outOfBoundError); // NOLINT: Test move

    // 移动赋值运算符
    array<int*> arr3(2);
    int d = 4, e = 5;
    arr3[0] = &d;
    arr3[1] = &e;

    arr3 = std::move(arr2);
    EXPECT_EQ(*arr3[0], 1);
    EXPECT_EQ(*arr3[1], 2);
    EXPECT_EQ(*arr3[2], 3);

    // arr2 变为空，不能访问它的元素
    EXPECT_THROW(arr2[0], outOfBoundError); // NOLINT: Test move
}

    // 测试析构函数是否正确释放资源
    TEST(ArrayTest, DestructionWithPointer) {
    array<int*> arr1(3);
    int a = 1, b = 2, c = 3;
    arr1.set(0, &a);
    arr1.set(1, &b);
    arr1.set(2, &c);

    // 让 arr 超出作用域，检查是否有内存泄漏
    {
        array<int*> arr2 = arr1; // NOLINT: Test copy
        EXPECT_EQ(arr1, arr2);
    }  // arr2 超出作用域，检查是否无崩溃或内存泄漏
    EXPECT_TRUE(true);
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

TEST(ArrayMoveOnlyTest, ConstructFromInitializerList) {
    array arr = { MoveOnly(1), MoveOnly(2), MoveOnly(3) };
    EXPECT_EQ(arr.size(), 3);
    EXPECT_EQ(arr[0].value, 1);
    EXPECT_EQ(arr[1].value, 2);
    EXPECT_EQ(arr[2].value, 3);
}

TEST(ArrayMoveOnlyTest, MoveConstructor) {
    array arr1 = { MoveOnly(10), MoveOnly(20) };
    array arr2(std::move(arr1));

    EXPECT_EQ(arr2.size(), 2);
    EXPECT_EQ(arr2[0].value, 10);
    EXPECT_EQ(arr2[1].value, 20);

    // arr1 应该处于有效但未指定状态，size 应该为 0
    EXPECT_EQ(arr1.size(), 0); // NOLINT: Test move
}

TEST(ArrayMoveOnlyTest, MoveAssignment) {
    array arr1 = { MoveOnly(100), MoveOnly(200) };
    array<MoveOnly> arr2 = std::move(arr1);

    EXPECT_EQ(arr2.size(), 2);
    EXPECT_EQ(arr2[0].value, 100);
    EXPECT_EQ(arr2[1].value, 200);

    EXPECT_EQ(arr1.size(), 0); // NOLINT: Test move
}

TEST(ArrayMoveOnlyTest, ElementAccessAndModification) {
    array<MoveOnly> arr(2);
    arr.set(0, MoveOnly(42));
    arr.set(1, MoveOnly(99));

    EXPECT_EQ(arr[0].value, 42);
    EXPECT_EQ(arr[1].value, 99);

    // move 元素出数组
    MoveOnly temp = std::move(arr[0]);
    EXPECT_EQ(temp.value, 42);
}

TEST(ArrayMoveOnlyTest, IteratorAccess) {
    array arr = { MoveOnly(5), MoveOnly(15), MoveOnly(25) };
    auto it = arr.first();
    EXPECT_EQ(it.get().val(), 5);
    it += 1;
    EXPECT_EQ(it.get().val(), 15);
    it += 1;
    EXPECT_EQ(it.get().val(), 25);
}

}  // namespace original
