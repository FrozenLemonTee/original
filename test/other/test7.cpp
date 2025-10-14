#include <iostream>
#include "array.h"
#include "async.h"
#include "singleton.h"
#include "tasks.h"
#include "maths.h"
#include "coroutines.h"
#include "generators.h"

original::array<int> matrixAdd(const original::array<int>& a, const original::array<int>& b)
{
    const auto len = original::maximum(a.size(), b.size());
    original::array<int> result(len);
    for (original::integer i = 0; i < len; i++) {
        if (i < a.size())
            result[i] += a[i];
        if (i < b.size())
            result[i] += b[i];
    }
    original::thread::sleep(original::seconds(1));
    return result;
}

int main() {
    auto simple_func = []{
        original::thread::sleep(original::seconds(1));
        return 0;
    };

    auto add_func = [](const int a, const int b){
        original::thread::sleep(original::seconds(1));
        return a + b;
    };

    auto sub_func = [](const int a, const int b) {
        original::thread::sleep(original::seconds(1));
        return a - b;
    };

    auto simple_func2 = [](const int a){
        original::thread::sleep(original::seconds(1));
        std::cout << "res = " << a << std::endl;
        return a;
    };

    auto pp = original::async::makePromise(simple_func2, 5);
    auto ff = pp.getFuture();
    original::thread  t{
        [pp = original::makeStrongPtr<decltype(pp)>(std::move(pp))] mutable
        {
            pp->run();
        }
    };
    std::cout << ff.result() << std::endl;

    std::cout << original::async::get(simple_func).result() << std::endl;
    std::cout << original::async::get(add_func, 1, 5).result() << std::endl;
    std::cout << original::async::get(matrixAdd, original::array{1, 2, 3, 4}, original::array{2, 4, 5, 7, 12}).result() << std::endl;
    std::cout << original::async::get(sub_func, 5, 1).result() << std::endl;

    original::singleton<original::taskDelegator>::init();
    auto& delegator = original::singleton<original::taskDelegator>::instance();
    auto futures = original::vector<original::async::future<int>>{};
    for (original::integer i = 0; i < 12; i++) {
        futures.pushEnd(delegator.submit(simple_func2, i));
    }
    for (auto& future : futures) {
        std::cout << future.result() << std::endl;
    }

    std::cout << "test generator1:" << std::endl;

    auto ranges = original::rangesOf(0, 100, 6);
    while (auto val = ranges.next()) {
        std::cout << *val << std::endl;
    }
    std::cout << "test generator2:" << std::endl;
    for (const int i : original::rangesOf(1, 5)) {
        std::cout << i << " "; // Output: 1 2 3 4
    }
    std::cout << std::endl;
    std::cout << "test filters:" << std::endl;
    auto r = original::filters(original::rangesOf(0, 11), [](const int e)
    {
        return e % 2 == 0;
    });
    for (const auto val : r)
    {
        std::cout << val << " ";
    }
    std::cout << std::endl;
    std::cout << "test enum:" << std::endl;
    auto vowels = original::array<std::string>{"a", "e", "i", "o", "u"};
    for (auto enums = original::enumerate(vowels.generator()); auto [idx, str] : enums)
    {
        std::cout << idx << ": " << str << std::endl;
    }
    auto ch = original::list<int, original::chain>(original::rangesOf(1, 11));
    std::cout << ch << std::endl;
    auto str1 = original::array<std::string>{"k", "e", "g", "m", "v"};
    auto str2 = original::vector<std::string>{"o", "q", "l", "s"};
    for (auto str_zip_gen = original::zip(str1.generator(), str2.generator()); auto [s1, s2] : str_zip_gen)
    {
        std::cout << s1 << " - " << s2 << std::endl;
    }
    auto nums = original::array{1, 2, 3};
    auto trans = original::transforms(nums.generator(), [](const int x)
    {
        return original::array{x, x * 10};
    });
    for (auto nums2 = original::list(std::move(trans)); const auto& x : nums2)
    {
        std::cout << x << std::endl;
    }
    std::cout << "test join:" << std::endl;
    for (auto repeat = original::join(nums.generator(), nums.generator()); const auto x : repeat)
    {
        std::cout << x << std::endl;
    }
    std::cout << "------" << std::endl;
    auto nums3 = original::array{1.1, 2.2, 3.3};
    for (auto join_gen = original::join(nums.generator(), nums3.generator()); const auto x : join_gen)
    {
        std::cout << x << std::endl;
    }

    auto result = vowels.generator()
    | original::filters([](const std::string& s) { return !s.empty(); })
    | original::transforms([](const std::string& s) { return s + "!"; })
    | original::take(2)
    | original::enumerate();

    for (auto [idx, str] : result) {
        std::cout << idx << ": " << str << std::endl;
    }
    original::hashSet<int> s;
    s.add(1);
    s.add(3);
    s.add(7);
    for (auto set_gen = s.generator(); auto e : set_gen)
    {
        std::cout << "hash set elem: " << e << std::endl;
    }
    return 0;
}
