#ifndef TUPLE_H
#define TUPLE_H

#include "printable.h"
#include "comparable.h"
#include "couple.h"
#include "types.h"

/**
 * @file tuple.h
 * @brief Heterogeneous tuple container implementation
 *
 * This file implements a type-safe, compile-time fixed-size tuple container with:
 * - Element access by index
 * - Element modification
 * - Equality comparison
 * - Formatted output
 * - Move semantics
 * - Structured binding support
 * - Tuple concatenation and slicing
 * - Compile-time bounds checking
 * - Type conversion validation
 *
 * @note Structured binding is enabled via `std::tuple_size` and `std::tuple_element` specializations.
 *       Value type deduction is used to ensure compatibility with `const auto& [a, b, c] = myTuple` syntax.
 * @note All index operations are compile-time checked for bounds safety.
 * @note Type conversion validation ensures type safety during element assignment.
 *
 * @example
 * original::tuple<int, std::string> t(10, "hi");
 * int x = t.get<0>(); // access
 * t.set<1>(std::string("hello")); // modify
 *
 * const auto& [a, b] = t; // structured binding
 */

namespace original {

    /**
     * @class tuple
     * @tparam TYPES Variadic template parameter list of element types
     * @brief Container for multiple heterogeneous elements
     * @extends printable
     * @extends comparable
     * @details Stores a sequence of elements with type safety. Provides:
     * - Compile-time fixed size container
     * - Element access via index (compile-time checked)
     * - Element modification via index
     * - Deep copy/move operations
     * - Lexicographical comparison
     * - String serialization through printable interface
     * - Comparable interface implementation
     * - Structured binding support via std::tuple_size and std::tuple_element specializations
     * - Tuple concatenation and slicing operations
     */
    template<typename... TYPES>
    class tuple final : public printable, public comparable<tuple<TYPES...>> {
        static constexpr u_integer SIZE = sizeof...(TYPES);

        /**
         * @class tupleImpl
         * @brief Recursive tuple implementation helper
         * @tparam I Current element index
         * @tparam TS Element type sequence
         *
         * This template implements the recursive element storage structure:
         * - Stores current element at index I
         * - Recursively contains next element(s)
         * - Provides indexed element access via template recursion
         */
        template<u_integer I, typename... TS>
        class tupleImpl;

        /**
         * @brief Specialization for single element (base case)
         * @tparam I Current element index
         * @tparam T Element type
         *
         * @details This is the base case of the recursive tuple implementation.
         *          Stores a single element and provides the termination condition
         *          for recursive template operations.
         */
        template<u_integer I, typename T>
        class tupleImpl<I, T> : public printable, public comparable<tupleImpl<I, T>> {
            T cur_elem;  ///< Current element storage

        public:
            explicit tupleImpl() = default;
            explicit tupleImpl(T&& cur);
            tupleImpl(tupleImpl&& other) noexcept;
            tupleImpl(const tupleImpl& other);
            tupleImpl& operator=(tupleImpl&& other) noexcept;
            void swap(tupleImpl& other) noexcept;

            template<u_integer I_DIFF>
            const auto& get() const;

            template<u_integer I_DIFF>
            auto& get();

            template<u_integer I_DIFF, typename E>
            void set(const E& e);

            integer compareTo(const tupleImpl& other) const override;
            std::string toString(bool enter) const override;

            friend class tuple;
        };

        /**
         * @brief Specialization for two elements
         * @tparam I Current element index
         * @tparam T First element type
         * @tparam TS Second element type
         *
         * @details Stores two elements in a recursive structure. The current element
         *          is stored directly, while the remaining elements are stored in
         *          a nested tupleImpl structure.
         */
        template<u_integer I, typename T, typename TS>
        class tupleImpl<I, T, TS> : public printable, public comparable<tupleImpl<I, T, TS>> {
            T cur_elem;                  ///< Current element storage
            tupleImpl<I + 1, TS> next;   ///< Recursive next element storage

        public:
            tupleImpl() = default;
            explicit tupleImpl(T&& cur, TS&& next_elems);
            tupleImpl(tupleImpl&& other) noexcept;
            tupleImpl(const tupleImpl& other);
            tupleImpl& operator=(tupleImpl&& other) noexcept;
            void swap(tupleImpl& other) noexcept;

            template<u_integer I_DIFF>
            const auto& get() const;

            template<u_integer I_DIFF>
            auto& get();

            template<u_integer I_DIFF, typename E>
            void set(const E& e);

            integer compareTo(const tupleImpl& other) const override;
            std::string toString(bool enter) const override;

