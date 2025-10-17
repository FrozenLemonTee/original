#ifndef CONDITION_H
#define CONDITION_H

#include "config.h"

#if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG
#include <pthread.h>
#elif ORIGINAL_COMPILER_MSVC
#endif

#include "mutex.h"
#include "zeit.h"

/**
 * @file condition.h
 * @brief Cross-platform condition variable implementation for thread synchronization
 * @details Provides condition variable functionality for coordinating
 * between threads, including:
 * - Basic wait/notify operations
 * - Timed waits with duration support
 * - Predicate-based waiting
 * - Integration with mutex.h locking mechanisms
 *
 * Platform Support:
 * - GCC/Clang: Uses pthread_cond_t for condition variable implementation (pCondition)
 * - MSVC: Uses CONDITION_VARIABLE for lightweight implementation (wCondition)
 * - All platforms: High-level condition class with consistent interface
 *
 * Key features:
 * - POSIX-based implementation for GCC/Clang
 * - Windows CONDITION_VARIABLE implementation for MSVC
 * - Thread-safe condition variable operations
 * - Timeout support using zeit.h duration types
 * - Predicate templates for safe condition checking
 * - Spurious wakeup protection through predicate loops
 */

namespace original
{
    /**
     * @class conditionBase
     * @brief Abstract base class for condition variable implementations
     * @details Provides the interface for thread synchronization operations:
     * - Waiting with mutex protection
     * - Timed waiting with duration support
     * - Notification of waiting threads
     * - Predicate-based waiting templates
     *
     * @note This is an abstract base class and cannot be instantiated directly.
     *       Derived classes must implement all pure virtual methods.
     */
    class conditionBase
    {
    public:
        /// Default constructor
        explicit conditionBase() = default;

        /**
         * @brief Waits for notification while holding the mutex
         * @param mutex Locked mutex to wait on
         * @throws sysError if wait operation fails
         * @note The mutex must be locked by the calling thread
         */
        virtual void wait(mutexBase& mutex) = 0;

        /**
         * @brief Waits for notification with timeout
         * @param mutex Locked mutex to wait on
         * @param d Maximum duration to wait
         * @return true if notified, false if timeout occurred
         * @throws sysError if wait operation fails
         * @note The mutex must be locked by the calling thread
         */
        virtual bool waitFor(mutexBase& mutex, time::duration d) = 0;

        /**
         * @brief Waits until predicate becomes true
         * @tparam Pred Predicate type (must be callable and return bool)
         * @param mutex Locked mutex to wait on
         * @param predicate Condition predicate to check
         * @throws sysError if wait operation fails
         * @note Implements the "wait with predicate" pattern to avoid spurious wakeups
         */
        template<typename Pred>
        void wait(mutexBase& mutex, Pred predicate) noexcept(noexcept(predicate()));

        /**
         * @brief Waits with timeout until predicate becomes true
         * @tparam Pred Predicate type (must be callable and return bool)
         * @param mutex Locked mutex to wait on
         * @param d Maximum duration to wait
         * @param predicate Condition predicate to check
         * @return true if predicate became true, false if timeout occurred
         * @throws sysError if wait operation fails
         */
        template<typename Pred>
        bool waitFor(mutexBase& mutex, const time::duration& d, Pred predicate) noexcept(noexcept(predicate()));

        /**
         * @brief Notifies one waiting thread
         * @throws sysError if notification fails
         */
        virtual void notify() = 0;

        /**
         * @brief Notifies all waiting threads
         * @throws sysError if notification fails
         */
        virtual void notifyAll() = 0;

        /**
         * @brief Notifies a specified number of waiting threads
         * @param n Number of threads to notify
         * @details This method provides a flexible notification mechanism:
         * - If n == 0: No operation is performed
         * - If n == 1: Equivalent to calling notify()
         * - If n >= 2: Equivalent to calling notifyAll()
         *
         * @note This is a convenience method for scenarios where the exact
         *       number of threads to notify depends on runtime conditions.
         *       For precise control over thread notification, use notify()
         *       or notifyAll() directly.
         */
        void notifySome(u_integer n);

        /// Virtual destructor
        virtual ~conditionBase() = default;

        /// Deleted copy constructor
        conditionBase(const conditionBase&) = delete;

