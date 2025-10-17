#ifndef MUTEX_H
#define MUTEX_H

#include "config.h"

#if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG
#include "pthread.h"
#elif ORIGINAL_COMPILER_MSVC
#endif

#include "error.h"
#include "tuple.h"
#include "zeit.h"
#include <iostream>

/**
 * @file mutex.h
 * @brief Cross-platform mutex and lock management utilities
 * @details
 * This header defines the mutex abstraction and RAII-based locking
 * mechanisms for multithreaded programming within the `original` namespace.
 *
 * Platform Support:
 * - GCC/Clang: Uses pthread_mutex_t for mutex implementation (pMutex)
 * - MSVC: Uses SRWLOCK for lightweight mutex implementation (wMutex)
 * - All platforms: High-level mutex class with consistent interface
 *
 * Features:
 * - RAII-based mutex management with automatic cleanup
 * - Multiple locking policies (automatic, manual, try-lock, adopt)
 * - Single and multiple mutex locking support
 * - Deadlock avoidance in multi-lock scenarios
 * - Exception-safe operations with proper error handling
 */


namespace original {
    /**
     * @class mutexBase
     * @brief Abstract base class for mutex implementations
     * @details Provides the interface for mutex operations including:
     * - Lock/unlock functionality
     * - Try-lock capability
     * - Access to native handle
     *
     * @note This is an abstract base class and cannot be instantiated directly.
     *       Derived classes must implement all pure virtual methods.
     */
    class mutexBase {
    public:
        /**
         * @brief Locks the mutex, blocking if necessary
         * @throws sysError if the lock operation fails
         */
        virtual void lock() = 0;

        /**
         * @brief Attempts to lock the mutex without blocking
         * @return true if lock was acquired, false otherwise
         * @throws sysError if the operation fails (other than EBUSY)
         */
        virtual bool tryLock() = 0;

        /**
         * @brief Unlocks the mutex
         * @throws sysError if the unlock operation fails
         */
        virtual void unlock() = 0;

        /**
         * @brief Gets a unique identifier for the mutex
         * @return Unique identifier based on mutex internal state
         */
        [[nodiscard]] virtual ul_integer id() const = 0;

        /// Default constructor
        explicit mutexBase() = default;

        /// Deleted copy constructor
        mutexBase(const mutexBase&) = delete;

        /// Deleted copy assignment operator
        mutexBase& operator=(const mutexBase&) = delete;

        /**
         * @brief Gets the native handle of the mutex
         * @return Pointer to the native mutex handle
         */
        [[nodiscard]] virtual void* nativeHandle() noexcept = 0;

        /// Virtual destructor
        virtual ~mutexBase() = default;
    };

    /**
     * @class lockGuard
     * @brief Abstract base class for lock guard implementations
     * @details Provides the interface for RAII-style lock management including:
     * - Various locking policies (automatic, manual, try-lock)
     * - Lock/unlock functionality
     * - Lock state query
     */
    class lockGuard {
    protected:
        /**
         * @brief Locks the associated mutex(es)
         * @throws sysError if the lock operation fails
         */
        virtual void lock() = 0;

        /**
         * @brief Attempts to lock the associated mutex(es) without blocking
         * @return true if lock was acquired, false otherwise
         * @throws sysError if the operation fails
         */
        virtual bool tryLock() = 0;

        /**
         * @brief Unlocks the associated mutex(es)
         * @throws sysError if the unlock operation fails
         */
        virtual void unlock() = 0;

        /**
         * @brief Checks if the guard currently holds the lock
         * @return true if locked, false otherwise
         */
        [[nodiscard]] virtual bool isLocked() const noexcept = 0;
    public:
        /**
         * @enum lockPolicy
         * @brief Locking policies for guard construction
         */
        enum class lockPolicy {
            MANUAL_LOCK,    ///< Don't lock automatically
            AUTO_LOCK,      ///< Lock immediately on construction
            TRY_LOCK,       ///< Try to lock immediately on construction
            ADOPT_LOCK,     ///< Assume lock is already held
        };