            friend class tuple;
        };

        /**
         * @brief General case for three or more elements
         * @tparam I Current element index
         * @tparam T First element type
         * @tparam TS Remaining element types
         *
         * @details Recursively stores multiple elements. Each level stores one element
         *          and contains the next level for remaining elements. This creates
         *          a compile-time recursive data structure.
         */
        template<u_integer I, typename T, typename... TS>
        class tupleImpl<I, T, TS...> : public printable, public comparable<tupleImpl<I, T, TS...>> {
            T cur_elem;                     ///< Current element storage
            tupleImpl<I + 1, TS...> next;   ///< Recursive next element storage

        public:
            explicit tupleImpl(T&& cur, TS&&... next_elems);
            explicit tupleImpl(const T& cur = T{}, const tupleImpl<I + 1, TS...>& nt = tupleImpl<I + 1, TS...>{});
            tupleImpl(tupleImpl&& other) noexcept;
            tupleImpl(const tupleImpl& other);
            tupleImpl& operator=(tupleImpl&& other) noexcept;
            void swap(tupleImpl& other) noexcept;

            template<u_integer I_DIFF>
            const auto& get() const;

            template<u_integer I_DIFF>
            auto& get();

            template<u_integer I_DIFF, typename E>
            void set(const E& e);

            integer compareTo(const tupleImpl& other) const override;
            std::string toString(bool enter) const override;

            friend class tuple;
        };

        tupleImpl<0, TYPES...> elems;  ///< Recursive element storage

        /**
         * @brief Internal helper to slice a tuple from a specific index with given indices
         * @tparam IDX_S Compile-time index sequence (used to construct elements)
         * @tparam BEGIN_IDX The beginning index of the slice
         * @param indexes Compile-time integer sequence representing indices to extract
         * @param begin Starting index marker
         * @return A new tuple containing elements from index BEGIN_IDX to BEGIN_IDX + N_ELEMS - 1
         *
         * @note This is an internal implementation detail used by the public slice() method.
         */
        template<u_integer... IDX_S, u_integer BEGIN_IDX>
        auto _slice(std::integer_sequence<u_integer, IDX_S...> indexes,
                    std::integral_constant<u_integer, BEGIN_IDX> begin) const;

        /**
         * @brief Internal helper to concatenate two tuples
         * @tparam O_TYPES Element types of the other tuple
         * @tparam T_SIZE Index sequence for this tuple
         * @tparam O_SIZE Index sequence for the other tuple
         * @param other The other tuple to concatenate
         * @param ts Compile-time index sequence for current tuple
         * @param os Compile-time index sequence for other tuple
         * @return A new tuple containing elements of both tuples
         *
         * @note This is an internal implementation detail used by the operator+ method.
         */
        template<typename... O_TYPES, u_integer... T_SIZE, u_integer... O_SIZE>
        tuple<TYPES..., O_TYPES...> _concat(const tuple<O_TYPES...>& other,
                                            indexSequence<T_SIZE...> ts,
                                            indexSequence<O_SIZE...> os) const;

        template<typename... O_TYPES, u_integer... T_SIZE, u_integer... O_SIZE>
        tuple<TYPES..., O_TYPES...> _concat(tuple<O_TYPES...>&& other,
                                            indexSequence<T_SIZE...> ts,
                                            indexSequence<O_SIZE...> os);

    public:
        tuple() = default;

        /**
         * @brief Constructs a tuple with the given elements
         * @param e Elements to store in the tuple
         *
         * @note Uses perfect forwarding to preserve value categories.
         */
        explicit tuple(TYPES&&... e);

        /**
         * @brief Copy constructor - creates a deep copy of another tuple
         * @param other Tuple to copy from
         *
         * @note Performs element-wise copy construction of all stored elements.
         *       Each element's copy constructor is invoked.
         */
        tuple(const tuple& other);

        /**
         * @brief Copy assignment operator - assigns contents from another tuple
         * @param other Tuple to assign from
         * @return Reference to this tuple
         *
         * @note Performs element-wise copy assignment of all stored elements.
         *       Self-assignment check is performed for safety.
         *       Each element's copy assignment operator is invoked.
         */
        tuple& operator=(const tuple& other);

        /**
         * @brief Move constructor - transfers ownership from another tuple
         * @param other Tuple to move from
         *
         * @note Performs element-wise move construction of all stored elements.
         *       The source tuple is left in a valid but unspecified state.
         *       Each element's move constructor is invoked.
         * @note noexcept qualified for compatibility with STL containers.
         */
        tuple(tuple&& other) noexcept;

