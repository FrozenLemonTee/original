#ifndef TYPES_H
#define TYPES_H

#include <type_traits>
#include <concepts>
#include <iosfwd>
#include "config.h"

/**
 * @file types.h
 * @brief Core type system foundations and concept definitions.
 * @details This header defines fundamental type traits and concepts used for
 *          template metaprogramming and interface constraints throughout the
 *          library. These concepts enforce compile-time interface requirements
 *          and enable type-safe template programming.
 *
 * @namespace original
 * @brief The main namespace containing all core type system components
 */

namespace original {

    // ==================== Fundamental Types ====================

    /**
     * @class none
     * @brief A placeholder type representing the absence of a value.
     * @details This class provides consteval operations and can be converted to bool,
     *          always returning false. Useful for template metaprogramming and
     *          representing empty states.
     */
    class none {
    public:
        /// @brief Default constructor (consteval)
        consteval explicit none() = default;

        /// @brief Default destructor (constexpr)
        constexpr ~none() = default;

        /**
         * @brief Bool conversion operator
         * @return Always returns false
         */
        consteval explicit operator bool() const;

        /**
         * @brief Logical NOT operator
         * @return Always returns true
         */
        consteval bool operator!() const;
    };

    // ==================== Core Concepts ====================

    /**
     * @concept NotNull
     * @brief Ensures the parameter pack is not empty.
     * @tparam ARGS Variadic template parameter pack to check
     * @details This concept checks that at least one type is provided in the
     *          parameter pack. Useful for enforcing non-empty template parameter lists.
     *
     * @code{.cpp}
     * static_assert(NotNull<int>);         // Succeeds
     * static_assert(!NotNull<>);           // Fails
     * @endcode
     */
    template<typename... ARGS>
    concept NotNull = sizeof...(ARGS) > 0;

    /**
     * @concept EnumType
     * @brief Requires type to be an enumeration.
     * @tparam TYPE The type to check.
     */
    template<typename TYPE>
    concept EnumType = std::is_enum_v<TYPE>;

    template<typename... Ts>
    concept AllCopyConstructible = (... && std::is_copy_constructible_v<Ts>);

    template<typename... Ts>
    concept AllCopyAssignable = (... && std::is_copy_assignable_v<Ts>);

    /**
     * @concept Comparable
     * @brief Requires type to support all comparison operators.
     * @tparam TYPE The type to check.
     * @details Enforces that the type provides all standard comparison operators
     *          (==, !=, <, <=, >, >=) returning boolean results.
     *
     * @code{.cpp}
     * static_assert(Comparable<int>);  // Succeeds
     * struct NoCompare {};
     * static_assert(!Comparable<NoCompare>); // Fails
     * @endcode
     */
    template <typename TYPE>
    concept Comparable = requires(const TYPE& t1, const TYPE& t2) {
        { t1 == t2 } -> std::same_as<bool>;
        { t1 != t2 } -> std::same_as<bool>;
        { t1 < t2 } -> std::same_as<bool>;
        { t1 <= t2 } -> std::same_as<bool>;
        { t1 > t2 } -> std::same_as<bool>;
        { t1 >= t2 } -> std::same_as<bool>;
    };

    /**
     * @concept Printable
     * @brief Requires type to support output stream insertion.
     * @tparam TYPE The type to check.
     * @details Ensures the type can be written to std::ostream using the << operator.
     *
     * @code{.cpp}
     * static_assert(Printable<int>);  // Succeeds
     * struct NoOutput {};
     * static_assert(!Printable<NoOutput>); // Fails
     * @endcode
     */
    template <typename TYPE>
    concept Printable = requires(std::ostream& os, const TYPE& t) {
        { os << t } -> std::same_as<std::ostream&>;
    };

    // ==================== Callback Concepts ====================

    /**
     * @concept CallbackOf
     * @brief Validates callback signature compatibility.
     * @tparam Callback Callable type to check
     * @tparam ReturnType Expected return type
     * @tparam Args Expected argument types
     * @details Ensures the callback can be invoked with specified arguments
     *          and returns the specified type. Supports perfect forwarding of arguments.
     *
     * @code{.cpp}
     * auto lambda = [](int x) { return x * 2; };
     * static_assert(CallbackOf<decltype(lambda), int, int>);    // Succeeds
     * static_assert(CallbackOf<decltype(lambda), double, int>); // Fails (return type mismatch)
     * @endcode
     */
    template <typename Callback, typename ReturnType, typename... Args>
    concept CallbackOf = requires(Callback callback, Args&&... args) {
        { callback(std::forward<Args>(args)...) } -> std::same_as<ReturnType>;
    };

