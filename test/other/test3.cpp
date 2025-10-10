#include "bitSet.h"
#include "deque.h"

#include "stack.h"
#include <iostream>
#include "prique.h"
#include "queue.h"
#include "chain.h"
#include "vector.h"
#include "algorithms.h"


int main(){
    original::chain<std::string> c1 = {"a", "aa", "bc", "vg", "rtqy", "upn"};
    original::vector<std::string> v1 = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10"};
    std::cout << "v1 before1: " << v1 << std::endl;
    original::algorithms::copy(c1.first(), c1.last(), v1.first());
    std::cout << "v1 after1: " << v1 << std::endl;
    original::algorithms::fill(v1.first(), v1.last());
    std::cout << "v1 before2: " << v1 << std::endl;
    original::algorithms::copy(c1.first(), c1.last(), v1.first(),
                               original::greaterFilter(std::string("bc")));
    std::cout << "v1 after2: " << v1 << std::endl;
    std::cout << "c1 before1: " << c1 << std::endl;
    original::algorithms::reverse(c1.first(), c1.last());
    std::cout << "c1 after1: " << c1 << std::endl;
    original::stack<int> s1;
    std::cout << "s1 before1: " << s1 << std::endl;
    for (int i = 0; i < v1.size(); i++) {
        s1.push(i);
    }
    std::cout << "s1 after1: " << s1 << std::endl;
    original::queue<int> q1;
    std::cout << "q1 before1: " << q1 << std::endl;
    for (int i = 0; i < v1.size(); i++) {
        q1.push(i);
    }
    std::cout << "q1 after1: " << q1 << std::endl;
    for (int i = 0; i < v1.size() / 2; i++) {
        q1.pop();
    }
    std::cout << "q1 after2: " << q1 << std::endl;
    const auto q2 = original::queue(q1);
    std::cout << "q2: " << q2 << std::endl;
    auto s2 = original::stack<int, original::vector>();
    std::cout << "s2: " << s2 << std::endl;
    for (int i = 0; i < 50; ++i) {
        s2.push(i);
    }
    std::cout << "s2: " << s2 << std::endl;
    for (int i = 0; i < 18; ++i) {
        s2.pop();
    }
    std::cout << "s2.top(): " << s2.top() << std::endl;
    auto c2 = original::chain({1, 3, 6, 9, 8, 4, 2});
    auto c3 = original::chain({7, 5});
    std::cout << c2 << " size: " << c2.size() << std::endl;
    std::cout << c3 << " size: " << c3.size() << std::endl;
    c2 += c3;
    std::cout << c2 << " size: " << c2.size() << std::endl;
    std::cout << c3 << " size: " << c3.size() << std::endl;
    original::stack<original::stack<int>> s3;
    for (int i = 0; i < 10; ++i) {
        auto ss = original::stack<int>();
        ss.push(i);
        s3.push(ss);
    }
    std::cout << "s3 before: " << s3 << std::endl;
    s3.pop();
    s3.pop();
    std::cout << "s3 after: " << s3 << std::endl;
    original::queue<original::queue<int>, original::vector> q3;
    for (int i = 0; i < 10; ++i) {
        auto qq = original::queue<int>();
        qq.push(i);
        q3.push(qq);
    }
    std::cout << "q3 before: " << q3 << std::endl;
    q3.pop();
    q3.pop();
    std::cout << "q3 after: " << q3 << std::endl;
    original::stack<original::queue<int>> s4;
    original::deque<int> d1;
    for (int i = 0; i < 10; ++i) {
        d1.pushBegin(i);
        d1.pushEnd(i);
    }
    std::cout << "d1 before: " << d1 << std::endl;
    for (int i = 0; i < 4; ++i) {
        d1.popBegin();
    }
    for (int i = 0; i < 3; ++i) {
        d1.popEnd();
    }
    std::cout << "d1 after: " << d1 << std::endl;
    auto b1 = original::bitSet({true, false, false, true, true, false, true});
    std::cout << "b1: " << b1 << std::endl;
    auto b2 = original::bitSet(20);
    std::cout << "b2 before: " << b2 << std::endl;
    for (int i = 1, index = 1; index < b2.size(); i++, index <<= 1) {
        b2.set(index - 1, true);
    }
    std::cout << "b2 after: " << b2 << std::endl;
    std::cout << "b2.get(3) = " << b2.get(3) << std::endl;
    std::cout << "b2.get(4) = " << b2.get(4) << std::endl;
    auto b3 = original::bitSet(100);
    std::cout << "b3 before: " << b3 << std::endl;
    b3.set(88, true);
    b3.set(86, false);
    std::cout << "b3 after: " << b3 << std::endl;
    std::cout << "b3.get(67) = " << b3.get(67) << std::endl;
    std::cout << "b3.get(88) = " << b3.get(88) << std::endl;
    // for (auto e: b1) {
    //     std::cout << e << std::endl;
    // } // Unsupported Method for class
    // original::bitSet<>::forEach(); //deleted
    b3.set(99, true);
    std::cout << "b3.get(99) = " << b3.get(99) << std::endl;
    auto b4 = b3.resize(129);
    std::cout << "b4: " << b4 << std::endl;
    std::cout << "b4.get(67) = " << b4.get(67) << std::endl;
    std::cout << "b4.get(88) = " << b4.get(88) << std::endl;
    std::cout << "b4.get(99) = " << b4.get(99) << std::endl;
    auto b5 = original::bitSet({true, false, true, false, true, false, true, false, true});
    auto b6 = original::bitSet({false, true, false, true, false, true, false, true, false});
    auto b7 = b5 | b6;
    auto b8 = b5 & b6;
    auto b9 = b5 ^ b6;
    auto b10 = ~b5;
    std::cout << "b5: " << b5 << std::endl;
    std::cout << "b6: " << b6 << std::endl;
    std::cout << "b7: " << b7 << std::endl;
    std::cout << "b8: " << b8 << std::endl;
    std::cout << "b9: " << b9 << std::endl;
    std::cout << "b10: " << b10 << std::endl;
    auto b11 = b4 | b5;
    std::cout << "b11: " << b11 << std::endl;
    auto b12 = (b4 | b5) & b6;
    std::cout << "b12: " << b12 << std::endl;
    std::cout << "b11.count() = " << b11.count() << std::endl;
    std::cout << "b12.count() = " << b12.count() << std::endl;
    std::cout << "b12.size() = " << b12.size() << std::endl;
    std::cout << "b12.last() - b12.first() = " << original::algorithms::distance(b12.last(), b12.first()) << std::endl;
    std::cout << "b12.end() - b12.begin() = " << original::algorithms::distance(b12.end(), b12.begin()) << std::endl;
    auto p1 = original::prique({10, 4, 5, 7, 6, 3, 2});
    auto p2 = original::prique(p1);
    std::cout << "p1 == p2: " << original::printable::formatString(p1 == p2) << std::endl;
    std::cout << "p1 < p2: " << original::printable::formatString(p1 < p2) << std::endl;
    auto cmp = p1 <=> p2;
    std::cout << (cmp == 0) << std::endl;
    std::cout << "p1: " << p1 << std::endl;
    while (!p1.empty())
    {
        std::cout << p1.pop() << std::endl;
        std::cout << "p1: " << p1 << std::endl;
    }
    std::cout << "p1 < p2: " << original::printable::formatString(p1 < p2) << std::endl;
    for (const auto v2 = original::vector({11, 8, 13, 16, 15}); const auto e : v2)
    {
        std::cout << "p2: " << p2 << std::endl;
        p2.push(e);
    }
    std::cout << "p2: " << p2 << std::endl;
    while (!p2.empty())
    {
        std::cout << "p2.top() = " << p2.top() << std::endl;
        std::cout << p2.pop() << std::endl;
        std::cout << "p2: " << p2 << std::endl;
    }
    auto p3 = original::prique<int, original::decreaseComparator>({11, 8, 13, 16, 15, 5, 7, 6, 3, 2});
    std::cout << "p3: " << p3 << std::endl;
    while (!p3.empty())
    {
        std::cout << p3.pop() << std::endl;
        std::cout << "p3: " << p3 << std::endl;
    }
    std::cout << c2 << std::endl;
    original::algorithms::forEach(c2.first(), c2.last(), original::addOptTransform(2), original::lessFilter(4));
    std::cout << c2 << std::endl;
    return 0;
}