        /// Constant for manual lock policy
        static constexpr auto MANUAL_LOCK = lockPolicy::MANUAL_LOCK;
        /// Constant for automatic lock policy
        static constexpr auto AUTO_LOCK = lockPolicy::AUTO_LOCK;
        /// Constant for try-lock policy
        static constexpr auto TRY_LOCK = lockPolicy::TRY_LOCK;
        /// Constant for adopt lock policy
        static constexpr auto ADOPT_LOCK = lockPolicy::ADOPT_LOCK;

        /// Default constructor
        explicit lockGuard() = default;

        /// Deleted copy constructor
        lockGuard(const lockGuard&) = delete;

        /// Deleted copy assignment operator
        lockGuard& operator=(const lockGuard&) = delete;

        /// Virtual destructor
        virtual ~lockGuard() = default;
    };

#if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG
    /**
     * @class pMutex
     * @brief POSIX thread mutex implementation for GCC and Clang compilers
     * @extends mutexBase
     * @details Wrapper around pthread_mutex_t with RAII semantics. Provides
     *          full-featured mutex functionality using POSIX threads API.
     *
     * Platform-Specific Features:
     * - Uses pthread_mutex_init for mutex initialization
     * - Implements pthread_mutex_lock for blocking acquisition
     * - Uses pthread_mutex_trylock for non-blocking attempts
     * - Provides pthread_mutex_unlock for release
     * - Uses pthread_mutex_destroy for cleanup in destructor
     *
     * @note Only available when compiled with GCC or Clang
     * @note Provides recursive mutex capabilities through POSIX implementation
     * @note Exception-safe with proper resource cleanup
     *
     * Example usage:
     * @code
     * original::pMutex mutex;
     * {
     *     original::uniqueLock lock(mutex); // Automatically locks
     *     // Critical section
     * } // Automatically unlocks
     * @endcode
     */
    class pMutex final : public mutexBase {
        pthread_mutex_t mutex_{}; ///< Internal POSIX mutex handle
    public:
        /// Native handle type (pthread_mutex_t)
        using native_handle = pthread_mutex_t;

        /**
         * @brief Constructs and initializes the POSIX mutex
         * @throws sysError if pthread_mutex_init fails
         * @note Uses default mutex attributes (non-recursive)
         * @implementation Calls pthread_mutex_init with nullptr attributes
         */
        explicit pMutex();

        /// Deleted move constructor (mutexes are not movable)
        pMutex(pMutex&&) = delete;

        /// Deleted move assignment operator (mutexes are not movable)
        pMutex& operator=(pMutex&&) = delete;

        /**
         * @brief Gets a unique identifier for the mutex
         * @return Unique identifier based on mutex memory address
         * @implementation Returns reinterpret_cast of mutex address
         */
        [[nodiscard]] ul_integer id() const override;

        /**
         * @brief Gets the native mutex handle
         * @return Pointer to the internal pthread_mutex_t
         * @note Useful for interoperability with POSIX thread functions
         */
        [[nodiscard]] void* nativeHandle() noexcept override;

        /**
         * @brief Locks the mutex, blocking if necessary
         * @throws sysError if pthread_mutex_lock fails
         * @implementation Calls pthread_mutex_lock and checks return code
         */
        void lock() override;

        /**
         * @brief Attempts to lock the mutex without blocking
         * @return true if lock was acquired, false if mutex is busy (EBUSY)
         * @throws sysError if the operation fails (other than EBUSY)
         * @implementation Calls pthread_mutex_trylock and handles EBUSY
         */
        bool tryLock() override;

        /**
         * @brief Unlocks the mutex
         * @throws sysError if pthread_mutex_unlock fails
         * @implementation Calls pthread_mutex_unlock and checks return code
         */
        void unlock() override;

