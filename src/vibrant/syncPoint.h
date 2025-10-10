#ifndef ORIGINAL_SYNCPOINT_H
#define ORIGINAL_SYNCPOINT_H
#include "mutex.h"
#include "condition.h"


namespace original {
    /**
     * @class syncPoint
     * @brief Synchronization point for multiple threads
     * @details
     * A synchronization primitive that allows multiple threads to wait for each other
     * at a specific point in execution. Threads call arrive() to indicate they have
     * reached the synchronization point, and the last arriving thread triggers the
     * completion function and releases all waiting threads.
     *
     * Features:
     * - Reusable barrier that can be used for multiple synchronization rounds
     * - Optional completion function executed by the last arriving thread
     * - Exception safety - exceptions in completion function are propagated to all threads
     * - Thread-safe arrival counting and notification
     *
     */
    class syncPoint {
        const u_integer max_arrived_;   ///< Maximum number of threads required for synchronization
        u_integer arrived_;             ///< Current number of arrived threads
        u_integer round_;               ///< Current synchronization round
        mutable mutex mutex_;          ///< Mutex for thread synchronization
        mutable condition condition_;  ///< Condition variable for thread waiting
        std::function<void()> complete_func_; ///< Completion function called by last thread
        std::exception_ptr e_;          ///< Exception pointer for propagating completion function errors

    public:
        /**
         * @brief Constructs a disabled synchronization point
         * @details
         * Creates a syncPoint with max_arrived = 0. arrive() calls on this instance
         * will return immediately without any synchronization.
         */
        syncPoint();

        /**
         * @brief Constructs a synchronization point with specified thread count
         * @param max_arrived Number of threads required to trigger synchronization
         * @param func Completion function called by the last arriving thread
         * @details
         * The completion function is executed by the last thread that calls arrive(),
         * before releasing all waiting threads. If the function throws an exception,
         * it will be propagated to all threads waiting at the barrier.
         */
        explicit syncPoint(u_integer max_arrived, const std::function<void()>& func = {});

        /**
         * @brief Arrive at the synchronization point
         * @details
         * Blocks the calling thread until the required number of threads (max_arrived)
         * have called arrive(). The last thread to arrive will execute the completion
         * function and release all waiting threads.
         *
         * @throw std::exception If the completion function throws an exception,
         *        all waiting threads will rethrow the same exception.
         */
        void arrive();

        /**
         * @brief Get the maximum number of threads required for synchronization
         * @return Maximum number of threads that must arrive
         */
        u_integer maxArrived() const;

        /**
         * @brief Get the current number of arrived threads
         * @return Number of threads that have arrived in the current round
         * @details
         * This value is reset to 0 when the required number of threads arrive
         * and a new synchronization round begins.
         */
        u_integer currentArrived() const;
    };
}

inline original::syncPoint::syncPoint()
    : max_arrived_(0), arrived_(0), round_(0) {}

inline original::syncPoint::syncPoint(const u_integer max_arrived, const std::function<void()>& func)
    : max_arrived_(max_arrived), arrived_(0), round_(0), complete_func_(func) {}

inline void original::syncPoint::arrive() {
    {
        uniqueLock lock{this->mutex_};
        if (max_arrived_ == 0) {
            return;
        }
        u_integer this_round = this->round_;
        this->arrived_ += 1;
        if (this->arrived_ != this->max_arrived_) {
            this->condition_.wait(this->mutex_, [this, this_round]
            {
                return this->round_ != this_round;
            });
        } else {
            this->round_ += 1;
            if (this->complete_func_) {
                try {
                    this->complete_func_();
                }catch (...){
                    this->e_ = std::current_exception();
                }
            }
            this->arrived_ -= this->max_arrived_;
            lock.unlock();
            this->condition_.notifyAll();
            if (this->e_) {
                std::rethrow_exception(this->e_);
            }
        }
    }
}

inline original::u_integer original::syncPoint::maxArrived() const
{
    return this->max_arrived_;
}

inline original::u_integer original::syncPoint::currentArrived() const
{
    uniqueLock lock{this->mutex_};
    return this->arrived_;
}

#endif //ORIGINAL_SYNCPOINT_H
