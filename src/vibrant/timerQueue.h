#ifndef ORIGINAL_TIMERQUEUE_H
#define ORIGINAL_TIMERQUEUE_H
#include "couple.h"
#include "zeit.h"
#include "lockedPrique.h"
#include "executor.h"


namespace original
{
    class timerQueue
    {
    public:
        using funcType = executor::Func;
        using taskType = couple<time::point, funcType>;
        using comparatorType = comparator<taskType>;
    private:
        template <typename COUPLE>
        struct comparator
        {
            bool operator()(const COUPLE& lhs, const COUPLE& rhs) const;
        };

        lockedPrique<taskType, comparator> queue_{};
    public:
        timerQueue() noexcept = default;

        [[nodiscard]] bool empty() const noexcept;

        [[nodiscard]] u_integer size() const noexcept;

        alternative<taskType> top() const noexcept;

        void push(const taskType& task) noexcept;

        taskType pop() noexcept;

        alternative<taskType> tryPop() noexcept;

        alternative<taskType> popFor(const time::duration& timeout);

        void clear() noexcept;
    };
}

template <typename COUPLE>
bool original::timerQueue::comparator<COUPLE>::operator()(const COUPLE& lhs, const COUPLE& rhs) const
{
    return lhs.first() < rhs.first();
}

inline bool original::timerQueue::empty() const noexcept
{
    return this->queue_.empty();
}

inline original::u_integer original::timerQueue::size() const noexcept
{
    return this->queue_.size();
}

inline original::alternative<original::timerQueue::taskType>
original::timerQueue::top() const noexcept
{
    return this->queue_.top();
}

inline void original::timerQueue::push(const taskType& task) noexcept
{
    this->queue_.push(task);
}

inline original::timerQueue::taskType original::timerQueue::pop() noexcept
{
    return this->queue_.pop();
}

inline original::alternative<original::timerQueue::taskType>
original::timerQueue::tryPop() noexcept
{
    return this->queue_.tryPop();
}

inline original::alternative<original::timerQueue::taskType>
original::timerQueue::popFor(const time::duration& timeout)
{
    return this->queue_.popFor(timeout);
}

inline void original::timerQueue::clear() noexcept
{
    this->queue_.clear();
}

#endif //ORIGINAL_TIMERQUEUE_H
