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
 * - Value semantics
 * - Deep copy/move operations
 * - Index-based element access
 * - Lexicographical comparison
 * - String serialization
 *
 * @note The implementation uses recursive template instantiation for element storage
 */

namespace original{

    /**
     * @class tuple
     * @brief A fixed-size heterogeneous container that stores elements of different types
     * @tparam TYPES... Variadic template parameter list of element types
     * @inherits printable : Provides string representation capabilities
     * @inherits comparable : Provides comparison operations between tuples
     * @details This tuple implementation:
     * - Supports element access via compile-time index
     * - Provides element modification via set() method
     * - Implements lexicographical comparison
     * - Supports deep copying and move semantics
     * - Provides type-safe element storage
     */
    template<typename... TYPES>
    class tuple final : public printable, public comparable<tuple<TYPES...>>{
        static constexpr uint32_t SIZE = sizeof...(TYPES);

        /**
         * @class tupleImpl
         * @brief Recursive tuple implementation helper
         * @tparam I Current element index
         * @tparam TS... Element type sequence
         *
         * This template implements the recursive element storage structure:
         * - Stores current element at index I
         * - Recursively contains next element(s)
         * - Provides indexed element access via template recursion
         */
        template<uint32_t I, typename... TS>
        class tupleImpl;

        template<uint32_t I, typename T>
        class tupleImpl<I, T> : public printable, public comparable<tupleImpl<I, T>>{
            T cur_elem;  ///< Current element storage

            /**
             * @brief Constructor of tupleImpl that initializes the current element
             * @param cur The current element to be stored
             */
            explicit tupleImpl(T cur);

            tupleImpl(const tupleImpl<I, T>& other);

            template<uint32_t I_DIFF>
            auto get() const;

            template<uint32_t I_DIFF, typename E>
            void set(const E& e);
        public:
            friend class tuple;
            int64_t compareTo(const tupleImpl<I, T>& other) const override;

            std::string toString(bool enter) const override;
        };

        template<uint32_t I, typename T, typename TS>
        class tupleImpl<I, T, TS> : public printable, public comparable<tupleImpl<I, T, TS>>{
            T cur_elem;                  ///< Current element storage
            tupleImpl<I + 1, TS> next;   ///< Recursive next element storage

            /**
             * @brief Constructor of tupleImpl that initializes the current element
             * @param cur The current element to be stored
             * @param next_elems next element to storage
             */
            explicit tupleImpl(T cur, TS next_elems);

            tupleImpl(const tupleImpl<I, T, TS>& other);

            template<uint32_t I_DIFF>
            auto get() const;

            template<uint32_t I_DIFF, typename E>
            void set(const E& e);
        public:
            friend class tuple;
            int64_t compareTo(const tupleImpl<I, T, TS>& other) const override;

            std::string toString(bool enter) const override;
        };

        template<uint32_t I, typename T, typename... TS>
        class tupleImpl<I, T, TS...> : public printable, public comparable<tupleImpl<I, T, TS...>>{
            T cur_elem;                     ///< Current element storage
            tupleImpl<I + 1, TS...> next;   ///< Recursive next element storage

            /**
             * @brief Constructor of tupleImpl that initializes the current element
             * @param cur The current element to be stored
             * @param next_elems next elements to storage
             */
            explicit tupleImpl(T cur, TS... next_elems);

            tupleImpl(const tupleImpl<I, T, TS...>& other);

            template<uint32_t I_DIFF>
            auto get() const;

            template<uint32_t I_DIFF, typename E>
            void set(const E& e);
        public:
            friend class tuple;
            int64_t compareTo(const tupleImpl<I, T, TS...>& other) const override;

            std::string toString(bool enter) const override;
        };

        tupleImpl<0, TYPES...> elems;  ///< Recursive element storage

        template<uint32_t... IDX_S, uint32_t BEGIN_IDX>
        auto _slice(std::integer_sequence<uint32_t, IDX_S...> indexes, std::integral_constant<uint32_t, BEGIN_IDX> begin) const;

        template<typename... O_TYPES, uint32_t... T_SIZE, uint32_t... O_SIZE>
        tuple<TYPES..., O_TYPES...> _concat(const tuple<O_TYPES...>& other,
                                            std::integer_sequence<uint32_t, T_SIZE...> ts,
                                            std::integer_sequence<uint32_t, O_SIZE...> os) const;

