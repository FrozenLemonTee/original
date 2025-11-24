#include <gtest/gtest.h>
#include "sets.h"
#include <string>
#include <vector>
#include <algorithm>

using namespace original;

class JSetTest : public testing::Test {
protected:
    void SetUp() override {
        intSet = new JSet<int>();
        stringSet = new JSet<std::string>();
    }

    void TearDown() override {
        delete intSet;
        delete stringSet;
    }

    JSet<int>* intSet{};
    JSet<std::string>* stringSet{};
};

namespace
{
    template <typename>
    struct ReverseCompare {
        bool operator()(int a, int b) const {
            return a > b;
        }
    };
}

// 基础功能测试
TEST_F(JSetTest, InitialState) {
    EXPECT_EQ(intSet->size(), 0);
    EXPECT_EQ(intSet->className(), "JSet");
}

TEST_F(JSetTest, AddAndContains) {
    EXPECT_TRUE(intSet->add(10));
    EXPECT_TRUE(intSet->contains(10));
    EXPECT_FALSE(intSet->contains(20));

    EXPECT_TRUE(stringSet->add("hello"));
    EXPECT_TRUE(stringSet->contains("hello"));
}

TEST_F(JSetTest, AddDuplicate) {
    EXPECT_TRUE(intSet->add(1));
    EXPECT_FALSE(intSet->add(1));  // duplicate
    EXPECT_EQ(intSet->size(), 1);
}

TEST_F(JSetTest, RemoveElement) {
    intSet->add(1);
    intSet->add(2);
    EXPECT_TRUE(intSet->remove(1));
    EXPECT_FALSE(intSet->contains(1));
    EXPECT_TRUE(intSet->contains(2));
    EXPECT_EQ(intSet->size(), 1);

    EXPECT_FALSE(intSet->remove(100));  // 不存在
}

// 迭代器有序性测试（跳表应保持元素有序）
TEST_F(JSetTest, IteratorOrder) {
    std::vector<int> values = {5, 1, 4, 3, 2};
    for (int v : values) {
        intSet->add(v);
    }

    auto it = intSet->begins();
    std::vector<int> result;
    while (it->isValid()) {
        result.push_back(it->get());
        it->next();
    }
    delete it;

    EXPECT_TRUE(std::is_sorted(result.begin(), result.end()));
    EXPECT_EQ(result.size(), 5);
}

// 拷贝与移动构造测试
TEST_F(JSetTest, CopyConstructor) {
    intSet->add(10);
    intSet->add(20);
    JSet<int> copy(*intSet);
    EXPECT_TRUE(copy.contains(10));
    EXPECT_TRUE(copy.contains(20));
    EXPECT_EQ(copy.size(), 2);
}

TEST_F(JSetTest, MoveConstructor) {
    intSet->add(10);
    intSet->add(20);
    JSet<int> moved(std::move(*intSet));
    EXPECT_TRUE(moved.contains(10));
    EXPECT_TRUE(moved.contains(20));
    EXPECT_EQ(intSet->size(), 0);
}

TEST_F(JSetTest, CopyAssignment) {
    intSet->add(1);
    intSet->add(2);
    JSet<int> copy = *intSet;
    EXPECT_TRUE(copy.contains(1));
    EXPECT_TRUE(copy.contains(2));
}

TEST_F(JSetTest, MoveAssignment) {
    intSet->add(1);
    intSet->add(2);
    JSet<int> moved = std::move(*intSet);
    EXPECT_TRUE(moved.contains(1));
    EXPECT_TRUE(moved.contains(2));
    EXPECT_EQ(intSet->size(), 0);
}

// 大量元素插入
TEST_F(JSetTest, LargeNumberOfElements) {
    constexpr int count = 100000;
    for (int i = 0; i < count; ++i) {
        EXPECT_TRUE(intSet->add(i));
    }
    EXPECT_EQ(intSet->size(), count);

    for (int i = 0; i < count; ++i) {
        EXPECT_TRUE(intSet->contains(i));
    }

    for (int i = 0; i < count; ++i) {
        EXPECT_TRUE(intSet->remove(i));
    }
    EXPECT_EQ(intSet->size(), 0);
}

// toString 输出
TEST_F(JSetTest, ToString) {
    intSet->add(10);
    intSet->add(20);
    std::string s = intSet->toString(false);
    EXPECT_TRUE(s.find("JSet") != std::string::npos);
    EXPECT_TRUE(s.find("10") != std::string::npos);
    EXPECT_TRUE(s.find("20") != std::string::npos);
}

// 自定义比较器测试（反向排序）
TEST(JSetCustomCompareTest, CustomCompare) {
    JSet<int, ReverseCompare<int>> set;
    for (int i = 1; i <= 3; ++i) set.add(i);

    auto it = set.begins();
    std::vector<int> result;
    while (it->isValid()) {
        result.push_back(it->get());
        it->next();
    }
    delete it;

    EXPECT_EQ(result, std::vector<int>({3, 2, 1}));
}