        /**
         * @brief Move assignment operator - transfers ownership from another tuple
         * @param other Tuple to move from
         * @return Reference to this tuple
         *
         * @note Performs element-wise move assignment of all stored elements.
         *       Self-assignment check is performed for safety.
         *       The source tuple is left in a valid but unspecified state.
         *       Each element's move assignment operator is invoked.
         * @note noexcept qualified for compatibility with STL containers.
         */
        tuple& operator=(tuple&& other) noexcept;

        /**
         * @brief Swaps contents with another tuple
         * @param other Tuple to swap with
         *
         * @note Performs element-wise swap of all stored elements.
         *       Self-swap check is performed (no-op if same object).
         *       Each element's swap operation or move semantics are used.
         * @note noexcept qualified for compatibility with STL algorithms.
         * @note More efficient than copy-and-swap for large tuples.
         */
        void swap(tuple& other) noexcept;

        /**
         * @brief Gets the number of elements in the tuple
         * @return Compile-time constant representing tuple size
         *
         * @note consteval ensures this is evaluated at compile-time.
         * @note The size is determined from the template parameter pack.
         * @example
         * constexpr auto s = tuple<int, string, double>::size(); // s == 3
         */
        static consteval u_integer size() noexcept;

        /**
         * @brief Gets the element at the specified index (const version)
         * @tparam IDX Index of the element to retrieve
         * @return Const reference to the element at index IDX
         *
         * @note Compile-time bounds checking is performed.
         */
        template<u_integer IDX>
        const auto& get() const;

        /**
         * @brief Gets the element at the specified index (non-const version)
         * @tparam IDX Index of the element to retrieve
         * @return Reference to the element at index IDX
         *
         * @note Compile-time bounds checking is performed.
         */
        template<u_integer IDX>
        auto& get();

        /**
         * @brief Sets the element at the specified index
         * @tparam IDX Index of the element to set
         * @tparam E Type of the value to assign (must be convertible to target type)
         * @param e Value to assign
         * @return Reference to this tuple for chaining
         *
         * @note Compile-time type conversion validation is performed.
         */
        template<u_integer IDX, typename E>
        tuple& set(const E& e);

        /**
         * @brief Extracts a sub-tuple from this tuple
         * @tparam BEGIN_IDX Starting index of the slice
         * @tparam N_ELEMS Number of elements to extract
         * @return A new tuple containing the specified slice of elements
         *
         * @note Compile-time bounds checking ensures the slice is valid.
         */
        template<u_integer BEGIN_IDX, u_integer N_ELEMS>
        auto slice() const;

        /**
         * @brief Compares this tuple with another tuple lexicographically
         * @param other Tuple to compare with
         * @return Negative integer if this < other, zero if equal, positive if this > other
         *
         * @note Performs element-wise comparison in order from first to last element.
         *       Comparison stops at the first unequal element.
         *       Only comparable elements (satisfying Comparable concept) participate in comparison.
         *       If all comparable elements are equal, tuples are considered equal.
         * @example
         * tuple<int, string> t1(1, "a"), t2(1, "b");
         * t1.compareTo(t2); // returns negative (1 == 1, but "a" < "b")
         */
        integer compareTo(const tuple& other) const override;

        /**
         * @brief Converts the tuple to a human-readable string representation
         * @param enter If true, includes newlines for formatting (currently unused)
         * @return String representation of the tuple in format "tuple(elem1, elem2, ...)"
         *
         * @note Delegates to individual element's toString/formatString methods.
         *       The output format is consistent and parseable.
         *       Example: tuple(1, "hello", 3.14)
         * @note The enter parameter is provided for interface consistency but may be used
         *       in future versions for pretty-printing with indentation.
         */
        std::string toString(bool enter) const override;

        /**
         * @brief Returns the class name identifier for this tuple type
         * @return String "tuple" representing the class name
         *
         * @note Used for runtime type identification and debugging.
         *       Consistent naming allows for predictable serialization formats.
         *       All tuple specializations return the same class name.
         * @see toString() Uses this method to prefix the string representation.
         */
        std::string className() const override;

        /**
         * @brief Concatenates this tuple with another tuple
         * @tparam O_TYPES Element types of the other tuple
         * @param other Tuple to concatenate with this one
         * @return A new tuple containing all elements from both tuples
         *
         * @note The resulting tuple has type tuple<TYPES..., O_TYPES...>
         */
        template<typename... O_TYPES>
        tuple<TYPES..., O_TYPES...> operator+(const tuple<O_TYPES...>& other) const;

