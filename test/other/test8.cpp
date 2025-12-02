#include "coroutines.h"
#include "executors.h"
#include "singleton.h"
#include "tasks.h"

int main()
{
    original::singleton<original::taskDelegator>::init();
    auto& delegator = original::singleton<original::taskDelegator>::instance();
    original::singleton<original::threadPoolExecutor>::init(delegator);
    auto& thread_pool = original::singleton<original::threadPoolExecutor>::instance();
    original::singleton<original::syncExecutor>::init();
    auto& event_loop = original::singleton<original::syncExecutor>::instance();
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
    auto res1 = event_loop.spinWait(std::move(chain1));
    std::cout << original::printable::formatStrings("res1 = ", res1) << std::endl;
    std::cout << original::printable::formatStrings("flag1 = ", flag1) << std::endl;

    auto task4 = original::coroutine::makeTask(thread_pool, [](const int x)
    {
        return x * 2;
    }, 10);
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
    auto chain2 = task5 | task6 | task7;
    auto res2 = event_loop.spinWait(std::move(chain2));
    std::cout << original::printable::formatStrings("res2 = ", res2) << std::endl;
    std::cout << original::printable::formatStrings("arr = ", arr) << std::endl;
    return 0;
}
