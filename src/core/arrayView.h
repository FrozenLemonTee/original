#ifndef ORIGINAL_ARRAYVIEW_H
#define ORIGINAL_ARRAYVIEW_H
#include "error.h"


namespace original {
    /**
     * @class arrayView
     * @tparam TYPE Type of the elements viewed by the arrayView
     * @brief A lightweight, non-owning view over a contiguous sequence of elements.
     * @details The `arrayView` class provides a uniform interface for accessing array-like data structures
     *          including built-in arrays and other contiguous containers. It does not own the data and
     *          provides read and write access to the underlying elements through a safe, bounds-checked interface.
     *
     *          This class supports iteration, slicing, and element access operations, making it suitable for
     *          functions that need to work with different array-like types without copying data.
     */
    template<typename TYPE>
    class arrayView {
        TYPE* const data_;     ///< Pointer to the viewed data
        const u_integer count_; ///< Number of elements in the view

    public:
        /**
         * @class iterator
         * @brief Random access iterator for arrayView
         * @details Provides random access iteration over the elements of an arrayView.
         *          Supports standard iterator operations including increment, decrement,
         *          and pointer arithmetic.
         */
        class iterator {
            TYPE* data_; ///< Current pointer position

            /**
             * @brief Constructs an iterator pointing to the specified data
             * @param data Pointer to the element
             */
            explicit iterator(TYPE* data);
        public:
            friend arrayView;

            using iterator_category = std::random_access_iterator_tag; ///< Iterator category
            using value_type = TYPE;              ///< Type of elements
            using difference_type = std::ptrdiff_t; ///< Difference type
            using pointer = TYPE*;                ///< Pointer type
            using reference = TYPE&;              ///< Reference type

            /**
             * @brief Pre-increment operator
             * @return Reference to this iterator after incrementing
             */
            iterator& operator++();

            /**
             * @brief Dereference operator
             * @return Reference to the current element
             */
            TYPE& operator*();

            /**
             * @brief Arrow operator
             * @return Pointer to the current element
             */
            TYPE* operator->();

            /**
             * @brief Inequality comparison
             * @param other Iterator to compare with
             * @return true if iterators point to different elements
             */
            bool operator!=(const iterator& other) const;

            /**
             * @brief Equality comparison
             * @param other Iterator to compare with
             * @return true if iterators point to the same element
             */
            bool operator==(const iterator& other) const;

            /**
             * @brief Compound addition assignment
             * @param offset Number of elements to advance
             * @return Reference to this iterator
             */
            iterator& operator+=(difference_type offset);

            /**
             * @brief Compound subtraction assignment
             * @param offset Number of elements to retreat
             * @return Reference to this iterator
             */
            iterator& operator-=(difference_type offset);

            /**
             * @brief Addition operator
             * @param offset Number of elements to advance
             * @return New iterator at the advanced position
             */
            iterator operator+(difference_type offset) const;

            /**
             * @brief Difference operator
             * @param other Iterator to subtract from this one
             * @return Number of elements between the two iterators
             */
            std::size_t operator-(const iterator& other) const;
        };

        /**
         * @brief Constructs an empty arrayView
         */
        arrayView();

        /**
         * @brief Copy constructor (default)
         */
        arrayView(const arrayView&) = default;

        /**
         * @brief Copy assignment operator (deleted)
         */
        arrayView& operator=(const arrayView&) = delete;

        /**
         * @brief Move constructor (default)
         */
        arrayView(arrayView&&) noexcept = default;

        /**
         * @brief Move assignment operator (deleted)
         */
        arrayView& operator=(arrayView&&) noexcept = delete;

        /**
         * @brief Constructs an arrayView from a pointer and count
         * @param data Pointer to the data
         * @param count Number of elements in the data
         */
        arrayView(TYPE* data, u_integer count);

