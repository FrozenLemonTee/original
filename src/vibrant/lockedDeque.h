#ifndef ORIGINAL_LOCKEDDEQUE_H
#define ORIGINAL_LOCKEDDEQUE_H

#include "atomic.h"
#include "condition.h"
#include "deque.h"
#include "core/meta.h"
#include "mutex.h"
#include "vector.h"

namespace original
{
    /**
     * @class lockedDeque
     * @brief Thread-safe double-ended queue with locking mechanism
     * @tparam TYPE Element type stored in the deque
     * @tparam SERIAL Underlying container type (default: vector)
     * @tparam ALLOC Allocator type (default: allocator)
     * @details
     * Provides a thread-safe wrapper around a double-ended queue with support for:
     * - Push and pop operations at both ends
     * - Blocking and non-blocking operations at both ends
     * - Timeout-based pop operations at both ends
     * - Condition variable synchronization
     * - Atomic size tracking
     * - Head and tail access
     *
     * All operations are thread-safe and protected by an internal mutex.
     * Size is maintained atomically for efficient empty() and size() checks.
     *
     * @note For a work-stealing queue, the owner can push/pop at the end while
     *       other workers steal from the beginning.
     */
    template <typename TYPE, template <typename, typename> typename SERIAL = vector,
              template <typename> typename ALLOC = allocator>
    class lockedDeque : public noMeta
    {
        deque<TYPE, SERIAL, ALLOC> deque_; ///< Underlying double-ended queue
        mutable mutex mutex_{}; ///< Mutex for thread safety
        mutable condition condition_{}; ///< Condition variable for synchronization
        atomic<u_integer> size_; ///< Atomic size counter

    public:
        /**
         * @brief Constructs an empty locked deque
         */
        explicit lockedDeque();

        /**
         * @brief Checks if the deque is empty
         * @return True if deque is empty, false otherwise
         * @note Uses atomic size check for efficiency
         */
        [[nodiscard]] bool empty() const noexcept;

        /**
         * @brief Gets the number of elements in the deque
         * @return Current deque size
         * @note Uses atomic size check for efficiency
         */
        [[nodiscard]] u_integer size() const noexcept;

        /**
         * @brief Retrieves the first element without removing it
         * @return Optional containing the first element if available
         * @note Thread-safe, returns empty alternative if deque is empty
         */
        alternative<TYPE> head() const noexcept;

        /**
         * @brief Retrieves the last element without removing it
         * @return Optional containing the last element if available
         * @note Thread-safe, returns empty alternative if deque is empty
         */
        alternative<TYPE> tail() const noexcept;

        /**
         * @brief Pushes an element at the beginning
         * @param e Element to push
         * @note Notifies one waiting thread after insertion
         */
        void pushBegin(TYPE e);

        /**
         * @brief Pushes an element at the end
         * @param e Element to push
         * @note Notifies one waiting thread after insertion
         */
        void pushEnd(TYPE e);

        /**
         * @brief Removes and returns the first element (blocking)
         * @return The first element from the deque
         * @details Blocks the calling thread until an element is available.
         */
        TYPE popBegin();

        /**
         * @brief Removes and returns the last element (blocking)
         * @return The last element from the deque
         * @details Blocks the calling thread until an element is available.
         */
        TYPE popEnd();

        /**
         * @brief Attempts to remove and return the first element (non-blocking)
         * @return Optional containing the first element if available
         * @note Returns empty alternative immediately if deque is empty
         */
        alternative<TYPE> tryPopBegin();

        /**
         * @brief Attempts to remove and return the last element (non-blocking)
         * @return Optional containing the last element if available
         * @note Returns empty alternative immediately if deque is empty
         */
        alternative<TYPE> tryPopEnd();

        /**
         * @brief Removes and returns the first element with timeout
         * @param timeout Maximum duration to wait for an element
         * @return Optional containing the first element if available within timeout
         * @note Returns empty alternative if timeout expires
         */
        alternative<TYPE> popBeginFor(time::duration timeout);

        /**
         * @brief Removes and returns the last element with timeout
         * @param timeout Maximum duration to wait for an element
         * @return Optional containing the last element if available within timeout
         * @note Returns empty alternative if timeout expires
         */
        alternative<TYPE> popEndFor(time::duration timeout);

        /**
         * @brief Removes all elements from the deque
         * @note Thread-safe clear operation
         */
        void clear() noexcept;
    };
} // namespace original

#endif // ORIGINAL_LOCKEDDEQUE_H
