#include <iostream>
#include "array.h"
#include "chain.h"
#include "maths.h"
#include "prique.h"
#include "vector.h"


int main(){
    auto arr1 = original::array<int>(4);
    std::printf("%s", arr1.toString(true).c_str());
    original::array arr2 = {1, 4, 5, 3, 8};
    std::printf("%s", arr2.toString(true).c_str());
    std::printf("maximum(5,3):%d\n", original::maximum<int>(5, 3));
    auto arr3 = arr2;
    std::printf("arr3:%p, arr2:%p\n", &arr3, &arr2);
    std::printf("arr3:%s\narr2:%s\n", arr3.toString(false).c_str(), arr2.toString(false).c_str());
    auto arr4 = original::array(arr2);
    std::printf("arr4:%p, arr2:%p\n", &arr4, &arr2);
    std::printf("arr4:%s\narr2:%s\n", arr4.toString(false).c_str(), arr2.toString(false).c_str());
    std::printf("arr2[2]:%d\n", arr2[2]);
    std::printf("arr3[-1]:%d\n", arr3[-1]);
    auto arr5 = original::array<int>(0);
    auto chain1 = original::chain(arr4);
    std::printf("chain1:%s", chain1.toCString(true));
    chain1.set(2, 10);
    std::printf("chain1:%s", chain1.toCString(true));
    for (int i = 0; i < chain1.size(); ++i) {
        std::printf("chain1[%d] = %d\n", i, chain1[i]);
    }
    for (auto it = chain1.begin(); it.isValid(); it.next()) {
        std::printf("chain1 element = %d, Iterator: %s\n", it.get(), it.toCString(false));
    }
    std::printf("\n");
    for (auto it = chain1.end(); it.isValid(); it.prev()) {
        std::printf("chain1 element = %d, Iterator: %s\n", it.get(), it.toCString(false));
    }
    std::printf("\n");
    auto chain2 = original::chain({6, 7, 3, 9, 4, 2, 10, 14, -5});
    auto l = chain2.first();
    auto r = chain2.last();
    for (; r - l > 0; l.next(), r.prev()) {
        const int val = l.get();
        l.set(r.get());
        r.set(val);
    }
    for (int i = 0; i < chain2.size(); ++i) {
        std::printf("chain2[%d] = %d\n", i, chain2[i]);
    }
    std::printf("\n");
    std::printf("chain1 before:%s", chain1.toCString(true));
    chain1.forEach([](int &value) {
        value *= 2;
    });
    std::printf("chain1 after:%s", chain1.toCString(true));
    std::printf("\n");
    for (const int value : chain2) {
        std::printf("chain2 element: %d\n", value);
    }
    std::printf("\n");
    std::printf("chain2 before:%s", chain2.toCString(true));
    for (int& value : chain2) {
        value *= 3;
    }
    std::printf("chain2 after:%s", chain2.toCString(true));
    auto chain3 = original::chain<int>();
    std::printf("chain3 phase1:%s", chain3.toCString(false));
    std::printf("\n");
    for (int i = 0; i < 21; i++){
        chain3.pushBegin(i);
    }
    std::printf("chain3 phase2:%s", chain3.toCString(true));
    for (int i = 1; i < chain3.size() - 1; i += 3){
        chain3.push(i, i);
    }
    chain3.pushEnd(100);
    std::printf("chain3 phase3:%s", chain3.toCString(true));
    std::printf("Does chain3 contains 100: %s\n", original::printable::formatCString(chain3.contains(100)));
    std::printf("-3**-3=%f, 0**4=%f, 2**0=%f, 5.2**6=%f\n",
        original::pow(-3, -3), original::pow(0, 4), original::pow(2, 0), original::pow(5.2, 6));
    while (!chain3.empty()){
        int midIndex = static_cast<int>(chain3.size() / 2);
        chain3.pop(midIndex);
        std::printf("chain3: %s", chain3.toCString(true));
    }
    auto vector1 = original::vector<double>({1.3, 2.7, 5, 8.9, 4.1, 8, 9.5, 11.45, -0.7, -2, -5.8, 6.4, 23, 56, 65, 0.03, 2.07});
    std::printf("vector1: %s", vector1.toCString(true));
    std::printf("index of 9.5 in vector1: %u\n", vector1.indexOf(9.5));
     for (auto &e : vector1)
     {
         e *= 3.5;
     }
     vector1.forEach([](auto &e)
     {
         e *= 3.5;
     });
    std::printf("vector1: %s", vector1.toCString(true));
    std::printf("\n");
    while (!vector1.empty()) {
        int midIndex = static_cast<int>(vector1.size() / 2);
        vector1.pop(midIndex);
        std::printf("vector1: %s", vector1.toCString(true));
    }
    vector1.pushBegin(1);
    vector1.pushEnd(6);
    std::printf("vector1: %s", vector1.toCString(true));
    std::printf("index of 6 in vector1: %u\n", vector1.indexOf(6));
    auto vector2 = original::vector<original::vector<int>>();
    for (int i = 0; i < 10; ++i) {
        vector2.pushEnd(original::vector({1*i, 2*i, 3*i}));
    }
    std::printf("vector2: %s", vector2.toCString(true));
    for (const auto& vec: vector2) {
        std::printf("vector: %s", vec.toCString(true));
        for (auto e: vec) {
            std::printf("%d ", e);
        }
        std::printf("\n");
    }
    auto chain4 = original::chain<original::chain<int>>();
    for (int i = 0; i < 10; ++i) {
        chain4.pushEnd(original::chain({2*i, 4*i, 6*i}));
    }
    std::printf("chain4: %s", chain4.toCString(true));
    for (const auto& ch: chain4) {
        std::printf("chain: %s", ch.toCString(true));
        for (const auto e: ch) {
            std::printf("%d ", e);
        }
        std::printf("\n");
    }
    auto vector3 = original::vector<original::chain<int>>();
    for (int i = 0; i < 3; ++i) {
        vector3.pushEnd(original::chain({1*i, 3*i}));
    }
    std::printf("vector3: %s", vector3.toCString(true));
    for (const auto& ch: vector3) {
        std::printf("chain: %s", ch.toCString(true));
        for (auto e: ch) {
            std::printf("%d ", e);
        }
        std::printf("\n");
    }
    auto arr6 = original::array<original::chain<double>>(4);
    for (int i = 0; i < arr6.size(); ++i) {
        arr6[i] = original::chain({static_cast<double>(original::E*i), static_cast<double>(original::PI*i)});
    }
    std::printf("arr6: %s", arr6.toCString(true));
    for (auto & i : arr6) {
        std::printf("chain: %s", i.toCString(true));
        for (const auto e: i) {
            std::printf("%lf ", e);
        }
        std::printf("\n");
    }
    std::cout << arr6 << std::endl;
    std::cout << vector3 << std::endl;
    std::cout << vector2 << std::endl;
    auto arr7 = original::array<original::array<int>>(8);
    for (int i = 0; i < 8; ++i)
    {
        auto arr = original::array<int>(8);
        for (int j = 0; j < 8; ++j)
        {
            arr[j] = 8 * i + j;
        }
        arr7[i] = arr;
    }
    std::cout << arr7 << std::endl;
    const original::vector vector4 = {1, 2, 3, 4, 5, 6};
    std::cout << "vector4: " << vector4 << std::endl;
    auto arr8 = original::array<int>(vector4.size());
    for (auto& e: arr8) {
        e = 10;
    }
    std::cout << "arr8: " << arr8 << std::endl;
    const original::chain chain5 = {3, 5, 6, 8, 11, 10, 19, 12, 14, 7, 2, 1};
    auto arr9 = original::array<bool>(15);
    std::cout << "arr9 before: " << arr9 << std::endl;
    int i = 0;
    for (auto& e: arr9) {
        e = chain5.contains(i);
        i += 1;
    }
    std::cout << "arr9 after: " << arr9 << std::endl;
    std::cout << original::array({"hello, original!"}) << std::endl;
    struct unprintable {};
    std::cout << original::printable::formatString(unprintable{}) << std::endl;
    auto lhs = original::array<int, original::objPoolAllocator<int>>{1, 2, 3};
    auto rhs = original::array<int, original::objPoolAllocator<int>>{4, 5, 6};
    std::cout << lhs << std::endl;
    std::cout << rhs << std::endl;
    std::swap(lhs, rhs);
    std::cout << lhs << std::endl;
    std::cout << rhs << std::endl;
    auto three_way_cmp_res = arr1 <=> arr2;
    std::cout << original::printable::formatString(three_way_cmp_res) << std::endl;
    std::cout << original::printable::formatString(three_way_cmp_res < 0) << std::endl;
    std::cout << original::printable::formatString(arr1 < arr2) << std::endl;
    std::less<> less;
    std::cout << original::printable::formatString(less(arr1, arr2)) << std::endl;
    std::cout << arr1.toHash() << std::endl;
    std::cout << arr2.toHash() << std::endl;
    std::equal_to<> eq;
    std::cout << original::printable::formatString(eq(arr1, arr2)) << std::endl;
    std::hash<original::array<int>> hash;
    std::cout << hash(arr1) << std::endl;
    std::hash<original::prique<int>> hash2;
    original::prique p {4, 5, 6};
    std::cout << "------" << std::endl;
    std::cout << p.toHash() << std::endl;
    std::cout << hash2(p) << std::endl;
    return 0;
}
