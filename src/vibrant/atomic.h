#ifndef ORIGINAL_ATOMIC_H
#define ORIGINAL_ATOMIC_H

/**
 * @file atomic.h
 * @brief Cross-platform atomic operations implementation
 * @details Provides type-safe atomic operations with automatic lock-free detection:
 * - Lock-free implementation for supported types using hardware atomic instructions
 * - Mutex-based fallback for non-lock-free types
 * - Consistent API across different platforms and compilers
 *
 * Platform Support:
 * - GCC/Clang: Uses GCC __atomic builtins for lock-free operations (pMutex for fallback)
 * - MSVC: Uses Windows Interlocked APIs for lock-free operations (wMutex for fallback)
 * - All platforms: Automatic selection between lock-free and mutex-based implementations
 *
 * Key Features:
 * - Automatic lock-free detection based on type characteristics
 * - Full memory ordering support (relaxed, acquire, release, acq_rel, seq_cst)
 * - Atomic arithmetic operations (add, subtract)
 * - Compare-and-exchange (CAS) operations
 * - Type-safe interface with operator overloading
 * - Factory functions for convenient object creation
 */

#include "config.h"

#if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG
#include <cstring>
#elif ORIGINAL_COMPILER_MSVC
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <intrin.h>
#endif

#include "optional.h"
#include "mutex.h"

namespace original {
#if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG

    // ==================== Memory Order Enumeration ====================

    /**
     * @enum memOrder
     * @brief Memory ordering constraints for atomic operations
     * @details Specifies the memory synchronization behavior of atomic operations:
     * - RELAXED: No ordering constraints, only atomicity guaranteed
     * - ACQUIRE: Subsequent reads cannot be reordered before this operation
     * - RELEASE: Previous writes cannot be reordered after this operation
     * - ACQ_REL: Combination of ACQUIRE and RELEASE semantics
     * - SEQ_CST: Sequential consistency (strongest ordering)
     */
    enum class memOrder {
        RELAXED = __ATOMIC_RELAXED,
        ACQUIRE = __ATOMIC_ACQUIRE,
        RELEASE = __ATOMIC_RELEASE,
        ACQ_REL = __ATOMIC_ACQ_REL,
        SEQ_CST = __ATOMIC_SEQ_CST,
    };

    // ==================== Forward Declarations ====================

    template<typename TYPE, bool USE_LOCK>
    class atomicImpl;

    /**
     * @brief Alias template for atomic type selection
     * @tparam TYPE The underlying type to make atomic
     * @details Automatically selects lock-free implementation if possible:
     * - Type must be trivially copyable and destructible
     * - GCC's __atomic_always_lock_free must return true for the type size
     * - Otherwise falls back to mutex-based implementation
     */
    template<typename TYPE>
    using atomic = atomicImpl<
        TYPE,
        !( std::is_trivially_copyable_v<TYPE> &&
           std::is_trivially_destructible_v<TYPE> &&
           __atomic_always_lock_free(sizeof(TYPE), nullptr) )
    >;

    // ==================== Lock-Free Implementation ====================

    /**
     * @class atomicImpl
     * @brief Lock-free atomic implementation using GCC builtins
     * @tparam TYPE The underlying atomic type
     * @details Provides atomic operations without locking for types that support
     *          hardware-level atomic operations. Uses GCC's __atomic builtins.
     */
    template<typename TYPE>
    class atomicImpl<TYPE, false> {
        alignas(TYPE) byte data_[sizeof(TYPE)]{};  ///< Properly aligned storage

        /**
         * @brief Default constructor (zero-initializes storage)
         */
        atomicImpl();

        /**
         * @brief Value constructor with memory ordering
         * @param value Initial value to store
         * @param order Memory ordering constraint (default: SEQ_CST)
         */
        explicit atomicImpl(TYPE value, memOrder order = SEQ_CST);

    public:
        // Memory ordering constants for convenience
        static constexpr auto RELAXED = memOrder::RELAXED;
        static constexpr auto ACQUIRE = memOrder::ACQUIRE;
        static constexpr auto RELEASE = memOrder::RELEASE;
        static constexpr auto ACQ_REL = memOrder::ACQ_REL;
        static constexpr auto SEQ_CST = memOrder::SEQ_CST;

        // Disable copying and moving
        atomicImpl(const atomicImpl&) = delete;
        atomicImpl(atomicImpl&&) = delete;
        atomicImpl& operator=(const atomicImpl&) = delete;
        atomicImpl& operator=(atomicImpl&&) = delete;

        /**
         * @brief Checks if the atomic implementation is lock-free
         * @return Always true for this specialization
         */
        static constexpr bool isLockFree() noexcept;

