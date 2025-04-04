#ifndef COUPLE_H
#define COUPLE_H

/**
 * @file couple.h
 * @brief Generic pair container implementation
 * @details Provides a template class for storing two elements of different types
 * with printing capabilities. Supports equality comparison and formatted output.
 */

#include "printable.h"
#include "comparable.h"
#include "types.h"
#include "error.h"


namespace original
{
    /**
     * @class couple
     * @tparam F_TYPE Type of the first element
     * @tparam S_TYPE Type of the second element
     * @brief Container for two heterogeneous elements
     * @extends printable
     * @extends comparable
     * @details Stores a pair of elements with type safety. Provides:
     * - Element access and copy operations
     * - Lexicographical comparison (first element precedence)
     * - Formatted string output through printable interface
     * - Comparable interface implementation
     */
    template<typename F_TYPE, typename S_TYPE>
    class couple final : public printable, public comparable<couple<F_TYPE, S_TYPE>>
    {
        F_TYPE first_;  ///< Storage for the first element
        S_TYPE second_; ///< Storage for the second element

    public:
        /**
         * @brief Default constructs both elements
         */
        explicit couple();

        /**
         * @brief Constructs from pointer elements
         * @param first Pointer to first element
         * @param second Pointer to second element
         */
        couple(F_TYPE* first, S_TYPE* second);

        /**
         * @brief Constructs from element copies
         * @param first First element to copy
         * @param second Second element to copy
         */
        couple(const F_TYPE& first, const S_TYPE& second);

        /**
         * @brief Copy constructor
         * @param other couple to copy from
         */
        couple(const couple& other);

        /**
         * @brief Copy assignment operator
         * @param other couple to copy from
         * @return Reference to this object
         */
        couple& operator=(const couple& other);

        /**
         * @brief element access template method
         * @tparam IDX element index (0=1st element, 1=2nd element)
         * @return Constant reference to the corresponding element
         */
        template<u_integer IDX>
        auto get() const;

        /**
         * @brief element modifies the template method
         * @tparam IDX element index (0=1st element, 1=2nd element)
         * @tparam T Element type (auto-derivation)
         * @param e new element value
         */
        template<u_integer IDX, typename T>
        void set(const T& e);

        /**
         * @brief Lexicographical comparison operation
         * @param other couple to compare with
         * @return integer Negative if less, positive if greater, zero if equal
         * @details Compares first elements first, then second elements if firsts are equal
         */
        integer compareTo(const couple &other) const override;

        /**
         * @brief Access first element
         * @return Mutable reference to first element
         */
        F_TYPE& first();

        /**
         * @brief Access second element
         * @return Mutable reference to second element
         */
        S_TYPE& second();

        /**
         * @brief Default destructor
         */
        ~couple() override;

        /**
         * @brief Gets class name identifier
         * @return "couple" string constant
         */
        [[nodiscard]] std::string className() const override;

        /**
         * @brief Formats pair elements as string
         * @param enter Add newline at end if true
         * @return Formatted string "(first: value, second: value)"
         */
        [[nodiscard]] std::string toString(bool enter) const override;
    };
}

    template <typename F_TYPE, typename S_TYPE>
    original::couple<F_TYPE, S_TYPE>::couple() : first_(), second_() {}

    template <typename F_TYPE, typename S_TYPE>
    original::couple<F_TYPE, S_TYPE>::couple(F_TYPE* first, S_TYPE* second)
        : first_(*first), second_(*second) {}

    template <typename F_TYPE, typename S_TYPE>
    original::couple<F_TYPE, S_TYPE>::couple(const F_TYPE& first, const S_TYPE& second)
        : first_(first), second_(second) {}

    template <typename F_TYPE, typename S_TYPE>
    original::couple<F_TYPE, S_TYPE>::couple(const couple& other)
        : first_(other.first_), second_(other.second_) {}

    template <typename F_TYPE, typename S_TYPE>
    auto original::couple<F_TYPE, S_TYPE>::operator=(const couple& other) -> couple&
    {
        if (this == &other) return *this;
        first_ = other.first_;
        second_ = other.second_;
        return *this;
    }

    template<typename F_TYPE, typename S_TYPE>
    template<original::u_integer IDX>
    auto original::couple<F_TYPE, S_TYPE>::get() const {
        staticError<outOfBoundError, (IDX > 1)>{};
        if constexpr (IDX == 0){
            return this->first_;
        }else {
            return this->second_;
        }
    }

    template<typename F_TYPE, typename S_TYPE>
    template<original::u_integer IDX, typename T>
    void original::couple<F_TYPE, S_TYPE>::set(const T &e) {
        staticError<outOfBoundError, (IDX > 1)>{};
        if constexpr (IDX == 0){
            staticError<valueError, !std::is_convertible_v<T, F_TYPE>>{};
            this->first_ = static_cast<F_TYPE>(e);
        } else{
            staticError<valueError, !std::is_convertible_v<T, S_TYPE>>{};
            this->second_ = static_cast<S_TYPE>(e);
        }
    }

    template<typename F_TYPE, typename S_TYPE>
    auto original::couple<F_TYPE, S_TYPE>::compareTo(const couple& other) const -> integer
    {
        if constexpr (Comparable<F_TYPE>){
            if (this->first_ < other.first_)
                return -1;
            if (this->first_ > other.first_)
                return 1;
        }
        if constexpr (Comparable<S_TYPE>){
            if (this->second_ < other.second_)
                return -1;
            if (this->second_ > other.second_)
                return 1;
        }
        return 0;
    }

    template <typename F_TYPE, typename S_TYPE>
    auto original::couple<F_TYPE, S_TYPE>::first() -> F_TYPE&
    {
        return this->first_;
    }

    template <typename F_TYPE, typename S_TYPE>
    auto original::couple<F_TYPE, S_TYPE>::second() -> S_TYPE&
    {
        return this->second_;
    }

    template <typename F_TYPE, typename S_TYPE>
    original::couple<F_TYPE, S_TYPE>::~couple() = default;

    template <typename F_TYPE, typename S_TYPE>
    auto original::couple<F_TYPE, S_TYPE>::className() const -> std::string
    {
        return "couple";
    }

    template <typename F_TYPE, typename S_TYPE>
    auto original::couple<F_TYPE, S_TYPE>::toString(const bool enter) const -> std::string
    {
        std::stringstream ss;
        ss << this->className() << "(" << formatString(this->first_)
           << ", " << formatString(this->second_) << ")";
        if (enter) ss << "\n";
        return ss.str();
    }

#endif //COUPLE_H
