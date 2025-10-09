#ifndef TYPES_H
#define TYPES_H

#include <type_traits>
#include <concepts>
#include <iosfwd>
#include <functional>
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

    /**
     * @concept EnumClassType
     * @brief Requires type to be a scoped enumeration.
     * @tparam TYPE The type to check.
     */
    template<typename TYPE>
    concept EnumClassType = std::is_enum_v<TYPE> && !std::is_convertible_v<TYPE, std::underlying_type_t<TYPE>>;

    // ==================== Comparison Concepts ====================

    /**
     * @concept EqualityComparable
     * @brief Requires type to support equality comparison operators.
     * @tparam T The type to check.
     * @details Enforces that the type provides equality operators (==, !=).
     */
    template <typename T>
    concept EqualityComparable = requires(const T& a, const T& b) {
        { a == b } -> std::convertible_to<bool>;
        { a != b } -> std::convertible_to<bool>;
    };

    /**
     * @concept WeaklyOrdered
     * @brief Requires type to support relational comparison operators.
     * @tparam T The type to check.
     * @details Enforces that the type provides relational operators (<, <=, >, >=).
     */
    template <typename T>
    concept WeaklyOrdered = requires(const T& a, const T& b) {
        { a < b } -> std::convertible_to<bool>;
        { a <= b } -> std::convertible_to<bool>;
        { a > b } -> std::convertible_to<bool>;
        { a >= b } -> std::convertible_to<bool>;
    };

    /**
     * @concept PartiallyComparable
     * @brief Requires type to support either equality or relational comparisons.
     * @tparam T The type to check.
     * @details Useful for types that may only support partial ordering.
     */
    template <typename T>
    concept PartiallyComparable = EqualityComparable<T> || WeaklyOrdered<T>;

    /**
     * @concept TotallyComparable
     * @brief Requires type to support all comparison operators.
     * @tparam T The type to check.
     * @details The strictest comparison concept.
     */
    template <typename T>
    concept TotallyComparable = EqualityComparable<T> && WeaklyOrdered<T>;

    /**
     * @concept ThreeWayComparable
     * @brief Requires type to support three-way comparison (spaceship operator).
     * @tparam T The type to check.
     * @details For modern C++20 types that implement <=> operator.
     */
    template <typename T>
    concept ThreeWayComparable = requires(const T& a, const T& b) {
        { a <=> b } -> std::convertible_to<std::partial_ordering>;
    };

    /**
     * @concept StronglyOrdered
     * @brief Requires type to support strong ordering via three-way comparison.
     * @tparam T The type to check.
     * @details For types that provide strong ordering guarantees.
     */
    template <typename T>
    concept StronglyOrdered = requires(const T& a, const T& b) {
        { a <=> b } -> std::convertible_to<std::strong_ordering>;
    };

    // Alias of TotallyComparable
    template <typename T>
    concept Comparable = TotallyComparable<T>;

    /**
     * @concept CmpTraits
     * @brief Requires type to implement the comparable interface with compareTo method
     * @tparam T The type to check
     * @details This concept enforces that a type provides a Java-style comparison interface
     *          through a `compareTo` method that returns an integer indicating ordering.
     *          Types satisfying this concept can be used with the `comparable` base class
     *          and benefit from automatically generated comparison operators.
     *
     * Requirements:
     * - Must have method: `integer compareTo(const T& other) const`
     * - Return value semantics: negative (less), zero (equal), positive (greater)
     *
     * @code{.cpp}
     * // Example of a type satisfying CmpTraits
     * class MyComparable {
     * public:
     *     integer compareTo(const MyComparable& other) const {
     *         return this->value - other.value;
     *     }
     *     // Automatically gets ==, !=, <, >, <=, >= operators
     * private:
     *     int value;
     * };
     *
     * static_assert(CmpTraits<MyComparable>);  // Succeeds
     * @endcode
     *
     * @see comparable
     */
    template <typename T>
    concept CmpTraits = requires(const T& a, const T& b) {
        { a.compareTo(b) } -> std::same_as<integer>;
    };

    // ==================== Stream Concepts ====================

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

    /**
     * @concept InputStreamable
     * @brief Requires type to support input stream extraction.
     * @tparam TYPE The type to check.
     * @details Ensures the type can be read from std::istream using the >> operator.
     */
    template <typename TYPE>
    concept InputStreamable = requires(std::istream& is, TYPE& t) {
        { is >> t } -> std::same_as<std::istream&>;
    };

    /**
     * @concept Streamable
     * @brief Requires type to support both input and output stream operations.
     * @tparam TYPE The type to check.
     */
    template <typename TYPE>
    concept Streamable = Printable<TYPE> && InputStreamable<TYPE>;

    // ==================== Hash Concepts ====================

    /**
     * @concept Hashable
     * @brief Concept checking for types that can be hashed.
     * @tparam T The type to be checked
     * @details The type must satisfy the concept in this case:
     * By having std::hash<T> specialization
     *
     * @code{.cpp}
     * namespace std {
     *     template<> struct hash<MyHashable1> {
     *         size_t operator()(const MyHashable1& obj) const {
     *             return std::hash<int>{}(obj.value);
     *         }
     *     };
     * }
     * @endcode
     */
    template <typename T>
    concept Hashable =
        requires(const T& obj) {
            { std::hash<T>{}(obj) } -> std::convertible_to<std::size_t>;
        };

    /**
     * @concept Equatable
     * @brief Requires type to support equality comparison for hashing.
     * @tparam T The type to check.
     * @details Complementary concept for Hashable to ensure proper equality.
     */
    template <typename T>
    concept Equatable =
        requires(const T& a, const T& b) {
            { a == b } -> std::convertible_to<bool>;
        } ||
        requires(const T& a, const T& b) {
            { a.equals(b) } -> std::convertible_to<bool>;
        };

    /**
     * @concept HashTraits
     * @brief Requires type to implement the hashable interface with toHash method
     * @tparam T The type to check
     * @details This concept enforces that a type provides a custom hashing interface
     *          through a `toHash` method and supports equality comparison. Types
     *          satisfying this concept can be used with the `hashable` base class
     *          and benefit from automatic STL hash integration.
     *
     * Requirements:
     * - Must have method: `u_integer toHash() const noexcept`
     * - Must support equality comparison (either `operator==` or `equals` method)
     * - Should provide consistent hashing (equal objects have equal hash codes)
     *
     * @code{.cpp}
     * // Example of a type satisfying HashTraits
     * class MyHashable {
     * public:
     *     u_integer toHash() const noexcept {
     *         return std::hash<int>{}(value);
     *     }
     *     bool operator==(const MyHashable& other) const {
     *         return value == other.value;
     *     }
     * private:
     *     int value;
     * };
     *
     * static_assert(HashTraits<MyHashable>);  // Succeeds
     * @endcode
     *
     * Hash Consistency Requirement:
     * - If `a == b`, then `a.toHash() == b.toHash()`
     * - The converse is not required (hash collisions are allowed)
     *
     * @see hashable
     * @see hash
     */
    template <typename T>
    concept HashTraits =
        requires(const T& t) {
        { t.toHash() } -> std::same_as<u_integer>;
    } && Equatable<T>;

    // ==================== Callback Concepts ====================

    /**
     * @concept Callable
     * @brief Basic concept for any callable type.
     * @tparam F The callable type to check.
     * @tparam Args The argument types for invocation.
     */
    template <typename F, typename... Args>
    concept Callable = requires(F&& f, Args&&... args) {
        std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
    };

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
        { callback(std::forward<Args>(args)...) } -> std::convertible_to<ReturnType>;
    };

    /**
     * @concept Predicate
     * @brief Constraint for boolean predicate callbacks.
     * @tparam Callback Predicate function type
     * @tparam Args Input types for predicate
     * @details Requires a callback that returns a boolean value.
     */
    template <typename Callback, typename... Args>
    concept Predicate = CallbackOf<Callback, bool, Args...>;

    /**
     * @concept Compare
     * @brief Combines Comparable and CallbackOf for comparison callbacks.
     * @tparam Callback Comparison function type
     * @tparam TYPE Type being compared
     * @details Requires a callback that takes two TYPE references and returns bool.
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
    concept Compare = Predicate<Callback, const TYPE&, const TYPE&>;

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
    concept Condition = Predicate<Callback, const TYPE&>;

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

    /**
     * @concept Transformer
     * @brief Constraint for transformation operations.
     * @tparam Callback Transformation function type
     * @tparam Input Input type
     * @tparam Output Output type
     * @details Requires a callback that transforms Input to Output.
     */
    template <typename Callback, typename Input, typename Output>
    concept Transformer = CallbackOf<Callback, Output, Input>;

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

    /**
     * @concept ConvertibleTo
     * @brief Checks if type is convertible to another type.
     * @tparam From Source type
     * @tparam To Target type
     */
    template <typename From, typename To>
    concept ConvertibleTo = std::is_convertible_v<From, To>;

    /**
     * @concept SameAs
     * @brief Checks if two types are exactly the same.
     * @tparam T First type
     * @tparam U Second type
     */
    template <typename T, typename U>
    concept SameAs = std::is_same_v<T, U>;

    // ==================== Container Concepts ====================

    /**
     * @concept Container
     * @brief Basic container concept.
     * @tparam C Container type
     * @details Requires basic container interface: begin(), end(), size().
     */
    template <typename C>
    concept Container = requires(C& c) {
        { c.begin() } -> std::input_or_output_iterator;
        { c.end() } -> std::input_or_output_iterator;
        { c.size() } -> std::convertible_to<std::size_t>;
    };

    /**
     * @concept SequenceContainer
     * @brief Sequence container concept.
     * @tparam C Container type
     * @details Requires sequence container operations.
     */
    template <typename C>
    concept SequenceContainer = Container<C> && requires(C& c, typename C::value_type v) {
        c.push_back(v);
        c.pop_back();
    };

    template<bool MATCH, typename MATCH_TYPE, typename OTHER_TYPE>
    class some;

    template<typename MATCH_TYPE, typename OTHER_TYPE>
    class some<true, MATCH_TYPE, OTHER_TYPE>
    {
    public:
        using type = MATCH_TYPE;
    };

    template<typename MATCH_TYPE, typename OTHER_TYPE>
    class some<false, MATCH_TYPE, OTHER_TYPE>
    {
    public:
        using type = OTHER_TYPE;
    };

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