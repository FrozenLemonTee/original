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

    template <typename TYPE, std::size_t N>
    bool compareArrayViewWithRawArray(const arrayView<TYPE>& view, const TYPE (&rawArr)[N]) {
        if (view.count() != N) return false;
        for (std::size_t i = 0; i < N; ++i) {
            if (view[i] != rawArr[i]) return false;
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
    EXPECT_EQ(arr1.size(), 0);
    EXPECT_THROW(arr1[0], outOfBoundError);  // arr1 should be in an invalid state

    array arr3 = {10, 20, 30};
    arr3 = std::move(arr2);  // Move assignment
    EXPECT_EQ(arr3.size(), 5);
    EXPECT_THROW(arr2[0], outOfBoundError);  // arr2 should be in an invalid state
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
    EXPECT_EQ(arr1.size(), 0);  // arr1 should be empty after move
    EXPECT_THROW(arr1[0], outOfBoundError);  // arr1 should be in an invalid state

    // Test move assignment
    array arr3 = {10, 20, 30};
    arr3 = std::move(arr2);
    EXPECT_EQ(arr3.size(), 5);
    EXPECT_THROW(arr2[0], outOfBoundError);
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
    EXPECT_THROW(arr1[0], outOfBoundError);

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
    EXPECT_THROW(arr2[0], outOfBoundError);
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
        array<int*> arr2 = arr1; // NOLINT: copy test
        EXPECT_EQ(arr1, arr2);
    }  // arr2 超出作用域，检查是否无崩溃或内存泄漏
    EXPECT_TRUE(true);
}

TEST(ArrayTest, ConstructFromStaticArray) {
    // 测试从静态数组构造
    int rawArray[5] = {1, 2, 3, 4, 5};
    array<int> arr(rawArray);

    EXPECT_EQ(arr.size(), 5);
    EXPECT_TRUE(compareArrayViewWithRawArray(arr.view(), rawArray));

    // 验证元素访问
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(arr[i], rawArray[i]);
    }
}

TEST(ArrayTest, ConstructFromStaticArrayWithAllocator) {
    // 测试从静态数组构造并指定分配器
    double rawArray[3] = {1.1, 2.2, 3.3};
    array<double> arr(rawArray, allocator<double>{});

    EXPECT_EQ(arr.size(), 3);
    EXPECT_TRUE(compareArrayViewWithRawArray(arr.view(), rawArray));

    // 修改原始数组，验证array是独立的拷贝
    rawArray[1] = 99.9;
    EXPECT_EQ(arr[1], 2.2); // arr应该保持原始值
}

TEST(ArrayTest, ConstructFromEmptyStaticArray) {
    // 测试从空静态数组构造
    int emptyArray[0] = {};
    const array<int> arr(emptyArray);

    EXPECT_EQ(arr.size(), 0);
    EXPECT_TRUE(arr.empty());
}

TEST(ArrayTest, ConstructFromConstStaticArray) {
    // 测试从const静态数组构造
    constexpr int constArray[4] = {10, 20, 30, 40};
    array<int> arr(constArray);

    EXPECT_EQ(arr.size(), 4);
    EXPECT_TRUE(compareArrayViewWithRawArray(arr.view(), constArray));
}

// ============ 新增测试用例：arrayView互转方法 ============

TEST(ArrayTest, ConstructFromArrayView) {
    // 测试从arrayView构造array
    int data[6] = {1, 2, 3, 4, 5, 6};
    arrayView<int> view(data, 6);
    array<int> arr(view);

    EXPECT_EQ(arr.size(), 6);
    EXPECT_TRUE(compareArrayViewWithRawArray(arr.view(), data));

    // 修改原始数据，验证array是独立的拷贝
    data[0] = 100;
    EXPECT_EQ(arr[0], 1); // arr应该保持原始值
}

TEST(ArrayTest, ConstructFromArrayViewWithAllocator) {
    // 测试从arrayView构造array并指定分配器
    std::string strings[3] = {"hello", "world", "test"};
    arrayView<std::string> view(strings, 3);
    array<std::string> arr(view, allocator<std::string>{});

    EXPECT_EQ(arr.size(), 3);
    for (int i = 0; i < 3; ++i) {
        EXPECT_EQ(arr[i], strings[i]);
    }
}