    /**
     * @concept Compare
     * @brief Combines Comparable and CallbackOf for comparison callbacks.
     * @tparam Callback Comparison function type
     * @tparam TYPE Type being compared
     * @details Requires a callback that takes two TYPE references and returns bool,
     *          while TYPE itself must satisfy Comparable.
     *
     * @code{.cpp}
     * // Validates custom comparator
     * auto int_comparator = [](const int& a, const int& b) { return a < b; };
     * static_assert(Compare<decltype(int_comparator), int>);  // Passes
     *
     * // Invalid example (wrong return type)
     * auto bad_comp = [](const int& a, const int& b) { return a - b; };
     * static_assert(!Compare<decltype(bad_comp), int>);  // Fails: returns int instead of bool
     * @endcode
     */
    template <typename Callback, typename TYPE>
    concept Compare = Comparable<TYPE> &&
                     CallbackOf<Callback, bool, const TYPE&, const TYPE&>;

    /**
     * @concept Condition
     * @brief Constraint for predicate callbacks.
     * @tparam Callback Predicate function type
     * @tparam TYPE Input type for predicate
     * @details Requires a callback that takes a TYPE reference and returns bool.
     *          Useful for filtering and conditional operations.
     *
     * @code{.cpp}
     * // Validates positive number checker
     * auto is_positive = [](const auto& x) { return x > 0; };
     * static_assert(Condition<decltype(is_positive), double>);  // Passes
     *
     * // Invalid example (parameter type mismatch)
     * auto string_check = [](std::string s) { return !s.empty(); };
     * static_assert(!Condition<decltype(string_check), const std::string&>);  // Fails
     * @endcode
     */
    template <typename Callback, typename TYPE>
    concept Condition = CallbackOf<Callback, bool, const TYPE&>;

    /**
     * @concept Operation
     * @brief Constraint for mutating operations.
     * @tparam Callback Mutation function type
     * @tparam TYPE Type being modified
     * @details Requires a callback that takes a mutable TYPE reference and returns void.
     *          Used for in-place modifications.
     *
     * @code{.cpp}
     * // Validates increment operation
     * auto increment = [](auto& x) { x += 1; };
     * static_assert(Operation<decltype(increment), int>);  // Passes
     *
     * // Invalid example (wrong return type)
     * auto bad_op = [](int& x) { return x++; };
     * static_assert(!Operation<decltype(bad_op), int>);  // Fails: returns int instead of void
     * @endcode
     */
    template <typename Callback, typename TYPE>
    concept Operation = CallbackOf<Callback, void, TYPE&>;

    // ==================== Type Relationship Concepts ====================

    /**
     * @concept SuperOf
     * @brief Checks inheritance or type equality.
     * @tparam Base Base class or type
     * @tparam Derive Derived class or same type
     * @details Combines std::is_base_of_v and std::is_same_v to validate
     *          "is-a" relationships. More permissive than ExtendsOf.
     *
     * @code{.cpp}
     * struct Base {};
     * struct Derived : Base {};
     *
     * static_assert(SuperOf<Base, Derived>);    // Passes: inheritance
     * static_assert(SuperOf<int, int>);         // Passes: same type
     * static_assert(!SuperOf<Derived, Base>);   // Fails: reverse inheritance
     * @endcode
     */
    template <typename Base, typename Derive>
    concept SuperOf = std::is_base_of_v<Base, Derive> || std::is_same_v<Base, Derive>;

    /**
     * @concept ExtendsOf
     * @brief Checks derivation or type identity using std::derived_from.
     * @tparam Base Base class or same type
     * @tparam Derive Derived class or same type
     * @details Uses std::derived_from for explicit inheritance checking,
     *          which is stricter than is_base_of_v in some cases.
     *
     * @code{.cpp}
     * struct Animal {};
     * struct Cat : Animal {};
     *
     * static_assert(ExtendsOf<Animal, Cat>);    // Passes: valid inheritance
     * static_assert(ExtendsOf<int, int>);       // Passes: same type
     * static_assert(!ExtendsOf<Cat, Animal>);   // Fails: invalid inheritance
     * @endcode
     */
    template <typename Base, typename Derive>
    concept ExtendsOf = std::derived_from<Derive, Base> || std::is_same_v<Base, Derive>;