        /// Deleted copy assignment operator
        conditionBase& operator=(const conditionBase&) = delete;
    };

#if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG
    /**
     * @class pCondition
     * @brief POSIX condition variable implementation for GCC and Clang compilers
     * @extends conditionBase
     * @details Wrapper around pthread_cond_t with RAII semantics.
     * Provides thread synchronization using POSIX condition variables.
     *
     * Platform-Specific Features:
     * - Uses pthread_cond_init for initialization
     * - Implements pthread_cond_wait for blocking waits
     * - Uses pthread_cond_timedwait for timed waits with absolute time
     * - Provides pthread_cond_signal for single thread notification
     * - Uses pthread_cond_broadcast for all threads notification
     * - Calls pthread_cond_destroy for cleanup in destructor
     *
     * @note Only available when compiled with GCC or Clang
     * @note Requires POSIX mutex (pMutex) for proper operation
     * @note Timed waits use absolute time (CLOCK_REALTIME) for precision
     *
     * Example usage:
     * @code
     * original::pCondition cond;
     * original::pMutex mtx;
     *
     * // Waiting thread
     * {
     *     original::uniqueLock lock(mtx);
     *     cond.wait(mtx, [&](){ return data_ready; });
     *     // Process data
     * }
     *
     * // Notifying thread
     * {
     *     original::uniqueLock lock(mtx);
     *     data_ready = true;
     *     cond.notify();
     * }
     * @endcode
     */
    class pCondition final : public conditionBase
    {
        pthread_cond_t cond_; ///< Internal POSIX condition variable handle

    public:
        // Inherit template methods from conditionBase
        using conditionBase::wait;
        using conditionBase::waitFor;

        /**
         * @brief Constructs and initializes the POSIX condition variable
         * @throws sysError if pthread_cond_init fails
         * @note Uses default condition variable attributes
         * @details Calls pthread_cond_init with nullptr attributes
         */
        explicit pCondition();

        /**
         * @brief Waits for notification while holding the mutex
         * @param mutex Locked mutex to wait on (must be pMutex)
         * @throws sysError if pthread_cond_wait fails
         * @throws valueError if mutex is not a pMutex
         * @note The mutex must be locked by the calling thread
         * @details Uses static type checking to ensure mutex compatibility
         */
        void wait(mutexBase& mutex) override;

        /**
         * @brief Waits for notification with timeout
         * @param mutex Locked mutex to wait on (must be pMutex)
         * @param d Maximum duration to wait
         * @return true if notified, false if timeout occurred (ETIMEDOUT)
         * @throws sysError if pthread_cond_timedwait fails
         * @throws valueError if mutex is not a pMutex
         * @note Uses absolute time calculation for precise timeout handling
         * @details Converts duration to timespec for pthread_cond_timedwait
         */
        bool waitFor(mutexBase& mutex, time::duration d) override;

        /**
         * @brief Notifies one waiting thread
         * @throws sysError if pthread_cond_signal fails
         * @details Calls pthread_cond_signal
         */
        void notify() override;

        /**
         * @brief Notifies all waiting threads
         * @throws sysError if pthread_cond_broadcast fails
         * @details Calls pthread_cond_broadcast
         */
        void notifyAll() override;

        /**
         * @brief Destroys the condition variable
         * @note Logs warning but doesn't throw if destruction fails
         * @details Calls pthread_cond_destroy
         */
        ~pCondition() override;
    };

#elif ORIGINAL_COMPILER_MSVC
    /**
     * @class wCondition
     * @brief Windows condition variable implementation for MSVC compiler
     * @extends conditionBase
     * @details Wrapper around CONDITION_VARIABLE with RAII semantics.
     * Provides thread synchronization using Windows condition variables.
     *
     * Platform-Specific Features:
     * - Uses InitializeConditionVariable for initialization
     * - Implements SleepConditionVariableSRW for blocking waits
     * - Uses SleepConditionVariableSRW with timeout for timed waits
     * - Provides WakeConditionVariable for single thread notification
     * - Uses WakeAllConditionVariable for all threads notification
     * - No explicit destruction required (Windows manages resources)
     *
     * @note Only available when compiled with MSVC
     * @note Requires SRW lock (wMutex) for proper operation
     * @note Timed waits use relative time in milliseconds
     * @note More efficient than POSIX condition variables on Windows
     *
     * Example usage:
     * @code
     * original::wCondition cond;
     * original::wMutex mtx;
     *
     * // Waiting thread
     * {
     *     original::uniqueLock lock(mtx);
     *     cond.wait(mtx, [&](){ return data_ready; });
     *     // Process data
     * }
     *
     * // Notifying thread
     * {
     *     original::uniqueLock lock(mtx);
     *     data_ready = true;
     *     cond.notify();
     * }
     * @endcode
     */
    class wCondition final : public conditionBase
    {
        CONDITION_VARIABLE cond_; ///< Windows condition variable handle

    public:
        // Inherit template methods from conditionBase
        using conditionBase::wait;
        using conditionBase::waitFor;

        /**
         * @brief Constructs and initializes the Windows condition variable
         * @implementation Calls InitializeConditionVariable
         * @note Windows condition variables require no explicit destruction
         */
        explicit wCondition();