TEST(ArrayTest, ConstructFromEmptyArrayView) {
    // 测试从空arrayView构造
    arrayView<int> emptyView(nullptr, 0);
    array<int> arr(emptyView);

    EXPECT_EQ(arr.size(), 0);
    EXPECT_TRUE(arr.empty());
}

TEST(ArrayTest, ViewMethod) {
    // 测试view()方法返回arrayView
    array<int> arr = {10, 20, 30, 40, 50};
    arrayView<int> view = arr.view();

    EXPECT_EQ(view.count(), arr.size());

    // 验证view的内容与array一致
    for (u_integer i = 0; i < arr.size(); ++i) {
        EXPECT_EQ(view[i], arr[i]);
    }

    // 通过view修改array中的元素
    view[2] = 300;
    EXPECT_EQ(arr[2], 300);

    // 验证view的边界检查
    EXPECT_THROW(view.get(arr.size()), outOfBoundError);
}

TEST(ArrayTest, ViewMethodOnEmptyArray) {
    // 测试空array的view()方法
    array<int> emptyArr;
    arrayView<int> view = emptyArr.view();

    EXPECT_EQ(view.count(), 0);
    EXPECT_TRUE(view.empty());
    EXPECT_EQ(view.data(), nullptr);
}

TEST(ArrayTest, ArrayViewIteration) {
    // 测试通过arrayView进行迭代
    array<int> arr = {1, 3, 5, 7, 9};
    arrayView<int> view = arr.view();

    int sum = 0;
    for (auto it = view.begin(); it != view.end(); ++it) { // NOLINT: explicit iterator testing
        sum += *it;
    }
    EXPECT_EQ(sum, 25); // 1+3+5+7+9=25

    // 测试范围for循环
    sum = 0;
    for (const int value : view) {
        sum += value;
    }
    EXPECT_EQ(sum, 25);
}

TEST(ArrayTest, ArrayViewModification) {
    // 测试通过arrayView修改array元素
    array<int> arr = {1, 2, 3, 4, 5};
    arrayView<int> view = arr.view();

    // 通过view修改元素
    for (u_integer i = 0; i < view.count(); ++i) {
        view[i] = view[i] * 2;
    }

    // 验证array中的元素也被修改
    EXPECT_EQ(arr[0], 2);
    EXPECT_EQ(arr[1], 4);
    EXPECT_EQ(arr[2], 6);
    EXPECT_EQ(arr[3], 8);
    EXPECT_EQ(arr[4], 10);
}

TEST(ArrayTest, ArrayViewSubview) {
    // 测试arrayView的子视图功能
    array<int> arr = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    arrayView<int> view = arr.view();

    // 获取子视图
    auto subview = view.subview(2, 5); // 从索引2开始，取5个元素

    EXPECT_EQ(subview.count(), 5);
    EXPECT_EQ(subview[0], 3);
    EXPECT_EQ(subview[4], 7);

    // 修改子视图中的元素
    subview[0] = 300;
    EXPECT_EQ(arr[2], 300);
}

// ============ 综合测试用例 ============

TEST(ArrayTest, ArrayAndArrayViewIntegration) {
    // 综合测试array和arrayView的互操作性
    int rawData[8] = {1, 2, 3, 4, 5, 6, 7, 8};

    // 从静态数组构造array
    array<int> arr1(rawData);

    // 从array获取view
    arrayView<int> view1 = arr1.view();

    // 从view构造新的array
    array<int> arr2(view1);

    // 验证三个容器内容一致
    EXPECT_TRUE(compareArrayViewWithRawArray(arr1.view(), rawData));
    EXPECT_TRUE(compareArrayViewWithRawArray(view1, rawData));
    EXPECT_TRUE(compareArrayViewWithRawArray(arr2.view(), rawData));

    // 验证它们是独立的拷贝
    arr1[0] = 100;
    EXPECT_EQ(view1[0], 100); // view1看到arr1的修改
    EXPECT_EQ(arr2[0], 1);    // arr2保持原始值
}

