#include <gtest/gtest.h>
#include "sets.h"
#include <string>
#include <vector>
#include <algorithm>

using namespace original;

class TreeSetTest : public testing::Test {
protected:
    void SetUp() override {
        // Common setup for all tests
        intSet = new treeSet<int>();
        stringSet = new treeSet<std::string>();
    }

    void TearDown() override {
        delete intSet;
        delete stringSet;
    }

    treeSet<int>* intSet{};
    treeSet<std::string>* stringSet{};
};

// Basic Functionality Tests
TEST_F(TreeSetTest, InitialState) {
    EXPECT_EQ(intSet->size(), 0);
    EXPECT_TRUE(intSet->className() == "treeSet");
}

TEST_F(TreeSetTest, AddAndContains) {
    EXPECT_TRUE(intSet->add(42));
    EXPECT_EQ(intSet->size(), 1);
    EXPECT_TRUE(intSet->contains(42));
    EXPECT_FALSE(intSet->contains(43));

    EXPECT_TRUE(stringSet->add("test"));
    EXPECT_TRUE(stringSet->contains("test"));
}

TEST_F(TreeSetTest, AddDuplicate) {
    EXPECT_TRUE(intSet->add(10));
    EXPECT_FALSE(intSet->add(10)); // Adding duplicate should fail
    EXPECT_EQ(intSet->size(), 1);
}

TEST_F(TreeSetTest, Remove) {
    intSet->add(1);
    intSet->add(2);
    EXPECT_TRUE(intSet->remove(1));
    EXPECT_EQ(intSet->size(), 1);
    EXPECT_FALSE(intSet->contains(1));
    EXPECT_TRUE(intSet->contains(2));

    EXPECT_FALSE(intSet->remove(99)); // Remove non-existent
}

// Iterator Tests - treeSet should maintain order
TEST_F(TreeSetTest, IteratorOrder) {
    // Insert elements out of order
    intSet->add(3);
    intSet->add(1);
    intSet->add(2);
    intSet->add(5);
    intSet->add(4);

    const auto it = intSet->begins();
    EXPECT_TRUE(it->isValid());

    std::vector<int> values;
    while (it->isValid()) {
        values.push_back(it->get());
        it->next();
    }
    delete it;

    // Verify elements are in order
    EXPECT_EQ(values.size(), 5);
    for (size_t i = 0; i < values.size() - 1; ++i) {
        EXPECT_LT(values[i], values[i + 1]) << "Elements not in order at position " << i;
    }
}

TEST_F(TreeSetTest, IteratorReverseOrder) {
    intSet->add(1);
    intSet->add(2);
    intSet->add(3);

    const auto it = intSet->ends();
    EXPECT_TRUE(it->isValid());

    std::vector<int> values;
    while (it->isValid()) {
        values.push_back(it->get());
        it->prev();
    }
    delete it;

    // Verify elements are in reverse order
    EXPECT_EQ(values.size(), 3);
    for (size_t i = 0; i < values.size() - 1; ++i) {
        EXPECT_GT(values[i], values[i + 1]) << "Elements not in reverse order at position " << i;
    }
}

TEST_F(TreeSetTest, IteratorEnd) {
    intSet->add(1);
    const auto begin = intSet->begin();
    const auto end = intSet->end();

    EXPECT_TRUE(begin.isValid());
    EXPECT_FALSE(end.isValid());
}

// Boundary Tests
TEST_F(TreeSetTest, LargeNumberOfElements) {
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
        EXPECT_FALSE(intSet->contains(i));
    }
}

TEST_F(TreeSetTest, StringElements) {
    const std::vector<std::string> testStrings = {"apple", "banana", "cherry"};

    for (const auto& s : testStrings) {
        stringSet->add(s);
    }

    EXPECT_EQ(stringSet->size(), 3);
    for (const auto& s : testStrings) {
        EXPECT_TRUE(stringSet->contains(s));
    }
}

