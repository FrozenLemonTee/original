#include <iostream>
#include "chain.h"
#include "ownerPtr.h"
#include "refCntPtr.h"
#include "vector.h"


int main(){
    const auto raw1 = new int(25);
    auto p1 = original::ownerPtr(raw1);
    std::cout << *p1 << std::endl;
    *p1 = 100;
    std::cout << *p1 << std::endl;
    original::ownerPtr<int> p2 = std::move(p1);
    auto p3 = original::makeOwnerPtr<original::vector<int>>();
    for (int i = 0; i < 10; ++i) {
        std::cout << p3 << ", " << *p3 << std::endl;
        p3->pushEnd(i);
    }
    auto p4 = original::makeOwnerPtr<original::chain<int>>();
    for (int i = 0; i < 10; ++i) {
        std::cout << p4 << ", " << *p4 << std::endl;
        p4->pushEnd(i);
    }
    auto p5 = original::makeOwnerPtr<original::chain<int>>();
    *p5 = std::move(*p4);
    std::cout << *p5 << std::endl;
    std::cout << *p4 << std::endl;
    auto p6 = original::ownerPtr(p5->begins());
    while (p6->isValid()) {
        std::cout << **p6 << std::endl;
        p6->operator++();
    }
    std::cout << std::endl;
    constexpr int size = 10;
    auto p7 = original::makeOwnerPtr<int>(size);
    for (int i = 0; i < size; ++i) {
        p7[i] = i;
    }
    for (int i = 0; i < size; ++i) {
        std::cout << p7[i] << std::endl;
    }
    auto p8 = original::ownerPtr<original::array<int>>(10);
    std::cout << p8 << ", " << *p8 << std::endl;
    const auto p9 = original::strongPtr<original::array<int>>();
    std::cout << p9 << ", " << *p9 << std::endl;
    return 0;
}