        /**
         * @brief Waits for notification while holding the mutex
         * @param mutex Locked mutex to wait on (must be wMutex)
         * @throws sysError if SleepConditionVariableSRW fails
         * @note Uses INFINITE timeout for blocking wait
         * @implementation Calls SleepConditionVariableSRW with INFINITE timeout
         */
        void wait(mutexBase& mutex) override;

        /**
         * @brief Waits for notification with timeout
         * @param mutex Locked mutex to wait on (must be wMutex)
         * @param d Maximum duration to wait
         * @return true if notified, false if timeout occurred (ERROR_TIMEOUT)
         * @throws sysError if SleepConditionVariableSRW fails (other than timeout)
         * @note Converts duration to milliseconds for Windows API
         * @implementation Uses SleepConditionVariableSRW with millisecond timeout
         */
        bool waitFor(mutexBase& mutex, time::duration d) override;

        /**
         * @brief Notifies one waiting thread
         * @implementation Calls WakeConditionVariable
         */
        void notify() override;

        /**
         * @brief Notifies all waiting threads
         * @implementation Calls WakeAllConditionVariable
         */
        void notifyAll() override;

        /**
         * @brief Destructor
         * @note No explicit cleanup required for Windows condition variables
         * @implementation Windows automatically manages condition variable resources
         */
        ~wCondition() override;
    };
#endif

    /**
     * @class condition
     * @brief High-level cross-platform condition variable wrapper
     * @extends conditionBase
     * @details Provides a unified condition variable interface that automatically
     * selects the appropriate platform-specific implementation:
     * - pCondition for GCC/Clang on Linux/macOS (POSIX condition variables)
     * - wCondition for MSVC on Windows (Windows condition variables)
     *
     * Key Features:
     * - Consistent API across all platforms
     * - RAII semantics with automatic cleanup
     * - Exception-safe operations
     * - Delegates all operations to platform-specific implementation
     * - Inherits predicate-based waiting templates from conditionBase
     *
     * Platform Abstraction:
     * - GCC/Clang: Delegates to pCondition (pthread_cond_t)
     * - MSVC: Delegates to wCondition (CONDITION_VARIABLE)
     * - All platforms: Identical interface and behavior
     *
     * Example usage:
     * @code
     * original::condition cond;
     * original::mutex mtx;
     * bool data_ready = false;
     *
     * // Waiting thread - works the same on all platforms
     * {
     *     original::uniqueLock lock(mtx);
     *     cond.wait(lock, [&](){ return data_ready; });
     *     // Critical section - condition satisfied
     * }
     *
     * // Notifying thread - works the same on all platforms
     * {
     *     original::uniqueLock lock(mtx);
     *     data_ready = true;
     *     cond.notify(); // or cond.notifyAll() for multiple waiters
     * }
     * @endcode
     *
     * @see original::pCondition (GCC/Clang implementation)
     * @see original::wCondition (MSVC implementation)
     * @see original::conditionBase
     * @see original::uniqueLock
     */
    class condition final : public conditionBase {
    #if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG
        pCondition cond_; ///< POSIX condition variable implementation (GCC/Clang)
    #elif ORIGINAL_COMPILER_MSVC
        wCondition cond_; ///< Windows condition variable implementation (MSVC)
    #endif

    public:
        // Inherit template methods from conditionBase
        using conditionBase::wait;
        using conditionBase::waitFor;

        /**
         * @brief Constructs the platform-specific condition variable
         * @implementation Delegates to pCondition or wCondition constructor
         */
        condition();

        /**
         * @brief Waits for notification while holding the mutex
         * @param mutex Locked mutex to wait on
         * @throws sysError if wait operation fails
         * @implementation Delegates to underlying condition implementation
         */
        void wait(mutexBase& mutex) override;

        /**
         * @brief Waits for notification with timeout
         * @param mutex Locked mutex to wait on
         * @param d Maximum duration to wait
         * @return true if notified, false if timeout occurred
         * @throws sysError if wait operation fails
         * @implementation Delegates to underlying condition implementation
         */
        bool waitFor(mutexBase& mutex, time::duration d) override;

        /**
         * @brief Notifies one waiting thread
         * @throws sysError if notification fails
         * @implementation Delegates to underlying condition implementation
         */
        void notify() override;

        /**
         * @brief Notifies all waiting threads
         * @throws sysError if notification fails
         * @implementation Delegates to underlying condition implementation
         */
        void notifyAll() override;
    };

}
template<typename Pred>
void original::conditionBase::wait(mutexBase& mutex, Pred predicate) noexcept(noexcept(predicate())) {
    while (!predicate()){
        this->wait(mutex);
    }
}

