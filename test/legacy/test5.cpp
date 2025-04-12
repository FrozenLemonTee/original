#include <iostream>
#include "chain.h"
#include "maps.h"
#include "ownerPtr.h"
#include "refCntPtr.h"
#include "sets.h"
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
    auto p7 = original::makeOwnerPtrArray<int>(size);
    for (int i = 0; i < size; ++i) {
        p7[i] = i;
    }
    for (int i = 0; i < size; ++i) {
        std::cout << p7[i] << std::endl;
    }
    auto p8 = original::makeOwnerPtrArray<original::array<int>>(10);
    std::cout << p8 << ", " << *p8 << std::endl;
    const auto p9 = original::makeStrongPtr<original::array<int>>(std::initializer_list{10, 7});
    std::cout << p9 << ", " << *p9 << std::endl;
    auto p10 = original::makeStrongPtrArray<int>(size);
    auto p11 = p10;
    for (int i = 0; i < size; ++i) {
        p11[i] = 0;
    }
    if (p11) {
        p11[0] = 8;
        p11[1] = 9;
        p11[2] = 10;
        std::cout << "p11 is valid" << std::endl;
    }
    std::cout << p11 << std::endl;
    for (int i = 0; i < size; ++i) {
        std::cout << p11[i] << std::endl;
    }
    auto p12 = original::makeStrongPtrArray<original::array<int>>(10, 8);
    for (original::u_integer i = 0; i < 10; ++i)
    {
        std::cout << p12[i] << std::endl;
    }
    auto vectorPool = original::vector<int, original::objPoolAllocator<int>>();
    auto chainPool = original::chain<int, original::objPoolAllocator<int>>({1,2,3,4,5,6,7,8,9,10});
    for (int i = 0; i < 20; ++i) {
        vectorPool.pushEnd(i);
    }
    for (auto num: vectorPool) {
        std::cout << num << std::endl;
    }
    auto copy_vector_pool = vectorPool;
    auto move_vector_pool = std::move(vectorPool);
    auto chainPool2 = original::chain<int, original::objPoolAllocator<int>>({11,12,13,14});
    chainPool += chainPool2;
    std::cout << copy_vector_pool << std::endl;
    std::cout << chainPool << std::endl;
    std::cout << chainPool2 << std::endl;
    auto arr = original::array({1, 5, 3, 1, 1, 3, 2, 4, 2, 1, 4, 4, 5, 2});
    auto cnt = original::hashMap<int, int, original::hash<int>, original::objPoolAllocator<int>>();
    auto exist = original::hashSet<int>();
    for (auto num: arr) {
        if (!cnt.containsKey(num)) {
            cnt.add(num, 1);
        }else {
            cnt[num] += 1;
        }
        if (!exist.contains(num)) {
            exist.add(num);
        }
    }
    for (int i = 1; i < 6; i++) {
        std::cout << "cnt[" << i << "] = " << cnt[i] << std::endl;
        std::cout << i << " at exist: " << original::printable::formatString(exist.contains(i)) << std::endl;
    }
    auto map1 = original::hashMap<std::string, std::string>();
    map1["foo"] = "bar";
    map1["bar"] = "baz";
    map1["baz"] = "qux";
    std::cout << map1["foo"] << std::endl;
    std::cout << map1["bar"] << std::endl;
    std::cout << map1["baz"] << std::endl;
    return 0;
}