        template<typename... O_TYPES>
        tuple<TYPES..., O_TYPES...> operator+(tuple<O_TYPES...>&& other);

        std::tuple<TYPES...> toStdTuple() const &;

        std::tuple<TYPES...> toStdTuple() &&;

        ~tuple() override = default;

        template<typename F_TYPE, typename S_TYPE>
        friend tuple<F_TYPE, S_TYPE> makeTuple(const couple<F_TYPE, S_TYPE>& cp);

        template<typename F_TYPE, typename S_TYPE>
        friend tuple<F_TYPE, S_TYPE> makeTuple(couple<F_TYPE, S_TYPE>&& cp);
    };

    template <typename Func, typename... Args>
    struct TupleArgs<Func, tuple<Args...>> {
        using type = std::invoke_result_t<Func, Args...>;
    };

    /**
     * @brief Creates a tuple from a couple (copy version)
     * @tparam F_TYPE First element type
     * @tparam S_TYPE Second element type
     * @param cp Couple to convert to tuple
     * @return A new tuple containing the couple's elements
     *
     * @note Elements are copy-constructed from the couple.
     */
    template<typename F_TYPE, typename S_TYPE>
    tuple<F_TYPE, S_TYPE> makeTuple(const couple<F_TYPE, S_TYPE>& cp);

    /**
     * @brief Creates a tuple from a couple (move version)
     * @tparam F_TYPE First element type
     * @tparam S_TYPE Second element type
     * @param cp Couple to convert to tuple
     * @return A new tuple containing the couple's elements
     *
     * @note Elements are move-constructed from the couple.
     */
    template<typename F_TYPE, typename S_TYPE>
    tuple<F_TYPE, S_TYPE> makeTuple(couple<F_TYPE, S_TYPE>&& cp);

    template <typename Func, typename Tuple, u_integer... I>
    decltype(auto) applyImpl(Func&& f, Tuple&& t, indexSequence<I...>);

    template <typename Func, typename... Ts>
    decltype(auto) apply(Func&& f, const tuple<Ts...>& t);

    template <typename Func, typename... Ts>
    decltype(auto) apply(Func&& f, tuple<Ts...>&& t);
}

namespace std {
    template<typename... TYPES>
    void swap(original::tuple<TYPES...>& lhs, original::tuple<TYPES...>& rhs) noexcept; // NOLINT

    /**
     * @brief Specialization of tuple_size for tuple to enable structured bindings
     * @tparam TYPES Variadic template parameter list of element types
     */
    template<typename... TYPES>
    struct tuple_size<original::tuple<TYPES...>> //NOLINT
            : std::integral_constant<std::size_t, sizeof...(TYPES)> {};

    /**
     * @brief Specialization of tuple_element for tuple to enable structured bindings
     * @tparam I Index of element
     * @tparam TYPES Variadic template parameter list of element types
     *
     * @details This specialization defines the type of the I-th element
     *          in the custom tuple. It relies on std::tuple_element to deduce
     *          the correct base type (not reference type), which is crucial
     *          for structured binding compatibility with const-qualified tuples.
     *
     */
    template<std::size_t I, typename... TYPES>
    struct tuple_element<I, original::tuple<TYPES...>> { //NOLINT
        using type = std::tuple_element_t<I, std::tuple<TYPES...>>;
    };

    /**
     * @brief Structured binding support - get for non-const lvalue reference
     * @tparam I Index of element
     * @tparam TYPES Variadic template parameter list of element types
     * @param t tuple to get element from
     * @return Reference to the requested element
     * @note noexcept qualified
     */
    template<std::size_t I, typename... TYPES>
    constexpr auto& get(original::tuple<TYPES...>& t) noexcept; //NOLINT

    /**
     * @brief Structured binding support - get for const lvalue reference
     * @tparam I Index of element
     * @tparam TYPES Variadic template parameter list of element types
     * @param t tuple to get element from
     * @return Const reference to the requested element
     * @note noexcept qualified
     */
    template<std::size_t I, typename... TYPES>
    constexpr const auto& get(const original::tuple<TYPES...>& t) noexcept; //NOLINT

    /**
     * @brief Structured binding support - get for rvalue reference
     * @tparam I Index of element
     * @tparam TYPES Variadic template parameter list of element types
     * @param t tuple to get element from
     * @return Rvalue reference to the requested element
     * @note noexcept qualified
     */
    template<std::size_t I, typename... TYPES>
    constexpr auto&& get(original::tuple<TYPES...>&& t) noexcept; //NOLINT
}