template<typename Pred>
bool original::conditionBase::waitFor(mutexBase& mutex, const time::duration& d, Pred predicate) noexcept(noexcept(predicate())) {
    const time::point start = time::point::now();
    while (!predicate()) {
        auto elapsed = time::point::now() - start;
        if (elapsed >= d)
            return false;
        if (!this->waitFor(mutex, d - elapsed))
            return false;
        if (predicate())
            return true;
    }
    return true;
}

inline void original::conditionBase::notifySome(const u_integer n) {
    if (n == 0){
        return;
    }
    if (n == 1){
        this->notify();
    } else {
        this->notifyAll();
    }
}

#if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG
inline original::pCondition::pCondition() : cond_{}
{
    if (const int code = pthread_cond_init(&this->cond_, nullptr); code != 0)
    {
        throw sysError("Failed to initialize condition variable (pthread_cond_init returned " + printable::formatString(code) + ")");
    }
}

inline void original::pCondition::wait(mutexBase& mutex)
{
    staticError<valueError, !std::is_same_v<mutex::native_handle, pMutex::native_handle>>::asserts();

    const auto handle = static_cast<mutex::native_handle*>(mutex.nativeHandle());
    if (const int code = pthread_cond_wait(&this->cond_, handle); code != 0) {
        throw sysError("Failed to wait on condition variable (pthread_cond_wait returned " + printable::formatString(code) + ")");
    }
}

inline bool original::pCondition::waitFor(mutexBase& mutex, const time::duration d)
{
    staticError<valueError, !std::is_same_v<mutex::native_handle, pMutex::native_handle>>::asserts();

    const auto deadline = time::point::now() + d;
    const auto ts = deadline.toTimespec();
    const auto handle = static_cast<mutex::native_handle*>(mutex.nativeHandle());
    const int code = pthread_cond_timedwait(&this->cond_, handle, &ts);
    if (code == 0) return true;
    if (code == ETIMEDOUT) return false;
    throw sysError("Failed to timed wait on condition variable (pthread_cond_timed-wait returned " + printable::formatString(code) + ")");
}

inline void original::pCondition::notify()
{
    if (const int code = pthread_cond_signal(&this->cond_); code != 0) {
        throw sysError("Failed to signal condition variable (pthread_cond_signal returned " + printable::formatString(code) + ")");
    }
}

inline void original::pCondition::notifyAll()
{
    if (const int code = pthread_cond_broadcast(&this->cond_); code != 0) {
        throw sysError("Failed to broadcast condition variable (pthread_cond_broadcast returned " + printable::formatString(code) + ")");
    }
}

inline original::pCondition::~pCondition()
{
    if (const int code = pthread_cond_destroy(&this->cond_); code != 0) {
        std::cerr << "Warning: Failed to destroy condition variable (pthread_cond_destroy returned "
                  << code << ")" << std::endl;
    }
}
#elif ORIGINAL_COMPILER_MSVC
inline original::wCondition::wCondition() : cond_{}
{
    InitializeConditionVariable(&this->cond_);
}

inline void original::wCondition::wait(mutexBase& mutex)
{
    if (const auto handle = static_cast<mutex::native_handle*>(mutex.nativeHandle());
        !SleepConditionVariableSRW(&this->cond_, handle, INFINITE, 0)) {
        throw sysError("Failed to wait on condition variable (SleepConditionVariableSRW returned error " +
                      printable::formatString(GetLastError()));
    }
}

inline bool original::wCondition::waitFor(mutexBase& mutex, const time::duration d)
{
    const auto handle = static_cast<mutex::native_handle*>(mutex.nativeHandle());
    if (const auto timeout_ms = d.toDWMilliseconds();
        !SleepConditionVariableSRW(&this->cond_, handle, timeout_ms, 0)) {
        const DWORD error = GetLastError();
        if (error == ERROR_TIMEOUT) {
            return false;
        }
        throw sysError("Failed to timed wait on condition variable (SleepConditionVariableSRW returned error " +
                      printable::formatString(error));
    }
    return true;
}

inline void original::wCondition::notify()
{
    WakeConditionVariable(&this->cond_);
}

inline void original::wCondition::notifyAll()
{
    WakeAllConditionVariable(&this->cond_);
}

inline original::wCondition::~wCondition() = default;
#endif

inline original::condition::condition() = default;

inline void original::condition::wait(mutexBase& mutex)
{
    this->cond_.wait(mutex);
}

inline bool original::condition::waitFor(mutexBase& mutex, time::duration d)
{
    return this->cond_.waitFor(mutex, d);
}

inline void original::condition::notify()
{
    this->cond_.notify();
}

inline void original::condition::notifyAll()
{
    this->cond_.notifyAll();
}

#endif //CONDITION_H
