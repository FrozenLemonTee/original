#include <gtest/gtest.h>
#include <algorithm> // std algorithm
#include <array>
#include "array.h"
#include "vector.h"
#include "algorithms.h" // original algorithm
#include "comparator.h"

namespace original {

    // 测试 distance 方法
    TEST(AlgorithmsTest, DistanceTest) {
        const array arr = {1, 2, 3, 4, 5};
        const auto beginIt = arr.first();
        const auto endIt = arr.last();
        EXPECT_EQ(algorithms::distance(endIt, beginIt), 4);
    }

    // 测试 frontOf 方法
    TEST(AlgorithmsTest, FrontOfTest) {
        const array arr = {1, 2, 3, 4, 5};
        const auto beginIt = arr.first();
        const auto newIt = algorithms::frontOf(beginIt, 2);
        EXPECT_EQ(newIt->get(), 3);
    }

    // 测试 backOf 方法
    TEST(AlgorithmsTest, BackOfTest) {
        const array arr = {1, 2, 3, 4, 5};
        const auto endIt = arr.last();
        const auto newIt = algorithms::backOf(endIt, 1);
        EXPECT_EQ(newIt->get(), 4);
    }

    // 测试 allOf 方法
    TEST(AlgorithmsTest, AllOfTest) {
        const vector vec = {1, 2, 3, 4, 5};
        const greaterFilter greaterThan2(2);
        EXPECT_FALSE(algorithms::allOf(vec.first(), vec.last(), greaterThan2));
    }

    // 测试 anyOf 方法
    TEST(AlgorithmsTest, AnyOfTest) {
        const vector vec = {1, 2, 3, 4, 5};
        const greaterFilter greaterThan4(4);
        EXPECT_TRUE(algorithms::anyOf(vec.first(), vec.last(), greaterThan4));
    }

    // 测试 noneOf 方法
    TEST(AlgorithmsTest, NoneOfTest) {
        const vector vec = {1, 2, 3, 4, 5};
        const lessFilter lessThan1(1);
        EXPECT_TRUE(algorithms::noneOf(vec.first(), vec.last(), lessThan1));
    }

    // 测试 find 方法（通过值查找）
    TEST(AlgorithmsTest, FindTest) {
        const vector vec = {1, 2, 3, 4, 5};
        const auto result = algorithms::find(vec.first(), vec.last(), 3);
        EXPECT_EQ(result->get(), 3);
    }

    // 测试 count 方法（计数元素出现次数）
    TEST(AlgorithmsTest, CountTest) {
        const vector vec = {1, 2, 3, 3, 4, 5};
        EXPECT_EQ(algorithms::count(vec.first(), vec.last(), 3), 2);
    }

    // 测试 equal 方法
    TEST(AlgorithmsTest, EqualTest) {
        const vector vec1 = {1, 2, 3, 4, 5};
        const vector vec2 = {1, 2, 3, 4, 5};
        EXPECT_TRUE(algorithms::equal(vec1.first(), vec1.last(), vec2.first(), vec2.last()));

        const vector vec3 = {1, 2, 3, 4};
        EXPECT_FALSE(algorithms::equal(vec1.first(), vec1.last(), vec3.first(), vec3.last()));
    }

    // 测试 forEach 方法（执行操作）
    TEST(AlgorithmsTest, ForEachTest) {
        const array arr = {1, 2, 3, 4, 5};
        int sum = 0;
        auto acc = [&sum](const int val) {
            sum += val;
        };
        algorithms::forEach(arr.first(), arr.last(), acc);
        EXPECT_EQ(sum, 15);
    }

    // 测试 fill 方法（填充值）
    TEST(AlgorithmsTest, FillTest) {
        const array arr = {1, 2, 3, 4, 5};
        algorithms::fill(arr.first(), arr.last(), 10);
        for (const auto it = arr.begin(); it != arr.end(); it.next()) {
            EXPECT_EQ(it.get(), 10);
        }
    }

    // 测试 swap 方法（交换元素）
    TEST(AlgorithmsTest, SwapTest) {
        const array arr = {1, 2, 3, 4, 5};
        const auto it1 = arr.first();
        const auto it2 = arr.first();
        it2.next();
        algorithms::swap(it1, it2);
        EXPECT_EQ(it1.get(), 2);
        EXPECT_EQ(it2.get(), 1);
    }

    // 测试 copy 方法（拷贝元素）
    TEST(AlgorithmsTest, CopyTest) {
        const array arr1 = {1, 2, 3, 4, 5};
        const array arr2 = {0, 0, 0, 0, 0};
        algorithms::copy(arr1.first(), arr1.last(), arr2.first());
        for (auto it = arr1.begin(); it != arr1.end(); it.next()) {
            EXPECT_EQ(it.get(), arr2.get(it - arr1.begin()));
        }
    }

    // 测试 reverse 方法（反转元素）
    TEST(AlgorithmsTest, ReverseTest) {
        const array arr = {1, 2, 3, 4, 5};
        algorithms::reverse(arr.first(), arr.last());
        const int expected[] = {5, 4, 3, 2, 1};
        int idx = 0;
        for (const auto it = arr.begin(); it != arr.end(); it.next()) {
            EXPECT_EQ(it.get(), expected[idx++]);
        }
    }

