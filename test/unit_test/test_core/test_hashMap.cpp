#include <algorithm>
#include <gtest/gtest.h>
#include "maps.h"
#include <string>
#include <vector>

using namespace original;

class HashMapTest : public testing::Test {
protected:
    void SetUp() override {
        // Common setup for all tests
        intMap = new hashMap<int, int>();
        stringMap = new hashMap<std::string, int>();
    }

    void TearDown() override {
        delete intMap;
        delete stringMap;
    }

    hashMap<int, int>* intMap{};
    hashMap<std::string, int>* stringMap{};
};

// Basic Functionality Tests
TEST_F(HashMapTest, InitialState) {
    EXPECT_EQ(intMap->size(), 0);
    EXPECT_TRUE(intMap->className() == "hashMap");
}

TEST_F(HashMapTest, AddAndContains) {
    EXPECT_TRUE(intMap->add(42, 100));
    EXPECT_EQ(intMap->size(), 1);
    EXPECT_TRUE(intMap->containsKey(42));
    EXPECT_FALSE(intMap->containsKey(43));
    EXPECT_EQ(intMap->get(42), 100);

    EXPECT_TRUE(stringMap->add("test", 200));
    EXPECT_TRUE(stringMap->containsKey("test"));
    EXPECT_EQ(stringMap->get("test"), 200);
}

TEST_F(HashMapTest, AddDuplicate) {
    EXPECT_TRUE(intMap->add(10, 1));
    EXPECT_FALSE(intMap->add(10, 2)); // Adding duplicate key should fail
    EXPECT_EQ(intMap->size(), 1);
    EXPECT_EQ(intMap->get(10), 1); // Value should remain unchanged
}

TEST_F(HashMapTest, Remove) {
    intMap->add(1, 10);
    intMap->add(2, 20);
    EXPECT_TRUE(intMap->remove(1));
    EXPECT_EQ(intMap->size(), 1);
    EXPECT_FALSE(intMap->containsKey(1));
    EXPECT_TRUE(intMap->containsKey(2));
    EXPECT_EQ(intMap->get(2), 20);

    EXPECT_FALSE(intMap->remove(99)); // Remove non-existent key
}

TEST_F(HashMapTest, Update) {
    intMap->add(1, 10);
    EXPECT_TRUE(intMap->update(1, 100));
    EXPECT_EQ(intMap->get(1), 100);
    EXPECT_FALSE(intMap->update(2, 200)); // Update non-existent key
}

