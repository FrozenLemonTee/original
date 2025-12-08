#ifndef ORIGINAL_AWAITABLE_H
#define ORIGINAL_AWAITABLE_H
#include <utility>
#include "zeit.h"
#include "executor.h"

namespace original {
    class delayAwaiter {
        time::duration duration_;

        explicit delayAwaiter(time::duration duration);

    public:
        void await_resume() const noexcept;

        template<typename Promise>
        void await_suspend(std::coroutine_handle<Promise> h) const;

        bool await_ready() const noexcept;

        friend delayAwaiter coDelay(time::duration duration);
    };

    template<typename E, typename Handle>
    class errorCatchAwaitable {
        Handle handle_;

        explicit errorCatchAwaitable(Handle h);
    public:
        using ExceptionType = E;
        struct catchAwaiter {
            Handle handle_;

            [[nodiscard]] bool await_ready() const noexcept;

            template<typename Promise>
            void await_suspend(std::coroutine_handle<Promise> h) const noexcept;

            const Handle& await_resume() const noexcept;
        };
        Handle& handle() noexcept;

        catchAwaiter operator co_await() const noexcept;

        template<typename Exception, typename Handler>
        friend auto coCatch(Handler&& handle);
    };

    delayAwaiter coDelay(time::duration duration);

    template<typename Exception, typename Handler>
    auto coCatch(Handler&& handle);
}

inline original::delayAwaiter::delayAwaiter(time::duration duration)
    : duration_(std::move(duration)) {}

inline void original::delayAwaiter::await_resume() const noexcept {} // NOLINT

template<typename Promise>
void original::delayAwaiter::await_suspend(std::coroutine_handle<Promise> h) const
{
    auto& promise = h.promise();
    executor* exec = promise.executor_;
    if (!exec)
        throw sysError("No executor available");
    exec->schedule(this->duration_, h);
}

inline bool original::delayAwaiter::await_ready() const noexcept
{
    return this->duration_.value(time::NANOSECOND) <= 0;
}

template <typename E, typename Handle>
original::errorCatchAwaitable<E, Handle>::errorCatchAwaitable(Handle h)
    : handle_(std::move(h)) {}

template <typename E, typename Handle>
Handle& original::errorCatchAwaitable<E, Handle>::handle() noexcept
{
    return this->handle_;
}

template <typename E, typename Handle>
original::errorCatchAwaitable<E, Handle>::catchAwaiter
original::errorCatchAwaitable<E, Handle>::operator co_await() const noexcept
{
    return catchAwaiter{this->handle_};
}

template <typename E, typename Handle>
bool original::errorCatchAwaitable<E, Handle>::catchAwaiter::await_ready() const noexcept { // NOLINT
    return true;
}

template <typename E, typename Handle>
template <typename Promise>
void original::errorCatchAwaitable<E, Handle>::catchAwaiter::await_suspend( // NOLINT
    std::coroutine_handle<Promise>) const noexcept {}

template <typename E, typename Handle>
const Handle&
original::errorCatchAwaitable<E, Handle>::catchAwaiter::await_resume() const noexcept
{
    return this->handle_;
}

inline original::delayAwaiter original::coDelay(time::duration duration)
{
    return delayAwaiter{std::move(duration)};
}

template <typename Exception, typename Handler>
auto original::coCatch(Handler&& handle)
{
    return errorCatchAwaitable<Exception, std::decay_t<Handler>>(std::forward<Handler>(handle));
}

#endif //ORIGINAL_AWAITABLE_H