TEST(ArrayTest, MoveSemanticsWithArrayView) {
    // 测试移动语义与arrayView的交互
    array<int> arr1 = {10, 20, 30};
    arrayView<int> view = arr1.view();

    // 移动构造
    array<int> arr2 = std::move(arr1);

    // view应该仍然有效，指向arr2的数据
    EXPECT_EQ(view.count(), 3);
    EXPECT_EQ(view[0], 10);
    EXPECT_EQ(view[1], 20);
    EXPECT_EQ(view[2], 30);

    // 通过view修改arr2
    view[1] = 200;
    EXPECT_EQ(arr2[1], 200);
}

// 测试边界情况和异常
TEST(ArrayTest, ArrayViewBoundaryCases) {
    array<int> arr = {1, 2, 3};
    arrayView<int> view = arr.view();

    // 测试有效的边界访问
    EXPECT_NO_THROW(view[0]);
    EXPECT_NO_THROW(view[2]);

    // 测试无效的边界访问
    EXPECT_THROW(view.get(3), outOfBoundError);
    EXPECT_THROW(view.get(100), outOfBoundError);

    // 测试空的arrayView
    array<int> emptyArr;
    arrayView<int> emptyView = emptyArr.view();
    EXPECT_THROW(emptyView.get(0), outOfBoundError);
}

TEST(ArrayTest, SliceBasicFunctionality) {
    // 测试slice方法的基本功能
    array<int> arr = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    // 获取中间切片
    arrayView<int> slice1 = arr.slice(2, 7);
    EXPECT_EQ(slice1.count(), 5);
    EXPECT_EQ(slice1[0], 2);
    EXPECT_EQ(slice1[4], 6);

    // 获取开头切片
    arrayView<int> slice2 = arr.slice(0, 3);
    EXPECT_EQ(slice2.count(), 3);
    EXPECT_EQ(slice2[0], 0);
    EXPECT_EQ(slice2[2], 2);

    // 获取结尾切片
    arrayView<int> slice3 = arr.slice(7, 10);
    EXPECT_EQ(slice3.count(), 3);
    EXPECT_EQ(slice3[0], 7);
    EXPECT_EQ(slice3[2], 9);
}

TEST(ArrayTest, SliceEmptyRange) {
    // 测试空范围的slice
    array<int> arr = {1, 2, 3, 4, 5};

    // start == end 应该返回空view
    arrayView<int> emptySlice = arr.slice(2, 2);
    EXPECT_EQ(emptySlice.count(), 0);
    EXPECT_TRUE(emptySlice.empty());

    // 空数组的slice
    array<int> emptyArr;
    arrayView<int> emptyArrSlice = emptyArr.slice(0, 0);
    EXPECT_EQ(emptyArrSlice.count(), 0);
    EXPECT_TRUE(emptyArrSlice.empty());
}

TEST(ArrayTest, SliceBoundaryCases) {
    // 测试边界情况
    array<int> arr = {10, 20, 30, 40, 50};

    // 整个数组的slice
    arrayView<int> fullSlice = arr.slice(0, arr.size());
    EXPECT_EQ(fullSlice.count(), arr.size());
    for (u_integer i = 0; i < arr.size(); ++i) {
        EXPECT_EQ(fullSlice[i], arr[i]);
    }

    // 单个元素的slice
    arrayView<int> singleSlice = arr.slice(2, 3);
    EXPECT_EQ(singleSlice.count(), 1);
    EXPECT_EQ(singleSlice[0], 30);
}

TEST(ArrayTest, SliceOutOfBounds) {
    // 测试越界情况
    array<int> arr = {1, 2, 3, 4, 5};

    // start越界
    EXPECT_THROW(arr.slice(10, 12), outOfBoundError);

    // end越界
    EXPECT_THROW(arr.slice(2, 10), outOfBoundError);

    // start和end都越界
    EXPECT_THROW(arr.slice(10, 15), outOfBoundError);

    // start > end
    EXPECT_NO_THROW(arr.slice(3, 2));
    EXPECT_TRUE(arr.slice(3, 2).empty());

    // 空数组的越界
    array<int> emptyArr;
    EXPECT_THROW(emptyArr.slice(0, 1), outOfBoundError);
    EXPECT_THROW(emptyArr.slice(1, 1), outOfBoundError);
}