    // ==================== Compile-time Index Sequences ====================

    /**
     * @class indexSequence
     * @brief Compile-time sequence of unsigned integers.
     * @tparam INTS Sequence of unsigned integer values
     * @details Used for template metaprogramming and compile-time index operations.
     */
    template <u_integer... INTS>
    class indexSequence {
        static constexpr u_integer SIZE = sizeof...(INTS);
    public:
        /**
         * @brief Gets the size of the sequence
         * @return Number of elements in the sequence
         */
        static consteval u_integer size() noexcept;
    };

    /**
     * @class makeIndexSequence
     * @brief Utility for generating index sequences.
     * @details Contains implementation details for creating index sequences.
     */
    class makeIndexSequence {
        template <u_integer NUM, u_integer... INTS>
        class indexSequenceImpl;

        template <u_integer... INTS>
        class indexSequenceImpl<0, INTS...> {
        public:
            using type = indexSequence<INTS...>;
        };

        template <u_integer NUM, u_integer... INTS>
        class indexSequenceImpl : public indexSequenceImpl<NUM - 1, NUM - 1, INTS...> {
        public:
            using type = indexSequenceImpl<NUM - 1, NUM - 1, INTS...>::type;
        };
    public:
        template <u_integer NUM>
        friend consteval auto makeSequence() noexcept;
    };

    /**
     * @brief Creates an index sequence of given length
     * @tparam NUM Length of the sequence to create
     * @return indexSequence instance with values 0..NUM-1
     */
    template <u_integer NUM>
    consteval auto makeSequence() noexcept; // NOLINT: Forward declaration for makeReverseSequence

    /**
     * @brief Implementation detail for reversing an index sequence
     * @tparam Indices Sequence indices to reverse
     * @param seq The index sequence to reverse
     * @return Reversed index sequence
     */
    template<u_integer... Indices>
    consteval auto reverseIndexSequenceImpl(indexSequence<Indices...> seq);

    /**
     * @brief Creates a reversed index sequence
     * @tparam N Length of the sequence to create
     * @return indexSequence instance with values N-1..0
     */
    template<u_integer N>
    using makeReverseSequence = decltype(
        reverseIndexSequenceImpl(makeSequence<N>())
    );

    // ==================== Function Traits ====================

    /**
     * @struct functionTraits
     * @brief Extracts function signature information from callable types.
     * @tparam Callback The callable type to analyze
     * @details Provides ReturnType and Signature type aliases for:
     *          - Function pointers
     *          - Function types
     *          - Lambda expressions
     *          - Member function pointers
     */
    template <typename Callback>
    struct functionTraits;

    /// @brief Specialization for function pointers
    template <typename R, typename... Args>
    struct functionTraits<R(*)(Args...)> {
        using ReturnType = R;
        using Signature = R(Args...);
    };

    /// @brief Specialization for function types
    template <typename R, typename... Args>
    struct functionTraits<R(Args...)> {
        using ReturnType = R;
        using Signature = R(Args...);
    };

    /// @brief Primary template for general callable types
    template <typename C>
    struct functionTraits {
    private:
        using CallType = functionTraits<decltype(&C::operator())>;
    public:
        using ReturnType = CallType::ReturnType;
        using Signature  = CallType::Signature;
    };

    /// @brief Specialization for const member function pointers
    template <typename C, typename R, typename... Args>
    struct functionTraits<R(C::*)(Args...) const> {
        using ReturnType = R;
        using Signature  = R(Args...);
    };

    /// @brief Specialization for non-const member function pointers
    template <typename C, typename R, typename... Args>
    struct functionTraits<R(C::*)(Args...)> {
        using ReturnType = R;
        using Signature  = R(Args...);
    };

} // namespace original

consteval original::none::operator bool() const {
    return false;
}

consteval bool original::none::operator!() const {
    return true;
}

template<original::u_integer... INTS>
consteval original::u_integer original::indexSequence<INTS...>::size() noexcept {
    return SIZE;
}

template<original::u_integer NUM>
consteval auto original::makeSequence() noexcept {
    using sequence = makeIndexSequence::indexSequenceImpl<NUM>::type;
    return sequence{};
}

template <original::u_integer... Indices>
consteval auto original::reverseIndexSequenceImpl(indexSequence<Indices...>)
{
    return indexSequence<sizeof...(Indices) - 1 - Indices...>{};
}

#endif // TYPES_H