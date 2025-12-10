#include "tuple.h"
#include "array.h"
#include <gtest/gtest.h>
#include <string>

using namespace original;

TEST(TupleTest, ConstructorAndGet) {
    tuple<int, double, std::string> t(42, 3.14, "hello");

    // Test get() methods
    EXPECT_EQ(t.get<0>(), 42);
    EXPECT_DOUBLE_EQ(t.get<1>(), 3.14);
    EXPECT_EQ(t.get<2>(), "hello");

    // Test const get()
    const auto& ct = t;
    EXPECT_EQ(ct.get<0>(), 42);
    EXPECT_DOUBLE_EQ(ct.get<1>(), 3.14);
    EXPECT_EQ(ct.get<2>(), "hello");
}

TEST(TupleTest, SetMethod) {
    tuple<int, double, std::string> t(0, 0.0, "");

    // Test basic set operations
    t.set<0>(100);
    t.set<1>(2.718);
    t.set<2>("updated");

    EXPECT_EQ(t.get<0>(), 100);
    EXPECT_DOUBLE_EQ(t.get<1>(), 2.718);
    EXPECT_EQ(t.get<2>(), "updated");

    // Test method chaining
    t.set<0>(200).set<1>(1.618).set<2>("chained");
    EXPECT_EQ(t.get<0>(), 200);
    EXPECT_DOUBLE_EQ(t.get<1>(), 1.618);
    EXPECT_EQ(t.get<2>(), "chained");

    // Test type conversion
    t.set<0>(3.14f);  // float to int
    t.set<1>(42);     // int to double
    EXPECT_EQ(t.get<0>(), 3);
    EXPECT_DOUBLE_EQ(t.get<1>(), 42.0);
}

TEST(TupleTest, SetMethodEdgeCases) {
    tuple<int, std::string> t(0, "");

    // Test setting with different but convertible types
    short s = 10;
    t.set<0>(s);
    EXPECT_EQ(t.get<0>(), 10);

    const char* cstr = "C-string";
    t.set<1>(cstr);
    EXPECT_EQ(t.get<1>(), "C-string");

    // Test out of bounds (compile-time check)
    // Uncomment to verify compile error:
//     t.set<2>("invalid"); // Should fail to compile
}

TEST(TupleTest, StructuredBinding) {
    tuple<int, std::string, double> t(42, "hello", 3.14);

    // Test structured binding with non-const tuple
    auto& [i, s, d] = t;
    EXPECT_EQ(i, 42);
    EXPECT_EQ(s, "hello");
    EXPECT_DOUBLE_EQ(d, 3.14);

    // Test modification through binding
    i = 100;
    s = "world";
    d = 2.718;
    EXPECT_EQ(t.get<0>(), 100);
    EXPECT_EQ(t.get<1>(), "world");
    EXPECT_DOUBLE_EQ(t.get<2>(), 2.718);

    // Test structured binding with const tuple
    const tuple<int, std::string, double> ct(42, "const", 3.14);
    const auto& [ci, cs, cd] = ct;
    EXPECT_EQ(ci, 42);
    EXPECT_EQ(cs, "const");
    EXPECT_DOUBLE_EQ(cd, 3.14);

    // Test structured binding with rvalue tuple
    auto&& [ri, rs, rd] = tuple<int, std::string, double>(1, "rvalue", 2.0);
    EXPECT_EQ(ri, 1);
    EXPECT_EQ(rs, "rvalue");
    EXPECT_DOUBLE_EQ(rd, 2.0);
}

TEST(TupleTest, MoveSemantics) {
    tuple<std::string, array<int>> t1("test", array{1, 2, 3});
    auto t2 = std::move(t1);

    // Verify move worked
    EXPECT_EQ(t2.get<0>(), "test");
    EXPECT_EQ(t2.get<1>(), (array{1, 2, 3}));

    // Verify original was moved from
    EXPECT_TRUE(t1.get<0>().empty()); // NOLINT: Test move
    EXPECT_TRUE(t1.get<1>().empty());

    // Test move assignment
    tuple<std::string, array<int>> t3;
    t3 = std::move(t2);
    EXPECT_EQ(t3.get<0>(), "test");
    EXPECT_EQ(t3.get<1>(), (array{1, 2, 3}));
    EXPECT_TRUE(t2.get<0>().empty()); // NOLINT: Test move
    EXPECT_TRUE(t2.get<1>().empty());
}

TEST(TupleTest, Comparison) {
    tuple<int, std::string> t1(1, "apple");
    tuple<int, std::string> t2(1, "banana");
    tuple<int, std::string> t3(2, "apple");
    tuple<int, std::string> t4(1, "apple");

    // Test comparison operators
    EXPECT_TRUE(t1 < t2);
    EXPECT_TRUE(t1 < t3);
    EXPECT_FALSE(t1 < t4);
    EXPECT_TRUE(t1 == t4);
    EXPECT_FALSE(t1 == t2);

    // Test compareTo method
    EXPECT_LT(t1.compareTo(t2), 0);
    EXPECT_GT(t3.compareTo(t1), 0);
    EXPECT_EQ(t1.compareTo(t4), 0);
}