template <typename ... TYPES>
template <original::u_integer I, typename T>
original::tuple<TYPES...>::tupleImpl<I, T>::tupleImpl(T&& cur)
    : cur_elem(std::forward<T>(cur)) {}

template<typename... TYPES>
template<original::u_integer I, typename T>
original::tuple<TYPES...>::tupleImpl<I, T>::tupleImpl(tupleImpl&& other) noexcept
    : cur_elem(std::move(other.cur_elem)) {}

template<typename... TYPES>
template<original::u_integer I, typename T>
original::tuple<TYPES...>::tupleImpl<I, T>::tupleImpl(const tupleImpl& other)
    : cur_elem(other.cur_elem) {}

template<typename... TYPES>
template<original::u_integer I, typename T>
auto original::tuple<TYPES...>::tupleImpl<I, T>::operator=(tupleImpl&& other) noexcept -> tupleImpl&
{
    if (this == &other)
        return *this;
    cur_elem = std::move(other.cur_elem);
    return *this;
}

template <typename ... TYPES>
template <original::u_integer I, typename T>
void original::tuple<TYPES...>::tupleImpl<I, T>::swap(tupleImpl& other) noexcept
{
    if (this == &other)
        return;

    std::swap(this->cur_elem, other.cur_elem);
}

template<typename... TYPES>
template<original::u_integer I, typename T>
template<original::u_integer I_DIFF>
const auto& original::tuple<TYPES...>::tupleImpl<I, T>::get() const {
    staticError<outOfBoundError, (I_DIFF > 0)>::asserts();

    return cur_elem;
}

template<typename... TYPES>
template<original::u_integer I, typename T>
template<original::u_integer I_DIFF>
auto& original::tuple<TYPES...>::tupleImpl<I, T>::get() {
    staticError<outOfBoundError, (I_DIFF > 0)>::asserts();

    return cur_elem;
}

template<typename... TYPES>
template<original::u_integer I, typename T>
template<original::u_integer I_DIFF, typename E>
void original::tuple<TYPES...>::tupleImpl<I, T>::set(const E& e) {
    staticError<outOfBoundError, (I_DIFF > 0)>::asserts();
    staticError<valueError, !std::is_convertible_v<E, T>>::asserts();

    cur_elem = static_cast<T>(e);
}

template<typename... TYPES>
template<original::u_integer I, typename T>
auto original::tuple<TYPES...>::tupleImpl<I, T>::compareTo(const tupleImpl& other) const -> integer
{
    if constexpr (Comparable<T>){
        if (cur_elem != other.cur_elem)
            return cur_elem < other.cur_elem ? -1 : 1;
    }
    return 0;
}

template<typename... TYPES>
template<original::u_integer I, typename T>
std::string original::tuple<TYPES...>::tupleImpl<I, T>::toString(bool enter) const {
    std::stringstream ss;
    if constexpr (I != 0)
        ss << ", ";
    ss << formatString(cur_elem);
    return ss.str();
}

template <typename ... TYPES>
template <original::u_integer I, typename T, typename TS>
original::tuple<TYPES...>::tupleImpl<I, T, TS>::tupleImpl(T&& cur, TS&& next_elems)
    : cur_elem(std::forward<T>(cur)), next(std::forward<TS>(next_elems)) {}

template<typename... TYPES>
template<original::u_integer I, typename T, typename TS>
original::tuple<TYPES...>::tupleImpl<I, T, TS>::tupleImpl(tupleImpl&& other) noexcept
    : cur_elem(std::move(other.cur_elem)), next(std::move(other.next)) {}

template<typename... TYPES>
template<original::u_integer I, typename T, typename TS>
original::tuple<TYPES...>::tupleImpl<I, T, TS>::tupleImpl(const tupleImpl& other)
    : cur_elem(other.cur_elem), next(other.next) {}

template<typename... TYPES>
template<original::u_integer I, typename T, typename TS>
auto original::tuple<TYPES...>::tupleImpl<I, T, TS>::operator=(
    tupleImpl&& other) noexcept -> tupleImpl&
{
    if (this == &other)
        return *this;
    cur_elem = std::move(other.cur_elem);
    next = std::move(other.next);
    return *this;
}

template <typename ... TYPES>
template <original::u_integer I, typename T, typename TS>
void original::tuple<TYPES...>::tupleImpl<I, T, TS>::swap(tupleImpl& other) noexcept
{
    if (this == &other)
        return;

    std::swap(this->cur_elem, other.cur_elem);
    this->next.swap(other.next);
}