        /**
         * @brief Atomically stores a value
         * @param value Value to store
         * @param order Memory ordering constraint (default: SEQ_CST)
         */
        void store(TYPE value, memOrder order = SEQ_CST);

        /**
         * @brief Atomically loads the current value
         * @param order Memory ordering constraint (default: SEQ_CST)
         * @return The current atomic value
         */
        TYPE load(memOrder order = SEQ_CST) const noexcept;

        /**
         * @brief Dereference operator (loads current value)
         * @return The current atomic value
         */
        TYPE operator*() const noexcept;

        /**
         * @brief Conversion operator to underlying type
         * @return The current atomic value
         */
        explicit operator TYPE() const noexcept;

        /**
         * @brief Assignment operator (atomically stores value)
         * @param value Value to store
         */
        atomicImpl& operator=(TYPE value) noexcept;

        /**
         * @brief Atomic addition assignment
         * @param value Value to add
         * @return Reference to this atomic object
         */
        atomicImpl& operator+=(TYPE value) noexcept;

        /**
         * @brief Atomic subtraction assignment
         * @param value Value to subtract
         * @return Reference to this atomic object
         */
        atomicImpl& operator-=(TYPE value) noexcept;

        /**
         * @brief Atomically exchanges value
         * @param value New value to store
         * @param order Memory ordering constraint (default: SEQ_CST)
         * @return The previous value
         */
        TYPE exchange(TYPE value, memOrder order = SEQ_CST) noexcept;

        /**
         * @brief Atomically compares and exchanges value (CAS operation)
         * @param expected Expected current value (updated if comparison fails)
         * @param desired Desired new value
         * @param order Memory ordering constraint (default: SEQ_CST)
         * @return True if exchange was successful, false otherwise
         */
        bool exchangeCmp(TYPE& expected, TYPE desired, memOrder order = SEQ_CST) noexcept;

        /// @brief Default destructor
        ~atomicImpl() = default;

        // Friend factory functions
        template<typename T>
        friend auto makeAtomic();

        template<typename T>
        friend auto makeAtomic(T value);
    };

    // ==================== Mutex-Based Implementation ====================

    /**
     * @class atomicImpl
     * @brief Mutex-based atomic implementation for non-lock-free types
     * @tparam TYPE The underlying atomic type
     * @details Provides atomic operations using mutex locking for types that
     *          cannot be handled by hardware atomic operations.
     */
    template<typename TYPE>
    class atomicImpl<TYPE, true> {
        mutable pMutex mutex_;          ///< Mutex for synchronization
        alternative<TYPE> data_;        ///< Optional storage for the value

        /**
         * @brief Default constructor
         */
        atomicImpl() = default;

        /**
         * @brief Value constructor
         * @param value Initial value to store
         * @param order Memory ordering (ignored in mutex implementation)
         */
        explicit atomicImpl(TYPE value, memOrder order = RELEASE);

    public:
        // Memory ordering constants (for API compatibility)
        static constexpr auto RELAXED = memOrder::RELAXED;
        static constexpr auto ACQUIRE = memOrder::ACQUIRE;
        static constexpr auto RELEASE = memOrder::RELEASE;
        static constexpr auto ACQ_REL = memOrder::ACQ_REL;
        static constexpr auto SEQ_CST = memOrder::SEQ_CST;

        // Disable copying and moving
        atomicImpl(const atomicImpl&) = delete;
        atomicImpl(atomicImpl&&) = delete;
        atomicImpl& operator=(const atomicImpl&) = delete;
        atomicImpl& operator=(atomicImpl&&) = delete;

        /**
         * @brief Checks if the atomic implementation is lock-free
         * @return Always false for this specialization
         */
        static constexpr bool isLockFree() noexcept;

        /**
         * @brief Atomically stores a value
         * @param value Value to store
         * @param order Memory ordering (ignored)
         */
        void store(TYPE value, memOrder order = SEQ_CST);

        /**
         * @brief Atomically loads the current value
         * @param order Memory ordering (ignored)
         * @return The current atomic value
         */
        TYPE load(memOrder order = SEQ_CST) const noexcept;

        /**
         * @brief Dereference operator (loads current value)
         * @return The current atomic value
         */
        TYPE operator*() const noexcept;

        /**
         * @brief Conversion operator to underlying type
         * @return The current atomic value
         */
        explicit operator TYPE() const noexcept;

        /**
         * @brief Assignment operator (atomically stores value)
         * @param value Value to store
         */
        atomicImpl& operator=(TYPE value) noexcept;

        /**
         * @brief Atomic addition assignment
         * @param value Value to add
         * @return Reference to this atomic object
         */
        atomicImpl& operator+=(TYPE value) noexcept;

