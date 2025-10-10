/**
 * @file semaphores.h
 * @brief Semaphore synchronization primitives and guards
 * @details
 * This header defines the `semaphore` class template and `semaphoreGuard` RAII wrapper
 * for managing concurrent access to shared resources with counting semaphores.
 *
 * The file provides:
 * - `semaphore<MAX_CNT>`: A counting semaphore with maximum count constraint
 * - `semaphore<0>`: Specialization for unbounded semaphore (no maximum limit)
 * - `semaphoreGuard<MAX_CNT>`: RAII wrapper for automatic semaphore acquisition/release
 *
 * Features:
 * - Thread-safe acquisition and release operations
 * - Timeout support for both acquisition and release operations
 * - Bounded and unbounded semaphore variants
 * - Exception safety through RAII guards
 * - Configurable initial and maximum counts
 *
 * @note Semaphores follow the classic synchronization pattern where acquire() decrements
 *       the count and blocks if zero, while release() increments the count and notifies
 *       waiting threads.
 */

#ifndef ORIGINAL_SEMAPHORES_H
#define ORIGINAL_SEMAPHORES_H
#include "config.h"
#include "error.h"
#include "mutex.h"
#include "condition.h"


namespace original {
    /**
     * @class semaphore
     * @brief Counting semaphore with maximum count constraint
     * @tparam MAX_CNT Maximum allowed semaphore count (default: 1, binary semaphore)
     * @details
     * A counting semaphore that controls access to a shared resource pool. The semaphore
     * count represents the number of available resources. acquire() waits for an available
     * resource, while release() returns a resource to the pool.
     *
     * For MAX_CNT = 1, behaves as a binary semaphore/mutex.
     * For MAX_CNT > 1, implements a counting semaphore for resource pools.
     * For MAX_CNT = 0, see specialization below for unbounded semaphore.
     */
    template<u_integer MAX_CNT = 1>
    class semaphore {
        u_integer count_;           ///< Current semaphore count
        mutex mutex_;              ///< Mutex for synchronization
        condition condition_;      ///< Condition variable for waiting

    public:
        /**
         * @brief Constructs a semaphore with maximum count
         * @details Initializes the semaphore with MAX_CNT available resources
         */
        semaphore();

        /**
         * @brief Constructs a semaphore with specified initial count
         * @param init_count Initial semaphore count
         * @throw valueError if init_count exceeds MAX_CNT
         */
        explicit semaphore(u_integer init_count);

        /**
         * @brief Acquires one resource (decrements count)
         * @details
         * Blocks the calling thread until a resource becomes available (count > 0).
         * Decrements the semaphore count by 1 upon successful acquisition.
         */
        void acquire();

        /**
         * @brief Attempts to acquire one resource without blocking
         * @return true if resource was acquired, false if no resources available
         */
        bool tryAcquire();

        /**
         * @brief Attempts to acquire one resource with timeout
         * @param timeout Maximum duration to wait for resource availability
         * @return true if resource was acquired within timeout, false otherwise
         */
        bool acquireFor(time::duration timeout);

        /**
         * @brief Releases resources (increments count)
         * @param increase Number of resources to release (default: 1)
         * @throw valueError if increase would exceed MAX_CNT
         * @details
         * Blocks if releasing would exceed MAX_CNT, waiting until space becomes available.
         * Notifies waiting threads after successful release.
         */
        void release(u_integer increase = 1);

        /**
         * @brief Attempts to release resources without blocking
         * @param increase Number of resources to release (default: 1)
         * @return true if resources were released, false if release would exceed MAX_CNT
         */
        bool tryRelease(u_integer increase = 1);

        /**
         * @brief Attempts to release resources with timeout
         * @param increase Number of resources to release
         * @param timeout Maximum duration to wait for release condition
         * @return true if resources were released within timeout, false otherwise
         */
        bool releaseFor(u_integer increase, time::duration timeout);

        /**
         * @brief Attempts to release one resource with timeout
         * @param timeout Maximum duration to wait for release condition
         * @return true if resource was released within timeout, false otherwise
         */
        bool releaseFor(const time::duration& timeout);
    };

    /**
     * @class semaphore
     * @brief Specialization for unbounded semaphore (no maximum count)
     * @details
     * A semaphore with no upper limit on the count. release() operations never block,
     * as there's no constraint on the maximum semaphore value.
     */
    template<>
    class semaphore<0> {
        u_integer count_;           ///< Current semaphore count (unbounded)
        mutex mutex_;              ///< Mutex for synchronization
        condition condition_;      ///< Condition variable for waiting

    public:
        /**
         * @brief Constructs an unbounded semaphore with count 0
         */
        semaphore();

        /**
         * @brief Constructs an unbounded semaphore with specified initial count
         * @param init_count Initial semaphore count
         */
        explicit semaphore(u_integer init_count);

        /**
         * @brief Acquires one resource (decrements count)
         * @details Blocks until count > 0, then decrements count by 1
         */
        void acquire();

        /**
         * @brief Attempts to acquire one resource without blocking
         * @return true if resource was acquired, false if count was 0
         */
        bool tryAcquire();

        /**
         * @brief Attempts to acquire one resource with timeout
         * @param timeout Maximum duration to wait for resource availability
         * @return true if resource was acquired within timeout, false otherwise
         */
        bool acquireFor(const time::duration& timeout);

        /**
         * @brief Releases resources (increments count)
         * @param increase Number of resources to release (default: 1)
         * @details Never blocks since there's no maximum count constraint
         */
        void release(u_integer increase = 1);
    };