    // 测试 heapSort 方法（堆排序）
    TEST(AlgorithmsTest, HeapSortTest) {
        #define lst1 {5, 8, 7, 4, 3, 1, 6, 2, 0, 9}
        array originalArr1 = lst1;
        std::array stdArr1 = lst1;
        algorithms::heapSort(originalArr1.first(), originalArr1.last(), increaseComparator<int>());
        std::ranges::sort(stdArr1);
        for (u_integer i = 0; i < originalArr1.size(); i++){
            EXPECT_EQ(originalArr1[i], stdArr1[i]);
        }
    }

    TEST(AlgorithmsTest, InsertionSortTest){
        #define lst2 {5, 8, 7, 2, 8, 10, 4, 3, 1, 4, 6, 2, 0, 9}
        array originalArr1 = lst2;
        std::array stdArr1 = lst2;
        algorithms::insertionSort(originalArr1.first(), originalArr1.last(), increaseComparator<int>());
        std::ranges::sort(stdArr1);
        for (u_integer i = 0; i < originalArr1.size(); i++){
            EXPECT_EQ(originalArr1[i], stdArr1[i]);
        }
        array originalArr2 = lst2;
        std::array stdArr2 = lst2;
        algorithms::insertionSort(originalArr2.first(), originalArr2.last(), decreaseComparator<int>());
        std::ranges::sort(stdArr2, std::greater());
        for (u_integer i = 0; i < originalArr2.size(); i++){
            EXPECT_EQ(originalArr2[i], stdArr2[i]);
        }
    }

    TEST(AlgorithmsTest, IntroSortTest){
        #define lst3 {5, 8, 7, 2, 8, 10, 4, 3, 1, 4, 6, 2, 0, 9, 11, 14, 15, 12, 13, 19, 18, 16, 17, 20}
        array originalArr1 = lst3;
        std::array stdArr1 = lst3;
        algorithms::introSort(originalArr1.first(), originalArr1.last(), increaseComparator<int>());
        std::ranges::sort(stdArr1);
        for (u_integer i = 0; i < originalArr1.size(); i++){
            EXPECT_EQ(originalArr1[i], stdArr1[i]);
        }
        array originalArr2 = lst3;
        std::array stdArr2 = lst3;
        algorithms::introSort(originalArr2.first(), originalArr2.last(), decreaseComparator<int>());
        std::ranges::sort(stdArr2, std::greater());
        for (u_integer i = 0; i < originalArr2.size(); i++){
            EXPECT_EQ(originalArr2[i], stdArr2[i]);
        }
    }

    TEST(AlgorithmsTest, StableSortTest){
        #define lst5 {5, 8, 7, 2, 8, 10, 4, 3, 1, 4, 6, 2, 7, 0, 9, 11, 14, 4, 15, 12, 13, 0, 19, 18, 16, 17, 20, 8, 12}
        array originalArr1 = lst5;
        std::array stdArr1 = lst5;
        algorithms::stableSort(originalArr1.first(), originalArr1.last(), increaseComparator<int>());
        std::ranges::stable_sort(stdArr1);
        for (u_integer i = 0; i < originalArr1.size(); i++){
            EXPECT_EQ(originalArr1[i], stdArr1[i]);
        }
        array originalArr2 = lst5;
        std::array stdArr2 = lst5;
        algorithms::stableSort(originalArr2.first(), originalArr2.last(), decreaseComparator<int>());
        std::ranges::stable_sort(stdArr2, std::greater());
        for (u_integer i = 0; i < originalArr2.size(); i++){
            EXPECT_EQ(originalArr2[i], stdArr2[i]);
        }
    }

    TEST(AlgorithmsTest, SortTest){
        #define lst4 {5, 8, 7, 2, 8, 10, -8, 4, 3, 1, 21, 17, 19, 35, 4, 25, 6, 2, 0, -2, 31, 9, 11, 14, 15, 12, 13, 19, 18, 16, 17, 20}
        array originalArr1 = lst4;
        std::array stdArr1 = lst4;
        algorithms::sort(originalArr1.first(), originalArr1.last(), increaseComparator<int>());
        std::ranges::sort(stdArr1);
        for (u_integer i = 0; i < originalArr1.size(); i++){
            EXPECT_EQ(originalArr1[i], stdArr1[i]);
        }
        array originalArr2 = lst4;
        std::array stdArr2 = lst4;
        algorithms::sort(originalArr2.first(), originalArr2.last(), decreaseComparator<int>());
        std::ranges::sort(stdArr2, std::greater());
        for (u_integer i = 0; i < originalArr2.size(); i++){
            EXPECT_EQ(originalArr2[i], stdArr2[i]);
        }

        array originalArr3 = lst4;
        std::array stdArr3 = lst4;
        algorithms::sort(originalArr3.first(), originalArr3.last(), increaseComparator<int>(), true);
        std::ranges::stable_sort(stdArr3);
        for (u_integer i = 0; i < originalArr3.size(); i++){
            EXPECT_EQ(originalArr3[i], stdArr3[i]);
        }
        array originalArr4 = lst4;
        std::array stdArr4 = lst4;
        algorithms::sort(originalArr4.first(), originalArr4.last(), decreaseComparator<int>(), true);
        std::ranges::stable_sort(stdArr4, std::greater());
        for (u_integer i = 0; i < originalArr4.size(); i++){
            EXPECT_EQ(originalArr4[i], stdArr4[i]);
        }
    }
}