        /**
         * @brief Atomic subtraction assignment
         * @param value Value to subtract
         * @return Reference to this atomic object
         */
        atomicImpl& operator-=(TYPE value) noexcept;

        /**
         * @brief Atomically exchanges value
         * @param value New value to store
         * @param order Memory ordering (ignored)
         * @return The previous value
         */
        TYPE exchange(const TYPE& value, memOrder order = SEQ_CST) noexcept;

        /**
         * @brief Atomically compares and exchanges value (CAS operation)
         * @param expected Expected current value (updated if comparison fails)
         * @param desired Desired new value
         * @param order Memory ordering (ignored)
         * @return True if exchange was successful, false otherwise
         */
        bool exchangeCmp(TYPE& expected, const TYPE& desired, memOrder order = SEQ_CST) noexcept;

        /// @brief Default destructor
        ~atomicImpl() = default;

        // Friend factory functions
        template<typename T>
        friend auto makeAtomic();

        template<typename T>
        friend auto makeAtomic(T value);
    };
#elif ORIGINAL_COMPILER_MSVC
    /**
     * @enum memOrder
     * @brief Memory ordering constraints for atomic operations (MSVC implementation)
     * @details Specifies the memory synchronization behavior of atomic operations:
     * - RELAXED: No ordering constraints, only atomicity guaranteed
     * - ACQUIRE: Subsequent reads cannot be reordered before this operation
     * - RELEASE: Previous writes cannot be reordered after this operation
     * - ACQ_REL: Combination of ACQUIRE and RELEASE semantics
     * - SEQ_CST: Sequential consistency (strongest ordering)
     *
     * @note MSVC implementation uses different underlying mechanisms than GCC:
     * - RELAXED/ACQUIRE/RELEASE: Use compiler barriers (_ReadBarrier/_WriteBarrier)
     * - SEQ_CST: Uses Interlocked APIs for full memory ordering
     * @note Only available when compiled with MSVC
     */
    enum class memOrder {
        RELAXED,
        ACQUIRE,
        RELEASE,
        ACQ_REL,
        SEQ_CST,
    };

    /**
     * @brief Determines if a type can be implemented lock-free on MSVC
     * @tparam TYPE The type to check for lock-free support
     * @return true if type can be implemented lock-free, false otherwise
     * @details MSVC supports lock-free operations for 4-byte and 8-byte types
     * using Interlocked APIs. Other sizes require mutex-based implementation.
     * @implementation Checks if type size is 4 or 8 bytes
     * @note Only available when compiled with MSVC
     */
    template<typename TYPE>
    constexpr bool isLockFreeType() noexcept;

    template<typename TYPE, bool USE_LOCK>
    class atomicImpl;

    template<typename TYPE>
    using atomic = atomicImpl<TYPE, !isLockFreeType<TYPE>()>;

    /**
     * @class atomicImpl
     * @brief Lock-free atomic implementation using MSVC Interlocked APIs
     * @tparam TYPE The underlying atomic type (must be 4 or 8 bytes)
     * @details Provides atomic operations using Windows Interlocked functions
     * for types that support hardware-level atomic operations.
     *
     * Platform-Specific Features:
     * - Uses InterlockedExchange for atomic stores with SEQ_CST ordering
     * - Uses InterlockedCompareExchange for atomic loads with SEQ_CST ordering
     * - Uses InterlockedAdd for atomic arithmetic operations
     * - Uses InterlockedCompareExchange for CAS operations
     * - Uses compiler barriers (_ReadBarrier/_WriteBarrier) for weaker ordering
     *
     * Supported Type Sizes:
     * - 4-byte types: Uses InterlockedExchange, InterlockedAdd, etc.
     * - 8-byte types: Uses InterlockedExchange64, InterlockedAdd64, etc.
     *
     * @note Only available when compiled with MSVC
     * @note Only supports 4-byte and 8-byte types (checked by isLockFreeType)
     * @note For other type sizes, falls back to mutex-based implementation
     *
     * Example usage:
     * @code
     * // 4-byte integer (lock-free)
     * original::atomic<int> atomic_int(42);
     * atomic_int.store(100);
     * int value = atomic_int.load();
     *
     * // 8-byte integer (lock-free)
     * original::atomic<long long> atomic_ll(123LL);
     * atomic_ll += 10;
     * @endcode
     */
    template<typename TYPE>
    class atomicImpl<TYPE, false>
    {
        /// @brief Internal value type selection based on size
        using val_type = some_t<sizeof(TYPE) == 4, LONG, LONG64>;

        /// @brief Properly aligned atomic storage
        alignas(TYPE) volatile TYPE data_{};