    /**
     * @class semaphoreGuard
     * @brief RAII wrapper for automatic semaphore management
     * @tparam MAX_CNT Maximum semaphore count (matches semaphore template parameter)
     * @details
     * Acquires the semaphore on construction and automatically releases it on destruction.
     * Ensures exception-safe semaphore usage and prevents resource leaks.
     */
    template<u_integer MAX_CNT>
    class semaphoreGuard {
        semaphore<MAX_CNT>& semaphore_;  ///< Reference to managed semaphore

    public:
        /**
         * @brief Constructs a guard and acquires the semaphore
         * @param sem Semaphore to manage
         * @details Calls sem.acquire() during construction
         */
        explicit semaphoreGuard(semaphore<MAX_CNT>& sem);

        /**
         * @brief Destructor that releases the semaphore
         * @details Calls sem.release() during destruction
         */
        ~semaphoreGuard();
    };
}

template<original::u_integer MAX_CNT>
original::semaphore<MAX_CNT>::semaphore() : count_(MAX_CNT) {}

template<original::u_integer MAX_CNT>
original::semaphore<MAX_CNT>::semaphore(const u_integer init_count) : count_(init_count) {
    if (init_count > MAX_CNT) {
        throw valueError("Init count is " + printable::formatString(init_count) +
                         ", that is larger than the max count " + printable::formatString(MAX_CNT));
    }
}

template<original::u_integer MAX_CNT>
void original::semaphore<MAX_CNT>::acquire() {
    {
        uniqueLock lock{this->mutex_};
        this->condition_.wait(this->mutex_, [this]{
            return this->count_ > 0;
        });
        this->count_ -= 1;
    }
    this->condition_.notify();
}

template<original::u_integer MAX_CNT>
bool original::semaphore<MAX_CNT>::tryAcquire() {
    {
        uniqueLock lock{this->mutex_};
        if (this->count_ == 0) {
            return false;
        }
        this->count_ -= 1;
    }
    this->condition_.notify();
    return true;
}

template<original::u_integer MAX_CNT>
bool original::semaphore<MAX_CNT>::acquireFor(time::duration timeout) {
    bool success;
    {
        uniqueLock lock{this->mutex_};
        success = this->condition_.waitFor(this->mutex_, timeout, [this]{
            return this->count_ > 0;
        });
        if (success){
            this->count_ -= 1;
        }
    }
    if (success) {
        this->condition_.notify();
    }
    return success;
}

template<original::u_integer MAX_CNT>
void original::semaphore<MAX_CNT>::release(const u_integer increase) {
    if (increase > MAX_CNT) {
        throw valueError("Increase is larger than max count " + printable::formatString(MAX_CNT));
    }
    {
        uniqueLock lock{this->mutex_};
        this->condition_.wait(this->mutex_, [this, increase] {
            return this->count_ + increase <= MAX_CNT;
        });
        this->count_ += increase;
    }
    this->condition_.notifySome(increase);
}

template<original::u_integer MAX_CNT>
bool original::semaphore<MAX_CNT>::tryRelease(const u_integer increase) {
    {
        uniqueLock lock{this->mutex_};
        if (this->count_ + increase > MAX_CNT) {
            return false;
        }
        this->count_ += increase;
    }
    this->condition_.notifySome(increase);
    return true;
}

template<original::u_integer MAX_CNT>
bool original::semaphore<MAX_CNT>::releaseFor(const u_integer increase, time::duration timeout){
    bool success;
    {
        uniqueLock lock{this->mutex_};
        success = this->condition_.waitFor(this->mutex_, timeout, [this, increase]{
            return this->count_ + increase <= MAX_CNT;
        });
        if (success){
            this->count_ += increase;
        }
    }
    if (success){
        this->condition_.notifySome(increase);
    }
    return success;
}

template<original::u_integer MAX_CNT>
bool original::semaphore<MAX_CNT>::releaseFor(const time::duration& timeout) {
    return this->releaseFor(1, timeout);
}

inline original::semaphore<0>::semaphore() : count_(0) {}

inline original::semaphore<0>::semaphore(const u_integer init_count) : count_(init_count) {}

inline void original::semaphore<0>::acquire() {
    uniqueLock lock{this->mutex_};
    this->condition_.wait(this->mutex_, [this]{
        return this->count_ > 0;
    });
    this->count_ -= 1;
}

inline bool original::semaphore<0>::tryAcquire() {
    uniqueLock lock{this->mutex_};
    if (this->count_ == 0) {
        return false;
    }
    this->count_ -= 1;
    return true;
}

inline bool original::semaphore<0>::acquireFor(const time::duration& timeout) {
    uniqueLock lock{this->mutex_};
    const bool success = this->condition_.waitFor(this->mutex_, timeout, [this]{
        return this->count_ > 0;
    });
    if (success){
        this->count_ -= 1;
    }
    return success;
}

inline void original::semaphore<0>::release(const u_integer increase) {
    {
        uniqueLock lock{this->mutex_};
        this->count_ += increase;
    }
    this->condition_.notifySome(increase);
}

template<original::u_integer MAX_CNT>
original::semaphoreGuard<MAX_CNT>::semaphoreGuard(semaphore<MAX_CNT>& sem) : semaphore_(sem) {
    this->semaphore_.acquire();
}

template<original::u_integer MAX_CNT>
original::semaphoreGuard<MAX_CNT>::~semaphoreGuard() {
    this->semaphore_.release();
}

#endif //ORIGINAL_SEMAPHORES_H
