#include <gtest/gtest.h>
#include "maps.h"
#include <string>
#include <vector>
#include <algorithm>

using namespace original;

class JMapTest : public testing::Test {
protected:
    void SetUp() override {
        // Common setup for all tests
        intMap = new JMap<int, int>();
        stringMap = new JMap<std::string, int>();
    }

    void TearDown() override {
        delete intMap;
        delete stringMap;
    }

    JMap<int, int>* intMap{};
    JMap<std::string, int>* stringMap{};
};

namespace {
    template<typename>
    struct CustomCompare {
        bool operator()(const int a, const int b) const {
            return a > b; // Reverse order
        }
    };
}

// Basic Functionality Tests
TEST_F(JMapTest, InitialState) {
EXPECT_EQ(intMap->size(), 0);
EXPECT_TRUE(intMap->className() == "JMap");
}

TEST_F(JMapTest, AddAndContains) {
EXPECT_TRUE(intMap->add(42, 100));
EXPECT_EQ(intMap->size(), 1);
EXPECT_TRUE(intMap->containsKey(42));
EXPECT_FALSE(intMap->containsKey(43));
EXPECT_EQ(intMap->get(42), 100);

EXPECT_TRUE(stringMap->add("test", 200));
EXPECT_TRUE(stringMap->containsKey("test"));
EXPECT_EQ(stringMap->get("test"), 200);
}

TEST_F(JMapTest, AddDuplicate) {
EXPECT_TRUE(intMap->add(10, 1));
EXPECT_FALSE(intMap->add(10, 2)); // Adding duplicate key should fail
EXPECT_EQ(intMap->size(), 1);
EXPECT_EQ(intMap->get(10), 1); // Value should remain unchanged
}

TEST_F(JMapTest, Remove) {
intMap->add(1, 10);
intMap->add(2, 20);
EXPECT_TRUE(intMap->remove(1));
EXPECT_EQ(intMap->size(), 1);
EXPECT_FALSE(intMap->containsKey(1));
EXPECT_TRUE(intMap->containsKey(2));
EXPECT_EQ(intMap->get(2), 20);

EXPECT_FALSE(intMap->remove(99)); // Remove non-existent key
}

TEST_F(JMapTest, Update) {
intMap->add(1, 10);
EXPECT_TRUE(intMap->update(1, 100));
EXPECT_EQ(intMap->get(1), 100);
EXPECT_FALSE(intMap->update(2, 200)); // Update non-existent key
}