        /**
         * @brief Destroys the mutex
         * @note Calls std::terminate() if mutex destruction fails
         * @implementation Calls pthread_mutex_destroy and terminates on failure
         */
        ~pMutex() override;
    };
#elif ORIGINAL_COMPILER_MSVC
    /**
     * @class wMutex
     * @brief Windows Slim Reader/Writer (SRW) lock implementation for MSVC
     * @extends mutexBase
     * @details Wrapper around SRWLOCK with RAII semantics. Provides
     *          lightweight exclusive locking using Windows SRW locks.
     *
     * Platform-Specific Features:
     * - Uses InitializeSRWLock for lock initialization
     * - Implements AcquireSRWLockExclusive for blocking acquisition
     * - Uses TryAcquireSRWLockExclusive for non-blocking attempts
     * - Provides ReleaseSRWLockExclusive for release
     * - No explicit destruction required (Windows manages SRW lock lifecycle)
     *
     * @note Only available when compiled with MSVC
     * @note SRW locks are more efficient than CRITICAL_SECTION for exclusive access
     * @note Non-recursive lock - attempting to relock from same thread will deadlock
     *
     * Example usage:
     * @code
     * original::wMutex mutex;
     * {
     *     original::uniqueLock lock(mutex); // Automatically locks
     *     // Critical section
     * } // Automatically unlocks
     * @endcode
     */
    class wMutex final : public mutexBase {
        SRWLOCK lock_; ///< Windows SRW lock handle
    public:
        /// Native handle type (SRWLOCK)
        using native_handle = SRWLOCK;

        /**
         * @brief Constructs and initializes the SRW lock
         * @implementation Calls InitializeSRWLock
         * @note SRW locks require no explicit destruction in Windows
         */
        explicit wMutex();

        /// Deleted move constructor (locks are not movable)
        wMutex(wMutex&&) = delete;

        /// Deleted move assignment operator (locks are not movable)
        wMutex& operator=(wMutex&&) = delete;

        /**
         * @brief Gets a unique identifier for the lock
         * @return Unique identifier based on lock memory address
         * @implementation Returns reinterpret_cast of lock address
         */
        [[nodiscard]] ul_integer id() const override;

        /**
         * @brief Gets the native lock handle
         * @return Pointer to the internal SRWLOCK
         * @note Useful for interoperability with Windows thread functions
         */
        [[nodiscard]] void* nativeHandle() noexcept override;

        /**
         * @brief Locks the SRW lock exclusively, blocking if necessary
         * @implementation Calls AcquireSRWLockExclusive
         * @note This is an exclusive lock - only one thread can hold it
         */
        void lock() override;

        /**
         * @brief Attempts to lock the SRW lock exclusively without blocking
         * @return true if lock was acquired, false if lock is busy
         * @implementation Calls TryAcquireSRWLockExclusive
         */
        bool tryLock() override;

        /**
         * @brief Unlocks the SRW lock
         * @implementation Calls ReleaseSRWLockExclusive
         */
        void unlock() override;

        /**
         * @brief Destructor
         * @note SRW locks require no explicit destruction in Windows
         * @implementation No cleanup needed - Windows manages SRW lock lifecycle
         */
        ~wMutex() override;
    };
#endif

    /**
     * @class mutex
     * @brief High-level cross-platform mutex wrapper
     * @extends mutexBase
     * @details Provides a unified mutex interface that automatically selects
     *          the appropriate platform-specific implementation:
     *          - pMutex for GCC/Clang on Linux/macOS (POSIX threads)
     *          - wMutex for MSVC on Windows (SRW locks)
     *
     * Key Features:
     * - Consistent API across all platforms
     * - RAII semantics with automatic cleanup
     * - Exception-safe operations
     * - Delegates all operations to platform-specific implementation
     *
     * Platform Abstraction:
     * - GCC/Clang: Delegates to pMutex (pthread_mutex_t)
     * - MSVC: Delegates to wMutex (SRWLOCK)
     * - All platforms: Identical interface and behavior
     *
     * Example usage:
     * @code
     * original::mutex mtx;
     *
     * // Cross-platform locking - works the same on all platforms
     * mtx.lock();
     * try {
     *     // Critical section
     *     mtx.unlock();
     * } catch (...) {
     *     mtx.unlock();
     *     throw;
     * }
     *
     * // Or use RAII wrapper for exception safety
     * {
     *     original::uniqueLock lock(mtx);
     *     // Critical section - automatically unlocked
     * }
     * @endcode
     *
     * @see original::pMutex (GCC/Clang implementation)
     * @see original::wMutex (MSVC implementation)
     * @see original::uniqueLock
     */
    class mutex final : public mutexBase {
    #if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG
        pMutex mutex_; ///< POSIX mutex implementation (GCC/Clang)
    #elif ORIGINAL_COMPILER_MSVC
        wMutex mutex_; ///< Windows SRW lock implementation (MSVC)
    #endif

