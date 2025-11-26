#ifndef ORIGINAL_LOCKEDPRIQUE_H
#define ORIGINAL_LOCKEDPRIQUE_H

#include "allocator.h"
#include "vector.h"
#include "comparator.h"
#include "prique.h"
#include "atomic.h"
#include "condition.h"
#include "mutex.h"

namespace original {
    template<typename TYPE,
        template <typename> typename Callback = increaseComparator,
        template <typename, typename> typename SERIAL = vector,
        template <typename> typename ALLOC = allocator>
    requires Compare<Callback<TYPE>, TYPE>
    class lockedPrique {
        prique<TYPE, Callback, SERIAL, ALLOC> prique_;
        mutable mutex mutex_{};
        mutable condition condition_{};
        atomic<u_integer> size_;

    public:
        explicit lockedPrique();

        lockedPrique(const lockedPrique&) = delete;
        lockedPrique& operator=(const lockedPrique&) = delete;
        lockedPrique(lockedPrique&&) noexcept = delete;
        lockedPrique& operator=(lockedPrique&&) noexcept = delete;

        [[nodiscard]] bool empty() const noexcept;

        [[nodiscard]] u_integer size() const noexcept;

        alternative<TYPE> top() const noexcept;

        void push(TYPE e);

        TYPE pop();

        alternative<TYPE> tryPop();

        alternative<TYPE> popFor(time::duration timeout);

        void clear() noexcept;
    };
}

template <typename TYPE,
          template <typename> typename Callback,
          template <typename, typename> typename SERIAL,
          template <typename> typename ALLOC>
requires original::Compare<Callback<TYPE>, TYPE>
original::lockedPrique<TYPE, Callback, SERIAL, ALLOC>::lockedPrique()
    : size_(makeAtomic<u_integer>(0)) {}

template <typename TYPE,
          template <typename> typename Callback,
          template <typename, typename> typename SERIAL,
          template <typename> typename ALLOC>
requires original::Compare<Callback<TYPE>, TYPE>
bool original::lockedPrique<TYPE, Callback, SERIAL, ALLOC>::empty() const noexcept
{
    return *this->size_ == 0;
}

template <typename TYPE,
          template <typename> typename Callback,
          template <typename, typename> typename SERIAL,
          template <typename> typename ALLOC>
requires original::Compare<Callback<TYPE>, TYPE>
original::u_integer original::lockedPrique<TYPE, Callback, SERIAL, ALLOC>::size() const noexcept
{
    return *this->size_;
}

template <typename TYPE,
          template <typename> typename Callback,
          template <typename, typename> typename SERIAL,
          template <typename> typename ALLOC>
requires original::Compare<Callback<TYPE>, TYPE>
original::alternative<TYPE> original::lockedPrique<TYPE, Callback, SERIAL, ALLOC>::top() const noexcept
{
    uniqueLock lock{this->mutex_};
    if (!this->empty())
        return alternative<TYPE>{std::move(this->prique_.top())};
    return alternative<TYPE>{};
}

template <typename TYPE,
          template <typename> typename Callback,
          template <typename, typename> typename SERIAL,
          template <typename> typename ALLOC>
requires original::Compare<Callback<TYPE>, TYPE>
void original::lockedPrique<TYPE, Callback, SERIAL, ALLOC>::push(TYPE e)
{
    {
        uniqueLock lock{this->mutex_};
        this->prique_.push(std::move(e));
        this->size_ += 1;
    }
    this->condition_.notify();
}

template <typename TYPE,
          template <typename> typename Callback,
          template <typename, typename> typename SERIAL,
          template <typename> typename ALLOC>
requires original::Compare<Callback<TYPE>, TYPE>
TYPE original::lockedPrique<TYPE, Callback, SERIAL, ALLOC>::pop()
{
    uniqueLock lock{this->mutex_};
    this->condition_.wait(this->mutex_, [this]
    {
        return !this->empty();
    });
    TYPE e = std::move(this->prique_.pop());
    this->size_ -= 1;
    return e;
}

template <typename TYPE,
          template <typename> typename Callback,
          template <typename, typename> typename SERIAL,
          template <typename> typename ALLOC>
requires original::Compare<Callback<TYPE>, TYPE>
original::alternative<TYPE> original::lockedPrique<TYPE, Callback, SERIAL, ALLOC>::tryPop()
{
    uniqueLock lock{this->mutex_};
    if (this->empty()) {
        return alternative<TYPE>{};
    }
    TYPE e = std::move(this->prique_.pop());
    this->size_ -= 1;
    return alternative<TYPE>{std::move(e)};
}

template <typename TYPE,
          template <typename> typename Callback,
          template <typename, typename> typename SERIAL,
          template <typename> typename ALLOC>
requires original::Compare<Callback<TYPE>, TYPE>
original::alternative<TYPE> original::lockedPrique<TYPE, Callback, SERIAL, ALLOC>::popFor(time::duration timeout)
{
    uniqueLock lock{this->mutex_};
    const bool success = this->condition_.waitFor(this->mutex_, timeout, [this]
    {
       return !this->empty();
    });
    if (success) {
        TYPE e = std::move(this->prique_.pop());
        this->size_ -= 1;
        return alternative<TYPE>{std::move(e)};
    }
    return alternative<TYPE>{};
}

template <typename TYPE,
          template <typename> typename Callback,
          template <typename, typename> typename SERIAL,
          template <typename> typename ALLOC>
requires original::Compare<Callback<TYPE>, TYPE>
void original::lockedPrique<TYPE, Callback, SERIAL, ALLOC>::clear() noexcept
{
    uniqueLock lock{this->mutex_};
    this->prique_.clear();
    this->size_ = 0;
}

#endif //ORIGINAL_LOCKEDPRIQUE_H
