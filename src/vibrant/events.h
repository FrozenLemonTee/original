#ifndef ORIGINAL_EVENTS_H
#define ORIGINAL_EVENTS_H
#include <utility>

#include "zeit.h"
#include "executor.h"

namespace original {
    class delayEvent {
        time::duration duration_;

        explicit delayEvent(time::duration duration);

    public:
        void await_resume() const noexcept;

        template<typename Promise>
        void await_suspend(std::coroutine_handle<Promise> h) const;

        bool await_ready() const noexcept;

        friend delayEvent coDelay(time::duration duration);
    };

    delayEvent coDelay(time::duration duration);
}

inline original::delayEvent::delayEvent(time::duration duration)
    : duration_(std::move(duration)) {}

inline void original::delayEvent::await_resume() const noexcept {} // NOLINT

template<typename Promise>
void original::delayEvent::await_suspend(std::coroutine_handle<Promise> h) const
{
    auto& promise = h.promise();
    executor* exec = promise.executor_;
    if (!exec)
        throw sysError("No executor available");
    exec->schedule(this->duration_, h);
}

inline bool original::delayEvent::await_ready() const noexcept
{
    return this->duration_.value(time::NANOSECOND) <= 0;
}

inline original::delayEvent original::coDelay(time::duration duration)
{
    return delayEvent{std::move(duration)};
}

#endif //ORIGINAL_EVENTS_H