TEST_F(HashMapTest, OperatorAccess) {
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

// Iterator Tests
TEST_F(HashMapTest, IteratorBasic) {
    intMap->add(1, 10);
    intMap->add(2, 20);
    intMap->add(3, 30);

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

    EXPECT_EQ(keys.size(), 3);
    EXPECT_EQ(values.size(), 3);
    EXPECT_TRUE(std::ranges::find(keys, 1) != keys.end());
    EXPECT_TRUE(std::ranges::find(keys, 2) != keys.end());
    EXPECT_TRUE(std::ranges::find(keys, 3) != keys.end());
    EXPECT_TRUE(std::ranges::find(values, 10) != values.end());
    EXPECT_TRUE(std::ranges::find(values, 20) != values.end());
    EXPECT_TRUE(std::ranges::find(values, 30) != values.end());
}

TEST_F(HashMapTest, IteratorEnd) {
    intMap->add(1, 10);
    const auto begin = intMap->begin();
    const auto end = intMap->end();

    EXPECT_TRUE(begin.isValid());
    EXPECT_FALSE(end.isValid()); // End iterator should be invalid
}

// Boundary Tests
TEST_F(HashMapTest, LargeNumberOfElements) {
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

TEST_F(HashMapTest, StringKeyElements) {
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
TEST_F(HashMapTest, CopyConstructor) {
    intMap->add(1, 10);
    intMap->add(2, 20);

    const hashMap copy(*intMap);
    EXPECT_EQ(copy.size(), 2);
    EXPECT_TRUE(copy.containsKey(1));
    EXPECT_TRUE(copy.containsKey(2));
    EXPECT_EQ(copy.get(1), 10);
    EXPECT_EQ(copy.get(2), 20);
}

TEST_F(HashMapTest, MoveConstructor) {
    intMap->add(1, 10);
    intMap->add(2, 20);

    const hashMap moved(std::move(*intMap));
    EXPECT_EQ(moved.size(), 2);
    EXPECT_TRUE(moved.containsKey(1));
    EXPECT_TRUE(moved.containsKey(2));
    EXPECT_EQ(moved.get(1), 10);
    EXPECT_EQ(moved.get(2), 20);
    EXPECT_EQ(intMap->size(), 0); // NOLINT(bugprone-use-after-move)
}

TEST_F(HashMapTest, CopyAssignment) {
    intMap->add(1, 10);
    intMap->add(2, 20);

    const hashMap<int, int> copy = *intMap;
    EXPECT_EQ(copy.size(), 2);
    EXPECT_TRUE(copy.containsKey(1));
    EXPECT_TRUE(copy.containsKey(2));
    EXPECT_EQ(copy.get(1), 10);
    EXPECT_EQ(copy.get(2), 20);
}

TEST_F(HashMapTest, MoveAssignment) {
    intMap->add(1, 10);
    intMap->add(2, 20);

    const hashMap<int, int> moved = std::move(*intMap);
    EXPECT_EQ(moved.size(), 2);
    EXPECT_TRUE(moved.containsKey(1));
    EXPECT_TRUE(moved.containsKey(2));
    EXPECT_EQ(moved.get(1), 10);
    EXPECT_EQ(moved.get(2), 20);
    EXPECT_EQ(intMap->size(), 0); // NOLINT(bugprone-use-after-move)
}

// Exception Safety Tests
TEST_F(HashMapTest, IteratorInvalidation) {
    intMap->add(1, 10);
    const auto it = intMap->begins();

    // Adding elements might cause rehash
    for (int i = 2; i < 100; ++i) {
        intMap->add(i, i * 10);
    }

    // Iterator might be invalid after rehash
    // This depends on implementation - might need adjustment
    EXPECT_NO_THROW(it->get());
    delete it;
}

// toString Test
TEST_F(HashMapTest, ToString) {
    intMap->add(1, 10);
    intMap->add(2, 20);
    const std::string str = intMap->toString(false);

    // Basic checks - exact format might vary
    EXPECT_TRUE(str.find("hashMap") != std::string::npos);
    EXPECT_TRUE(str.find('1') != std::string::npos);
    EXPECT_TRUE(str.find("10") != std::string::npos);
    EXPECT_TRUE(str.find('2') != std::string::npos);
    EXPECT_TRUE(str.find("20") != std::string::npos);
}

namespace
{
    template<typename>
    struct CustomHash {
        u_integer operator()(const int key) const {
            return key % 10; // Simple hash for testing
        }
    };
}

// Custom Hash Function Test
TEST(HashMapCustomHashTest, CustomHashFunction) {
    hashMap<int, int, CustomHash<int>> customMap;
    for (int i = 0; i < 20; ++i) {
        customMap.add(i, i * 10);
    }

    EXPECT_EQ(customMap.size(), 20); // All should be added despite hash collisions
    for (int i = 0; i < 20; ++i) {
        EXPECT_TRUE(customMap.containsKey(i));
        EXPECT_EQ(customMap.get(i), i * 10);
    }
}

// Contains with value test
TEST_F(HashMapTest, ContainsKeyValuePair) {
    intMap->add(1, 10);
    intMap->add(2, 20);

    EXPECT_TRUE(intMap->contains(couple<const int, int>(1, 10)));
    EXPECT_FALSE(intMap->contains(couple<const int, int>(1, 20))); // Wrong value
    EXPECT_FALSE(intMap->contains(couple<const int, int>(3, 30))); // Key doesn't exist
}