        /**
         * @brief Constructs an arrayView from a built-in array
         * @tparam N Size of the array
         * @param arr Reference to the built-in array
         */
        template<u_integer N>
        explicit constexpr arrayView(TYPE (&arr)[N]) noexcept;

        /**
         * @brief Constructs an arrayView from a zero-sized built-in array
         * @param arr Reference to the zero-sized array
         */
        explicit constexpr arrayView(TYPE (&arr)[0]) noexcept;

        /**
         * @brief Subscript operator for element access
         * @param index Index of the element to access
         * @return Reference to the element at the specified index
         * @throws outOfBoundError if index is out of bounds
         */
        TYPE& operator[](u_integer index);

        /**
         * @brief Const subscript operator for element access
         * @param index Index of the element to access
         * @return Const reference to the element at the specified index
         * @throws outOfBoundError if index is out of bounds
         */
        const TYPE& operator[](u_integer index) const;

        /**
         * @brief Bounds-checked element access
         * @param index Index of the element to access
         * @return Reference to the element at the specified index
         * @throws outOfBoundError if index is out of bounds
         */
        TYPE& get(u_integer index);

        /**
         * @brief Const bounds-checked element access
         * @param index Index of the element to access
         * @return Const reference to the element at the specified index
         * @throws outOfBoundError if index is out of bounds
         */
        const TYPE& get(u_integer index) const;

        /**
         * @brief Returns a pointer to the underlying data
         * @return Const pointer to the data
         */
        const TYPE* data() const;

        /**
         * @brief Returns the number of elements in the view
         * @return Number of elements
         */
        [[nodiscard]] u_integer count() const;

        /**
         * @brief Checks if the view is empty
         * @return true if the view contains no elements
         */
        [[nodiscard]] bool empty() const;

        /**
         * @brief Returns an iterator to the first element
         * @return Iterator to the beginning
         */
        iterator begin();

        /**
         * @brief Returns an iterator to the element after the last
         * @return Iterator to the end
         */
        iterator end();

        /**
         * @brief Creates a subview of this view
         * @param start Starting index of the subview
         * @param count Number of elements in the subview
         * @return New arrayView covering the specified range
         * @throws outOfBoundError if the range is invalid
         */
        arrayView subview(u_integer start, u_integer count) const;

        /**
         * @brief Creates a slice of this view from start to end index
         * @param start Starting index of the slice (inclusive)
         * @param end Ending index of the slice (exclusive)
         * @return New arrayView covering the specified range [start, end)
         * @throws outOfBoundError if start or end is out of bounds [0, count()]
         *
         * @details The slice method creates a new arrayView that represents a contiguous
         *          subsequence of the original view. The range includes the element at
         *          the start index and excludes the element at the end index, following
         *          the common [start, end) half-open interval convention.
         *
         *          If start >= end, an empty arrayView is returned.
         *          If start or end exceeds the view's bounds, an outOfBoundError is thrown.
         *
         * @example
         * arrayView<int> view = ...; // view with elements [0, 1, 2, 3, 4]
         * auto slice1 = view.slice(1, 4); // contains [1, 2, 3]
         * auto slice2 = view.slice(2, 2); // empty view
         * auto slice3 = view.slice(0, 5); // entire view
         */
        arrayView slice(u_integer start, u_integer end) const;

        /**
         * @brief Destructor (default)
         */
        ~arrayView() = default;
    };
}

template <typename TYPE>
original::arrayView<TYPE>::iterator::iterator(TYPE* data) : data_(data) {}

template <typename TYPE>
original::arrayView<TYPE>::iterator&
original::arrayView<TYPE>::iterator::operator++()
{
    ++this->data_;
    return *this;
}

template <typename TYPE>
TYPE& original::arrayView<TYPE>::iterator::operator*()
{
    return *this->data_;
}

template <typename TYPE>
TYPE* original::arrayView<TYPE>::iterator::operator->()
{
    return this->data_;
}