// Copy and Move Tests
TEST_F(TreeSetTest, CopyConstructor) {
    intSet->add(1);
    intSet->add(2);

    const treeSet copy(*intSet);
    EXPECT_EQ(copy.size(), 2);
    EXPECT_TRUE(copy.contains(1));
    EXPECT_TRUE(copy.contains(2));
}

TEST_F(TreeSetTest, MoveConstructor) {
    intSet->add(1);
    intSet->add(2);

    const treeSet moved(std::move(*intSet));
    EXPECT_EQ(moved.size(), 2);
    EXPECT_TRUE(moved.contains(1));
    EXPECT_TRUE(moved.contains(2));
    EXPECT_EQ(intSet->size(), 0); // NOLINT(bugprone-use-after-move)
}

TEST_F(TreeSetTest, CopyAssignment) {
    intSet->add(1);
    intSet->add(2);

    const treeSet<int> copy = *intSet;
    EXPECT_EQ(copy.size(), 2);
    EXPECT_TRUE(copy.contains(1));
    EXPECT_TRUE(copy.contains(2));
}

TEST_F(TreeSetTest, MoveAssignment) {
    intSet->add(1);
    intSet->add(2);

    const treeSet<int> moved = std::move(*intSet);
    EXPECT_EQ(moved.size(), 2);
    EXPECT_TRUE(moved.contains(1));
    EXPECT_TRUE(moved.contains(2));
    EXPECT_EQ(intSet->size(), 0); // NOLINT(bugprone-use-after-move)
}

namespace
{
    template<typename>
    struct CustomCompare {
        bool operator()(const int a, const int b) const {
            return a > b; // Reverse order
        }
    };
}

// Custom Comparator Test
TEST(TreeSetCustomCompareTest, CustomCompareFunction) {
    treeSet<int, CustomCompare<int>> customSet;
    customSet.add(1);
    customSet.add(2);
    customSet.add(3);

    const auto it = customSet.begins();
    EXPECT_TRUE(it->isValid());

    std::vector<int> values;
    while (it->isValid()) {
        values.push_back(it->get());
        it->next();
    }
    delete it;

    // Verify elements are in reverse order
    EXPECT_EQ(values.size(), 3);
    for (size_t i = 0; i < values.size() - 1; ++i) {
        EXPECT_GT(values[i], values[i + 1]) << "Elements not in reverse order at position " << i;
    }
}

// toString Test
TEST_F(TreeSetTest, ToString) {
    intSet->add(1);
    intSet->add(2);
    const std::string str = intSet->toString(false);

    // Basic checks - exact format might vary
    EXPECT_TRUE(str.find("treeSet") != std::string::npos);
    EXPECT_TRUE(str.find('1') != std::string::npos);
    EXPECT_TRUE(str.find('2') != std::string::npos);
}

// Test predecessor and successor functionality through iterator
TEST_F(TreeSetTest, IteratorPredecessorSuccessor) {
    intSet->add(1);
    intSet->add(3);
    intSet->add(5);

    auto it = ownerPtr(intSet->begins());
    EXPECT_EQ(it->get(), 1);

    it->next();
    EXPECT_EQ(it->get(), 3);

    it->next();
    EXPECT_EQ(it->get(), 5);

    it->prev();
    EXPECT_EQ(it->get(), 3);

    it->prev();
    EXPECT_EQ(it->get(), 1);
}

// Test tree balancing by inserting elements in reverse order
TEST_F(TreeSetTest, ReverseOrderInsertion) {
    constexpr int count = 1000;
    for (int i = count; i > 0; --i) {
        EXPECT_TRUE(intSet->add(i));
    }

    EXPECT_EQ(intSet->size(), count);

    // Verify all elements are present and in order
    auto it = ownerPtr(intSet->begins());
    int expected = 1;
    while (it->isValid()) {
        EXPECT_EQ(it->get(), expected);
        it->next();
        expected++;
    }
}
