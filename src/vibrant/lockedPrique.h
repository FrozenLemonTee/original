#ifndef ORIGINAL_LOCKEDPRIQUE_H
#define ORIGINAL_LOCKEDPRIQUE_H

#include "allocator.h"
#include "vector.h"
#include "comparator.h"
#include "prique.h"
#include "atomic.h"
#include "condition.h"
#include "meta.h"
#include "mutex.h"

namespace original {

    /**
     * @class lockedPrique
     * @brief Thread-safe priority queue with locking mechanism
     * @tparam TYPE Element type stored in the queue
     * @tparam Callback Comparator type for ordering elements (default: increaseComparator)
     * @tparam SERIAL Underlying container type (default: vector)
     * @tparam ALLOC Allocator type (default: allocator)
     * @details
     * Provides a thread-safe wrapper around a priority queue with support for:
     * - Blocking and non-blocking operations
     * - Timeout-based operations
     * - Condition variable synchronization
     * - Atomic size tracking
     *
     * All operations are thread-safe and protected by internal mutex.
     * Size is maintained atomically for efficient empty() and size() checks.
     */
    template<typename TYPE,
        template <typename> typename Callback = increaseComparator,
        template <typename, typename> typename SERIAL = vector,
        template <typename> typename ALLOC = allocator>
    requires Compare<Callback<TYPE>, TYPE>
    class lockedPrique : public noMeta {
        prique<TYPE, Callback, SERIAL, ALLOC> prique_;  ///< Underlying priority queue
        mutable mutex mutex_{};                         ///< Mutex for thread safety
        mutable condition condition_{};                 ///< Condition variable for synchronization
        atomic<u_integer> size_;                        ///< Atomic size counter

    public:
        /**
         * @brief Constructs an empty locked priority queue
         */
        explicit lockedPrique();

        /**
         * @brief Checks if the queue is empty
         * @return True if queue is empty, false otherwise
         * @note Uses atomic size check for efficiency
         */
        [[nodiscard]] bool empty() const noexcept;

        /**
         * @brief Gets the number of elements in the queue
         * @return Current queue size
         * @note Uses atomic size check for efficiency
         */
        [[nodiscard]] u_integer size() const noexcept;

        /**
         * @brief Retrieves the top element without removing it
         * @return Optional containing the top element if available
         * @note Thread-safe, returns empty alternative if queue is empty
         */
        alternative<TYPE> top() const noexcept;

        /**
         * @brief Pushes an element into the queue
         * @param e Element to push
         * @note Notifies one waiting thread after insertion
         */
        void push(TYPE e);

        /**
         * @brief Removes and returns the top element (blocking)
         * @return The top element from the queue
         * @details
         * Blocks the calling thread until an element is available.
         * Thread will wait on condition variable if queue is empty.
         */
        TYPE pop();

        /**
         * @brief Attempts to remove and return the top element (non-blocking)
         * @return Optional containing the top element if available
         * @note Returns empty alternative immediately if queue is empty
         */
        alternative<TYPE> tryPop();

        /**
         * @brief Removes and returns the top element with timeout
         * @param timeout Maximum duration to wait for an element
         * @return Optional containing the top element if available within timeout
         * @note Returns empty alternative if timeout expires
         */
        alternative<TYPE> popFor(time::duration timeout);

        /**
         * @brief Removes all elements from the queue
         * @note Thread-safe clear operation
         */
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