template<typename... TYPES>
template<original::u_integer I, typename T, typename TS>
template<original::u_integer I_DIFF>
const auto& original::tuple<TYPES...>::tupleImpl<I, T, TS>::get() const {
    if constexpr (I_DIFF == 0){
        return cur_elem;
    } else{
        return next.template get<I_DIFF - 1>();
    }
}

template<typename... TYPES>
template<original::u_integer I, typename T, typename TS>
template<original::u_integer I_DIFF>
auto& original::tuple<TYPES...>::tupleImpl<I, T, TS>::get() {
    if constexpr (I_DIFF == 0){
        return cur_elem;
    } else{
        return next.template get<I_DIFF - 1>();
    }
}

template<typename... TYPES>
template<original::u_integer I, typename T, typename TS>
template<original::u_integer I_DIFF, typename E>
void original::tuple<TYPES...>::tupleImpl<I, T, TS>::set(const E& e) {
    if constexpr (I_DIFF == 0){
        staticError<valueError, !std::is_convertible_v<E, T>>::asserts();

        cur_elem = static_cast<T>(e);
    } else{
        next.template set<I_DIFF - 1, E>(e);
    }
}

template<typename... TYPES>
template<original::u_integer I, typename T, typename TS>
auto original::tuple<TYPES...>::tupleImpl<I, T, TS>::compareTo(
    const tupleImpl& other) const -> integer
{
    if constexpr (Comparable<T>){
        if (cur_elem != other.cur_elem)
            return cur_elem < other.cur_elem ? -1 : 1;
    }
    return next.compareTo(other.next);
}

template<typename... TYPES>
template<original::u_integer I, typename T, typename TS>
std::string original::tuple<TYPES...>::tupleImpl<I, T, TS>::toString(bool enter) const {
    std::stringstream ss;
    if constexpr (I != 0)
        ss << ", ";
    ss << formatString(cur_elem);
    ss << formatString(next);
    return ss.str();
}

template <typename ... TYPES>
template <original::u_integer I, typename T, typename ... TS>
original::tuple<TYPES...>::tupleImpl<I, T, TS...>::tupleImpl(T&& cur, TS&&... next_elems)
    : cur_elem(std::forward<T>(cur)), next(std::forward<TS>(next_elems)...) {}

template<typename... TYPES>
template<original::u_integer I, typename T, typename... TS>
original::tuple<TYPES...>::tupleImpl<I, T, TS...>::tupleImpl(const T& cur, const tupleImpl<I + 1, TS...>& nt)
    : cur_elem(cur), next(nt) {}

template<typename... TYPES>
template<original::u_integer I, typename T, typename... TS>
original::tuple<TYPES...>::tupleImpl<I, T, TS...>::tupleImpl(tupleImpl&& other) noexcept
    : cur_elem(std::move(other.cur_elem)), next(std::move(other.next)) {}

template<typename... TYPES>
template<original::u_integer I, typename T, typename... TS>
original::tuple<TYPES...>::tupleImpl<I, T, TS...>::tupleImpl(const tupleImpl& other)
    : cur_elem(other.cur_elem), next(other.next) {}

template<typename... TYPES>
template<original::u_integer I, typename T, typename... TS>
auto original::tuple<TYPES...>::tupleImpl<I, T, TS...>::operator=(
    tupleImpl&& other) noexcept -> tupleImpl&
{
    if (this == &other)
        return *this;
    cur_elem = std::move(other.cur_elem);
    next = std::move(other.next);
    return *this;
}

template <typename ... TYPES>
template <original::u_integer I, typename T, typename ... TS>
void original::tuple<TYPES...>::tupleImpl<I, T, TS...>::swap(tupleImpl& other) noexcept
{
    if (this == &other)
        return;

    std::swap(this->cur_elem, other.cur_elem);
    this->next.swap(other.next);
}

template<typename... TYPES>
template<original::u_integer I, typename T, typename... TS>
template<original::u_integer I_DIFF>
const auto& original::tuple<TYPES...>::tupleImpl<I, T, TS...>::get() const {
    if constexpr (I_DIFF == 0){
        return cur_elem;
    } else{
        return next.template get<I_DIFF - 1>();
    }
}

template<typename... TYPES>
template<original::u_integer I, typename T, typename... TS>
template<original::u_integer I_DIFF>
auto& original::tuple<TYPES...>::tupleImpl<I, T, TS...>::get() {
    if constexpr (I_DIFF == 0){
        return cur_elem;
    } else{
        return next.template get<I_DIFF - 1>();
    }
}