    public:
        /**
         * @brief Constructs a tuple with given elements
         * @param e... Elements to initialize the tuple with
         */
        explicit tuple(const TYPES&... e);

        /**
         * @brief Copy constructor
         * @param other Tuple to copy from
         */
        tuple(const tuple& other);

        /**
         * @brief Copy assignment operator
         * @param other Tuple to copy from
         * @return Reference to this tuple
         */
        tuple& operator=(const tuple& other);

        /**
         * @brief Move constructor
         * @param other Tuple to move from
         */
        tuple(tuple&& other) noexcept;

        /**
         * @brief Move assignment operator
         * @param other Tuple to move from
         * @return Reference to this tuple
         */
        tuple& operator=(tuple&& other) noexcept;

        constexpr uint32_t size() const;

        /**
         * @brief Get element by index
         * @tparam IDX Zero-based index of the element to retrieve
         * @return Copy of the element at specified index
         * @throw outOfBoundError If index exceeds tuple size
         */
        template<uint32_t IDX>
        auto get() const;

        /**
         * @brief Set element by index
         * @tparam IDX Zero-based index of the element to modify
         * @tparam E Type of the new value (auto-deduced)
         * @param e New value to assign
         * @throw outOfBoundError If index exceeds tuple size
         * @throw valueError If type E is incompatible with target element type
         */
        template<uint32_t IDX, typename E>
        void set(const E& e);

        template<uint32_t BEGIN_IDX, uint32_t N_ELEMS>
        auto slice() const;

        template<typename... O_TYPES>
        tuple<TYPES..., O_TYPES...> operator+(const tuple<O_TYPES...>& other) const;

        /**
         * @brief Compare two tuples lexicographically
         * @param other Tuple to compare with
         * @return Comparison result:
         *         - Negative if this tuple is smaller
         *         - Positive if this tuple is larger
         *         - Zero if tuples are equal
         *
         * @details Comparison rules:
         * 1. Compares elements pairwise from first to last
         * 2. Stops at first unequal element pair
         * 3. Elements must be comparable using < operator
         */
        int64_t compareTo(const tuple& other) const override;

        /**
         * @brief Generate formatted string representation
         * @param enter Whether to add newline (implementation specific)
         * @return String in format "tuple(element1, element2, ...)"
         */
        std::string toString(bool enter) const override;

        /**
         * @brief Get class name identifier
         * @return "tuple" string literal
         */
        std::string className() const override;

        template<typename F_TYPE, typename S_TYPE>
        friend tuple<F_TYPE, S_TYPE> makeTuple(const original::couple<F_TYPE, S_TYPE> &cp);

        template<typename... L_TYPES, typename... R_TYPES>
        friend tuple<L_TYPES..., R_TYPES...> operator+(const tuple<L_TYPES...>& lt, const tuple<R_TYPES...>& rt);
    };

    template<typename F_TYPE, typename S_TYPE>
    tuple<F_TYPE, S_TYPE> makeTuple(const original::couple<F_TYPE, S_TYPE> &cp);

    template<typename... L_TYPES, typename... R_TYPES>
    tuple<L_TYPES..., R_TYPES...> operator+(const tuple<L_TYPES...>& lt, const tuple<R_TYPES...>& rt);
}

template<typename... TYPES>
template<uint32_t I, typename T>
original::tuple<TYPES...>::tupleImpl<I, T>::tupleImpl(T cur)
    : cur_elem(cur) {}

template<typename... TYPES>
template<uint32_t I, typename T>
original::tuple<TYPES...>::tupleImpl<I, T>::tupleImpl(const tupleImpl<I, T>& other)
    : cur_elem(other.cur_elem) {}

template<typename... TYPES>
template<uint32_t I, typename T>
template<uint32_t I_DIFF>
auto original::tuple<TYPES...>::tupleImpl<I, T>::get() const {
    if constexpr (I_DIFF == 0){
        return cur_elem;
    } else{
        error::asserts<outOfBoundError>();
    }
}