TEST_F(JMapTest, OperatorAccess) {
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

// Iterator Tests - JMap should maintain sorted order
TEST_F(JMapTest, IteratorOrder) {
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

TEST_F(JMapTest, IteratorEnd) {
intMap->add(1, 10);
const auto begin = intMap->begins();
const auto end = intMap->ends();

EXPECT_TRUE(begin->isValid());
EXPECT_TRUE(end->isValid()); // ends() returns iterator to last element in JMap
delete begin;
delete end;
}

// Boundary Tests
TEST_F(JMapTest, LargeNumberOfElements) {
constexpr int count = 100000;
for (int i = 0; i < count; ++i) {
EXPECT_TRUE(intMap->add(i, i * 10));
EXPECT_EQ(intMap->size(), i + 1);
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

TEST_F(JMapTest, StringKeyElements) {
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
TEST_F(JMapTest, CopyConstructor) {
intMap->add(1, 10);
intMap->add(2, 20);

const JMap copy(*intMap);
EXPECT_EQ(copy.size(), 2);
EXPECT_TRUE(copy.containsKey(1));
EXPECT_TRUE(copy.containsKey(2));
EXPECT_EQ(copy.get(1), 10);
EXPECT_EQ(copy.get(2), 20);
}

TEST_F(JMapTest, MoveConstructor) {
intMap->add(1, 10);
intMap->add(2, 20);

const JMap moved(std::move(*intMap));
EXPECT_EQ(moved.size(), 2);
EXPECT_TRUE(moved.containsKey(1));
EXPECT_TRUE(moved.containsKey(2));
EXPECT_EQ(moved.get(1), 10);
EXPECT_EQ(moved.get(2), 20);
EXPECT_EQ(intMap->size(), 0); // NOLINT(bugprone-use-after-move)
}

TEST_F(JMapTest, CopyAssignment) {
intMap->add(1, 10);
intMap->add(2, 20);

const JMap<int, int> copy = *intMap;
EXPECT_EQ(copy.size(), 2);
EXPECT_TRUE(copy.containsKey(1));
EXPECT_TRUE(copy.containsKey(2));
EXPECT_EQ(copy.get(1), 10);
EXPECT_EQ(copy.get(2), 20);
}

TEST_F(JMapTest, MoveAssignment) {
intMap->add(1, 10);
intMap->add(2, 20);

const JMap<int, int> moved = std::move(*intMap);
EXPECT_EQ(moved.size(), 2);
EXPECT_TRUE(moved.containsKey(1));
EXPECT_TRUE(moved.containsKey(2));
EXPECT_EQ(moved.get(1), 10);
EXPECT_EQ(moved.get(2), 20);
EXPECT_EQ(intMap->size(), 0); // NOLINT(bugprone-use-after-move)
}

// Custom Comparator Test
TEST(JMapCustomCompareTest, CustomCompareFunction) {

JMap<int, int, CustomCompare<int>> customMap;
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
TEST_F(JMapTest, ToString) {
intMap->add(1, 10);
intMap->add(2, 20);
const std::string str = intMap->toString(false);

// Basic checks - exact format might vary
EXPECT_TRUE(str.find("JMap") != std::string::npos);
EXPECT_TRUE(str.find('1') != std::string::npos);
EXPECT_TRUE(str.find("10") != std::string::npos);
EXPECT_TRUE(str.find('2') != std::string::npos);
EXPECT_TRUE(str.find("20") != std::string::npos);
}

// Contains with value test
TEST_F(JMapTest, ContainsKeyValuePair) {
intMap->add(1, 10);
intMap->add(2, 20);

EXPECT_TRUE(intMap->contains(couple<const int, int>(1, 10)));
EXPECT_FALSE(intMap->contains(couple<const int, int>(1, 20))); // Wrong value
EXPECT_FALSE(intMap->contains(couple<const int, int>(3, 30))); // Key doesn't exist
}

// Test sequential insertion and iteration
TEST_F(JMapTest, SequentialInsertion) {
constexpr int count = 1000;
for (int i = 0; i < count; ++i) {
EXPECT_TRUE(intMap->add(i, i * 10));
}

EXPECT_EQ(intMap->size(), count);

// Verify all elements are present and in order
auto it = ownerPtr(intMap->begins());
int expected = 0;
while (it->isValid()) {
EXPECT_EQ(it->get().first(), expected);
EXPECT_EQ(it->get().second(), expected * 10);
it->next();
expected++;
}
}

// Test random insertion order
TEST_F(JMapTest, RandomInsertionOrder) {
std::vector<int> keys = {5, 2, 8, 1, 9, 3, 7, 4, 6, 0};
for (int key : keys) {
EXPECT_TRUE(intMap->add(key, key * 10));
}

EXPECT_EQ(intMap->size(), keys.size());

// Verify all elements are present and in order
auto it = ownerPtr(intMap->begins());
int expected = 0;
while (it->isValid()) {
EXPECT_EQ(it->get().first(), expected);
EXPECT_EQ(it->get().second(), expected * 10);
it->next();
expected++;
}
}

// Test iterator distance functionality
TEST_F(JMapTest, IteratorDistance) {
for (int i = 0; i < 10; ++i) {
intMap->add(i, i * 10);
}

auto it1 = ownerPtr(intMap->begins());
auto it2 = ownerPtr(intMap->begins());

// Move second iterator forward
it2->next();
it2->next();
it2->next();

// Check distance
integer distance = *it2 - *it1;
EXPECT_EQ(distance, 3);
}