template<typename... TYPES>
template<original::u_integer I, typename T, typename... TS>
template<original::u_integer I_DIFF, typename E>
void original::tuple<TYPES...>::tupleImpl<I, T, TS...>::set(const E& e) {
    if constexpr (I_DIFF == 0){
        staticError<valueError, !std::is_convertible_v<E, T>>::asserts();

        cur_elem = static_cast<T>(e);
    } else{
        next.template set<I_DIFF - 1, E>(e);
    }
}

template<typename... TYPES>
template<original::u_integer I, typename T, typename... TS>
original::integer
original::tuple<TYPES...>::tupleImpl<I, T, TS...>::compareTo(const tupleImpl& other) const {
    if constexpr (Comparable<T>){
        if (cur_elem != other.cur_elem)
            return cur_elem < other.cur_elem ? -1 : 1;
    }
    return next.compareTo(other.next);
}

template<typename... TYPES>
template<original::u_integer I, typename T, typename... TS>
std::string original::tuple<TYPES...>::tupleImpl<I, T, TS...>::toString(bool enter) const {
    std::stringstream ss;
    if constexpr (I != 0)
        ss << ", ";
    ss << formatString(cur_elem);
    ss << formatString(next);
    return ss.str();
}

template<typename... TYPES>
template<original::u_integer... IDX_S, original::u_integer BEGIN_IDX>
auto original::tuple<TYPES...>::_slice(std::integer_sequence<u_integer, IDX_S...>,
                                       std::integral_constant<u_integer, BEGIN_IDX>) const {
    return tuple<decltype(this->get<BEGIN_IDX + IDX_S>())...>(this->get<BEGIN_IDX + IDX_S>()...);
}

template<typename... TYPES>
template<typename... O_TYPES, original::u_integer... T_SIZE, original::u_integer... O_SIZE>
original::tuple<TYPES..., O_TYPES...>
original::tuple<TYPES...>::_concat(const tuple<O_TYPES...> &other,
                                   indexSequence<T_SIZE...>,
                                   indexSequence<O_SIZE...>) const {
    return tuple<TYPES..., O_TYPES...>{TYPES(this->get<T_SIZE>())..., O_TYPES(other.template get<O_SIZE>())...};
}

template <typename ... TYPES>
template <typename ... O_TYPES, original::u_integer... T_SIZE, original::u_integer... O_SIZE>
original::tuple<TYPES..., O_TYPES...>
original::tuple<TYPES...>::_concat(tuple<O_TYPES...>&& other,
                                   indexSequence<T_SIZE...>,
                                   indexSequence<O_SIZE...>) {
    return tuple<TYPES..., O_TYPES...>{std::move(this->get<T_SIZE>())..., std::move(other.template get<O_SIZE>())...};
}

template <typename ... TYPES>
original::tuple<TYPES...>::tuple(TYPES&&... e)
    : elems(std::forward<TYPES>(e)...) {}

template<typename... TYPES>
original::tuple<TYPES...>::tuple(const tuple& other) : elems(other.elems) {}

template<typename... TYPES>
original::tuple<TYPES...>& original::tuple<TYPES...>::operator=(const tuple& other) {
    if (this == &other) return *this;
    this->elems = other.elems;
    return *this;
}

template<typename... TYPES>
original::tuple<TYPES...>::tuple(tuple&& other) noexcept
    : elems(std::move(other.elems)) {}

template<typename... TYPES>
original::tuple<TYPES...>& original::tuple<TYPES...>::operator=(tuple&& other) noexcept {
    if (this == &other) return *this;
    this->elems = std::move(other.elems);
    return *this;
}

template <typename ... TYPES>
void original::tuple<TYPES...>::swap(tuple& other) noexcept
{
    if (this == &other)
        return;

    this->elems.swap(other.elems);
}

template<typename... TYPES>
consteval original::u_integer original::tuple<TYPES...>::size() noexcept {
    return SIZE;
}

template<typename... TYPES>
template<original::u_integer IDX>
const auto& original::tuple<TYPES...>::get() const {
    return elems.template get<IDX>();
}

template<typename... TYPES>
template<original::u_integer IDX>
auto& original::tuple<TYPES...>::get() {
    return elems.template get<IDX>();
}

template<typename... TYPES>
template<original::u_integer IDX, typename E>
original::tuple<TYPES...>& original::tuple<TYPES...>::set(const E &e) {
    elems.template set<IDX, E>(e);
    return *this;
}

