#ifndef STEPITERATOR_H
#define STEPITERATOR_H

#include <limits>

#include "iterator.h"
#include "ownerPtr.h"
#include "wrapper.h"
#include "error.h"
#include "maths.h"

/**
 * @file stepIterator.h
 * @brief Step iterator base class for single-step iterators
 * @details Provides common functionality for iterators that support step-based traversal operations.
 *          Includes methods for pointer arithmetic, traversal, and element access.
 *          Intended for use as a base class for all iterators that iterate through elements one step at a time.
 */

namespace original
{
    /**
     * @class stepIterator
     * @tparam TYPE Type of elements being iterated
     * @brief Abstract base class for step iterators
     * @extends baseIterator
     * @details Implements core functionality for iterators that iterate through elements one step at a time.
     *          Supports:
     *          - Stepwise forward traversal (`next` and `prev`)
     *          - Cloning and comparison operations
     *          - Element access/modification
     *          - Position validity checking
     *
     * @note Derived iterators should provide the specific behavior of moving to the next element and accessing the current element.
     */
    template<typename TYPE>
    class stepIterator : public baseIterator<TYPE>
    {
    protected:
        mutable wrapper<TYPE>* _ptr;  ///< Pointer to the current wrapper

        /**
         * @brief Protected constructor for derived classes
         * @param ptr Raw pointer to the element
         */
        explicit stepIterator(wrapper<TYPE>* ptr);

        /**
         * @brief Calculates the distance between two iterators
         * @param start Iterator pointing to the start position
         * @param end Iterator pointing to the end position
         * @return The number of steps between the two iterators
         */
        static integer ptrDistance(const stepIterator* start, const stepIterator* end);

        /**
         * @brief Equality comparison for two iterators
         * @param other Iterator to compare with
         * @return True if the iterators point to the same element, otherwise false
         */
        bool equalPtr(const iterator<TYPE>* other) const override;

    public:
        /**
         * @brief Copy constructor for stepIterator
         * @param other The iterator to copy
         */
        stepIterator(const stepIterator& other);

        /**
         * @brief Copy assignment operator for stepIterator
         * @param other The iterator to copy
         * @return Reference to this iterator
         */
        stepIterator& operator=(const stepIterator& other);

        /**
         * @brief Creates a heap-allocated copy of the iterator
         * @return A new heap-allocated stepIterator
         */
        stepIterator* clone() const override;

        /**
         * @brief Checks if the iterator can move forward
         * @return True if the iterator is not at the end, otherwise false
         */
        [[nodiscard]] bool hasNext() const override;

        /**
         * @brief Checks if the iterator can move backward
         * @return Always throws unSupportedMethodError since this iterator only supports forward movement
         */
        [[nodiscard]] bool hasPrev() const override;

        /**
         * @brief Checks if the current iterator is at the previous position relative to another iterator
         * @param other The iterator to compare with
         * @return Always throws unSupportedMethodError for backward checks
         */
        bool atPrev(const iterator<TYPE>* other) const override;

        /**
         * @brief Checks if the current iterator is at the next position relative to another iterator
         * @param other The iterator to compare with
         * @return Always throws unSupportedMethodError for forward checks
         */
        bool atNext(const iterator<TYPE>* other) const override;

        /**
         * @brief Advances to the next position in the container
         */
        void next() const override;

        /**
         * @brief Retreats to the previous position in the container
         * @details Always throws unSupportedMethodError since backward movement is not supported for step iterators.
         */
        void prev() const override;

        /**
         * @brief Moves forward by a specified number of steps
         * @param steps The number of positions to move forward
         */
        void operator+=(integer steps) const override;

        /**
         * @brief Moves backward by a specified number of steps
         * @param steps The number of positions to move backward
         * @details This operation is supported by internally calling `operator+=` with a negative value of `steps`.
         */
        void operator-=(integer steps) const override;

        /**
         * @brief Calculates the number of steps between the current iterator and another iterator
         * @param other The iterator to compare with
         * @return The number of steps between the two iterators
         */
        integer operator-(const iterator<TYPE>& other) const override;

        /**
         * @brief Creates a new iterator pointing to the next element
         * @return A new stepIterator pointing to the next element
         */
        stepIterator* getNext() const override;

        /**
         * @brief Creates a new iterator pointing to the previous element
         * @return Always throws unSupportedMethodError for backward iteration
         */
        iterator<TYPE>* getPrev() const override;

        /**
         * @brief Gets the reference of current element in the container
         * @return A reference to the current element
         */
        TYPE& get() override;

        /**
         * @brief Gets the current element in the container
         * @return A constant reference to the current element
         */
        TYPE get() const override;

        /**
         * @brief Sets the value of the current element
         * @param data The value to set to the current element
         */
        void set(const TYPE& data) override;

        /**
         * @brief Checks if the iterator is valid
         * @return True if the iterator is valid (points to a valid element), otherwise false
         */
        [[nodiscard]] bool isValid() const override;

