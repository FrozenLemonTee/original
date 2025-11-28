#ifndef ORIGINAL_EXECUTOR_H
#define ORIGINAL_EXECUTOR_H

#include <coroutine>

namespace original {

    class executor {
    public:
        class awaitable {
            executor& executor_;

        public:
            explicit awaitable(executor& executor);

            [[nodiscard]] bool await_ready() const noexcept;

            void await_suspend(std::coroutine_handle<> handle) const noexcept;

            void await_resume() const noexcept;
        };

        virtual ~executor() = default;

        virtual void schedule(std::coroutine_handle<> handle) = 0;

        awaitable operator co_await() noexcept;
    };
}

original::executor::awaitable::awaitable(executor& executor)
    : executor_(executor) {}

bool original::executor::awaitable::await_ready() const noexcept { // NOLINT
    return false;
}

void original::executor::awaitable::await_suspend(
     std::coroutine_handle<> handle) const noexcept {
    this->executor_.schedule(handle);
}

void original::executor::awaitable::await_resume() const noexcept {}

original::executor::awaitable
original::executor::operator co_await() noexcept {
    return awaitable{*this};
}

#endif //ORIGINAL_EXECUTOR_H