    public:
        /// Native handle type (platform-specific)
        using native_handle =
    #if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG
            pMutex::native_handle; ///< pthread_mutex_t on GCC/Clang
    #elif ORIGINAL_COMPILER_MSVC
            wMutex::native_handle; ///< SRWLOCK on MSVC
    #endif

        /**
         * @brief Constructs the platform-specific mutex
         * @details Delegates to pMutex or wMutex constructor
         */
        mutex();

        /// Deleted move constructor
        mutex(mutex&&) = delete;

        /// Deleted copy assignment operator
        mutex& operator=(mutex&) = delete;

        /**
         * @brief Gets a unique identifier for the mutex
         * @return Platform-specific mutex identifier
         * @details Delegates to underlying mutex implementation
         */
        [[nodiscard]] ul_integer id() const override;

        /**
         * @brief Gets the native mutex handle
         * @return Platform-specific native handle
         * @details Delegates to underlying mutex implementation
         */
        [[nodiscard]] void* nativeHandle() noexcept override;

        /**
         * @brief Locks the mutex, blocking if necessary
         * @throws sysError if lock operation fails
         * @details Delegates to underlying mutex implementation
         */
        void lock() override;

        /**
         * @brief Attempts to lock the mutex without blocking
         * @return true if lock was acquired, false otherwise
         * @throws sysError if operation fails
         * @details Delegates to underlying mutex implementation
         */
        bool tryLock() override;

        /**
         * @brief Unlocks the mutex
         * @throws sysError if unlock operation fails
         * @details Delegates to underlying mutex implementation
         */
        void unlock() override;
    };

    /**
     * @class uniqueLock
     * @brief RAII wrapper for single mutex locking
     * @extends lockGuard
     * @details Provides scoped lock management for a single pMutex
     * with various locking policies.
     */
    class uniqueLock final : public lockGuard {
        mutexBase& mutex_;   ///< Reference to managed mutex
        bool is_locked;     ///< Current lock state

    public:
        /**
         * @brief Constructs a uniqueLock with specified policy
         * @param mutex Mutex to manage
         * @param policy Locking policy (default: AUTO_LOCK)
         * @throws sysError if locking fails
         */
        explicit uniqueLock(mutexBase& mutex, lockPolicy policy = AUTO_LOCK);

        /// Deleted move constructor
        uniqueLock(uniqueLock&&) = delete;

        /// Deleted move assignment operator
        uniqueLock& operator=(uniqueLock&&) = delete;

        /**
         * @brief Checks if the lock is currently held
         * @return true if locked, false otherwise
         */
        [[nodiscard]] bool isLocked() const noexcept override;

        /**
         * @brief Locks the associated mutex
         * @throws sysError if already locked or locking fails
         */
        void lock() override;

        /**
         * @brief Attempts to lock the associated mutex without blocking
         * @return true if lock was acquired, false otherwise
         * @throws sysError if already locked or operation fails
         */
        bool tryLock() override;

        /**
         * @brief Unlocks the associated mutex
         * @throws sysError if not locked or unlock fails
         */
        void unlock() override;

        /**
         * @brief Destructor - automatically unlocks if locked
         */
        ~uniqueLock() override;
    };

    /**
     * @class multiLock
     * @brief RAII wrapper for multiple mutex locking
     * @extends lockGuard
     * @tparam MUTEX Variadic template parameter pack for mutex types
     * @details Provides scoped lock management for multiple mutexes
     * with deadlock avoidance and various locking policies.
     */
    template<typename... MUTEX>
    class multiLock final : public lockGuard {
        tuple<MUTEX* ...> m_list;    ///< Tuple of managed mutex pointers
        bool is_locked_all;           ///< Current lock state for all mutexes

        /**
         * @brief Locks all managed mutexes
         * @tparam IDXES Template parameter pack for index sequence
         * @param sequence Index sequence for tuple access
         */
        template<u_integer... IDXES>
        void lockAll(indexSequence<IDXES...> sequence);

        /**
         * @brief Attempts to lock all managed mutexes without blocking
         * @tparam IDXES Template parameter pack for index sequence
         * @param sequence Index sequence for tuple access
         * @return true if all locks were acquired, false otherwise
         */
        template<u_integer... IDXES>
        bool tryLockAll(indexSequence<IDXES...> sequence);