        /**
         * @brief Returns the class name of the iterator
         * @return The class name as a string
         */
        [[nodiscard]] std::string className() const override;
    };
}

    template <typename TYPE>
    original::stepIterator<TYPE>::stepIterator(wrapper<TYPE>* ptr)
        : _ptr(ptr) {}

    template <typename TYPE>
    auto original::stepIterator<TYPE>::ptrDistance(const stepIterator* start, const stepIterator* end) -> integer
    {
        auto s = ownerPtr(start->clone());
        auto e = ownerPtr(end->clone());
        integer dis = 0;
        while (s->isValid()){
            if (s->_ptr == e->_ptr){
                return dis;
            }
            dis += 1;
            s->next();
        }
        if (e->isValid()){
            dis = (std::numeric_limits<integer>::max)();
        }
        return dis;
    }

    template<typename TYPE>
    auto original::stepIterator<TYPE>::equalPtr(const iterator<TYPE> *other) const -> bool {
        auto* other_it = dynamic_cast<const stepIterator*>(other);
        return other_it != nullptr && _ptr == other_it->_ptr;
    }

    template <typename TYPE>
    original::stepIterator<TYPE>::stepIterator(const stepIterator& other)
        : _ptr(nullptr)
    {
        this->operator=(other);
    }

    template <typename TYPE>
    auto original::stepIterator<TYPE>::operator=(
        const stepIterator& other) -> stepIterator&
    {
        if (this == &other) {
            return *this;
        }
        this->_ptr = other._ptr;
        return *this;
    }

    template<typename TYPE>
    auto original::stepIterator<TYPE>::clone() const -> stepIterator* {
        return new stepIterator(*this);
    }

    template <typename TYPE>
    auto original::stepIterator<TYPE>::hasNext() const -> bool
    {
        return this->isValid();
    }

    template<typename TYPE>
    auto original::stepIterator<TYPE>::hasPrev() const -> bool {
        throw unSupportedMethodError();
    }

    template <typename TYPE>
    auto original::stepIterator<TYPE>::atPrev(const iterator<TYPE>*) const -> bool
    {
        throw unSupportedMethodError();
    }

    template <typename TYPE>
    auto original::stepIterator<TYPE>::atNext(const iterator<TYPE>*) const -> bool
    {
        throw unSupportedMethodError();
    }

    template <typename TYPE>
    void original::stepIterator<TYPE>::next() const
    {
        if (!this->isValid()) throw nullPointerError();
        this->_ptr = this->_ptr->getPNext();
    }

    template<typename TYPE>
    auto original::stepIterator<TYPE>::prev() const -> void {
        throw unSupportedMethodError();
    }

    template <typename TYPE>
    auto original::stepIterator<TYPE>::operator+=(const integer steps) const -> void
    {
        if (steps < 0) {
            this->operator-=(abs(steps));
            return;
        }

        for (integer i = 0; i < steps; i++)
        {
            this->next();
        }
    }

    template <typename TYPE>
    auto original::stepIterator<TYPE>::operator-=(const integer steps) const -> void
    {
        if (steps < 0) {
            this->operator+=(abs(steps));
            return;
        }

        for (integer i = 0; i < steps; i++)
        {
            this->prev();
        }
    }

    template <typename TYPE>
    auto original::stepIterator<TYPE>::operator-(const iterator<TYPE>& other) const -> integer
    {
        auto* other_it = dynamic_cast<const stepIterator*>(&other);
        if (other_it == nullptr)
            return this > &other ?
                (std::numeric_limits<integer>::max)() :
                (std::numeric_limits<integer>::min)();
        if (const integer pos_dis = ptrDistance(other_it, this);
            pos_dis != (std::numeric_limits<integer>::max)()) return pos_dis;
        if (const integer neg_dis = ptrDistance(this, other_it);
            neg_dis != (std::numeric_limits<integer>::max)()) return -neg_dis;
        return this->_ptr > other_it->_ptr ?
            (std::numeric_limits<integer>::max)() :
            (std::numeric_limits<integer>::min)();
    }

    template <typename TYPE>
    auto original::stepIterator<TYPE>::getNext() const -> stepIterator* {
        return new stepIterator(this->_ptr->getPNext());
    }

    template<typename TYPE>
    auto original::stepIterator<TYPE>::getPrev() const -> iterator<TYPE>* {
        throw unSupportedMethodError();
    }

    template <typename TYPE>
    auto original::stepIterator<TYPE>::get() -> TYPE&
    {
        if (!this->isValid()) throw nullPointerError();
        return this->_ptr->getVal();
    }

    template <typename TYPE>
    auto original::stepIterator<TYPE>::get() const -> TYPE
    {
        if (!this->isValid()) throw nullPointerError();
        return this->_ptr->getVal();
    }

    template <typename TYPE>
    auto original::stepIterator<TYPE>::set(const TYPE& data) -> void
    {
        if (!this->isValid()) throw nullPointerError();
        this->_ptr->setVal(data);
    }

    template <typename TYPE>
    auto original::stepIterator<TYPE>::isValid() const -> bool
    {
        return this->_ptr != nullptr;
    }

    template <typename TYPE>
    auto original::stepIterator<TYPE>::className() const -> std::string
    {
        return "stepIterator";
    }

#endif //STEPITERATOR_H
