#include <gtest/gtest.h>
#include <vector>
#include <algorithm>
#include "arrayView.h"

// 测试夹具类
class ArrayViewTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 设置测试数据
        for (int i = 0; i < 10; ++i) {
            testData[i] = i + 1; // 1, 2, 3, ..., 10
        }
    }

    int testData[10] = {};
    std::vector<int> vecData = {10, 20, 30, 40, 50};
};

// 测试构造函数
TEST_F(ArrayViewTest, ConstructFromArray) {
    original::arrayView<int> view(testData, 10);
    
    EXPECT_EQ(view.count(), 10);
    EXPECT_EQ(view.data(), testData);
    EXPECT_FALSE(view.empty());
}

TEST_F(ArrayViewTest, ConstructFromStaticArray) {
    original::arrayView<int> view(testData);
    
    EXPECT_EQ(view.count(), 10);
    EXPECT_EQ(view.data(), testData);
}

TEST_F(ArrayViewTest, EmptyView) {
    original::arrayView<int> view(nullptr, 0);
    
    EXPECT_EQ(view.count(), 0);
    EXPECT_TRUE(view.empty());
    EXPECT_EQ(view.data(), nullptr);
}

// 测试元素访问
TEST_F(ArrayViewTest, ElementAccess) {
    original::arrayView<int> view(testData, 10);
    
    // 测试 operator[]
    EXPECT_EQ(view[0], 1);
    EXPECT_EQ(view[5], 6);
    EXPECT_EQ(view[9], 10);
    
    // 测试 get 方法
    EXPECT_EQ(view.get(0), 1);
    EXPECT_EQ(view.get(5), 6);
    EXPECT_EQ(view.get(9), 10);
    
    // 测试修改元素
    view[0] = 100;
    EXPECT_EQ(testData[0], 100);
    view[0] = 1; // 恢复
    EXPECT_EQ(testData[0], 1);
}

// 测试边界检查
TEST_F(ArrayViewTest, BoundaryCheck) {
    original::arrayView<int> view(testData, 10);
    
    // 测试有效访问
    EXPECT_NO_THROW(view.get(0));
    EXPECT_NO_THROW(view.get(9));
    
    // 测试越界访问
    EXPECT_THROW(view.get(10), original::outOfBoundError);
    EXPECT_THROW(view.get(100), original::outOfBoundError);
}

// 测试迭代器
TEST_F(ArrayViewTest, Iterator) {
    original::arrayView<int> view(testData, 10);
    
    // 测试迭代器遍历
    int sum = 0;
    for (auto it = view.begin(); it != view.end(); ++it) { // NOLINT: explicit iterator testing
        sum += *it;
    }
    EXPECT_EQ(sum, 55); // 1+2+...+10 = 55
    
    // 测试范围for循环
    sum = 0;
    for (const int value : view) {
        sum += value;
    }
    EXPECT_EQ(sum, 55);
    
    // 测试迭代器操作符
    auto it1 = view.begin();
    auto it2 = view.begin();
    EXPECT_EQ(it1, it2);
    
    ++it1;
    EXPECT_NE(it1, it2);
    
    // 测试箭头操作符
    struct TestStruct {
        int a;
        int b;
    };
    
    TestStruct structs[2] = {{1, 2}, {3, 4}};
    original::arrayView<TestStruct> structView(structs, 2);
    
    auto it = structView.begin();
    EXPECT_EQ(it->a, 1);
    EXPECT_EQ(it->b, 2);
}

// 测试子视图
TEST_F(ArrayViewTest, Subview) {
    original::arrayView<int> view(testData, 10);
    
    // 测试有效子视图
    auto sub = view.subview(2, 5);
    EXPECT_EQ(sub.count(), 5);
    EXPECT_EQ(sub.data(), testData + 2);
    EXPECT_EQ(sub[0], 3);
    EXPECT_EQ(sub[4], 7);
    
    // 测试边界情况
    auto fullSub = view.subview(0, 10);
    EXPECT_EQ(fullSub.count(), 10);
    
    auto endSub = view.subview(5, 5);
    EXPECT_EQ(endSub.count(), 5);
    EXPECT_EQ(endSub[0], 6);
}

// 测试无效子视图
TEST_F(ArrayViewTest, InvalidSubview) {
    original::arrayView<int> view(testData, 10);
    
    // 测试越界子视图
    EXPECT_THROW(view.subview(10, 1), original::outOfBoundError);
    EXPECT_THROW(view.subview(5, 10), original::outOfBoundError);
    EXPECT_THROW(view.subview(0, 11), original::outOfBoundError);
}

// 测试与STL算法配合
TEST_F(ArrayViewTest, WithSTLAlgorithms) {
    original::arrayView<int> view(testData, 10);
    
    // 测试 std::find
    auto it = std::find(view.begin(), view.end(), 5);
    EXPECT_NE(it, view.end());
    EXPECT_EQ(*it, 5);
    
    // 测试 std::count
    const auto count = std::count(view.begin(), view.end(), 3);
    EXPECT_EQ(count, 1);
    
    // 测试 std::for_each
    int sum = 0;
    std::for_each(view.begin(), view.end(), [&sum](int x) { sum += x; });
    EXPECT_EQ(sum, 55);
    
    // 测试 std::transform
    std::vector<int> doubled(10);
    std::transform(view.begin(), view.end(), doubled.begin(), 
                   [](int x) { return x * 2; });
    EXPECT_EQ(doubled[0], 2);
    EXPECT_EQ(doubled[9], 20);
}

// 测试常量视图
TEST_F(ArrayViewTest, ConstView) {
    const original::arrayView<int> view(testData, 10);
    
    // 测试常量访问
    EXPECT_EQ(view[0], 1);
    EXPECT_EQ(view.get(5), 6);
    EXPECT_EQ(view.count(), 10);
    EXPECT_FALSE(view.empty());
    EXPECT_EQ(view.data(), testData);
    
    // 测试常量迭代器（需要添加const迭代器支持）
    // 目前代码中没有const迭代器，这里注释掉
    /*
    int sum = 0;
    for (auto it = view.begin(); it != view.end(); ++it) {
        sum += *it;
    }
    EXPECT_EQ(sum, 55);
    */
}

// 测试移动和拷贝语义
TEST_F(ArrayViewTest, MoveCopySemantics) {
    original::arrayView<int> view1(testData, 10);
    
    // 测试拷贝构造
    const original::arrayView<int> view2 = view1;
    EXPECT_EQ(view2.count(), 10);
    EXPECT_EQ(view2.data(), testData);
    
    // 测试移动构造
    const original::arrayView<int> view3 = std::move(view1); // NOLINT: move test
    EXPECT_EQ(view3.count(), 10);
    EXPECT_EQ(view3.data(), testData);
    
    // 拷贝赋值被删除，应该无法编译
    // view2 = view3; // 这行应该导致编译错误
}

// 性能测试：确保迭代器是轻量的
TEST_F(ArrayViewTest, IteratorPerformance) {
    original::arrayView<int> view(testData, 10);
    
    // 多次迭代应该很快
    for (int i = 0; i < 1000; ++i) {
        int sum = 0;
        for (const int value : view) {
            sum += value;
        }
        EXPECT_EQ(sum, 55);
    }
}