template<typename... TYPES>
template<uint32_t I, typename T>
template<uint32_t I_DIFF, typename E>
void original::tuple<TYPES...>::tupleImpl<I, T>::set(const E& e) {
    if constexpr (I_DIFF == 0 && std::same_as<T, E>){
        cur_elem = e;
    } else{
        if constexpr (I_DIFF != 0)
            error::asserts<outOfBoundError>();
        if constexpr (!std::same_as<T, E>)
            error::asserts<valueError>();
    }
}

template<typename... TYPES>
template<uint32_t I, typename T>
int64_t original::tuple<TYPES...>::tupleImpl<I, T>::compareTo(const tupleImpl<I, T>& other) const {
    if constexpr (Comparable<T>){
        if (cur_elem != other.cur_elem)
            return cur_elem < other.cur_elem ? -1 : 1;
    }
    return 0;
}

template<typename... TYPES>
template<uint32_t I, typename T>
std::string original::tuple<TYPES...>::tupleImpl<I, T>::toString(bool enter) const {
    std::stringstream ss;
    if constexpr (I != 0)
        ss << ", ";
    ss << cur_elem;
    return ss.str();
}

template<typename... TYPES>
template<uint32_t I, typename T, typename TS>
original::tuple<TYPES...>::tupleImpl<I, T, TS>::tupleImpl(T cur, TS next_elems)
    : cur_elem(cur), next(next_elems) {}

template<typename... TYPES>
template<uint32_t I, typename T, typename TS>
original::tuple<TYPES...>::tupleImpl<I, T, TS>::tupleImpl(const tupleImpl<I, T, TS>& other)
    : cur_elem(other.cur_elem), next(other.next) {}

template<typename... TYPES>
template<uint32_t I, typename T, typename TS>
template<uint32_t I_DIFF>
auto original::tuple<TYPES...>::tupleImpl<I, T, TS>::get() const {
    if constexpr (I_DIFF == 0){
        return cur_elem;
    } else{
        return next.template get<I_DIFF - 1>();
    }
}

template<typename... TYPES>
template<uint32_t I, typename T, typename TS>
template<uint32_t I_DIFF, typename E>
void original::tuple<TYPES...>::tupleImpl<I, T, TS>::set(const E& e) {
    if constexpr (I_DIFF == 0){
        if constexpr (!std::same_as<T, E>)
            error::asserts<valueError>();
        cur_elem = e;
    } else{
        next.template set<I_DIFF - 1, E>(e);
    }
}

template<typename... TYPES>
template<uint32_t I, typename T, typename TS>
int64_t
original::tuple<TYPES...>::tupleImpl<I, T, TS>::compareTo(const tupleImpl<I, T, TS>& other) const {
    if constexpr (Comparable<T>){
        if (cur_elem != other.cur_elem)
            return cur_elem < other.cur_elem ? -1 : 1;
    }
    return next.compareTo(other.next);
}

template<typename... TYPES>
template<uint32_t I, typename T, typename TS>
std::string original::tuple<TYPES...>::tupleImpl<I, T, TS>::toString(bool enter) const {
    std::stringstream ss;
    if constexpr (I != 0)
        ss << ", ";
    ss << cur_elem;
    ss << next;
    return ss.str();
}

template<typename... TYPES>
template<uint32_t I, typename T, typename... TS>
original::tuple<TYPES...>::tupleImpl<I, T, TS...>::tupleImpl(T cur, TS... next_elems)
    : cur_elem(cur), next(next_elems...) {}

template<typename... TYPES>
template<uint32_t I, typename T, typename... TS>
original::tuple<TYPES...>::tupleImpl<I, T, TS...>::tupleImpl(const tupleImpl<I, T, TS...>& other)
    : cur_elem(other.cur_elem), next(other.next) {}

template<typename... TYPES>
template<uint32_t I, typename T, typename... TS>
template<uint32_t I_DIFF>
auto original::tuple<TYPES...>::tupleImpl<I, T, TS...>::get() const {
    if constexpr (I_DIFF == 0){
        return cur_elem;
    } else{
        return next.template get<I_DIFF - 1>();
    }
}