TEST(ArrayTest, SliceModification) {
    // 测试通过slice修改原数组
    array<int> arr = {1, 2, 3, 4, 5, 6};
    arrayView<int> slice = arr.slice(1, 4);  // 元素: 2, 3, 4

    // 通过slice修改元素
    slice[0] = 20;
    slice[1] = 30;
    slice[2] = 40;

    // 验证原数组被修改
    EXPECT_EQ(arr[1], 20);
    EXPECT_EQ(arr[2], 30);
    EXPECT_EQ(arr[3], 40);

    // 原数组其他元素保持不变
    EXPECT_EQ(arr[0], 1);
    EXPECT_EQ(arr[4], 5);
    EXPECT_EQ(arr[5], 6);
}

TEST(ArrayTest, SliceIteration) {
    // 测试slice的迭代功能
    array<int> arr = {0, 10, 20, 30, 40, 50};
    arrayView<int> slice = arr.slice(1, 5);  // 元素: 10, 20, 30, 40

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

    TEST(ArrayTest, SliceOfSlice) {
        // 测试slice的链式操作
        array<int> arr = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

        // 第一次slice
        arrayView<int> slice1 = arr.slice(1, 9);  // 1,2,3,4,5,6,7,8
        EXPECT_EQ(slice1.count(), 8);

        // 对slice再次slice
        arrayView<int> slice2 = slice1.subview(2, 4);  // 3,4,5,6
        EXPECT_EQ(slice2.count(), 4);
        EXPECT_EQ(slice2[0], 3);
        EXPECT_EQ(slice2[3], 6);

        // 验证修改传播
        slice2[0] = 300;
        EXPECT_EQ(arr[3], 300);  // 原数组对应位置被修改
    }

    TEST(ArrayTest, SliceWithComplexTypes) {
        // 测试复杂类型的slice
        struct Person {
            std::string name;
            int age;
            bool operator==(const Person& other) const {
                return name == other.name && age == other.age;
            }
        };

        array<Person> people = {
            Person{"Alice", 25},
            Person{"Bob", 30},
            Person{"Charlie", 35},
            Person{"David", 40},
            Person{"Eve", 45}
        };

        arrayView<Person> slice = people.slice(1, 4);  // Bob, Charlie, David

        EXPECT_EQ(slice.count(), 3);
        EXPECT_TRUE((slice[0] == Person{"Bob", 30}));
        EXPECT_TRUE((slice[1] == Person{"Charlie", 35}));
        EXPECT_TRUE((slice[2] == Person{"David", 40}));

        // 通过slice修改
        slice[1] = Person{"Charles", 36};
        EXPECT_TRUE((people[2] == Person{"Charles", 36}));
    }

    TEST(ArrayTest, SliceAndViewComparison) {
        // 测试slice和view方法的关系
        array<int> arr = {1, 2, 3, 4, 5, 6, 7, 8};

        // view() 相当于 slice(0, size())
        arrayView<int> fullView = arr.view();
        arrayView<int> fullSlice = arr.slice(0, arr.size());

        EXPECT_EQ(fullView.count(), fullSlice.count());
        for (u_integer i = 0; i < fullView.count(); ++i) {
            EXPECT_EQ(fullView[i], fullSlice[i]);
        }

        // 验证它们指向相同的数据
        fullSlice[0] = 100;
        EXPECT_EQ(fullView[0], 100);
        EXPECT_EQ(arr[0], 100);
    }

    TEST(ArrayTest, SliceFromConstArray) {
        // 测试const数组的slice
        const array<int> arr = {10, 20, 30, 40, 50};

        // const数组的slice也应该是const的
        auto slice = arr.slice(1, 4);
        EXPECT_EQ(slice.count(), 3);
        EXPECT_EQ(slice[0], 20);
        EXPECT_EQ(slice[1], 30);
        EXPECT_EQ(slice[2], 40);

        // 不能通过const slice修改元素
        // slice[0] = 200;  // 这应该编译错误
    }

}  // namespace original
