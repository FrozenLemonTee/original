#ifndef ORIGINAL_LOCKEDQUEUE_H
#define ORIGINAL_LOCKEDQUEUE_H

#include "queue.h"
#include "vector.h"
#include "atomic.h"
#include "mutex.h"
#include "condition.h"
#include "refCntPtr.h"

namespace original {
    template<typename TYPE,
         template <typename, typename> typename SERIAL = vector,
         template <typename> typename ALLOC = allocator>
    class lockedQueue {
        queue<TYPE, SERIAL, ALLOC> queue_;
        mutable mutex mutex_{};
        mutable condition condition_{};
        atomic<u_integer> size_;

    public:
        explicit lockedQueue();

        lockedQueue(const lockedQueue&) = delete;
        lockedQueue& operator=(const lockedQueue&) = delete;
        lockedQueue(lockedQueue&&) noexcept = delete;
        lockedQueue& operator=(lockedQueue&&) noexcept = delete;

        [[nodiscard]] bool empty() const noexcept;

        [[nodiscard]] u_integer size() const noexcept;

        alternative<TYPE> head() const noexcept;

        alternative<TYPE> tail() const noexcept;

        void push(TYPE e);

        TYPE pop();

        alternative<TYPE> tryPop();

        strongPtr<TYPE> tryPop2();

        alternative<TYPE> popFor(time::duration timeout);

        void clear() noexcept;
    };
}

template <typename TYPE,
          template <typename, typename> typename SERIAL,
          template <typename> typename ALLOC>
original::lockedQueue<TYPE, SERIAL, ALLOC>::lockedQueue()
    : size_(makeAtomic<u_integer>(0)) {}

template <typename TYPE,
          template <typename, typename> typename SERIAL,
          template <typename> typename ALLOC>
bool original::lockedQueue<TYPE, SERIAL, ALLOC>::empty() const noexcept
{
    return *this->size_ == 0;
}

template <typename TYPE,
          template <typename, typename> typename SERIAL,
          template <typename> typename ALLOC>
original::u_integer original::lockedQueue<TYPE, SERIAL, ALLOC>::size() const noexcept
{
    return *this->size_;
}

template <typename TYPE,
          template <typename, typename> typename SERIAL,
          template <typename> typename ALLOC>
original::alternative<TYPE> original::lockedQueue<TYPE, SERIAL, ALLOC>::head() const noexcept
{
    uniqueLock lock{this->mutex_};
    if (!this->empty())
        return alternative<TYPE>{std::move(this->queue_.head())};
    return alternative<TYPE>{};
}

template <typename TYPE,
          template <typename, typename> typename SERIAL,
          template <typename> typename ALLOC>
original::alternative<TYPE> original::lockedQueue<TYPE, SERIAL, ALLOC>::tail() const noexcept
{
    uniqueLock lock{this->mutex_};
    if (!this->empty())
        return alternative<TYPE>{std::move(this->queue_.tail())};
    return alternative<TYPE>{};
}

template <typename TYPE,
          template <typename, typename> typename SERIAL,
          template <typename> typename ALLOC>
void original::lockedQueue<TYPE, SERIAL, ALLOC>::push(TYPE e)
{
    {
        uniqueLock lock{this->mutex_};
        this->queue_.push(std::move(e));
        this->size_ += 1;
    }
    this->condition_.notify();
}

template <typename TYPE,
          template <typename, typename> typename SERIAL,
          template <typename> typename ALLOC>
TYPE original::lockedQueue<TYPE, SERIAL, ALLOC>::pop()
{
    uniqueLock lock{this->mutex_};
    this->condition_.wait(this->mutex_, [this]
    {
        return !this->empty();
    });
    TYPE e = std::move(this->queue_.pop());
    this->size_ -= 1;
    return e;
}

template <typename TYPE,
          template <typename, typename> typename SERIAL,
          template <typename> typename ALLOC>
original::alternative<TYPE> original::lockedQueue<TYPE, SERIAL, ALLOC>::tryPop()
{
    uniqueLock lock{this->mutex_};
    if (this->empty()) {
        return alternative<TYPE>{};
    }
    TYPE e = std::move(this->queue_.pop());
    this->size_ -= 1;
    return alternative<TYPE>{std::move(e)};
}

template <typename TYPE,
        template <typename, typename> typename SERIAL,
        template <typename> typename ALLOC>
original::strongPtr<TYPE> original::lockedQueue<TYPE, SERIAL, ALLOC>::tryPop2() {
    uniqueLock lock{this->mutex_};
    if (this->empty()) {
        return original::strongPtr<TYPE>{};
    }
    TYPE e = std::move(this->queue_.pop());
    this->size_ -= 1;
    return makeStrongPtr<TYPE>(std::move(e));
}

template <typename TYPE,
          template <typename, typename> typename SERIAL,
          template <typename> typename ALLOC>
original::alternative<TYPE>
original::lockedQueue<TYPE, SERIAL, ALLOC>::popFor(time::duration timeout)
{
    uniqueLock lock{this->mutex_};
    const bool success = this->condition_.waitFor(this->mutex_, timeout, [this]
    {
       return !this->empty();
    });
    if (success) {
        TYPE e = std::move(this->queue_.pop());
        this->size_ -= 1;
        return alternative<TYPE>{std::move(e)};
    }
    return alternative<TYPE>{};
}

template <typename TYPE,
          template <typename, typename> typename SERIAL,
          template <typename> typename ALLOC>
void original::lockedQueue<TYPE, SERIAL, ALLOC>::clear() noexcept
{
    uniqueLock lock{this->mutex_};
    this->queue_.clear();
    this->size_ = 0;
}

#endif //ORIGINAL_LOCKEDQUEUE_H