        /**
         * @brief Unlocks all managed mutexes
         * @tparam IDXES Template parameter pack for index sequence
         * @param sequence Index sequence for tuple access
         */
        template<u_integer... IDXES>
        void unlockAll(indexSequence<IDXES...> sequence);
    public:
        /**
         * @brief Constructs a multiLock with AUTO_LOCK policy
         * @param mutex References to mutexes to manage
         */
        explicit multiLock(MUTEX&... mutex);

        /**
         * @brief Constructs a multiLock with specified policy
         * @param policy Locking policy to use
         * @param mutex References to mutexes to manage
         */
        explicit multiLock(lockPolicy policy, MUTEX&... mutex);

        /**
         * @brief Checks if all mutexes are currently locked
         * @return true if all locked, false otherwise
         */
        [[nodiscard]] bool isLocked() const noexcept override;

        /**
         * @brief Locks all managed mutexes
         * @throws sysError if already locked or locking fails
         */
        void lock() override;

        /**
         * @brief Attempts to lock all managed mutexes without blocking
         * @return true if all locks were acquired, false otherwise
         * @throws sysError if already locked or operation fails
         */
        bool tryLock() override;

        /**
         * @brief Unlocks all managed mutexes
         * @throws sysError if not locked or unlock fails
         */
        void unlock() override;

        /// Deleted move constructor
        multiLock(multiLock&&) = delete;

        /// Deleted move assignment operator
        multiLock& operator=(multiLock&&) = delete;

        /**
         * @brief Destructor - automatically unlocks all if locked
         */
        ~multiLock() override;
    };
}

#if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG
inline original::pMutex::pMutex() : mutex_{} {
    if (const int code = pthread_mutex_init(&this->mutex_, nullptr);
        code != 0){
        throw sysError("Failed to initialize mutex (pthread_mutex_init returned " + printable::formatString(code) + ")");
    }
}

inline original::ul_integer original::pMutex::id() const {
    return reinterpret_cast<ul_integer>(&this->mutex_);
}

inline void* original::pMutex::nativeHandle() noexcept
{
    return &this->mutex_;
}

inline void original::pMutex::lock() {
    if (const int code = pthread_mutex_lock(&this->mutex_);
        code != 0) {
        throw sysError("Failed to lock mutex (pthread_mutex_lock returned " + printable::formatString(code) + ")");
    }
}

inline bool original::pMutex::tryLock() {
    if (const int code = pthread_mutex_trylock(&this->mutex_);
            code != 0) {
        if (code == EBUSY)
            return false;

        throw sysError("Failed to try-lock mutex (pthread_mutex_try-lock returned " + printable::formatString(code) + ")");
    }
    return true;
}

inline void original::pMutex::unlock() {
    if (const int code = pthread_mutex_unlock(&this->mutex_);
        code != 0){
        throw sysError("Failed to unlock mutex (pthread_mutex_unlock returned " + printable::formatString(code) + ")");
    }
}

inline original::pMutex::~pMutex() {
    if (const int code = pthread_mutex_destroy(&this->mutex_);
        code != 0){
        std::cerr << "Fatal error: Failed to destroy mutex (pthread_mutex_destroy returned "
                  << code << ")" << std::endl;
        std::terminate();
    }
}
#elif ORIGINAL_COMPILER_MSVC
inline original::wMutex::wMutex()
{
    InitializeSRWLock(&this->lock_);
}

inline original::ul_integer original::wMutex::id() const
{
    return reinterpret_cast<ul_integer>(&this->lock_);
}

inline void* original::wMutex::nativeHandle() noexcept
{
    return &this->lock_;
}

inline void original::wMutex::lock()
{
    AcquireSRWLockExclusive(&this->lock_);
}

inline bool original::wMutex::tryLock()
{
    return TryAcquireSRWLockExclusive(&this->lock_) != 0;
}

inline void original::wMutex::unlock()
{
    ReleaseSRWLockExclusive(&this->lock_);
}

inline original::wMutex::~wMutex() = default;

#endif

inline original::mutex::mutex() = default;

inline original::ul_integer original::mutex::id() const
{
    return this->mutex_.id();
}

inline void* original::mutex::nativeHandle() noexcept
{
    return this->mutex_.nativeHandle();
}

inline void original::mutex::lock()
{
    this->mutex_.lock();
}

