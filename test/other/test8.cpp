#include <random>

#include "coroutines.h"
#include "awaitable.h"
#include "executors.h"
#include "singleton.h"
#include "tasks.h"


using namespace original::literals;

int main()
{
    original::singleton<original::taskDelegator>::init();
    auto& delegator = original::singleton<original::taskDelegator>::instance();
    original::singleton<original::threadPoolExecutor>::init(delegator);
    auto& thread_pool = original::singleton<original::threadPoolExecutor>::instance();
    bool flag1 = false;
    auto task1 = original::coroutine::makeTask(thread_pool, [&flag1]
    {
        flag1 = true;
    });
    auto task2 = original::coroutine::makeTask(thread_pool, [](const int x)
    {
        return x + 1;
    }, 2);
    auto task3 = original::coroutine::makeTask(thread_pool, [](const original::floating n)
    {
       return n > 6.3;
    }, 6.5);
    auto chain1 = task1 | std::move(task2) | std::move(task3);
    auto res1 = original::coroutine::spinRun(std::move(chain1));
    std::cout << original::printable::formatStrings("res1 = ", res1) << std::endl;
    std::cout << original::printable::formatStrings("flag1 = ", flag1) << std::endl;
    auto arr = original::array{0, 0, 0};
    auto task5 = original::coroutine::makeTask(thread_pool, [&arr](const int x)
    {
        arr[0] = -1;
        return x + 1;
    }, 0);
    auto task6 = [&arr]
    {
        arr[1] = 1;
    };
    auto task7 = [&arr]
    {
        arr[2] = 2;
        return 1;
    };
    auto chain2 = (task5 | task6) >> task7;
    auto res2 = original::coroutine::spinRun(std::move(chain2));
    std::cout << original::printable::formatStrings("res2 = ", res2) << std::endl;
    std::cout << original::printable::formatStrings("arr = ", arr) << std::endl;
    auto increase = [](const int x){
        return x + 1;
    };
    auto task8 = original::coroutine::makeTask(thread_pool, increase, 0);
    auto chain3 = task8 >> increase >> increase >> increase;
    auto res3 = original::coroutine::spinRun(std::move(chain3));
    std::cout << original::printable::formatStrings("res3 = ", res3) << std::endl;

    bool complete = false;
    auto rand = [&complete] {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution dis(1, 100);
        complete = true;
        const auto res = dis(gen);
        std::cout << res << std::endl;
        return res;
    };
    auto judge = [](const int num){
        const auto res = num > 50;
        std::cout << original::printable::formatString(res) << std::endl;
        return res;
    };
    auto str = [](const bool res) -> std::string {
        return res ? "Larger than 50" : "Less than 50";
    };
    original::taskDelegator delegator2{2};
    original::threadPoolExecutor thread_pool_local{delegator2};
    auto rand_task = original::coroutine::makeTask(thread_pool, rand);
    auto chain4 = rand_task >> judge >> thread_pool_local >> str >> original::coDelay(200_ms);
    auto res4 = original::coroutine::run(std::move(chain4));
    std::cout << original::printable::formatStrings("res4 = ", res4) << std::endl;
    std::cout << original::printable::formatStrings("complete = ", complete) << std::endl;
    auto err = [](const original::floating div) -> original::floating {
        if (div == 0) {
            throw original::valueError("divide by zero");
        }
        return 1.0 / div;
    };
    auto catch_handler = [](const std::exception& e) -> original::floating {
        std::cout << "Caught an exception: " << e.what() << std::endl;
        return 0;
    };
    auto err_task = original::coroutine::makeTask(thread_pool, err, 0);
    auto err_chain = err_task >> original::coCatch<original::valueError>(catch_handler);
    auto res5 = original::coroutine::run(std::move(err_chain));
    std::cout << original::printable::formatStrings("res5 = ", res5) << std::endl;
    auto multi_err = [](const int x){
        if (x == 0)
            throw original::valueError("Element is " + original::printable::formatString(x));
        if (x == 1)
            throw original::noElementError("Element is " + original::printable::formatString(x));
        return x;
    };
    auto err_task2 = original::coroutine::makeTask(thread_pool_local, multi_err, 1);
    auto catch_handler2 = [](const original::valueError& e){
        std::cout << "Caught an exception: " << e.what() << std::endl;
        return 111;
    };
    auto catch_handler3 = [](const original::noElementError& e){
        std::cout << "Caught an exception: " << e.what() << std::endl;
        return 222;
    };
    auto err_chain2 = err_task2
            | original::coCatch<original::valueError>(catch_handler2)
            | original::coCatch<original::noElementError>(catch_handler3);
    auto res6 = original::coroutine::run(std::move(err_chain2));
    std::cout << original::printable::formatStrings("res6 = ", res6) << std::endl;
    original::singleton<original::eventsLoopExecutor>::init();
    auto& event_loop = original::singleton<original::eventsLoopExecutor>::instance();
    auto task9 = original::coroutine::makeTask(event_loop, increase, 1);
    task9.start();
    event_loop.runOnce();
    auto res7 = original::coroutine::spinRun(std::move(task9));
    std::cout << original::printable::formatStrings("res7 = ", res7) << std::endl;
    auto when_all1 = original::coroutine::makeTask(thread_pool, increase, 0);
    auto when_all2 = original::coroutine::makeTask(thread_pool, increase, 1);
    auto when_all3 = original::coroutine::makeTask(thread_pool, increase, 2);
    bool when_all_flag1 = false;
    auto when_all4 = original::coroutine::makeTask(thread_pool, [&when_all_flag1]{
        when_all_flag1 = true;
    });
    const auto& [wa_res1, wa_res2, wa_res3, wa_res4] = original::coroutine::whenAll(std::move(when_all1), std::move(when_all2), std::move(when_all3), std::move(when_all4));
    std::cout << original::printable::formatStrings("wa_res1 = ", wa_res1) << std::endl;
    std::cout << original::printable::formatStrings("wa_res2 = ", wa_res2) << std::endl;
    std::cout << original::printable::formatStrings("wa_res3 = ", wa_res3) << std::endl;
    std::cout << original::printable::formatStrings("wa_res4 = ", wa_res4) << std::endl;
    std::cout << original::printable::formatStrings("when_all_flag1 = ", when_all_flag1) << std::endl;
    return 0;
}
