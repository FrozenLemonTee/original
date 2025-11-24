#include <gtest/gtest.h>
#include "maps.h"
#include <string>
#include <vector>
#include <algorithm>

using namespace original;

class TreeMapTest : public testing::Test {
protected:
    void SetUp() override {
        // Common setup for all tests
        intMap = new treeMap<int, int>();
        stringMap = new treeMap<std::string, int>();
    }

    void TearDown() override {
        delete intMap;
        delete stringMap;
    }

    treeMap<int, int>* intMap{};
    treeMap<std::string, int>* stringMap{};
};

// Basic Functionality Tests
TEST_F(TreeMapTest, InitialState) {
    EXPECT_EQ(intMap->size(), 0);
    EXPECT_TRUE(intMap->className() == "treeMap");
}

TEST_F(TreeMapTest, AddAndContains) {
    EXPECT_TRUE(intMap->add(42, 100));
    EXPECT_EQ(intMap->size(), 1);
    EXPECT_TRUE(intMap->containsKey(42));
    EXPECT_FALSE(intMap->containsKey(43));
    EXPECT_EQ(intMap->get(42), 100);

    EXPECT_TRUE(stringMap->add("test", 200));
    EXPECT_TRUE(stringMap->containsKey("test"));
    EXPECT_EQ(stringMap->get("test"), 200);
}

TEST_F(TreeMapTest, AddDuplicate) {
    EXPECT_TRUE(intMap->add(10, 1));
    EXPECT_FALSE(intMap->add(10, 2)); // Adding duplicate key should fail
    EXPECT_EQ(intMap->size(), 1);
    EXPECT_EQ(intMap->get(10), 1); // Value should remain unchanged
}

TEST_F(TreeMapTest, Remove) {
    intMap->add(1, 10);
    intMap->add(2, 20);
    EXPECT_TRUE(intMap->remove(1));
    EXPECT_EQ(intMap->size(), 1);
    EXPECT_FALSE(intMap->containsKey(1));
    EXPECT_TRUE(intMap->containsKey(2));
    EXPECT_EQ(intMap->get(2), 20);

    EXPECT_FALSE(intMap->remove(99)); // Remove non-existent key
}

TEST_F(TreeMapTest, Update) {
    intMap->add(1, 10);
    EXPECT_TRUE(intMap->update(1, 100));
    EXPECT_EQ(intMap->get(1), 100);
    EXPECT_FALSE(intMap->update(2, 200)); // Update non-existent key
}

TEST_F(TreeMapTest, OperatorAccess) {
    (*intMap)[1] = 10;
    (*intMap)[2] = 20;

    // Const access
    const auto& constMap = *intMap;
    EXPECT_EQ(constMap[1], 10);
    EXPECT_EQ(constMap[2], 20);

    // Non-const access creates default if not exists
    EXPECT_EQ((*intMap)[3], int{});
    EXPECT_EQ(intMap->size(), 3);
}

// Iterator Tests - treeMap should maintain order
TEST_F(TreeMapTest, IteratorOrder) {
    // Insert elements out of order
    intMap->add(3, 30);
    intMap->add(1, 10);
    intMap->add(2, 20);
    intMap->add(5, 50);
    intMap->add(4, 40);

    const auto it = intMap->begins();
    EXPECT_TRUE(it->isValid());

    std::vector<int> keys;
    std::vector<int> values;
    while (it->isValid()) {
        auto pair = it->get();
        keys.push_back(pair.first());
        values.push_back(pair.second());
        it->next();
    }
    delete it;

    // Verify elements are in order
    EXPECT_EQ(keys.size(), 5);
    EXPECT_EQ(values.size(), 5);
    for (size_t i = 0; i < keys.size() - 1; ++i) {
        EXPECT_LT(keys[i], keys[i + 1]) << "Elements not in order at position " << i;
    }
}

TEST_F(TreeMapTest, IteratorReverseOrder) {
    intMap->add(1, 10);
    intMap->add(2, 20);
    intMap->add(3, 30);

    const auto it = intMap->ends();
    EXPECT_TRUE(it->isValid());

    std::vector<int> keys;
    std::vector<int> values;
    while (it->isValid()) {
        auto pair = it->get();
        keys.push_back(pair.first());
        values.push_back(pair.second());
        it->prev();
    }
    delete it;

    // Verify elements are in reverse order
    EXPECT_EQ(keys.size(), 3);
    EXPECT_EQ(values.size(), 3);
    for (size_t i = 0; i < keys.size() - 1; ++i) {
        EXPECT_GT(keys[i], keys[i + 1]) << "Elements not in reverse order at position " << i;
    }
}

TEST_F(TreeMapTest, IteratorEnd) {
    intMap->add(1, 10);
    const auto begin = intMap->begin();
    const auto end = intMap->end();

    EXPECT_TRUE(begin.isValid());
    EXPECT_FALSE(end.isValid());
}

// Boundary Tests
TEST_F(TreeMapTest, LargeNumberOfElements) {
    constexpr int count = 100000;
    for (int i = 0; i < count; ++i) {
        EXPECT_TRUE(intMap->add(i, i * 10));
    }
    EXPECT_EQ(intMap->size(), count);

    for (int i = 0; i < count; ++i) {
        EXPECT_TRUE(intMap->containsKey(i));
        EXPECT_EQ(intMap->get(i), i * 10);
    }

    for (int i = 0; i < count; ++i) {
        EXPECT_TRUE(intMap->remove(i));
        EXPECT_FALSE(intMap->containsKey(i));
    }
}

