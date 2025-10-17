#include <algorithm>
#include <gtest/gtest.h>
#include "sets.h"
#include <string>
#include <vector>

using namespace original;

class HashSetTest : public testing::Test {
protected:
    void SetUp() override {
        // Common setup for all tests
        intSet = new hashSet<int>();
        stringSet = new hashSet<std::string>();
    }

    void TearDown() override {
        delete intSet;
        delete stringSet;
    }

    hashSet<int>* intSet{};
    hashSet<std::string>* stringSet{};
};

// Basic Functionality Tests
TEST_F(HashSetTest, InitialState) {
    EXPECT_EQ(intSet->size(), 0);
    EXPECT_TRUE(intSet->className() == "hashSet");
}

TEST_F(HashSetTest, AddAndContains) {
    EXPECT_TRUE(intSet->add(42));
    EXPECT_EQ(intSet->size(), 1);
    EXPECT_TRUE(intSet->contains(42));
    EXPECT_FALSE(intSet->contains(43));

    EXPECT_TRUE(stringSet->add("test"));
    EXPECT_TRUE(stringSet->contains("test"));
}

TEST_F(HashSetTest, AddDuplicate) {
    EXPECT_TRUE(intSet->add(10));
    EXPECT_FALSE(intSet->add(10)); // Adding duplicate should fail
    EXPECT_EQ(intSet->size(), 1);
}

TEST_F(HashSetTest, Remove) {
    intSet->add(1);
    intSet->add(2);
    EXPECT_TRUE(intSet->remove(1));
    EXPECT_EQ(intSet->size(), 1);
    EXPECT_FALSE(intSet->contains(1));
    EXPECT_TRUE(intSet->contains(2));

    EXPECT_FALSE(intSet->remove(99)); // Remove non-existent
}

// Iterator Tests
TEST_F(HashSetTest, IteratorBasic) {
    intSet->add(1);
    intSet->add(2);
    intSet->add(3);

    const auto it = intSet->begins();
    EXPECT_TRUE(it->isValid());

    std::vector<int> values;
    while (it->isValid()) {
        values.push_back(it->get());
        it->next();
    }
    delete it;

    EXPECT_EQ(values.size(), 3);
    EXPECT_TRUE(std::ranges::find(values, 1) != values.end());
    EXPECT_TRUE(std::ranges::find(values, 2) != values.end());
    EXPECT_TRUE(std::ranges::find(values, 3) != values.end());
}

TEST_F(HashSetTest, IteratorEnd) {
    intSet->add(1);
    const auto begin = intSet->begin();
    const auto end = intSet->end();

    EXPECT_TRUE(begin.isValid());
    EXPECT_FALSE(end.isValid()); // End iterator should be invalid
}

// Boundary Tests
TEST_F(HashSetTest, LargeNumberOfElements) {
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

TEST_F(HashSetTest, StringElements) {
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
TEST_F(HashSetTest, CopyConstructor) {
    intSet->add(1);
    intSet->add(2);

    const hashSet copy(*intSet);
    EXPECT_EQ(copy.size(), 2);
    EXPECT_TRUE(copy.contains(1));
    EXPECT_TRUE(copy.contains(2));
}

TEST_F(HashSetTest, MoveConstructor) {
    intSet->add(1);
    intSet->add(2);

    const hashSet moved(std::move(*intSet));
    EXPECT_EQ(moved.size(), 2);
    EXPECT_TRUE(moved.contains(1));
    EXPECT_TRUE(moved.contains(2));
    EXPECT_EQ(intSet->size(), 0); // NOLINT(bugprone-use-after-move)
}

TEST_F(HashSetTest, CopyAssignment) {
    intSet->add(1);
    intSet->add(2);

    const hashSet<int> copy = *intSet;
    EXPECT_EQ(copy.size(), 2);
    EXPECT_TRUE(copy.contains(1));
    EXPECT_TRUE(copy.contains(2));
}

TEST_F(HashSetTest, MoveAssignment) {
    intSet->add(1);
    intSet->add(2);

    const hashSet<int> moved = std::move(*intSet);
    EXPECT_EQ(moved.size(), 2);
    EXPECT_TRUE(moved.contains(1));
    EXPECT_TRUE(moved.contains(2));
    EXPECT_EQ(intSet->size(), 0); // NOLINT(bugprone-use-after-move)
}

// Exception Safety Tests
TEST_F(HashSetTest, IteratorInvalidation) {
    intSet->add(1);
    const auto it = intSet->begins();

    // Adding elements might cause rehash
    for (int i = 2; i < 100; ++i) {
        intSet->add(i);
    }

    // Iterator might be invalid after rehash
    // This depends on implementation - might need adjustment
    EXPECT_NO_THROW(it->get());
    delete it;
}

// toString Test
TEST_F(HashSetTest, ToString) {
    intSet->add(1);
    intSet->add(2);
    const std::string str = intSet->toString(false);

    // Basic checks - exact format might vary
    EXPECT_TRUE(str.find("hashSet") != std::string::npos);
    EXPECT_TRUE(str.find('1') != std::string::npos);
    EXPECT_TRUE(str.find('2') != std::string::npos);
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
TEST(HashSetCustomHashTest, CustomHashFunction) {
    hashSet<int, CustomHash<int>> customSet;
    for (int i = 0; i < 20; ++i) {
        customSet.add(i);
    }

    EXPECT_EQ(customSet.size(), 20); // All should be added despite hash collisions
}