inline bool original::mutex::tryLock()
{
    return this->mutex_.tryLock();
}

inline void original::mutex::unlock()
{
    this->mutex_.unlock();
}

inline original::uniqueLock::uniqueLock(mutexBase& mutex, const lockPolicy policy)
    : mutex_(mutex), is_locked(false) {
    switch (policy) {
        case MANUAL_LOCK:
            break;
        case AUTO_LOCK:
            this->lock();
            break;
        case TRY_LOCK:
            this->tryLock();
            break;
        case ADOPT_LOCK:
            this->is_locked = true;
    }
}

inline bool original::uniqueLock::isLocked() const noexcept {
    return this->is_locked;
}

inline void original::uniqueLock::lock() {
    if (this->is_locked)
        throw sysError("Cannot lock uniqueLock: already locked");

    this->mutex_.lock();
    this->is_locked = true;
}

inline bool original::uniqueLock::tryLock() {
    if (this->is_locked)
        throw sysError("Cannot try-lock uniqueLock: already locked");

    this->is_locked = this->mutex_.tryLock();
    return this->is_locked;
}

inline void original::uniqueLock::unlock() {
    if (this->is_locked){
        this->mutex_.unlock();
        this->is_locked = false;
    }
}

inline original::uniqueLock::~uniqueLock() {
    this->unlock();
}

template<typename... MUTEX>
template<original::u_integer... IDXES>
void original::multiLock<MUTEX...>::lockAll(indexSequence<IDXES...>) {
    (..., this->m_list.template get<IDXES>()->lock());
    this->is_locked_all = true;
}

template<typename... MUTEX>
template<original::u_integer... IDXES>
bool original::multiLock<MUTEX...>::tryLockAll(indexSequence<IDXES...>) {
    bool success = true;
    bool lock_status[sizeof...(IDXES)] = {};
    auto tryLockStatusUpdate = [&](auto i, auto* mutex) {
        if (mutex->tryLock()) {
            lock_status[i] = true;
        } else {
            success = false;
        }
    };

    u_integer idx = 0;
    ((tryLockStatusUpdate(idx++, this->m_list.template get<IDXES>())), ...);

    if (!success) {
        idx = 0;
        ((lock_status[idx]
            ? this->m_list.template get<IDXES>()->unlock() : void(), ++idx), ...);
    }
    this->is_locked_all = success;
    return success;
}

template<typename... MUTEX>
template<original::u_integer... IDXES>
void original::multiLock<MUTEX...>::unlockAll(indexSequence<IDXES...>) {
    (..., this->m_list.template get<IDXES>()->unlock());
}

template<typename... MUTEX>
original::multiLock<MUTEX...>::multiLock(MUTEX&... mutex)
    : multiLock(AUTO_LOCK, mutex...) {}

template<typename... MUTEX>
original::multiLock<MUTEX...>::multiLock(const lockPolicy policy, MUTEX&... mutex)
    : m_list(&mutex...), is_locked_all(false) {
    switch (policy) {
        case MANUAL_LOCK:
            break;
        case AUTO_LOCK:
            this->lock();
            break;
        case TRY_LOCK:
            this->tryLock();
            break;
        case ADOPT_LOCK:
            this->is_locked_all = true;
    }
}

template<typename... MUTEX>
bool original::multiLock<MUTEX...>::isLocked() const noexcept {
    return this->is_locked_all;
}

template<typename... MUTEX>
void original::multiLock<MUTEX...>::lock() {
    if (this->is_locked_all)
        throw sysError("Cannot lock multiLock: already locked");

    this->lockAll(makeSequence<sizeof...(MUTEX)>());
}

template<typename... MUTEX>
bool original::multiLock<MUTEX...>::tryLock() {
    if (this->is_locked_all)
        throw sysError("Cannot try-lock multiLock: already locked");

    return this->tryLockAll(makeSequence<sizeof...(MUTEX)>());
}

template<typename... MUTEX>
void original::multiLock<MUTEX...>::unlock() {
    if (this->is_locked_all)
        this->unlockAll(makeReverseSequence<sizeof...(MUTEX)>());
}

template<typename... MUTEX>
original::multiLock<MUTEX...>::~multiLock(){
    this->unlock();
}

#endif //MUTEX_H
