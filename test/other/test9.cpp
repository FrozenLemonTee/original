#include "executors.h"
#include "singleton.h"

int main()
{
    original::singleton<original::taskDelegator>::init(4u);
    auto& delegator = original::singleton<original::taskDelegator>::instance();
    original::singleton<original::threadPoolExecutor>::init(delegator);
    auto& thread_pool = original::singleton<original::threadPoolExecutor>::instance();
    auto increase = [](const int x){
        return x + 1;
    };
    auto add_1 = [](const int x){
        return x + 1;
    };
    auto add_2 = [](const int x){
        return x + 2;
    };
    auto add_3 = [](const int x){
        return x + 3;
    };
    auto chain_parallel1 = original::coroutine::makeTask(thread_pool, increase, 0)
        >> original::coParallel(add_1, add_2, add_3);
    auto [res1, res2, res3] = original::coroutine::spinRun(std::move(chain_parallel1));
    std::cout << original::printable::formatStrings("res1 = ", res1) << std::endl;
    std::cout << original::printable::formatStrings("res2 = ", res2) << std::endl;
    std::cout << original::printable::formatStrings("res3 = ", res3) << std::endl;
    original::array flags1 {false, false, false};
    auto void_1 = [&flags1] {
        flags1[0] = true;
    };
    auto void_2 = [&flags1]{
        flags1[1] = true;
    };
    auto void_3 = [&flags1]{
        flags1[2] = true;
    };
    auto chain_parallel2 = thread_pool
        >> original::coParallel(void_1, void_2, void_3);
    auto [res4, res5, res6] = original::coroutine::spinRun(std::move(chain_parallel2));
    std::cout << original::printable::formatStrings("res4 = ", res4) << std::endl;
    std::cout << original::printable::formatStrings("res5 = ", res5) << std::endl;
    std::cout << original::printable::formatStrings("res6 = ", res6) << std::endl;
    std::cout << original::printable::formatStrings("flags1 = ", flags1) << std::endl;
    auto chain_condition1 = thread_pool
        >> []{return 0;}
        >> original::coIfElse(
            [](const int x){ return x > 0; },
            add_1,
            add_2
           );
    auto res7 = original::coroutine::spinRun(std::move(chain_condition1));
    std::cout << original::printable::formatStrings("res7 = ", res7) << std::endl;

    auto fn = [](const int a, const int b, const int c, const int d) {
        return a + b + c + d;
    };
    auto chain3  = thread_pool >> (original::coBind(1) | original::coBind(2, 3)) >> original::coBind(4, 5) >> fn;
    auto res8 = original::coroutine::spinRun(std::move(chain3));
    std::cout << original::printable::formatStrings("res8 = ", res8) << std::endl;
    return 0;
}