        /**
         * @brief Type conversion helper for atomic operations
         * @tparam From Source type
         * @tparam To Target type
         * @param value Value to convert
         * @return Converted value
         * @implementation Uses reinterpret_cast for pointers, static_cast for values
         */
        template<typename From, typename To>
        static To atomicCastTo(From value);

        /**
         * @brief Reverse type conversion helper
         * @tparam To Target type
         * @tparam From Source type
         * @param value Value to convert back
         * @return Original type value
         */
        template<typename To, typename From>
        static To atomicCastBack(From value);

        /**
         * @brief Default constructor (zero-initializes storage)
         * @implementation Uses memset for proper initialization
         */
        atomicImpl();

        /**
         * @brief Value constructor with memory ordering
         * @param value Initial value to store
         * @param order Memory ordering constraint (default: SEQ_CST)
         * @implementation Delegates to store() method
         */
        explicit atomicImpl(TYPE value, memOrder order = SEQ_CST);

    public:
        // Memory ordering constants for convenience
        static constexpr auto RELAXED = memOrder::RELAXED;
        static constexpr auto ACQUIRE = memOrder::ACQUIRE;
        static constexpr auto RELEASE = memOrder::RELEASE;
        static constexpr auto ACQ_REL = memOrder::ACQ_REL;
        static constexpr auto SEQ_CST = memOrder::SEQ_CST;

        // Disable copying and moving
        atomicImpl(const atomicImpl&) = delete;
        atomicImpl(atomicImpl&&) = delete;
        atomicImpl& operator=(const atomicImpl&) = delete;
        atomicImpl& operator=(atomicImpl&&) = delete;

        /**
         * @brief Checks if the atomic implementation is lock-free
         * @return Always true for this specialization
         * @implementation Returns true since this is the lock-free specialization
         */
        static constexpr bool isLockFree() noexcept;

        /**
         * @brief Atomically stores a value with specified memory ordering
         * @param value Value to store
         * @param order Memory ordering constraint (default: SEQ_CST)
         * @implementation MSVC-specific:
         * - RELAXED: Direct assignment
         * - RELEASE: _WriteBarrier + direct assignment
         * - SEQ_CST: InterlockedExchange/InterlockedExchange64
         * - ACQUIRE/ACQ_REL: Not supported for stores, uses SEQ_CST
         */
        void store(TYPE value, memOrder order = SEQ_CST);

        /**
         * @brief Atomically loads the current value with specified memory ordering
         * @param order Memory ordering constraint (default: SEQ_CST)
         * @return The current atomic value
         * @implementation MSVC-specific:
         * - RELAXED: Direct read
         * - ACQUIRE: Direct read + _ReadBarrier
         * - SEQ_CST: InterlockedCompareExchange with 0
         * - RELEASE/ACQ_REL: Not supported for loads, uses SEQ_CST
         */
        TYPE load(memOrder order = SEQ_CST) const noexcept;

        /**
         * @brief Dereference operator (loads current value with SEQ_CST ordering)
         * @return The current atomic value
         */
        TYPE operator*() const noexcept;

        /**
         * @brief Conversion operator to underlying type
         * @return The current atomic value
         */
        explicit operator TYPE() const noexcept;

        /**
         * @brief Assignment operator (atomically stores value with SEQ_CST ordering)
         * @param value Value to store
         */
        atomicImpl& operator=(TYPE value) noexcept;

        /**
         * @brief Atomic addition assignment
         * @param value Value to add
         * @return Reference to this atomic object
         * @implementation Uses InterlockedAdd/InterlockedAdd64
         */
        atomicImpl& operator+=(TYPE value) noexcept;

        /**
         * @brief Atomic subtraction assignment
         * @param value Value to subtract
         * @return Reference to this atomic object
         * @implementation Implements as addition of negative value
         */
        atomicImpl& operator-=(TYPE value) noexcept;

        /**
         * @brief Atomically exchanges value
         * @param value New value to store
         * @param order Memory ordering constraint (default: SEQ_CST)
         * @return The previous value
         * @implementation Uses InterlockedExchange/InterlockedExchange64
         */
        TYPE exchange(TYPE value, memOrder order = SEQ_CST) noexcept;

        /**
         * @brief Atomically compares and exchanges value (CAS operation)
         * @param expected Expected current value (updated if comparison fails)
         * @param desired Desired new value
         * @param order Memory ordering constraint (default: SEQ_CST)
         * @return True if exchange was successful, false otherwise
         * @implementation Uses InterlockedCompareExchange/InterlockedCompareExchange64
         */
        bool exchangeCmp(TYPE& expected, TYPE desired, memOrder order = SEQ_CST) noexcept;

        /// @brief Default destructor
        ~atomicImpl() = default;

        // Friend factory functions
        template<typename T>
        friend auto makeAtomic();

        template<typename T>
        friend auto makeAtomic(T value);
    };

