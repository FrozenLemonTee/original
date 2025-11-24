#ifndef ORIGINAL_OPTIONAL_H
#define ORIGINAL_OPTIONAL_H
#include <utility>
#include "types.h"
#include "error.h"

/**
 * @file optional.h
 * @brief Type-safe optional value container
 * @details Provides an alternative<TYPE> class that can either contain a value of type 'TYPE'
 * or be in an empty state (represented by original::null from types.h). This implementation
 * provides:
 * - Value semantics with proper construction/destruction
 * - Safe access operations with error checking
 * - In-place construction
 * - Move and copy operations
 * - Specialization for void type
 * - Integration with std::swap
 *
 * Key features:
 * - Type-safe alternative to raw pointers for optional values
 * - No dynamic memory allocation (uses union storage)
 * - Explicit empty state handling with original::null
 * - Exception-safe operations
 * - STL-compatible interface
 */

namespace original {

    /**
     * @class alternative
     * @brief A type-safe container that may or may not contain a value
     * @tparam TYPE The type of value to store
     * @details This class provides a way to represent optional values without
     * using pointers. It can either contain a value of type TYPE or be empty.
     * The empty state is represented using original::null from types.h.
     *
     * The implementation uses a union for storage to avoid dynamic allocation
     * and ensure optimal performance. All operations provide strong exception
     * safety guarantee when TYPE operations are exception-safe.
     *
     * Example usage:
     * @code
     * alternative<int> opt;          // Empty state
     * alternative<int> opt2(42);     // Contains value 42
     * if (opt2) {                    // Check if has value
     *     int val = *opt2;           // Access the value
     * }
     *
     * // In-place construction
     * alternative<std::string> str(std::in_place, "hello", 5);
     *
     * // Safe access with error checking
     * try {
     *     auto value = *opt;  // Throws valueError if empty
     * } catch (const valueError& e) {
     *     // Handle empty case
     * }
     * @endcode
     */
    template<typename TYPE>
    class alternative {
        /**
         * @union storage
         * @brief Internal storage for either TYPE or null
         * @details Uses a union to store either the value type or null,
         * with proper construction/destruction handling. The union ensures
         * that only one member is active at any time, managed by the
         * non_null_type_ flag.
         */
        union storage {
            TYPE type_;     ///< Storage for the contained value
            null null_;     ///< Storage for empty state (from types.h)

            /**
             * @brief Default constructor (initializes to null)
             */
            storage() noexcept;

            /// Default copy constructor
            storage(const storage& other) noexcept = default;

            /// Default copy assignment
            storage& operator=(const storage& other) noexcept = default;

            /// Destructor
            ~storage();
        };

        bool non_null_type_; ///< Flag indicating whether value is present (true = TYPE, false = null)
        storage val_;       ///< The actual storage union

        /**
         * @brief Safely destroys the current contents
         * @note Properly calls destructor based on current state
         * @details If non_null_type_ is true, calls TYPE destructor,
         * otherwise calls null destructor. Ensures RAII compliance.
         */
        void destroy() noexcept;

    public:
        /**
         * @brief Constructs an empty alternative
         * @post hasValue() == false
         */
        explicit alternative();

        /**
         * @brief Constructs an alternative containing a value
         * @tparam Args Argument types for TYPE construction
         * @param args Arguments to forward to TYPE constructor
         * @post hasValue() == true
         */
        template<typename... Args>
        explicit alternative(Args&&... args);

        /**
         * @brief Copy constructor
         * @param other Alternative to copy from
         * @post hasValue() == other.hasValue()
         */
        alternative(const alternative& other);

        /**
         * @brief Copy assignment
         * @param other Alternative to copy from
         * @return Reference to this alternative
         * @post hasValue() == other.hasValue()
         */
        alternative& operator=(const alternative& other);

        /**
         * @brief Move constructor
         * @param other Alternative to move from
         * @post other is left in valid but unspecified state
         */
        alternative(alternative&& other) noexcept;

        /**
         * @brief Move assignment
         * @param other Alternative to move from
         * @return Reference to this alternative
         * @post other is left in valid but unspecified state
         */
        alternative& operator=(alternative&& other) noexcept;

        /**
         * @brief Swaps contents with another alternative
         * @param other Alternative to swap with
         * @note No-throw guarantee if TYPE's swap is noexcept
         * @details Efficiently exchanges contents without copying
         */
        void swap(alternative& other) noexcept;

