#include <gtest/gtest.h>
#include <queue>
#include "prique.h"
#include "tuple.h"

#define lst {40, 20, 10, 30, 50, 70, 60, 100, 110, 50, 20, 90, 80, 80, 40}

// Custom comparator to create a min-heap instead of max-heap
template <typename T>
struct lessComparator {
    bool operator()(const T& lhs, const T& rhs) const {
        return lhs < rhs;  // Min-heap behavior
    }
};

template <typename T, typename Container = std::vector<T>, typename Comparator = std::greater<T>>
auto initPriQue(const std::initializer_list<T> &list) -> std::priority_queue<T, Container, Comparator> {
    auto pq = std::priority_queue<T, Container, Comparator>{};
    for (auto it = list.begin(); it != list.end(); ++it) {
        pq.push(*it);
    }
    return pq;
}

// Helper function to compare two priques (original::prique and std::priority_queue)
template <typename T,
          template <typename> typename Callback,
          template <typename, typename> typename SERIAL,
          typename Container = std::vector<T>,
          typename Comparator = std::greater<T>>
bool comparePriques(const original::prique<T, Callback, SERIAL>& originalPrique,
                    const std::priority_queue<T, Container, Comparator>& stdQueue) {
    std::priority_queue<T, Container, Comparator> tempStd = stdQueue;
    original::prique<T, Callback, SERIAL> tempOriginal = originalPrique;

    while (!tempOriginal.empty()) {
        if (tempOriginal.top() != tempStd.top()) {
            return false;
        }
        tempOriginal.pop();
        tempStd.pop();
    }

    // Both should be empty after popping all elements
    return tempStd.empty();
}

// Test prique with `blocksList` as the underlying container and increaseComparator as the comparator
TEST(PriqueTest, BlocksListPrique) {
    auto list = lst;
    original::prique p1 = list;
    auto p2 = initPriQue(list);

    EXPECT_EQ(p1.size(), p2.size());
    EXPECT_TRUE(comparePriques(p1, p2));

    // Test push and pop
    int val1 = 10;
    p1.push(val1);
    p2.push(val1);
    EXPECT_EQ(p1.top(), p2.top());
    EXPECT_TRUE(comparePriques(p1, p2));

    int val2 = 20;
    p1.push(val2);
    p2.push(val2);
    EXPECT_EQ(p1.top(), p2.top());
    EXPECT_TRUE(comparePriques(p1, p2));

    p1.pop();
    p2.pop();
    EXPECT_EQ(p1.top(), p2.top());
    EXPECT_TRUE(comparePriques(p1, p2));

    p1.pop();
    p2.pop();
    EXPECT_EQ(p1.top(), p2.top());
    EXPECT_TRUE(comparePriques(p1, p2));
}

// Test prique with a custom comparator
TEST(PriqueTest, CustomComparator) {
    auto list = lst;
    original::prique<int, lessComparator> p1 = list;
    auto p2 = initPriQue<int, std::vector<int>>(list);

    EXPECT_EQ(p1.size(), p2.size());
    EXPECT_TRUE(comparePriques(p1, p2));

    // Test push and pop
    int val1 = 10;
    p1.push(val1);
    p2.push(val1);
    EXPECT_EQ(p1.top(), p2.top());
    EXPECT_TRUE(comparePriques(p1, p2));

    int val2 = 20;
    p1.push(val2);
    p2.push(val2);
    EXPECT_EQ(p1.top(), p2.top());
    EXPECT_TRUE(comparePriques(p1, p2));

    p1.pop();
    p2.pop();
    EXPECT_EQ(p1.top(), p2.top());
    EXPECT_TRUE(comparePriques(p1, p2));

    p1.pop();
    p2.pop();
    EXPECT_EQ(p1.top(), p2.top());
    EXPECT_TRUE(comparePriques(p1, p2));
}

// Test prique with vector as the underlying container (heap simulation)
TEST(PriqueTest, VectorPrique) {
    auto list = lst;
    original::prique<int, original::increaseComparator, original::vector> p1 = list;
    auto p2 = initPriQue<int>(list);

    EXPECT_EQ(p1.size(), p2.size());
    EXPECT_TRUE(comparePriques(p1, p2));

    // Test push and pop
    int val1 = 10;
    p1.push(val1);
    p2.push(val1);
    EXPECT_EQ(p1.top(), p2.top());
    EXPECT_TRUE(comparePriques(p1, p2));

    int val2 = 20;
    p1.push(val2);
    p2.push(val2);
    EXPECT_EQ(p1.top(), p2.top());
    EXPECT_TRUE(comparePriques(p1, p2));

    p1.pop();
    p2.pop();
    EXPECT_EQ(p1.top(), p2.top());
    EXPECT_TRUE(comparePriques(p1, p2));

    p1.pop();
    p2.pop();
    EXPECT_EQ(p1.top(), p2.top());
    EXPECT_TRUE(comparePriques(p1, p2));
}

