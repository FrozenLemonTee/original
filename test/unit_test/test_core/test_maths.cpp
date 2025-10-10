#include <gtest/gtest.h>
#include "maths.h"
#include <cmath>

using namespace original;

// Tests for abs
TEST(MathsTests, AbsIntPositive) {
    EXPECT_EQ(original::abs(5), std::abs(5));
}

TEST(MathsTests, AbsIntNegative) {
    EXPECT_EQ(original::abs(-5), std::abs(-5));
}

TEST(MathsTests, AbsDoublePositive) {
    EXPECT_EQ(original::abs(5.5), std::abs(5.5));
}

TEST(MathsTests, AbsDoubleNegative) {
    EXPECT_EQ(original::abs(-5.5), std::abs(-5.5));
}

// Tests for maximum
TEST(MathsTests, MaxInt) {
    EXPECT_EQ(original::maximum(3, 5), std::max(3, 5));
}

TEST(MathsTests, MaxDouble) {
    EXPECT_EQ(original::maximum(3.5, 5.5), std::max(3.5, 5.5));
}

TEST(MathsTests, MaxEqual) {
    EXPECT_EQ(original::maximum(3, 3), std::max(3, 3));
}

// Tests for minimum
TEST(MathsTests, MinInt) {
    EXPECT_EQ(original::minimum(3, 5), std::min(3, 5));
}

TEST(MathsTests, MinDouble) {
    EXPECT_EQ(original::minimum(3.5, 5.5), std::min(3.5, 5.5));
}

TEST(MathsTests, MinEqual) {
    EXPECT_EQ(original::minimum(3, 3), std::min(3, 3));
}

// Tests for pow
TEST(MathsTests, PowPositiveExp) {
    EXPECT_EQ(original::pow(2.0, 3), std::pow(2.0, 3));
}

TEST(MathsTests, PowNegativeExp) {
    EXPECT_EQ(original::pow(2.0, -3), std::pow(2.0, -3));
}

TEST(MathsTests, PowZeroBase) {
    EXPECT_EQ(original::pow(0.0, 5), std::pow(0.0, 5));
}

TEST(MathsTests, PowZeroExp) {
    EXPECT_EQ(original::pow(2.0, 0), std::pow(2.0, 0));
}

TEST(MathsTests, PowZeroBaseZeroExp) {
    EXPECT_THROW(original::pow(0.0, 0), valueError);
}

TEST(MathsTests, RangesOfSimpleRange) {
    auto gen = rangesOf<integer>(0, 5);
    std::vector<integer> result;
    while (auto val = gen.next()) {
        result.push_back(*val);
    }
    EXPECT_EQ((std::vector<integer>{0, 1, 2, 3, 4}), result);
}

TEST(MathsTests, RangesOfStepRange) {
    auto gen = rangesOf(0, 10, 2);
    std::vector<int> result;
    while (auto val = gen.next()) {
        result.push_back(*val);
    }
    EXPECT_EQ((std::vector{0, 2, 4, 6, 8}), result);
}

TEST(MathsTests, RangesOfNegativeStepRange) {
    auto gen = rangesOf(5, -1, -2);
    std::vector<int> result;
    while (auto val = gen.next()) {
        result.push_back(*val);
    }
    EXPECT_EQ((std::vector{5, 3, 1}), result);
}

TEST(MathsTests, RangesOfPeekDoesNotAdvance) {
    auto gen = rangesOf(0, 3);
    EXPECT_FALSE(gen.peek()); // peek first
    if (auto val = gen.next()) {
        EXPECT_EQ(0, *val); // next returns same value
    }
    if (auto val = gen.peek()) {
        EXPECT_EQ(0, *val); // peek moves with next
    }
    if (auto val = gen.next()) {
        EXPECT_EQ(1, *val);
    }
}

TEST(MathsTests, RangesOfEmptyWhenInvalidStep) {
    auto gen = rangesOf(0, 10, 0); // steps == 0
    EXPECT_FALSE(gen.next());
}

TEST(MathsTests, RangesOfEmptyWhenStartBeyondEnd) {
    auto gen = rangesOf(10, 0, 1); // start > end
    EXPECT_FALSE(gen.next());
}

TEST(MathsTests, RangesOfIteratorTraverse){
    auto gen = rangesOf<integer>(0, 10);
    std::vector<integer> result;
    for (auto it = gen.begin();it != coroutine::generator<integer>::end(); ++it)
    {
        result.push_back(*it);
    }
    EXPECT_EQ((std::vector<integer>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}), result);
}

TEST(MathsTests, RangesOfIteratorRangedFor){
    auto gen = rangesOf<integer>(0, 10);
    std::vector<integer> result;
    for (const auto& elem : gen)
    {
        result.push_back(elem);
    }
    EXPECT_EQ((std::vector<integer>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}), result);

    auto gen2 = rangesOf(10, 0, -1);
    std::vector<int> result2;
    for (const auto& elem : gen2)
    {
        result2.push_back(elem);
    }
    EXPECT_EQ((std::vector{10, 9, 8, 7, 6, 5, 4, 3, 2, 1}), result2);
}

TEST(MathsTests, RangesOfIteratorEdgeCase) {
    auto g = rangesOf(0, 10, 0); // 步长为0
    EXPECT_EQ(g.begin(), g.end());

    auto end_it = coroutine::generator<int>::end();
    EXPECT_THROW(*end_it, nullPointerError);
}