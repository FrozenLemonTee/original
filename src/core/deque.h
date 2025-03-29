#ifndef DEQUE_H
#define DEQUE_H

/**
 * @file deque.h
 * @brief Double-ended queue (deque) container adapter implementation
 * @details Provides a deque adapter supporting insertion/removal at both ends.
 * Works with different underlying container types through template parameters.
 */

#include "chain.h"
#include "containerAdapter.h"

namespace original {

    /**
     * @class deque
     * @tparam TYPE Type of elements stored in the deque
     * @tparam SERIAL Underlying container type (default: chain), see constraints from @ref original::containerAdapter
     * @brief Double-ended queue container adapter
     * @extends original::containerAdapter
     * @details Implements deque operations using the specified underlying container.
     * Supports efficient insertion and removal at both front and back ends.
     * Inherits template constraints from @ref original::containerAdapter.
     */
    template <typename TYPE,
              template <typename, typename> typename SERIAL = chain,
              template <typename> typename ALLOC = allocator>
    class deque final : public containerAdapter<TYPE, SERIAL, ALLOC> {
    public:
        /**
         * @brief Constructs deque with specified underlying container
         * @param serial Container instance to initialize deque (default: empty)
         */
        explicit deque(const SERIAL<TYPE, ALLOC<TYPE>>& serial = SERIAL<TYPE, ALLOC<TYPE>>{});

        /**
         * @brief Constructs deque from initializer list
         * @param lst List of elements for initial content
         */
        deque(const std::initializer_list<TYPE>& lst);

        /**
         * @brief Copy constructor
         * @param other Deque instance to copy from
         */
        deque(const deque& other);

        /**
         * @brief Copy assignment operator
         * @param other Deque instance to copy from
         * @return Reference to this deque
         */
        deque& operator=(const deque& other);

        /**
         * @brief Move constructor
         * @param other Deque instance to move from
         * @note noexcept guarantees exception safety during move
         */
        deque(deque&& other) noexcept;

        /**
         * @brief Move assignment operator
         * @param other Deque instance to move from
         * @return Reference to this deque
         * @note noexcept guarantees exception safety during move
         */
        deque& operator=(deque&& other) noexcept;

        /**
         * @brief Inserts element at the front
         * @param e Element to insert
         */
        void pushBegin(const TYPE &e);

        /**
         * @brief Inserts element at the back
         * @param e Element to insert
         */
        void pushEnd(const TYPE &e);

        /**
         * @brief Removes and returns front element
         * @return The removed front element
         * @pre Deque must not be empty
         */
        TYPE popBegin();

        /**
         * @brief Removes and returns back element
         * @return The removed back element
         * @pre Deque must not be empty
         */
        TYPE popEnd();

        /**
         * @brief Accesses front element
         * @return Const reference to front element
         * @pre Deque must not be empty
         */
        TYPE head() const;

        /**
         * @brief Accesses back element
         * @return Const reference to back element
         * @pre Deque must not be empty
         */
        TYPE tail() const;

        /**
         * @brief Gets class name identifier
         * @return "deque" string identifier
         */
        [[nodiscard]] std::string className() const override;
    };
}

    template <typename TYPE,
              template <typename, typename> typename SERIAL,
              template <typename> typename ALLOC>
    original::deque<TYPE, SERIAL, ALLOC>::deque(const SERIAL<TYPE, ALLOC<TYPE>>& serial)
        : containerAdapter<TYPE, SERIAL, ALLOC>(serial) {}

    template <typename TYPE,
              template <typename, typename> typename SERIAL,
              template <typename> typename ALLOC>
    original::deque<TYPE, SERIAL, ALLOC>::deque(const std::initializer_list<TYPE> &lst)
        : deque(SERIAL<TYPE, ALLOC<TYPE>>(lst)) {}

    template <typename TYPE,
              template <typename, typename> typename SERIAL,
              template <typename> typename ALLOC>
    original::deque<TYPE, SERIAL, ALLOC>::deque(const deque& other) : deque() {
        this->operator=(other);
    }

    template <typename TYPE,
              template <typename, typename> typename SERIAL,
              template <typename> typename ALLOC>
    auto original::deque<TYPE, SERIAL, ALLOC>::operator=(const deque& other) -> deque& {
        if (this == &other) return *this;
        this->serial_ = other.serial_;
        return *this;
    }

    template <typename TYPE,
              template <typename, typename> typename SERIAL,
              template <typename> typename ALLOC>
    original::deque<TYPE, SERIAL, ALLOC>::deque(deque&& other) noexcept : deque()
    {
        this->operator=(std::move(other));
    }

    template <typename TYPE,
              template <typename, typename> typename SERIAL,
              template <typename> typename ALLOC>
    auto original::deque<TYPE, SERIAL, ALLOC>::operator=(deque&& other) noexcept -> deque&
    {
        if (this == &other)
            return *this;

        this->serial_ = std::move(other.serial_);
        return *this;
    }

    template <typename TYPE,
              template <typename, typename> typename SERIAL,
              template <typename> typename ALLOC>
    auto original::deque<TYPE, SERIAL, ALLOC>::pushBegin(const TYPE &e) -> void {
        this->serial_.pushBegin(e);
    }

    template <typename TYPE,
              template <typename, typename> typename SERIAL,
              template <typename> typename ALLOC>
    auto original::deque<TYPE, SERIAL, ALLOC>::pushEnd(const TYPE &e) -> void {
        this->serial_.pushEnd(e);
    }

    template <typename TYPE,
              template <typename, typename> typename SERIAL,
              template <typename> typename ALLOC>
    auto original::deque<TYPE, SERIAL, ALLOC>::popBegin() -> TYPE {
        return this->serial_.popBegin();
    }

    template <typename TYPE,
              template <typename, typename> typename SERIAL,
              template <typename> typename ALLOC>
    auto original::deque<TYPE, SERIAL, ALLOC>::popEnd() -> TYPE {
        return this->serial_.popEnd();
    }

    template <typename TYPE,
              template <typename, typename> typename SERIAL,
              template <typename> typename ALLOC>
    auto original::deque<TYPE, SERIAL, ALLOC>::head() const -> TYPE {
        return this->serial_.getBegin();
    }

    template <typename TYPE,
              template <typename, typename> typename SERIAL,
              template <typename> typename ALLOC>
    auto original::deque<TYPE, SERIAL, ALLOC>::tail() const -> TYPE {
        return this->serial_.getEnd();
    }

    template <typename TYPE,
              template <typename, typename> typename SERIAL,
              template <typename> typename ALLOC>
    auto original::deque<TYPE, SERIAL, ALLOC>::className() const -> std::string {
        return "deque";
    }

#endif //DEQUE_H