        /**
         * @brief Const value access
         * @return Const reference to contained value
         * @throws valueError if empty
         */
        const TYPE& operator*() const;

        /**
         * @brief Mutable value access
         * @return Reference to contained value
         * @throws valueError if empty
         */
        TYPE& operator*();

        /**
         * @brief Const member access
         * @return Const pointer to contained value
         * @throws valueError if empty
         */
        const TYPE* operator->() const;

        /**
         * @brief Mutable member access
         * @return Pointer to contained value
         * @throws valueError if empty
         */
        TYPE* operator->();

        /**
         * @brief Gets const pointer to value
         * @return Pointer to value or nullptr if empty
         */
        const TYPE* get() const;

        /**
         * @brief Gets mutable pointer to value
         * @return Pointer to value or nullptr if empty
         */
        TYPE* get();

        /**
         * @brief Resets to empty state
         * @post hasValue() == false
         */
        void reset() noexcept;

        /**
         * @brief Constructs value in-place
         * @tparam Args Argument types for TYPE construction
         * @param args Arguments to forward to TYPE constructor
         * @post hasValue() == true
         */
        template<typename... Args>
        void emplace(Args&&... args);

        /**
         * @brief Sets value by copy
         * @param t Value to copy
         * @post hasValue() == true
         */
        void set(const TYPE& t);

        /**
         * @brief Copy assignment from value type
         * @param t Value to copy into the alternative
         * @return Reference to this alternative
         * @post hasValue() == true
         * @details Replaces the current contents (if any) with a copy of the given value.
         * If the alternative was previously empty, constructs a new value. If it already
         * contained a value, assigns the new value using TYPE's copy assignment operator.
         * Provides strong exception safety guarantee if TYPE's copy constructor/assignment
         * is exception-safe.
         *
         * Example usage:
         * @code
         * alternative<std::string> opt;
         * opt = "hello";  // Now contains "hello"
         *
         * alternative<int> opt2(42);
         * opt2 = 100;     // Replaces 42 with 100
         * @endcode
         */
        alternative& operator=(const TYPE& t);

        /**
         * @brief Checks if contains a value
         * @return true if contains value, false if empty
         */
        explicit operator bool() const;

        /**
         * @brief Checks if contains a value
         * @return true if contains value, false if empty
         */
        [[nodiscard]] bool hasValue() const;

        /**
         * @brief Destructor
         * @note Properly destroys contained value if present
         */
        ~alternative();
    };

    /**
     * @class alternative
     * @brief Specialization for void type alternative
     * @details Represents a simple boolean flag indicating presence or absence
     * of a value without storing actual data. Useful for signaling states
     * without associated data, similar to std::optional<std::monostate>.
     *
     * This specialization is more memory efficient than alternative<bool>
     * and provides a clean interface for presence/absence signaling.
     *
     * Example usage:
     * @code
     * alternative<void> flag;                    // Empty state
     * alternative<void> present(std::in_place); // Present state
     * if (present) {
     *     // Do something when value is present
     * }
     * @endcode
     */
    template<>
    class alternative<void> {
        bool has_value_ = false; ///< Flag indicating value presence

    public:
        /**
         * @brief Constructs an empty alternative<void>
         * @post hasValue() == false
         */
        explicit alternative();

        /**
         * @brief Constructs from null (empty state)
         * @param n null value indicating empty state
         * @post hasValue() == false
         */
        explicit alternative(null n);

        /**
         * @brief Constructs with value present
         * @param t std::in_place_t tag to indicate value should be present
         * @post hasValue() == true
         */
        explicit alternative(std::in_place_t t);

        /**
         * @brief Sets the alternative to have a value
         * @post hasValue() == true
         */
        void set() noexcept;

        /**
         * @brief Assignment from in_place tag to set as present
         * @param t std::in_place_t tag to indicate value should be present
         * @return Reference to this alternative
         * @post hasValue() == true
         * @details Sets the alternative<void> to the "present" state without storing
         * any actual data. This is equivalent to calling set() but allows for more
         * expressive syntax in template code.
         *
         * Example usage:
         * @code
         * alternative<void> flag;
         * flag = std::in_place;  // Now has value present
         *
         * // In template context:
         * template<typename T>
         * void setPresent(alternative<T>& opt) {
         *     opt = std::in_place;
         * }
         * @endcode
         * @note This operation is noexcept and always succeeds.
         * @see set()
         */
        alternative& operator=(std::in_place_t t) noexcept;