// Test copy constructor
TEST(PriqueTest, CopyConstructor) {
    original::prique<int> p1;
    p1.push(10);
    p1.push(20);

    const original::prique<int> p2 = p1;
    auto p3 = initPriQue<int>({});
    p3.push(20);
    p3.push(10);

    EXPECT_TRUE(comparePriques(p2, p3));
}

// Test move constructor
TEST(PriqueTest, MoveConstructor) {
    original::prique<int> p1;
    p1.push(10);
    p1.push(20);

    auto p2 = initPriQue<int>({});
    p2.push(20);
    p2.push(10);

    const original::prique<int> p3 = std::move(p1);

    EXPECT_TRUE(comparePriques(p3, p2));
    EXPECT_TRUE(p1.empty());  // p1 should be empty after move // NOLINT: Test move
}

// Test copy assignment operator
TEST(PriqueTest, CopyAssignmentOperator) {
    original::prique<int> p1;
    p1.push(10);
    p1.push(20);

    original::prique<int> p2 = p1;

    auto p3 = initPriQue<int>({});
    p3.push(20);
    p3.push(10);

    EXPECT_TRUE(comparePriques(p2, p3));
}

// Test move assignment operator
TEST(PriqueTest, MoveAssignmentOperator) {
    original::prique<int> p1;
    p1.push(10);
    p1.push(20);

    auto p2 = initPriQue<int>({});
    p2.push(20);
    p2.push(10);

    const original::prique<int> p3 = std::move(p1);

    EXPECT_TRUE(comparePriques(p3, p2));
    EXPECT_TRUE(p1.empty());  // p1 should be empty after move  // NOLINT: Test move
}

// // 自定义 move-only 类型
// struct MoveOnly {
//     int value;
//     explicit MoveOnly(const int v = 0) : value(v) {}
//     MoveOnly(const MoveOnly&) = delete;            // 禁止拷贝
//     MoveOnly& operator=(const MoveOnly&) = delete; // 禁止拷贝赋值
//     MoveOnly(MoveOnly&& other) noexcept : value(other.value) { other.value = 0; }
//     MoveOnly& operator=(MoveOnly&& other) noexcept {
//         if (this != &other) { value = other.value; other.value = 0; }
//         return *this;
//     }
//     bool operator==(const MoveOnly& other) const { return value == other.value; }
//     [[nodiscard]] int val() const noexcept { return value; }
// };
//
// template<typename TUPLE>
// struct MoveOnlyTupleComparator {
//     bool operator()(const TUPLE& lhs, const TUPLE& rhs) const {
//         return lhs.template get<0>() < rhs.template get<0>();
//     }
// };
//
// using MoveOnlyTuple = original::tuple<int, MoveOnly, MoveOnly>;
//
// TEST(PriqueMoveOnlyTupleTest, PushAndPop) {
//     // 使用自定义比较器的优先队列
//     original::prique<MoveOnlyTuple, MoveOnlyTupleComparator, original::vector> pq;
//
//     // push move-only 元素
//     pq.push(MoveOnlyTuple(10, MoveOnly(100), MoveOnly(200)));
//     pq.push(MoveOnlyTuple(5, MoveOnly(50), MoveOnly(75)));
//     pq.push(MoveOnlyTuple(20, MoveOnly(300), MoveOnly(400)));
//
//     // pop 元素，按第 0 个值比较
//     auto top = pq.pop();
//     EXPECT_EQ(top.get<0>(), 20);
//     EXPECT_EQ(top.get<1>().val(), 300);
//     EXPECT_EQ(top.get<2>().val(), 400);
//
//     top = pq.pop();
//     EXPECT_EQ(top.get<0>(), 10);
//     EXPECT_EQ(top.get<1>().val(), 100);
//     EXPECT_EQ(top.get<2>().val(), 200);
//
//     top = pq.pop();
//     EXPECT_EQ(top.get<0>(), 5);
//     EXPECT_EQ(top.get<1>().val(), 50);
//     EXPECT_EQ(top.get<2>().val(), 75);
// }
//
// TEST(PriqueMoveOnlyTupleTest, TopPeek) {
//     original::prique<MoveOnlyTuple, MoveOnlyTupleComparator, original::vector> pq;
//
//     pq.push(MoveOnlyTuple(3, MoveOnly(30), MoveOnly(60)));
//     pq.push(MoveOnlyTuple(7, MoveOnly(70), MoveOnly(140)));
//
//     // top 不 pop
//     auto top = pq.top();
//     EXPECT_EQ(top.get<0>(), 7);
//     EXPECT_EQ(top.get<1>().val(), 70);
//     EXPECT_EQ(top.get<2>().val(), 140);
//
//     // 确认 pop 后仍然正确
//     top = pq.pop();
//     EXPECT_EQ(top.get<0>(), 7);
// }
