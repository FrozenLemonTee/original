#ifndef MUTEX_H
#define MUTEX_H

#if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG
#include "pthread.h"
#elif ORIGINAL_COMPILER_MSVC
#include "synchapi.h"
#endif

#include "error.h"
#include "tuple.h"
#include <iostream>

/**
 * @file mutex.h
 * @brief Cross-platform mutex and lock management utilities
 * @details
 * This header defines the mutex abstraction and RAII-based locking
 * mechanisms for multithreaded programming within the `original` namespace.
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
     * @brief POSIX thread mutex implementation
     * @extends mutexBase
     * @details Wrapper around pthread_mutex_t with RAII semantics.
     * Provides basic mutex functionality with proper initialization
     * and cleanup.
     */
    class pMutex final : public mutexBase {
        pthread_mutex_t mutex_{}; ///< Internal POSIX mutex handle
    public:
        /// Native handle type (pthread_mutex_t)
        using native_handle = pthread_mutex_t;

        /**
         * @brief Constructs and initializes the mutex
         * @throws sysError if mutex initialization fails
         */
        explicit pMutex();

        /// Deleted move constructor
        pMutex(pMutex&&) = delete;

        /// Deleted move assignment operator
        pMutex& operator=(pMutex&&) = delete;

        /**
         * @brief Gets a unique identifier for the mutex
         * @return Unique identifier based on mutex internal state
         */
        [[nodiscard]] ul_integer id() const override;

        /**
         * @brief Gets the native mutex handle
         * @return Pointer to the internal pthread_mutex_t
         */
        [[nodiscard]] void* nativeHandle() noexcept override;

        /**
         * @brief Locks the mutex, blocking if necessary
         * @throws sysError if the lock operation fails
         */
        void lock() override;

        /**
         * @brief Attempts to lock the mutex without blocking
         * @return true if lock was acquired, false if mutex is busy
         * @throws sysError if the operation fails (other than EBUSY)
         */
        bool tryLock() override;

        /**
         * @brief Unlocks the mutex
         * @throws sysError if the unlock operation fails
         */
        void unlock() override;

        /**
         * @brief Destroys the mutex
         * @note Calls std::terminate() if mutex destruction fails
         */
        ~pMutex() override;
    };
#elif ORIGINAL_COMPILER_MSVC
    class wMutex final : public mutexBase {
        CRITICAL_SECTION cs_;
    public:
        using native_handle = CRITICAL_SECTION;

        explicit wMutex();

        wMutex(wMutex&&) = delete;

        wMutex& operator=(wMutex&&) = delete;

        [[nodiscard]] ul_integer id() const override;

        [[nodiscard]] void* nativeHandle() noexcept override;

        void lock() override;

        bool tryLock() override;

        void unlock() override;

        ~wMutex() override;
    };
#endif

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
inline original::wMutex::wMutex() : cs_{}
{
    InitializeCriticalSection(&this->cs_);
}

inline original::ul_integer original::wMutex::id() const
{
    return reinterpret_cast<ul_integer>(&this->cs_);
}

inline void* original::wMutex::nativeHandle() noexcept
{
    return &this->cs_;
}

inline void original::wMutex::lock()
{
    EnterCriticalSection(&this->cs_);
}

inline bool original::wMutex::tryLock()
{
    return TryEnterCriticalSection(&this->cs_) != 0;
}

inline void original::wMutex::unlock()
{
    LeaveCriticalSection(&this->cs_);
}

inline original::wMutex::~wMutex()
{
    DeleteCriticalSection(&this->cs_);
}
#endif

inline original::uniqueLock::uniqueLock(mutexBase& mutex, lockPolicy policy)
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