        /**
         * @brief Assignment from null to reset to empty state
         * @param n null value indicating empty state
         * @return Reference to this alternative
         * @post hasValue() == false
         * @details Resets the alternative<void> to the empty state. This operation
         * is equivalent to calling reset() but provides a more expressive syntax
         * when working with null values. The assignment clears the "present" state
         * and ensures the alternative is empty.
         *
         * Example usage:
         * @code
         * alternative<void> flag(std::in_place);  // Present state
         * flag = original::null;                  // Now empty
         *
         * if (!flag) {
         *     // This will execute since flag is now empty
         * }
         *
         * // In generic code:
         * template<typename T>
         * void clearAlternative(alternative<T>& opt) {
         *     opt = original::null;  // Works for both alternative<T> and alternative<void>
         * }
         * @endcode
         *
         * @note This operation is noexcept and always succeeds.
         * @see reset()
         */
        alternative& operator=(null n) noexcept;

        /**
         * @brief Resets to empty state
         * @post hasValue() == false
         */
        void reset() noexcept;

        /**
         * @brief Checks if contains a value
         * @return true if contains value, false if empty
         */
        [[nodiscard]] bool hasValue() const;

        /**
         * @brief Boolean conversion operator
         * @return true if contains value, false if empty
         */
        explicit operator bool() const;

        /// Default copy constructor
        alternative(const alternative& other) = default;

        /// Default copy assignment
        alternative& operator=(const alternative& other) = default;

        /// Move constructor
        alternative(alternative&& other) noexcept;

        /// Move assignment
        alternative& operator=(alternative&& other) noexcept;

        /**
         * @brief Swaps contents with another alternative<void>
         * @param other Alternative to swap with
         * @note No-throw guarantee
         */
        void swap(alternative& other) noexcept;

        /// Default destructor
        ~alternative() = default;
    };
}

/**
 * @namespace std
 * @brief Standard namespace extensions for original::alternative
 */
namespace std {
    /**
     * @brief Specialization of std::swap for original::alternative
     * @tparam TYPE The value type of the alternative
     * @param lhs First alternative to swap
     * @param rhs Second alternative to swap
     * @note No-throw guarantee if alternative<TYPE>::swap is noexcept
     * @details Enables ADL-friendly swapping with standard algorithms
     */
    template<typename TYPE>
    void swap(original::alternative<TYPE>& lhs, original::alternative<TYPE>& rhs) noexcept; // NOLINT
}
template<typename TYPE>
original::alternative<TYPE>::storage::storage() noexcept
{
    new(&this->null_) null;
}

template<typename TYPE>
original::alternative<TYPE>::storage::~storage() {}

template<typename TYPE>
void original::alternative<TYPE>::destroy() noexcept {
    if (this->non_null_type_){
        this->val_.type_.~TYPE();
        this->non_null_type_ = false;
    } else {
        this->val_.null_.~null();
        this->non_null_type_ = true;
    }
}

template<typename TYPE>
original::alternative<TYPE>::alternative()
    : non_null_type_(false), val_() {}

template<typename TYPE>
template<typename... Args>
original::alternative<TYPE>::alternative(Args &&... args)
    : non_null_type_(true), val_() {
    new (&this->val_.type_) TYPE{ std::forward<Args>(args)... };
}

template<typename TYPE>
original::alternative<TYPE>::alternative(const alternative& other) {
    this->non_null_type_ = other.non_null_type_;
    if (other.non_null_type_) {
        new (&val_.type_) TYPE{ other.val_.type_ };
    } else {
        new (&val_.null_) null;
    }
}

template<typename TYPE>
original::alternative<TYPE>&
original::alternative<TYPE>::operator=(const alternative& other) {
    if (this == &other)
        return *this;

    this->destroy();
    this->non_null_type_ = other.non_null_type_;
    if (other.non_null_type_) {
        new (&val_.type_) TYPE{ other.val_.type_ };
    } else {
        new (&val_.null_) null;
    }
    return *this;
}

template<typename TYPE>
original::alternative<TYPE>::alternative(alternative&& other) noexcept {
    this->non_null_type_ = other.non_null_type_;
    if (this->non_null_type_){
        new (&val_.type_) TYPE{ std::move(other.val_.type_) };
    } else{
        new (&val_.null_) null;
    }
}

