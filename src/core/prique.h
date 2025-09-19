#ifndef PRIQUE_H
#define PRIQUE_H

/**
 * @file prique.h
 * @brief Priority queue container implementation
 * @details Provides a heap-based priority queue with configurable comparison logic
 * and underlying storage. Supports efficient insertion and extraction of highest
 * priority elements according to the specified comparator.
 */

#include "algorithms.h"
#include "blocksList.h"
#include "comparator.h"
#include "containerAdapter.h"
#include "types.h"

namespace original
{
    /**
     * @class prique
     * @tparam TYPE Type of elements stored in the priority queue
     * @tparam Callback Comparison functor type (default: increaseComparator)
     * @tparam SERIAL Underlying container type (default: blocksList)
     * @tparam ALLOC Allocator template for memory management (default: allocator)
     * @brief Heap-based priority queue container
     * @extends containerAdapter
     * @details Implements a priority queue using heap algorithms over an underlying
     * container. The element priority is determined by the provided comparator.
     *
     * The allocator is propagated to both the priority queue and the underlying
     * serial container for consistent memory management of elements.
     *
     * @note Template parameters are constrained by:
     * @code
     * requires Compare<Callback<TYPE>, TYPE>
     * @endcode
     * Ensures the Callback type provides valid comparison operations for TYPE.
     *
     * @see @ref original::containerAdapter for SERIAL container requirements
     */
    template<typename TYPE,
            template <typename> typename Callback = increaseComparator,
            template <typename, typename> typename SERIAL = blocksList,
            template <typename> typename ALLOC = allocator>
    requires Compare<Callback<TYPE>, TYPE>
    class prique final : public containerAdapter<TYPE, SERIAL, ALLOC>
    {
        Callback<TYPE> compare_; ///< Comparison functor instance for priority ordering

    public:
        /**
         * @brief Constructs priority queue with container, comparator and allocator
         * @param serial Underlying container instance (default: empty)
         * @param compare Comparison functor instance (default: default-constructed)
         * @details Initializes heap structure using algorithms::heapInit.
         * The allocator from the provided container will be used for all memory operations.
         */
        explicit prique(const SERIAL<TYPE, ALLOC<TYPE>>& serial = SERIAL<TYPE, ALLOC<TYPE>>{},
                        const Callback<TYPE>& compare = Callback<TYPE>{});

        /**
         * @brief Constructs from initializer list with comparator and allocator
         * @param lst Elements to initialize the queue
         * @param compare Comparison functor instance
         * @details Uses the default allocator to construct the underlying container
         * and populate it with elements from the initializer list.
         */
        prique(const std::initializer_list<TYPE>& lst,
               const Callback<TYPE>& compare = Callback<TYPE>{});

        /**
         * @brief Copy constructs a priority queue with allocator propagation
         * @param other Priority queue to copy from
         * @note The allocator is copied if ALLOC::propagate_on_container_copy_assignment is true
         */
        prique(const prique& other);

        /**
         * @brief Copy assignment operator
         * @param other Priority queue to copy from
         * @return Reference to this queue
         * @note The allocator is copied if ALLOC::propagate_on_container_copy_assignment is true
         */
        prique& operator=(const prique& other);

        /**
         * @brief Move constructs a priority queue with allocator propagation
         * @param other Queue to move from
         * @note The allocator is moved if ALLOC::propagate_on_container_move_assignment is true
         * @note noexcept guarantees exception safety during move
         */
        prique(prique&& other) noexcept;

        /**
         * @brief Move assignment operator
         * @param other Queue to move from
         * @return Reference to this queue
         * @note noexcept guarantees exception safety during move
         * @note The allocator is moved if ALLOC::propagate_on_container_move_assignment is true
         */
        prique& operator=(prique&& other) noexcept;

        /**
         * @brief Inserts element maintaining heap property
         * @param e Element to insert
         * @details Uses algorithms::heapAdjustUp after insertion.
         * The element is constructed using the queue's allocator.
         */
        void push(const TYPE& e);

        void push(TYPE&& e);

        /**
         * @brief Extracts highest priority element
         * @return The extracted element
         * @throw original::noElementError if queue is empty
         * @details Uses algorithms::heapAdjustDown after extraction.
         * The element is properly destroyed using the queue's allocator.
         */
        TYPE pop();

        /**
         * @brief Accesses highest priority element
         * @return Const reference to top element
         * @pre Queue must not be empty
         */
        TYPE top() const;

