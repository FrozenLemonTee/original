#ifndef ORIGINAL_ARRAYVIEW_H
#define ORIGINAL_ARRAYVIEW_H
#include "error.h"


namespace original {
    template<typename TYPE>
    class arrayView {
        TYPE* const data_;
        const u_integer count_;

    public:
        class iterator {
            TYPE* data_;

            explicit iterator(TYPE* data);
        public:
            friend arrayView;

            using iterator_category = std::random_access_iterator_tag;
            using value_type = TYPE;
            using difference_type = std::ptrdiff_t;
            using pointer = TYPE*;
            using reference = TYPE&;

            iterator& operator++();

            TYPE& operator*();

            TYPE* operator->();

            bool operator!=(const iterator& other) const;

            bool operator==(const iterator& other) const;

            iterator& operator+=(difference_type offset);

            iterator& operator-=(difference_type offset);

            iterator operator+(difference_type offset) const;

            std::size_t operator-(const iterator& other) const;
        };

        arrayView();

        arrayView(const arrayView&) = default;
        arrayView& operator=(const arrayView&) = delete;
        arrayView(arrayView&&) noexcept = default;
        arrayView& operator=(arrayView&&) noexcept = delete;

        arrayView(TYPE* data, u_integer count);

        template<u_integer N>
        explicit constexpr arrayView(TYPE (&arr)[N]) noexcept;

        explicit constexpr arrayView(TYPE (&arr)[0]) noexcept;

        TYPE& operator[](u_integer index);

        const TYPE& operator[](u_integer index) const;

        TYPE& get(u_integer index);

        const TYPE& get(u_integer index) const;

        const TYPE* data() const;

        [[nodiscard]] u_integer count() const;

        [[nodiscard]] bool empty() const;

        iterator begin();

        iterator end();

        arrayView subview(u_integer start, u_integer count) const;

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
original::arrayView<TYPE> original::arrayView<TYPE>::subview(u_integer start, u_integer count) const
{
    if (start + count > this->count_)
        throw outOfBoundError(printable::formatStrings("Subview range [", start,
                              ", ", start + count,
                              "] out of bounds [0, ", this->count_, "]."));

    return arrayView{const_cast<TYPE*>(this->data()) + start, count};
}

#endif //ORIGINAL_ARRAYVIEW_H