TEST_F(TreeMapTest, StringKeyElements) {
    const std::vector<std::string> testStrings = {"apple", "banana", "cherry"};

    for (size_t i = 0; i < testStrings.size(); ++i) {
        stringMap->add(testStrings[i], static_cast<int>(i));
    }

    EXPECT_EQ(stringMap->size(), 3);
    for (size_t i = 0; i < testStrings.size(); ++i) {
        EXPECT_TRUE(stringMap->containsKey(testStrings[i]));
        EXPECT_EQ(stringMap->get(testStrings[i]), static_cast<int>(i));
    }
}

// Copy and Move Tests
TEST_F(TreeMapTest, CopyConstructor) {
    intMap->add(1, 10);
    intMap->add(2, 20);

    const treeMap copy(*intMap);
    EXPECT_EQ(copy.size(), 2);
    EXPECT_TRUE(copy.containsKey(1));
    EXPECT_TRUE(copy.containsKey(2));
    EXPECT_EQ(copy.get(1), 10);
    EXPECT_EQ(copy.get(2), 20);
}

TEST_F(TreeMapTest, MoveConstructor) {
    intMap->add(1, 10);
    intMap->add(2, 20);

    const treeMap moved(std::move(*intMap));
    EXPECT_EQ(moved.size(), 2);
    EXPECT_TRUE(moved.containsKey(1));
    EXPECT_TRUE(moved.containsKey(2));
    EXPECT_EQ(moved.get(1), 10);
    EXPECT_EQ(moved.get(2), 20);
    EXPECT_EQ(intMap->size(), 0); // NOLINT(bugprone-use-after-move)
}

TEST_F(TreeMapTest, CopyAssignment) {
    intMap->add(1, 10);
    intMap->add(2, 20);

    const treeMap<int, int> copy = *intMap;
    EXPECT_EQ(copy.size(), 2);
    EXPECT_TRUE(copy.containsKey(1));
    EXPECT_TRUE(copy.containsKey(2));
    EXPECT_EQ(copy.get(1), 10);
    EXPECT_EQ(copy.get(2), 20);
}

TEST_F(TreeMapTest, MoveAssignment) {
    intMap->add(1, 10);
    intMap->add(2, 20);

    const treeMap<int, int> moved = std::move(*intMap);
    EXPECT_EQ(moved.size(), 2);
    EXPECT_TRUE(moved.containsKey(1));
    EXPECT_TRUE(moved.containsKey(2));
    EXPECT_EQ(moved.get(1), 10);
    EXPECT_EQ(moved.get(2), 20);
    EXPECT_EQ(intMap->size(), 0); // NOLINT(bugprone-use-after-move)
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
TEST(TreeMapCustomCompareTest, CustomCompareFunction) {
    treeMap<int, int, CustomCompare<int>> customMap;
    customMap.add(1, 10);
    customMap.add(2, 20);
    customMap.add(3, 30);

    const auto it = customMap.begins();
    EXPECT_TRUE(it->isValid());
    std::vector<int> keys;
    while (it->isValid()) {
        keys.push_back(it->get().first());
        it->next();
    }
    delete it;

    // Verify elements are in reverse order
    EXPECT_EQ(keys.size(), 3);
    for (size_t i = 0; i < keys.size() - 1; ++i) {
        EXPECT_GT(keys[i], keys[i + 1]) << "Elements not in reverse order at position " << i;
    }
}

// toString Test
TEST_F(TreeMapTest, ToString) {
    intMap->add(1, 10);
    intMap->add(2, 20);
    const std::string str = intMap->toString(false);

    // Basic checks - exact format might vary
    EXPECT_TRUE(str.find("treeMap") != std::string::npos);
    EXPECT_TRUE(str.find('1') != std::string::npos);
    EXPECT_TRUE(str.find("10") != std::string::npos);
    EXPECT_TRUE(str.find('2') != std::string::npos);
    EXPECT_TRUE(str.find("20") != std::string::npos);
}

// Contains with value test
TEST_F(TreeMapTest, ContainsKeyValuePair) {
    intMap->add(1, 10);
    intMap->add(2, 20);

    EXPECT_TRUE(intMap->contains(couple<const int, int>(1, 10)));
    EXPECT_FALSE(intMap->contains(couple<const int, int>(1, 20))); // Wrong value
    EXPECT_FALSE(intMap->contains(couple<const int, int>(3, 30))); // Key doesn't exist
}

// Test predecessor and successor functionality through iterator
TEST_F(TreeMapTest, IteratorPredecessorSuccessor) {
    intMap->add(1, 10);
    intMap->add(3, 30);
    intMap->add(5, 50);

    auto it = ownerPtr(intMap->begins());
    EXPECT_EQ(it->get().first(), 1);

    it->next();
    EXPECT_EQ(it->get().first(), 3);

    it->next();
    EXPECT_EQ(it->get().first(), 5);

    it->prev();
    EXPECT_EQ(it->get().first(), 3);

    it->prev();
    EXPECT_EQ(it->get().first(), 1);
}

// Test tree balancing by inserting elements in reverse order
TEST_F(TreeMapTest, ReverseOrderInsertion) {
    constexpr int count = 1000;
    for (int i = count; i > 0; --i) {
        EXPECT_TRUE(intMap->add(i, i * 10));
    }

    EXPECT_EQ(intMap->size(), count);

    // Verify all elements are present and in order
    auto it = ownerPtr(intMap->begins());
    int expected = 1;
    while (it->isValid()) {
        EXPECT_EQ(it->get().first(), expected);
        EXPECT_EQ(it->get().second(), expected * 10);
        it->next();
        expected++;
    }
}