        /**
         * @brief Gets class name identifier
         * @return "prique" string identifier
         */
        [[nodiscard]] std::string className() const override;
    };
}


    template<typename TYPE,
            template <typename> typename Callback,
            template <typename, typename> typename SERIAL,
            template <typename> typename ALLOC>
    requires original::Compare<Callback<TYPE>, TYPE>
    original::prique<TYPE, Callback, SERIAL, ALLOC>::prique(const SERIAL<TYPE, ALLOC<TYPE>>& serial, const Callback<TYPE>& compare)
        : containerAdapter<TYPE, SERIAL, ALLOC>(serial), compare_(compare)
    {
        algorithms::heapInit(this->serial_.begin(), this->serial_.last(), this->compare_);
    }

    template<typename TYPE,
            template <typename> typename Callback,
            template <typename, typename> typename SERIAL,
            template <typename> typename ALLOC>
    requires original::Compare<Callback<TYPE>, TYPE>
    original::prique<TYPE, Callback, SERIAL, ALLOC>::prique(const std::initializer_list<TYPE>& lst, const Callback<TYPE>& compare)
        : prique(SERIAL<TYPE, ALLOC<TYPE>>(lst), compare) {}

    template<typename TYPE,
            template <typename> typename Callback,
            template <typename, typename> typename SERIAL,
            template <typename> typename ALLOC>
    requires original::Compare<Callback<TYPE>, TYPE>
    original::prique<TYPE, Callback, SERIAL, ALLOC>::prique(const prique& other)
        : containerAdapter<TYPE, SERIAL, ALLOC>(other.serial_), compare_(other.compare_) {}

    template<typename TYPE,
            template <typename> typename Callback,
            template <typename, typename> typename SERIAL,
            template <typename> typename ALLOC>
    requires original::Compare<Callback<TYPE>, TYPE>
    auto original::prique<TYPE, Callback, SERIAL, ALLOC>::operator=(const prique& other) -> prique&
    {
        if (this == &other) return *this;
        this->serial_ = other.serial_;
        compare_ = other.compare_;
        return *this;
    }

    template<typename TYPE,
            template <typename> typename Callback,
            template <typename, typename> typename SERIAL,
            template <typename> typename ALLOC>
    requires original::Compare<Callback<TYPE>, TYPE>
    original::prique<TYPE, Callback, SERIAL, ALLOC>::prique(prique&& other) noexcept : prique()
    {
        this->operator=(std::move(other));
    }

    template<typename TYPE,
            template <typename> typename Callback,
            template <typename, typename> typename SERIAL,
            template <typename> typename ALLOC>
    requires original::Compare<Callback<TYPE>, TYPE>
    auto original::prique<TYPE, Callback, SERIAL, ALLOC>::operator=(prique&& other) noexcept -> prique&
    {
        if (this == &other)
            return *this;

        this->serial_ = std::move(other.serial_);
        this->compare_ = std::move(other.compare_);
        return *this;
    }

    template<typename TYPE,
            template <typename> typename Callback,
            template <typename, typename> typename SERIAL,
            template <typename> typename ALLOC>
    requires original::Compare<Callback<TYPE>, TYPE>
    auto original::prique<TYPE, Callback, SERIAL, ALLOC>::push(const TYPE& e) -> void
    {
        this->serial_.pushEnd(e);
        algorithms::heapAdjustUp(this->serial_.begin(), this->serial_.last(), this->compare_);
    }

    template<typename TYPE,
            template <typename> typename Callback,
            template <typename, typename> typename SERIAL,
            template <typename> typename ALLOC>
    requires original::Compare<Callback<TYPE>, TYPE>
    void original::prique<TYPE, Callback, SERIAL, ALLOC>::push(TYPE&& e)
    {
        this->serial_.pushEnd(std::move(e));
        algorithms::heapAdjustUp(this->serial_.begin(), this->serial_.last(), this->compare_);
    }

    template<typename TYPE,
            template <typename> typename Callback,
            template <typename, typename> typename SERIAL,
            template <typename> typename ALLOC>
    requires original::Compare<Callback<TYPE>, TYPE>
    auto original::prique<TYPE, Callback, SERIAL, ALLOC>::pop() -> TYPE
    {
        if (this->empty()) throw noElementError();

        algorithms::swap(this->serial_.begin(), this->serial_.last());
        TYPE res = std::move(this->serial_.popEnd());
        algorithms::heapAdjustDown(this->serial_.begin(), this->serial_.last(), this->serial_.begin(), this->compare_);
        return res;
    }

    template<typename TYPE,
            template <typename> typename Callback,
            template <typename, typename> typename SERIAL,
            template <typename> typename ALLOC>
    requires original::Compare<Callback<TYPE>, TYPE>
    auto original::prique<TYPE, Callback, SERIAL, ALLOC>::top() const -> TYPE
    {
        return this->serial_.getBegin();
    }

    template<typename TYPE,
            template <typename> typename Callback,
            template <typename, typename> typename SERIAL,
            template <typename> typename ALLOC>
    requires original::Compare<Callback<TYPE>, TYPE>
    auto original::prique<TYPE, Callback, SERIAL, ALLOC>::className() const -> std::string
    {
        return "prique";
    }

#endif //PRIQUE_H
