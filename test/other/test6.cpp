#include <iostream>

#include "atomic.h"
#include "array.h"
#include "allocator.h"
#include "thread.h"
#include "maps.h"
#include "mutex.h"
#include "zeit.h"
#include "condition.h"
#include "optional.h"
#include "prique.h"


using namespace original::literals;

int main()
{
    std::cout << "id of main thread: " << original::thread::thisId() << std::endl;
    original::mutex mutex;
    auto task1 = [&](const int a, const std::string& b)
    {
        original::uniqueLock lock{mutex};
        std::cout << b << a << std::endl;
    };
    original::thread t1{task1, 1, "show: "};
    original::thread::sleep(100_ms);
    std::cout << "id t1: " << t1.id() << std::endl;
    t1.join();
    original::thread t2{task1, 2, "show: "};
    t2.join();
    original::thread t3{task1, 3, "show: "};
    auto t4 = std::move(t3);
    t4.join();

    original::thread t5{task1, 4, "show: "};
    original::thread t6{task1, 5, "show: "};
    if (t5) {
        std::cout << "t5 is valid" << std::endl;
    }
    t6 = std::move(t5);
    if (!t5) {
        std::cout << "t5 is not valid" << std::endl;
    }
    t6.join();

    original::thread t7{task1, 6, "show: "};
    original::thread t8{task1, 7, "show: "};

    original::thread t9{task1, 8, "show: "};
    original::thread t10{std::move(t9), original::thread::AUTO_JOIN};
    original::thread::sleep(100_ms);
    std::cout << "t10 id: " << t10.id() << std::endl;
    original::thread t11{task1, 9, "show: "};
    original::thread t12{std::move(t11)};

    original::thread t13;

    original::thread t14{task1, 10, "show: "};
    t14.detach();

    auto j1 = original::JMap<int, int>();
    for (const auto l = {0, 1, 2, 4, 2, 2}; const auto& e: l) {
        if (j1.containsKey(e)){
            j1[e] += 1;
        } else {
            j1.add(e, 1);
        }
    }
    std::cout << "j1: " << j1 << std::endl;


    auto j2 = original::JMap<int, int>();
    for (int i = 0; i < 20; ++i) {
        j2.add(i, 1);
    }
    std::cout << j2 << std::endl;
    for (int i = 19; i >= 0; --i) {
        j2.remove(i);
    }
    std::cout << j2 << std::endl;

    std::cout << "On win64: " << original::printable::formatString(original::ON_WIN64()) << std::endl;
    std::cout << "Using GCC: " << original::printable::formatString(original::USING_GCC()) << std::endl;

    original::mutex m1;
    original::mutex m2;
    original::multiLock ml{m1, m2};

    auto d1 = original::time::duration(100, original::time::MILLISECOND);
    std::cout << d1 << std::endl;

    original::alternative<bool> res;
    std::cout << "res.get(): " << original::printable::formatString(res.get()) << std::endl;
    res.emplace(true);
    std::cout << "res.get(): " << original::printable::formatString(res.get()) << std::endl;
    std::cout << "value res: " << original::printable::formatString(*res) << std::endl;
    std::cout << "value res: " << original::printable::formatString(*res.get()) << std::endl; // NOLINT: Test alternative::get

    const auto now_utc = original::time::UTCTime::now();
    std::cout << "London now: " << now_utc << std::endl;
    std::cout << "Beijing now: " << now_utc + 8_h << std::endl;
    std::cout << "now_utc.weekday(): " << static_cast<original::integer>(now_utc.weekday()) << std::endl;

    const auto local_now = original::time::UTCTime::localNow();
    std::cout << (original::time::UTCTime{2025, 8, 7} - local_now.date()).value(original::time::DAY) << std::endl;
    std::cout << (local_now.date() - original::time::UTCTime{2025, 7, 27}).value(original::time::DAY) << std::endl;

    std::cout << "two days after today: " << now_utc.date() + 1_d + original::days() << std::endl;

    constexpr original::integer sec = 5;
    std::cout << "Sleep before: " << local_now << std::endl;
    std::cout << "Sleep for: " << sec << " second(s)." << std::endl;
    original::thread::sleep(original::seconds(sec));
    std::cout << "Sleep after: " << original::time::UTCTime::localNow() << std::endl;

    original::atomic<bool> flag = original::makeAtomic(true);
    std::cout << original::printable::formatString(*flag) << std::endl;
    std::cout << "-----" << std::endl;
    original::tuple swap_t1{original::array<int, original::objPoolAllocator<int>>{1, 2, 3}};
    original::tuple swap_t2{original::array<int, original::objPoolAllocator<int>>{4, 5, 6}};
    std::cout << swap_t1 << std::endl;
    std::cout << swap_t2 << std::endl;
    std::swap(swap_t1, swap_t2);
    std::cout << swap_t1 << std::endl;
    std::cout << swap_t2 << std::endl;
    std::cout << "std::to_string(swap_t2) = " << std::to_string(swap_t2) << std::endl;
    original::prique swap_p1 {original::array<int, original::objPoolAllocator<int>>{1, 2, 3}};
    original::prique swap_p2 {original::array<int, original::objPoolAllocator<int>>{4, 5, 6}};
    swap_p1.swap(swap_p2);
    std::cout << swap_p1.pop() << std::endl;
    std::cout << swap_p2.pop() << std::endl;
    return 0;
}