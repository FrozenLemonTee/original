#ifndef ORIGINAL_ATOMIC_H
#define ORIGINAL_ATOMIC_H

#include <type_traits>
#include <cstring>
#include "optional.h"
#include "config.h"
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
        alignas(TYPE) byte data_[sizeof(TYPE)];  ///< Properly aligned storage

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
        void operator=(TYPE value) noexcept;

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
        void operator=(TYPE value) noexcept;

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
#endif

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
void original::atomicImpl<TYPE, false>::operator=(TYPE value) noexcept
{
    this->store(std::move(value));
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
void original::atomicImpl<TYPE, true>::operator=(TYPE value) noexcept
{
    this->store(std::move(value));
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
#endif