    /**
     * @class atomicImpl
     * @brief Mutex-based atomic implementation for non-lock-free types on MSVC
     * @tparam TYPE The underlying atomic type
     * @details Provides atomic operations using Windows SRW locks for types that
     * cannot be handled by hardware atomic operations.
     *
     * Platform-Specific Features:
     * - Uses wMutex (Windows SRW lock) for synchronization
     * - Provides same API as lock-free implementation but with locking
     * - Memory ordering parameters are ignored (all operations are sequentially consistent)
     *
     * @note Only available when compiled with MSVC
     * @note Used for types that are not 4 or 8 bytes in size
     * @note All operations acquire the mutex, providing full sequential consistency
     *
     * Example usage:
     * @code
     * // Large struct (requires mutex-based implementation)
     * struct LargeData { char data[128]; };
     * original::atomic<LargeData> atomic_data;
     * atomic_data.store(LargeData{});
     * LargeData value = atomic_data.load();
     * @endcode
     */
    template<typename TYPE>
    class atomicImpl<TYPE, true>
    {
        mutable wMutex mutex_;          ///< Windows SRW lock for synchronization
        alternative<TYPE> data_;        ///< Optional storage for the value

        /**
         * @brief Default constructor
         */
        atomicImpl() = default;

        /**
         * @brief Value constructor
         * @param value Initial value to store
         * @param order Memory ordering (ignored in mutex implementation)
         */
        explicit atomicImpl(TYPE value, memOrder order = RELEASE);

    public:
        // Memory ordering constants (for API compatibility)
        static constexpr auto RELAXED = memOrder::RELAXED;
        static constexpr auto ACQUIRE = memOrder::ACQUIRE;
        static constexpr auto RELEASE = memOrder::RELEASE;
        static constexpr auto ACQ_REL = memOrder::ACQ_REL;
        static constexpr auto SEQ_CST = memOrder::SEQ_CST;

        // Disable copying and moving
        atomicImpl(const atomicImpl&) = delete;
        atomicImpl(atomicImpl&&) = delete;
        atomicImpl& operator=(const atomicImpl&) = delete;
        atomicImpl& operator=(atomicImpl&&) = delete;

        /**
         * @brief Checks if the atomic implementation is lock-free
         * @return Always false for this specialization
         */
        static constexpr bool isLockFree() noexcept;

        /**
         * @brief Atomically stores a value
         * @param value Value to store
         * @param order Memory ordering (ignored)
         * @implementation Locks mutex, stores value, unlocks mutex
         */
        void store(TYPE value, memOrder order = SEQ_CST);

        /**
         * @brief Atomically loads the current value
         * @param order Memory ordering (ignored)
         * @return The current atomic value
         * @implementation Locks mutex, reads value, unlocks mutex
         */
        TYPE load(memOrder order = SEQ_CST) const noexcept;

        /**
         * @brief Dereference operator (loads current value)
         * @return The current atomic value
         */
        TYPE operator*() const noexcept;

        /**
         * @brief Conversion operator to underlying type
         * @return The current atomic value
         */
        explicit operator TYPE() const noexcept;

        /**
         * @brief Assignment operator (atomically stores value)
         * @param value Value to store
         */
        atomicImpl& operator=(TYPE value) noexcept;

        /**
         * @brief Atomic addition assignment
         * @param value Value to add
         * @return Reference to this atomic object
         * @implementation Locks mutex, performs addition, stores result, unlocks mutex
         */
        atomicImpl& operator+=(TYPE value) noexcept;

        /**
         * @brief Atomic subtraction assignment
         * @param value Value to subtract
         * @return Reference to this atomic object
         * @implementation Locks mutex, performs subtraction, stores result, unlocks mutex
         */
        atomicImpl& operator-=(TYPE value) noexcept;

        /**
         * @brief Atomically exchanges value
         * @param value New value to store
         * @param order Memory ordering (ignored)
         * @return The previous value
         * @implementation Locks mutex, exchanges values, unlocks mutex
         */
        TYPE exchange(const TYPE& value, memOrder order = SEQ_CST) noexcept;

        /**
         * @brief Atomically compares and exchanges value (CAS operation)
         * @param expected Expected current value (updated if comparison fails)
         * @param desired Desired new value
         * @param order Memory ordering (ignored)
         * @return True if exchange was successful, false otherwise
         * @implementation Locks mutex, compares values, exchanges if equal, unlocks mutex
         */
        bool exchangeCmp(TYPE& expected, const TYPE& desired, memOrder order = SEQ_CST) noexcept;

        /// @brief Default destructor
        ~atomicImpl() = default;

        // Friend factory functions
        template<typename T>
        friend auto makeAtomic();