template<typename... TYPES>
template<original::u_integer BEGIN_IDX, original::u_integer N_ELEMS>
auto original::tuple<TYPES...>::slice() const {
    constexpr bool out_of_bound = BEGIN_IDX >= SIZE || BEGIN_IDX + N_ELEMS > SIZE;
    staticError<outOfBoundError, out_of_bound>::asserts();

    return this->_slice(
            std::make_integer_sequence<u_integer, N_ELEMS>{},
            std::integral_constant<u_integer, BEGIN_IDX>{}
    );
}

template<typename... TYPES>
original::integer original::tuple<TYPES...>::compareTo(const tuple& other) const {
    return elems.compareTo(other.elems);
}

template<typename... TYPES>
std::string original::tuple<TYPES...>::toString(bool enter) const {
    std::stringstream ss;
    ss << this->className();
    ss << "(" << elems << ")";
    return ss.str();
}

template<typename... TYPES>
std::string original::tuple<TYPES...>::className() const {
    return "tuple";
}

template <typename ... TYPES>
template <typename ... O_TYPES>
original::tuple<TYPES..., O_TYPES...>
original::tuple<TYPES...>::operator+(const tuple<O_TYPES...>& other) const
{
    return this->_concat(other,
                         makeSequence<sizeof...(TYPES)>(),
                         makeSequence<sizeof...(O_TYPES)>());
}

template <typename ... TYPES>
template <typename ... O_TYPES>
original::tuple<TYPES..., O_TYPES...>
original::tuple<TYPES...>::operator+(tuple<O_TYPES...>&& other)
{
    return this->_concat(std::move(other),
                        makeSequence<sizeof...(TYPES)>(),
                        makeSequence<sizeof...(O_TYPES)>());
}

template <typename ... TYPES>
std::tuple<TYPES...> original::tuple<TYPES...>::toStdTuple() const &
{
    return apply([](const auto&... args) {
        return std::tuple<TYPES...>(args...);
    }, *this);
}

template <typename ... TYPES>
std::tuple<TYPES...> original::tuple<TYPES...>::toStdTuple() &&
{
    return apply([](auto&&... args) {
        return std::tuple<TYPES...>(std::move(args)...);
    }, std::move(*this));
}

template<typename F_TYPE, typename S_TYPE>
original::tuple<F_TYPE, S_TYPE> original::makeTuple(const couple<F_TYPE, S_TYPE> &cp) {
    return original::tuple<F_TYPE, S_TYPE>{
        F_TYPE(cp.template get<0>()),
        S_TYPE(cp.template get<1>())
    };
}

template <typename F_TYPE, typename S_TYPE>
original::tuple<F_TYPE, S_TYPE> original::makeTuple(couple<F_TYPE, S_TYPE>&& cp)
{
    return original::tuple<F_TYPE, S_TYPE>{
        std::move(cp).template get<0>(),
        std::move(cp).template get<1>()
    };
}

template <typename Func, typename Tuple, original::u_integer... I>
decltype(auto) original::applyImpl(Func&& f, Tuple&& t, indexSequence<I...>)
{
    return std::invoke(
        std::forward<Func>(f),
        std::get<I>(std::forward<Tuple>(t))...
    );
}

template <typename Func, typename ... Ts>
decltype(auto) original::apply(Func&& f, const tuple<Ts...>& t)
{
    return applyImpl(
        std::forward<Func>(f),
        std::move(t),
        makeSequence<sizeof...(Ts)>()
    );
}

template <typename Func, typename ... Ts>
decltype(auto) original::apply(Func&& f, tuple<Ts...>&& t)
{
    return applyImpl(
        std::forward<Func>(f),
        std::move(t),
        makeSequence<sizeof...(Ts)>()
    );
}


template <typename... TYPES>
void std::swap(original::tuple<TYPES...>& lhs, original::tuple<TYPES...>& rhs) noexcept // NOLINT
{
    lhs.swap(rhs);
}

template<std::size_t I, typename... TYPES>
constexpr auto& std::get(original::tuple<TYPES...> &t) noexcept { //NOLINT
    return t.template get<I>();
}

template<std::size_t I, typename... TYPES>
constexpr const auto& std::get(const original::tuple<TYPES...> &t) noexcept { //NOLINT
    return t.template get<I>();
}

template<std::size_t I, typename... TYPES>
constexpr auto&& std::get(original::tuple<TYPES...> &&t) noexcept { //NOLINT
    return std::move(t.template get<I>());
}

#endif //TUPLE_H