template <typename TYPE>
bool original::arrayView<TYPE>::iterator::operator!=(const iterator& other) const
{
    return this->data_ != other.data_;
}

template <typename TYPE>
bool original::arrayView<TYPE>::iterator::operator==(const iterator& other) const
{
    return this->data_ == other.data_;
}

template <typename TYPE>
original::arrayView<TYPE>::iterator&
original::arrayView<TYPE>::iterator::operator+=(difference_type offset)
{
    this->data_ += offset;
    return *this;
}

template <typename TYPE>
original::arrayView<TYPE>::iterator&
original::arrayView<TYPE>::iterator::operator-=(difference_type offset)
{
    this->data_ -= offset;
    return *this;
}

template <typename TYPE>
original::arrayView<TYPE>::iterator
original::arrayView<TYPE>::iterator::operator+(difference_type offset) const
{
    auto front = *this;
    front += offset;
    return front;
}

template <typename TYPE>
std::size_t
original::arrayView<TYPE>::iterator::operator-(const iterator& other) const
{
    return static_cast<std::size_t>(this->data_ - other.data_);
}

template <typename TYPE>
original::arrayView<TYPE>::arrayView() : data_(nullptr), count_(0) {}

template <typename TYPE>
original::arrayView<TYPE>::arrayView(TYPE* data, const u_integer count) : data_(data), count_(count) {}

template <typename TYPE>
template <original::u_integer N>
constexpr original::arrayView<TYPE>::arrayView(TYPE(& arr)[N]) noexcept : data_(arr), count_(N) {}

template <typename TYPE>
constexpr original::arrayView<TYPE>::arrayView(TYPE(& arr)[0]) noexcept : data_(arr), count_(0) {}

template <typename TYPE>
TYPE& original::arrayView<TYPE>::operator[](u_integer index)
{
    return this->data_[index];
}

template <typename TYPE>
const TYPE& original::arrayView<TYPE>::operator[](u_integer index) const
{
    return this->data_[index];
}

template <typename TYPE>
TYPE& original::arrayView<TYPE>::get(u_integer index)
{
    if (index >= this->count_)
        throw outOfBoundError("Index " + printable::formatString(index) +
                                  " out of bound max index " + printable::formatString(this->count() - 1) + ".");
    return this->data_[index];
}

template <typename TYPE>
const TYPE& original::arrayView<TYPE>::get(u_integer index) const
{
    if (index >= this->count_)
        throw outOfBoundError("Index " + printable::formatString(index) +
                                  " out of bound max index " + printable::formatString(this->count() - 1) + ".");
    return this->data_[index];
}

template <typename TYPE>
const TYPE* original::arrayView<TYPE>::data() const
{
    return this->data_;
}

template <typename TYPE>
original::u_integer original::arrayView<TYPE>::count() const
{
    return this->count_;
}

template <typename TYPE>
bool original::arrayView<TYPE>::empty() const
{
    return this->count_ == 0;
}

template <typename TYPE>
original::arrayView<TYPE>::iterator original::arrayView<TYPE>::begin()
{
    return iterator{this->data_};
}

template <typename TYPE>
original::arrayView<TYPE>::iterator original::arrayView<TYPE>::end()
{
    return iterator{this->data_ + this->count_};
}

template <typename TYPE>
original::arrayView<TYPE> original::arrayView<TYPE>::subview(const u_integer start, const u_integer count) const
{
    return this->slice(start, start + count);
}

template <typename TYPE>
original::arrayView<TYPE> original::arrayView<TYPE>::slice(const u_integer start, const u_integer end) const
{
    if (start > this->count() || end > this->count())
        throw outOfBoundError(
            printable::formatStrings("Slice range [", start, ":", end,
                                     "] out of bounds [0:", this->count(), "]."));

    if (start >= end)
        return arrayView{};

    return arrayView{this->data_ + start, end - start};
}

#endif //ORIGINAL_ARRAYVIEW_H