        template<typename T>
        friend auto makeAtomic(T value);
    };
#endif
    // ==================== Factory Functions ====================

    /**
     * @brief Creates a default-constructed atomic object
     * @tparam TYPE The atomic type to create
     * @return A new atomic object with default-initialized value
     */
    template<typename TYPE>
    auto makeAtomic();

    /**
     * @brief Creates an atomic object with initial value
     * @tparam TYPE The atomic type to create
     * @param value Initial value for the atomic object
     * @return A new atomic object with the specified value
     */
    template<typename TYPE>
    auto makeAtomic(TYPE value);

} // namespace original

#if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG
template <typename TYPE>
original::atomicImpl<TYPE, false>::atomicImpl() {
    std::memset(this->data_, byte{}, sizeof(TYPE));
}

template <typename TYPE>
original::atomicImpl<TYPE, false>::atomicImpl(TYPE value, memOrder order) {
    __atomic_store(reinterpret_cast<TYPE*>(this->data_), &value, static_cast<integer>(order));
}

template <typename TYPE>
constexpr bool original::atomicImpl<TYPE, false>::isLockFree() noexcept {
    return true;
}

template <typename TYPE>
void original::atomicImpl<TYPE, false>::store(TYPE value, memOrder order) {
    __atomic_store(reinterpret_cast<TYPE*>(this->data_), &value, static_cast<integer>(order));
}

template <typename TYPE>
TYPE original::atomicImpl<TYPE, false>::load(memOrder order) const noexcept {
    TYPE result;
    __atomic_load(reinterpret_cast<const TYPE*>(this->data_), &result, static_cast<integer>(order));
    return result;
}

template <typename TYPE>
TYPE original::atomicImpl<TYPE, false>::operator*() const noexcept
{
    return this->load();
}

template <typename TYPE>
original::atomicImpl<TYPE, false>::operator TYPE() const noexcept
{
    return this->load();
}

template <typename TYPE>
original::atomicImpl<TYPE, false>& original::atomicImpl<TYPE, false>::operator=(TYPE value) noexcept
{
    this->store(std::move(value));
    return *this;
}

template <typename TYPE>
original::atomicImpl<TYPE, false>& original::atomicImpl<TYPE, false>::operator+=(TYPE value) noexcept
{
    __atomic_fetch_add(reinterpret_cast<TYPE*>(this->data_), value, static_cast<integer>(memOrder::SEQ_CST));
    return *this;
}

template <typename TYPE>
original::atomicImpl<TYPE, false>& original::atomicImpl<TYPE, false>::operator-=(TYPE value) noexcept
{
    __atomic_fetch_sub(reinterpret_cast<TYPE*>(this->data_), value, static_cast<integer>(memOrder::SEQ_CST));
    return *this;
}

template <typename TYPE>
TYPE original::atomicImpl<TYPE, false>::exchange(TYPE value, memOrder order) noexcept {
    TYPE result;
    __atomic_exchange(reinterpret_cast<TYPE*>(this->data_), &value,
                      &result, static_cast<integer>(order));
    return result;
}

template <typename TYPE>
bool original::atomicImpl<TYPE, false>::exchangeCmp(TYPE& expected, TYPE desired, memOrder order) noexcept
{
    return __atomic_compare_exchange_n(reinterpret_cast<TYPE*>(this->data_),
                                       &expected, std::move(desired), false,
                     static_cast<integer>(order), static_cast<integer>(order));
}

template <typename TYPE>
original::atomicImpl<TYPE, true>::atomicImpl(TYPE value, memOrder) {
    uniqueLock lock{this->mutex_};
    this->data_.set(value);
}

template <typename TYPE>
constexpr bool original::atomicImpl<TYPE, true>::isLockFree() noexcept {
    return false;
}

template <typename TYPE>
void original::atomicImpl<TYPE, true>::store(TYPE value, memOrder) {
    uniqueLock lock{this->mutex_};
    this->data_.set(value);
}

template <typename TYPE>
TYPE original::atomicImpl<TYPE, true>::load(memOrder) const noexcept {
    uniqueLock lock{this->mutex_};
    return *this->data_;
}

template <typename TYPE>
TYPE original::atomicImpl<TYPE, true>::operator*() const noexcept
{
    return this->load();
}

template <typename TYPE>
original::atomicImpl<TYPE, true>::operator TYPE() const noexcept
{
    return this->load();
}

template <typename TYPE>
original::atomicImpl<TYPE, true>&
original::atomicImpl<TYPE, true>::operator=(TYPE value) noexcept
{
    this->store(std::move(value));
    return *this;
}

template <typename TYPE>
original::atomicImpl<TYPE, true>& original::atomicImpl<TYPE, true>::operator+=(TYPE value) noexcept
{
    uniqueLock lock{this->mutex_};
    TYPE result = *this->data_ + value;
    this->data_.set(result);
    return *this;
}