TEST(TupleTest, SliceOperation) {
    tuple<int, double, std::string, char> t(1, 2.3, "slice", 'A');

    // Test basic slice
    auto t1 = t.slice<1, 2>();
    EXPECT_EQ(t1.size(), 2);
    EXPECT_DOUBLE_EQ(t1.get<0>(), 2.3);
    EXPECT_EQ(t1.get<1>(), "slice");

    // Test edge slice cases
    auto t2 = t.slice<0, 4>();
    EXPECT_EQ(t2.size(), 4);
    EXPECT_EQ(t2.get<3>(), 'A');

    auto t3 = t.slice<3, 1>();
    EXPECT_EQ(t3.size(), 1);
    EXPECT_EQ(t3.get<0>(), 'A');

    // Test out of bounds (compile-time check)
    // Uncomment to verify compile error:
    // auto t4 = t.slice<2, 3>(); // Should fail to compile
}

TEST(TupleTest, Concatenation) {
    tuple t1(1, 2.3);
    tuple<std::string, char> t2("concat", 'X');

    // Test basic concatenation
    auto t3 = t1 + t2;
    EXPECT_EQ(t3.size(), 4);
    EXPECT_EQ(t3.get<0>(), 1);
    EXPECT_DOUBLE_EQ(t3.get<1>(), 2.3);
    EXPECT_EQ(t3.get<2>(), "concat");
    EXPECT_EQ(t3.get<3>(), 'X');

    // Test chained concatenation
    tuple t4(true);
    auto t5 = t1 + t2 + t4 + tuple{1, 2.3};
    EXPECT_EQ(t5.size(), 7);
    EXPECT_TRUE(t5.get<4>());
}

TEST(TupleTest, MakeTupleFromCouple) {
    couple<int, std::string> cp(99, "couple");
    auto t = makeTuple(cp);

    EXPECT_EQ(t.size(), 2);
    EXPECT_EQ(t.get<0>(), 99);
    EXPECT_EQ(t.get<1>(), "couple");

    // Verify it's a proper tuple with all tuple operations
    t.set<0>(100);
    EXPECT_EQ(t.get<0>(), 100);
}

TEST(TupleTest, StdGetSupport) {
    tuple<int, std::string> t(42, "std::get");

    // Test std::get support
    EXPECT_EQ(std::get<0>(t), 42);
    EXPECT_EQ(std::get<1>(t), "std::get");

    // Test modification through std::get
    std::get<0>(t) = 100;
    EXPECT_EQ(t.get<0>(), 100);

    // Test const std::get
    const auto& ct = t;
    EXPECT_EQ(std::get<0>(ct), 100);
}

TEST(TupleTest, SetMethodChaining) {
    // 1. 基础链式调用测试
    tuple<int, std::string, double> t1;
    t1.set<0>(42).set<1>("链式").set<2>(3.14);

    EXPECT_EQ(t1.get<0>(), 42);
    EXPECT_EQ(t1.get<1>(), "链式");
    EXPECT_DOUBLE_EQ(t1.get<2>(), 3.14);

    // 2. 测试中间修改是否影响链式调用
    auto& ref = t1.set<0>(100);  // 返回引用应允许继续链式调用
    ref.set<1>("修改").set<2>(6.28);

    EXPECT_EQ(t1.get<0>(), 100);     // 第一次set结果
    EXPECT_EQ(t1.get<1>(), "修改");   // 链式第二次set结果
    EXPECT_DOUBLE_EQ(t1.get<2>(), 6.28); // 链式第三次set结果

    // 3. 边界条件测试（首个/末尾元素）
    tuple<int, int, int> t2;
    t2.set<0>(1).set<2>(3);  // 跳过中间元素
    EXPECT_EQ(t2.get<0>(), 1);
    EXPECT_EQ(t2.get<2>(), 3);  // get<1>()应保持默认值

    // 4. 类型安全测试（编译期检查）
    // 以下代码应无法编译（静态断言失败）：
    //     t1.set<0>("字符串").set<1>(42);  // 错误：类型不匹配
}

// 简单的move-only类型（复用CoupleMoveOnlyTest里的实现）
class MoveOnly {
    std::unique_ptr<int> data_;

public:
    explicit MoveOnly(int value = 0) : data_(std::make_unique<int>(value)) {}

    // 删除拷贝构造函数和拷贝赋值
    MoveOnly(const MoveOnly&) = delete;
    MoveOnly& operator=(const MoveOnly&) = delete;

    // 允许移动构造函数和移动赋值
    MoveOnly(MoveOnly&& other) noexcept = default;
    MoveOnly& operator=(MoveOnly&& other) noexcept = default;

    [[nodiscard]] int value() const { return data_ ? *data_ : -1; }