template<typename... TYPES>
template<uint32_t I, typename T, typename... TS>
template<uint32_t I_DIFF, typename E>
void original::tuple<TYPES...>::tupleImpl<I, T, TS...>::set(const E& e) {
    if constexpr (I_DIFF == 0){
        if constexpr (!std::same_as<T, E>)
            error::asserts<valueError>();
        cur_elem = e;
    } else{
        next.template set<I_DIFF - 1, E>(e);
    }
}

template<typename... TYPES>
template<uint32_t I, typename T, typename... TS>
int64_t
original::tuple<TYPES...>::tupleImpl<I, T, TS...>::compareTo(const tupleImpl<I, T, TS...>& other) const {
    if constexpr (Comparable<T>){
        if (cur_elem != other.cur_elem)
            return cur_elem < other.cur_elem ? -1 : 1;
    }
    return next.compareTo(other.next);
}

template<typename... TYPES>
template<uint32_t I, typename T, typename... TS>
std::string original::tuple<TYPES...>::tupleImpl<I, T, TS...>::toString(bool enter) const {
    std::stringstream ss;
    if constexpr (I != 0)
        ss << ", ";
    ss << cur_elem;
    ss << next;
    return ss.str();
}

template<typename... TYPES>
template<uint32_t... IDX_S, uint32_t BEGIN_IDX>
auto original::tuple<TYPES...>::_slice(std::integer_sequence<uint32_t, IDX_S...>,
                                       std::integral_constant<uint32_t, BEGIN_IDX>) const {
    return tuple<decltype(this->get<BEGIN_IDX + IDX_S>())...>(this->get<BEGIN_IDX + IDX_S>()...);
}

template<typename... TYPES>
template<typename... O_TYPES, uint32_t... T_SIZE, uint32_t... O_SIZE>
original::tuple<TYPES..., O_TYPES...>
original::tuple<TYPES...>::_concat(const original::tuple<O_TYPES...> &other,
                                   std::integer_sequence<uint32_t, T_SIZE...>,
                                   std::integer_sequence<uint32_t, O_SIZE...>) const {
    return tuple<TYPES..., O_TYPES...>{this->get<T_SIZE>()..., other.template get<O_SIZE>()...};
}

template<typename... TYPES>
original::tuple<TYPES...>::tuple(const TYPES&... e) : elems(e...) {}

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

template<typename... TYPES>
constexpr uint32_t original::tuple<TYPES...>::size() const {
    return SIZE;
}

template<typename... TYPES>
template<uint32_t IDX>
auto original::tuple<TYPES...>::get() const {
    return elems.template get<IDX>();
}

template<typename... TYPES>
template<uint32_t IDX, typename E>
void original::tuple<TYPES...>::set(const E &e) {
    elems.template set<IDX, E>(e);
}

template<typename... TYPES>
template<uint32_t BEGIN_IDX, uint32_t N_ELEMS>
auto original::tuple<TYPES...>::slice() const {
    if constexpr (BEGIN_IDX >= SIZE || BEGIN_IDX + N_ELEMS > SIZE){
        error::asserts<outOfBoundError>();
    }
    return this->_slice(
            std::make_integer_sequence<uint32_t, N_ELEMS>{},
            std::integral_constant<uint32_t, BEGIN_IDX>{}
    );
}

template<typename... TYPES>
template<typename... O_TYPES>
original::tuple<TYPES..., O_TYPES...>
original::tuple<TYPES...>::operator+(const original::tuple<O_TYPES...> &other) const {
    return this->_concat(
            other,
            std::make_integer_sequence<uint32_t, this->size()>{},
            std::make_integer_sequence<uint32_t, other.size()>{}
    );
}

template<typename... TYPES>
int64_t original::tuple<TYPES...>::compareTo(const tuple& other) const {
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

template<typename F_TYPE, typename S_TYPE>
original::tuple<F_TYPE, S_TYPE> original::makeTuple(const original::couple<F_TYPE, S_TYPE> &cp) {
    return original::tuple<F_TYPE, S_TYPE>{cp.template get<0>(), cp.template get<1>()};
}

template<typename... L_TYPES, typename... R_TYPES>
original::tuple<L_TYPES..., R_TYPES...>
original::operator+(const original::tuple<L_TYPES...> &lt, const original::tuple<R_TYPES...> &rt) {
    return lt + rt;
}

#endif //TUPLE_H