template <typename TYPE>
original::atomicImpl<TYPE, true>& original::atomicImpl<TYPE, true>::operator-=(TYPE value) noexcept
{
    uniqueLock lock{this->mutex_};
    TYPE result = *this->data_ - value;
    this->data_.set(result);
    return *this;
}

template <typename TYPE>
TYPE original::atomicImpl<TYPE, true>::exchange(const TYPE& value, memOrder) noexcept {
    uniqueLock lock{this->mutex_};
    TYPE result = *this->data_;
    this->data_.set(value);
    return result;
}

template <typename TYPE>
bool original::atomicImpl<TYPE, true>::exchangeCmp(TYPE& expected, const TYPE& desired, memOrder) noexcept {
    uniqueLock lock{this->mutex_};
    if (*this->data_ == expected) {
        this->data_.set(desired);
        return true;
    }
    expected = *this->data_;
    return false;
}
#elif ORIGINAL_COMPILER_MSVC

template<typename TYPE>
constexpr bool original::isLockFreeType() noexcept {
    return sizeof(TYPE) == 4 || sizeof(TYPE) == 8;
}

template <typename TYPE>
template <typename From, typename To>
To original::atomicImpl<TYPE, false>::atomicCastTo(From value)
{
    if constexpr (std::is_pointer_v<From>) {
        return reinterpret_cast<To>(value);
    } else {
        return static_cast<To>(value);
    }
}

template <typename TYPE>
template <typename To, typename From>
To original::atomicImpl<TYPE, false>::atomicCastBack(From value)
{
    if constexpr (std::is_pointer_v<To>) {
        return reinterpret_cast<To>(value);
    } else {
        return static_cast<To>(value);
    }
}

template <typename TYPE>
original::atomicImpl<TYPE, false>::atomicImpl()
{
    std::memset(this->data_, byte{}, sizeof(TYPE));
}

template <typename TYPE>
original::atomicImpl<TYPE, false>::atomicImpl(TYPE value, const memOrder order)
{
    this->store(value, order);
}

template <typename TYPE>
constexpr bool
original::atomicImpl<TYPE, false>::isLockFree() noexcept
{
    return true;
}

template <typename TYPE>
void original::atomicImpl<TYPE, false>::store(TYPE value, const memOrder order)
{
    switch (order) {
    case memOrder::RELAXED:
        this->data_ = value;
        break;
    case memOrder::RELEASE:
        _WriteBarrier();
        this->data_ = value;
        break;
    case memOrder::SEQ_CST:
    default:
        if constexpr (sizeof(TYPE) == 4) {
            InterlockedExchange(reinterpret_cast<volatile val_type*>(&this->data_),
                                atomicCastTo<TYPE, val_type>(value));
        } else {
            InterlockedExchange64(reinterpret_cast<volatile val_type*>(&this->data_),
                                  atomicCastTo<TYPE, val_type>(value));
        }
        break;
    }
}

template <typename TYPE>
TYPE original::atomicImpl<TYPE, false>::load(const memOrder order) const noexcept
{
    TYPE value;
    switch (order) {
    case memOrder::RELAXED:
        value = this->data_;
        break;
    case memOrder::ACQUIRE:
        value = this->data_;
        _ReadBarrier();
        break;
    case memOrder::SEQ_CST:
    default:
        if constexpr (sizeof(TYPE) == 4) {
            return atomicCastBack<TYPE, val_type>(
                InterlockedCompareExchange(
                    reinterpret_cast<volatile LONG*>(const_cast<TYPE*>(&this->data_)),
                    0, 0));
        } else {
            return atomicCastBack<TYPE, val_type>(
                InterlockedCompareExchange64(
                    reinterpret_cast<volatile LONG64*>(const_cast<TYPE*>(&this->data_)),
                    0, 0));
        }
        break;
    }
    return value;
}

template <typename TYPE>
TYPE original::atomicImpl<TYPE, false>::operator*() const noexcept
{
    return this->load();
}

template <typename TYPE>
original::atomicImpl<TYPE, false>::operator TYPE() const noexcept
{
    return this->load();
}

template <typename TYPE>
original::atomicImpl<TYPE, false>&
original::atomicImpl<TYPE, false>::operator=(TYPE value) noexcept
{
    this->store(std::move(value));
    return *this;
}

template <typename TYPE>
original::atomicImpl<TYPE, false>&
original::atomicImpl<TYPE, false>::operator+=(TYPE value) noexcept
{
    if constexpr (sizeof(TYPE) == 4) {
        InterlockedAdd(reinterpret_cast<volatile val_type*>(&this->data_),
                       atomicCastTo<TYPE, val_type>(value));
    } else {
        InterlockedAdd64(reinterpret_cast<volatile val_type*>(&this->data_),
                         atomicCastTo<TYPE, val_type>(value));
    }
    return *this;
}