template<typename TYPE>
original::alternative<TYPE>&
original::alternative<TYPE>::operator=(alternative&& other)  noexcept {
    if (this == &other)
        return *this;

    this->destroy();
    this->non_null_type_ = other.non_null_type_;
    if (this->non_null_type_){
        new (&val_.type_) TYPE{ std::move(other.val_.type_) };
    } else{
        new (&val_.null_) null;
    }
    return *this;
}

template <typename TYPE>
void original::alternative<TYPE>::swap(alternative& other) noexcept
{
    if (this == &other)
        return;

    std::swap(this->non_null_type_, other.non_null_type_);
    std::swap(this->val_, other.val_);
}

template<typename TYPE>
const TYPE&
original::alternative<TYPE>::operator*() const {
    if (!this->non_null_type_)
        throw valueError("Dereferencing a original::null value");

    return this->val_.type_;
}

template<typename TYPE>
TYPE&
original::alternative<TYPE>::operator*() {
    if (!this->non_null_type_)
        throw valueError("Dereferencing a original::null value");

    return this->val_.type_;
}

template<typename TYPE>
const TYPE*
original::alternative<TYPE>::operator->() const {
    if (!this->non_null_type_)
        throw valueError("Accessing member of a original::null value");

    return &this->val_.type_;
}

template<typename TYPE>
TYPE*
original::alternative<TYPE>::operator->() {
    if (!this->non_null_type_)
        throw valueError("Accessing member of a original::null value");

    return &this->val_.type_;
}

template<typename TYPE>
const TYPE* original::alternative<TYPE>::get() const {
    return this->non_null_type_ ? &this->val_.type_ : nullptr;
}

template<typename TYPE>
TYPE* original::alternative<TYPE>::get() {
    return this->non_null_type_ ? &this->val_.type_ : nullptr;
}

template<typename TYPE>
void original::alternative<TYPE>::reset() noexcept {
    this->destroy();
}

template<typename TYPE>
template<typename... Args>
void original::alternative<TYPE>::emplace(Args &&... args) {
    this->destroy();
    this->non_null_type_ = true;
    new (&val_.type_) TYPE{ std::forward<Args>(args)... };
}

template<typename TYPE>
void original::alternative<TYPE>::set(const TYPE &t) {
    this->destroy();
    this->non_null_type_ = true;
    new (&val_.type_) TYPE{ t };
}

template <typename TYPE>
original::alternative<TYPE>&
original::alternative<TYPE>::operator=(const TYPE& t)
{
    this->set(t);
    return *this;
}

template<typename TYPE>
original::alternative<TYPE>::operator bool() const {
    return this->non_null_type_;
}

template <typename TYPE>
bool original::alternative<TYPE>::hasValue() const
{
    return this->operator bool();
}

template<typename TYPE>
original::alternative<TYPE>::~alternative() {
    this->destroy();
}

inline original::alternative<void>::alternative() = default;

inline original::alternative<void>::alternative(null) {}

inline original::alternative<void>::alternative(std::in_place_t) : has_value_(true) {}

inline void original::alternative<void>::set() noexcept
{
    this->has_value_ = true;
}

inline original::alternative<void>&
original::alternative<void>::operator=(std::in_place_t) noexcept
{
    this->set();
    return *this;
}

inline original::alternative<void>&
original::alternative<void>::operator=(null) noexcept
{
    this->reset();
    return *this;
}

inline void original::alternative<void>::reset() noexcept
{
    this->has_value_ = false;
}

inline bool original::alternative<void>::hasValue() const
{
    return this->has_value_;
}

inline original::alternative<void>::operator bool() const
{
    return this->has_value_;
}

inline original::alternative<void>::alternative(alternative&& other) noexcept
{
    this->has_value_ = other.has_value_;
    other.has_value_ = false;
}

inline original::alternative<void>&
original::alternative<void>::operator=(alternative&& other) noexcept
{
    if (this == &other)
        return *this;

    this->has_value_ = other.has_value_;
    other.has_value_ = false;
    return *this;
}

inline void original::alternative<void>::swap(alternative& other) noexcept
{
    if (this == &other)
        return;

    std::swap(this->has_value_, other.has_value_);
}

template <typename TYPE>
void std::swap(original::alternative<TYPE>& lhs, original::alternative<TYPE>& rhs) noexcept // NOLINT
{
    lhs.swap(rhs);
}

#endif //ORIGINAL_OPTIONAL_H