    bool operator==(const MoveOnly& other) const { return *data_ == *other.data_; }
    bool operator!=(const MoveOnly& other) const { return *data_ != *other.data_; }
    bool operator<(const MoveOnly& other) const { return *data_ < *other.data_; }
    bool operator>(const MoveOnly& other) const { return *data_ > *other.data_; }
    bool operator<=(const MoveOnly& other) const { return *data_ <= *other.data_; }
    bool operator>=(const MoveOnly& other) const { return *data_ >= *other.data_; }

    friend std::ostream& operator<<(std::ostream& os, const MoveOnly& mo) {
        return os << mo.value();
    }
};

// ========== Tuple 的 move-only 测试 ==========
TEST(TupleMoveOnlyTest, DefaultConstructor) {
    tuple<MoveOnly, MoveOnly> t;
    EXPECT_EQ(t.get<0>().value(), 0);
    EXPECT_EQ(t.get<1>().value(), 0);
}

TEST(TupleMoveOnlyTest, MoveConstructor) {
    MoveOnly a(42);
    MoveOnly b(100);
    tuple t(std::move(a), std::move(b));
    EXPECT_EQ(t.get<0>().value(), 42);
    EXPECT_EQ(t.get<1>().value(), 100);
}

TEST(TupleMoveOnlyTest, TupleMoveConstructor) {
    tuple t1(MoveOnly(1), MoveOnly(2));
    tuple t2(std::move(t1));

    EXPECT_EQ(t2.get<0>().value(), 1);
    EXPECT_EQ(t2.get<1>().value(), 2);
}

TEST(TupleMoveOnlyTest, TupleMoveAssignment) {
    tuple t1(MoveOnly(3), MoveOnly(4));
    tuple<MoveOnly, MoveOnly> t2 = std::move(t1);

    EXPECT_EQ(t2.get<0>().value(), 3);
    EXPECT_EQ(t2.get<1>().value(), 4);
}

TEST(TupleMoveOnlyTest, GetMethod) {
    tuple t(MoveOnly(10), MoveOnly(20));

    MoveOnly& a = t.get<0>();
    MoveOnly& b = t.get<1>();
    EXPECT_EQ(a.value(), 10);
    EXPECT_EQ(b.value(), 20);

    a = MoveOnly(30);
    b = MoveOnly(40);
    EXPECT_EQ(t.get<0>().value(), 30);
    EXPECT_EQ(t.get<1>().value(), 40);
}

TEST(TupleMoveOnlyTest, StructuredBinding) {
    tuple<MoveOnly, MoveOnly> t(MoveOnly(50), MoveOnly(60));

    auto&& [x, y] = t;
    EXPECT_EQ(x.value(), 50);
    EXPECT_EQ(y.value(), 60);

    x = MoveOnly(70);
    y = MoveOnly(80);
    EXPECT_EQ(t.get<0>().value(), 70);
    EXPECT_EQ(t.get<1>().value(), 80);
}

TEST(TupleMoveOnlyTest, Comparison) {
    tuple t1(MoveOnly(1), MoveOnly(2));
    tuple t2(MoveOnly(1), MoveOnly(2));
    tuple t3(MoveOnly(3), MoveOnly(4));

    EXPECT_EQ(t1.compareTo(t2), 0);
    EXPECT_LT(t1.compareTo(t3), 0);
    EXPECT_GT(t3.compareTo(t1), 0);
}

TEST(TupleMoveOnlyTest, MixedTypes) {
    tuple<std::unique_ptr<int>, std::string> t;
    t.get<0>() = std::make_unique<int>(123);
    t.get<1>() = "tuple test";

    EXPECT_EQ(*t.get<0>(), 123);
    EXPECT_EQ(t.get<1>(), "tuple test");

    auto t2 = std::move(t);
    EXPECT_EQ(*t2.get<0>(), 123);
    EXPECT_EQ(t2.get<1>(), "tuple test");
    EXPECT_EQ(t.get<0>(), nullptr); // NOLINT: moved-from state
}

TEST(TupleMoveOnlyTest, TupleConcatenationForMoveOnly) {
    tuple t1(MoveOnly(1), MoveOnly(2));
    tuple t2(MoveOnly(3), MoveOnly(4));
    tuple t3(MoveOnly(5), MoveOnly(6));
    tuple concat{std::move(t1) + std::move(t2) + std::move(t3)};

    EXPECT_EQ(concat.get<0>().value(), 1);
    EXPECT_EQ(concat.get<1>().value(), 2);
    EXPECT_EQ(concat.get<2>().value(), 3);
    EXPECT_EQ(concat.get<3>().value(), 4);
    EXPECT_EQ(concat.get<4>().value(), 5);
    EXPECT_EQ(concat.get<5>().value(), 6);

    EXPECT_EQ(t1.get<0>().value(), -1);
    EXPECT_EQ(t1.get<1>().value(), -1);
    EXPECT_EQ(t2.get<0>().value(), -1);
    EXPECT_EQ(t2.get<1>().value(), -1);
    EXPECT_EQ(t3.get<0>().value(), -1);
    EXPECT_EQ(t3.get<1>().value(), -1);
}