template <typename TYPE>
original::atomicImpl<TYPE, false>&
original::atomicImpl<TYPE, false>::operator-=(TYPE value) noexcept
{
    return *this += -value;
}

template <typename TYPE>
TYPE original::atomicImpl<TYPE, false>::exchange(TYPE value, memOrder) noexcept
{
    if constexpr (sizeof(TYPE) == 4) {
        return atomicCastBack<TYPE, val_type>(
            InterlockedExchange(reinterpret_cast<volatile val_type*>(&this->data_),
                                atomicCastTo<TYPE, val_type>(value))
        );
    } else {
        return atomicCastBack<TYPE, val_type>(
            InterlockedExchange64(reinterpret_cast<volatile val_type*>(&this->data_),
                                  atomicCastTo<TYPE, val_type>(value))
        );
    }
}

template <typename TYPE>
bool original::atomicImpl<TYPE, false>::exchangeCmp(TYPE& expected, TYPE desired, memOrder) noexcept
{
    if constexpr (sizeof(TYPE) == 4) {
        val_type old = InterlockedCompareExchange(
            reinterpret_cast<volatile val_type*>(&this->data_),
            atomicCastTo<TYPE, val_type>(desired),
            atomicCastTo<TYPE, val_type>(expected));
        const bool success = old == atomicCastTo<TYPE, val_type>(expected);
        if (!success) expected = atomicCastBack<TYPE, val_type>(old);
        return success;
    } else {
        val_type old = InterlockedCompareExchange64(
            reinterpret_cast<volatile val_type*>(&this->data_),
            atomicCastTo<TYPE, val_type>(desired),
            atomicCastTo<TYPE, val_type>(expected));
        const bool success = old == atomicCastTo<TYPE, val_type>(expected);
        if (!success) expected = atomicCastBack<TYPE, val_type>(old);
        return success;
    }
}

template <typename TYPE>
original::atomicImpl<TYPE, true>::atomicImpl(TYPE value, memOrder) {
    uniqueLock lock{this->mutex_};
    this->data_.set(value);
}

template <typename TYPE>
constexpr bool original::atomicImpl<TYPE, true>::isLockFree() noexcept {
    return false;
}

template <typename TYPE>
void original::atomicImpl<TYPE, true>::store(TYPE value, memOrder) {
    uniqueLock lock{this->mutex_};
    this->data_.set(value);
}

template <typename TYPE>
TYPE original::atomicImpl<TYPE, true>::load(memOrder) const noexcept {
    uniqueLock lock{this->mutex_};
    return *this->data_;
}

template <typename TYPE>
TYPE original::atomicImpl<TYPE, true>::operator*() const noexcept
{
    return this->load();
}

template <typename TYPE>
original::atomicImpl<TYPE, true>::operator TYPE() const noexcept
{
    return this->load();
}

template <typename TYPE>
original::atomicImpl<TYPE, true>&
original::atomicImpl<TYPE, true>::operator=(TYPE value) noexcept
{
    this->store(std::move(value));
    return *this;
}

template <typename TYPE>
original::atomicImpl<TYPE, true>& original::atomicImpl<TYPE, true>::operator+=(TYPE value) noexcept
{
    uniqueLock lock{this->mutex_};
    TYPE result = *this->data_ + value;
    this->data_.set(result);
    return *this;
}

template <typename TYPE>
original::atomicImpl<TYPE, true>& original::atomicImpl<TYPE, true>::operator-=(TYPE value) noexcept
{
    uniqueLock lock{this->mutex_};
    TYPE result = *this->data_ - value;
    this->data_.set(result);
    return *this;
}

template <typename TYPE>
TYPE original::atomicImpl<TYPE, true>::exchange(const TYPE& value, memOrder) noexcept {
    uniqueLock lock{this->mutex_};
    TYPE result = *this->data_;
    this->data_.set(value);
    return result;
}

template <typename TYPE>
bool original::atomicImpl<TYPE, true>::exchangeCmp(TYPE& expected, const TYPE& desired, memOrder) noexcept {
    uniqueLock lock{this->mutex_};
    if (*this->data_ == expected) {
        this->data_.set(desired);
        return true;
    }
    expected = *this->data_;
    return false;
}

#endif

template<typename TYPE>
auto original::makeAtomic()
{
    return atomic<TYPE>{};
}

template<typename TYPE>
auto original::makeAtomic(TYPE value)
{
    return atomic<TYPE>{std::move(value)};
}
#endif
