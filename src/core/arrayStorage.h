#ifndef ARRAYSTORAGE_H
#define ARRAYSTORAGE_H

#include "allocator.h"
#include "error.h"
#include "types.h"

namespace original {
    /**
     * @file arrayStorage.h
     * @brief Non-iterable fixed-size contiguous storage.
     * @details Provides allocator-backed storage for fixed-size arrays without
     *          exposing iterator, stream, printable, comparable, or hashable
     *          interfaces. This is suitable for implementation details whose
     *          element types should not appear in public iterable APIs.
     */

    template<typename TYPE, typename ALLOC = allocator<TYPE>>
    class arrayStorage final {
        ALLOC allocator_;
        TYPE* body_;
        u_integer size_;

        void init(u_integer size);
        void destroy() noexcept;

    public:
        explicit arrayStorage(u_integer size = 0, ALLOC alloc = ALLOC{});
        arrayStorage(const arrayStorage& other);
        arrayStorage& operator=(const arrayStorage& other);
        arrayStorage(arrayStorage&& other) noexcept;
        arrayStorage& operator=(arrayStorage&& other) noexcept;

        void swap(arrayStorage& other) noexcept;

        [[nodiscard]] u_integer size() const noexcept;
        [[nodiscard]] bool empty() const noexcept;
        TYPE* data() noexcept;
        const TYPE* data() const noexcept;

        TYPE& operator[](u_integer index) noexcept;
        const TYPE& operator[](u_integer index) const noexcept;

        TYPE get(u_integer index) const;
        void set(u_integer index, const TYPE& e);

        ~arrayStorage();
    };
}

template<typename TYPE, typename ALLOC>
void original::arrayStorage<TYPE, ALLOC>::init(const u_integer size)
{
    this->size_ = size;
    this->body_ = this->allocator_.allocate(size);

    u_integer constructed = 0;
    try
    {
        for (; constructed < this->size_; constructed += 1)
        {
            this->allocator_.construct(&this->body_[constructed]);
        }
    }
    catch (...)
    {
        for (u_integer i = 0; i < constructed; i += 1)
        {
            ALLOC::destroy(&this->body_[i]);
        }
        this->allocator_.deallocate(this->body_, this->size_);
        this->body_ = nullptr;
        this->size_ = 0;
        throw;
    }
}

template<typename TYPE, typename ALLOC>
void original::arrayStorage<TYPE, ALLOC>::destroy() noexcept
{
    if (this->body_)
    {
        for (u_integer i = 0; i < this->size_; i += 1)
        {
            ALLOC::destroy(&this->body_[i]);
        }
        this->allocator_.deallocate(this->body_, this->size_);
        this->body_ = nullptr;
    }
    this->size_ = 0;
}

template<typename TYPE, typename ALLOC>
original::arrayStorage<TYPE, ALLOC>::arrayStorage(const u_integer size, ALLOC alloc)
    : allocator_(std::move(alloc)), body_(nullptr), size_(0)
{
    this->init(size);
}

template<typename TYPE, typename ALLOC>
original::arrayStorage<TYPE, ALLOC>::arrayStorage(const arrayStorage& other)
    : arrayStorage(other.size_)
{
    this->operator=(other);
}

template<typename TYPE, typename ALLOC>
auto original::arrayStorage<TYPE, ALLOC>::operator=(const arrayStorage& other) -> arrayStorage&
{
    if (this == &other)
    {
        return *this;
    }

    this->destroy();
    if constexpr (ALLOC::propagate_on_container_copy_assignment::value)
    {
        this->allocator_ = other.allocator_;
    }
    this->init(other.size_);
    for (u_integer i = 0; i < this->size_; i += 1)
    {
        this->set(i, other.get(i));
    }
    return *this;
}

template<typename TYPE, typename ALLOC>
original::arrayStorage<TYPE, ALLOC>::arrayStorage(arrayStorage&& other) noexcept
    : allocator_(std::move(other.allocator_)), body_(other.body_), size_(other.size_)
{
    other.body_ = nullptr;
    other.size_ = 0;
}

template<typename TYPE, typename ALLOC>
auto original::arrayStorage<TYPE, ALLOC>::operator=(arrayStorage&& other) noexcept -> arrayStorage&
{
    if (this == &other)
    {
        return *this;
    }

    this->destroy();
    if constexpr (ALLOC::propagate_on_container_move_assignment::value)
    {
        this->allocator_ = std::move(other.allocator_);
    }
    this->body_ = other.body_;
    this->size_ = other.size_;
    other.body_ = nullptr;
    other.size_ = 0;
    return *this;
}

template<typename TYPE, typename ALLOC>
void original::arrayStorage<TYPE, ALLOC>::swap(arrayStorage& other) noexcept
{
    if (this == &other)
    {
        return;
    }

    std::swap(this->size_, other.size_);
    std::swap(this->body_, other.body_);
    if constexpr (ALLOC::propagate_on_container_swap::value)
    {
        std::swap(this->allocator_, other.allocator_);
    }
}

template<typename TYPE, typename ALLOC>
auto original::arrayStorage<TYPE, ALLOC>::size() const noexcept -> u_integer
{
    return this->size_;
}

template<typename TYPE, typename ALLOC>
bool original::arrayStorage<TYPE, ALLOC>::empty() const noexcept
{
    return this->size_ == 0;
}

template<typename TYPE, typename ALLOC>
TYPE* original::arrayStorage<TYPE, ALLOC>::data() noexcept
{
    return this->body_;
}

template<typename TYPE, typename ALLOC>
const TYPE* original::arrayStorage<TYPE, ALLOC>::data() const noexcept
{
    return this->body_;
}

template<typename TYPE, typename ALLOC>
TYPE& original::arrayStorage<TYPE, ALLOC>::operator[](const u_integer index) noexcept
{
    return this->body_[index];
}

template<typename TYPE, typename ALLOC>
const TYPE& original::arrayStorage<TYPE, ALLOC>::operator[](const u_integer index) const noexcept
{
    return this->body_[index];
}

template<typename TYPE, typename ALLOC>
TYPE original::arrayStorage<TYPE, ALLOC>::get(const u_integer index) const
{
    if constexpr (std::is_copy_constructible_v<TYPE>)
    {
        return this->body_[index];
    }
    else if constexpr (std::is_move_constructible_v<TYPE>)
    {
        return std::move(this->body_[index]);
    }
    else
    {
        staticError<unSupportedMethodError, !std::is_copy_constructible_v<TYPE> && !std::is_move_constructible_v<TYPE>>::asserts();
        return TYPE{};
    }
}

template<typename TYPE, typename ALLOC>
void original::arrayStorage<TYPE, ALLOC>::set(const u_integer index, const TYPE& e)
{
    if constexpr (std::is_copy_assignable_v<TYPE>)
    {
        this->body_[index] = e;
    }
    else if constexpr (std::is_move_assignable_v<TYPE>)
    {
        this->body_[index] = std::move(const_cast<TYPE&>(e));
    }
    else
    {
        staticError<unSupportedMethodError, !std::is_copy_assignable_v<TYPE> && !std::is_move_assignable_v<TYPE>>::asserts();
    }
}

template<typename TYPE, typename ALLOC>
original::arrayStorage<TYPE, ALLOC>::~arrayStorage()
{
    this->destroy();
}

#endif //ARRAYSTORAGE_H
