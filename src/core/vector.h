#ifndef VECTOR_H
#define VECTOR_H

/**
 * @file vector.h
 * @brief Dynamic array container with automatic resizing
 * @details Implements a vector container with efficient operations
 * and auto-centering memory management. Supports random access and iterator-based traversal.
 */

#include "baseList.h"
#include "iterationStream.h"
#include "array.h"

namespace original {

    /**
     * @class vector
     * @tparam TYPE Element type stored in the vector
     * @tparam ALLOC Allocator type for memory management (default: allocator<TYPE>)
     * @brief Dynamic array container with amortized constant time operations
     * @extends baseList
     * @extends iterationStream
     * @details Features include:
     * - Auto-resizing with centered memory allocation
     * - Random access via operator[] with bounds checking
     * - Bidirectional element insertion/removal
     * - STL-style iterator support
     * - Initializer list and array conversion
     *
     * Memory management characteristics:
     * - Uses the provided allocator for all memory operations
     * - Grows exponentially (2x) when capacity is exceeded
     * - Maintains centered memory layout for efficient front/back operations
     * - Allocator propagation follows C++ standard rules
     *
     * The ALLOC type must meet C++ allocator requirements and provide:
     * - allocate()/deallocate() methods
     * - construct()/destroy() methods
     * - propagate_on_container_copy_assignment/move_assignment typedefs
     */
    template <typename TYPE, typename ALLOC = allocator<TYPE>>
    class vector final : public baseList<TYPE, ALLOC>, public iterationStream<TYPE, vector<TYPE, ALLOC>> {
        u_integer size_;                 ///< Current number of elements
        static constexpr u_integer INNER_SIZE_INIT = 16; ///< Initial buffer capacity
        u_integer max_size;              ///< Current buffer capacity
        u_integer inner_begin;           ///< Starting index in circular buffer
        TYPE* body;                     ///< Internal storage buffer

        // ==================== Private Methods ====================

        /**
         * @brief Initializes the vector with default settings.
         */
        void vectorInit();

        /**
         * @brief Destroys and deallocates the internal buffer using the vector's allocator
         * @details Performs the following operations in sequence:
         * 1. Destroys all constructed elements in reverse order (from last to first)
         *    using the allocator's `destroy()` method
         * 2. Deallocates the raw memory buffer using the allocator's `deallocate()`
         *
         * @note Key characteristics:
         * - No-throw guarantee: Marked noexcept as vector must remain destructible even if
         *   element destructors throw (though this may lead to resource leaks)
         * - Idempotent: Safe to call multiple times (checks nullptr)
         * - Preserves allocator state: The allocator object itself remains valid after destruction
         *
         * @warning If TYPE's destructor throws, some elements may not be destroyed properly.
         *          This is a trade-off for maintaining noexcept guarantee.
         *
         * @invariant After execution:
         * - `body` pointer is set to nullptr (implicitly, via de-allocation)
         * - Allocator remains in valid state
         * - No memory leaks occur for properly destructible elements
         */
        void vectorArrayDestroy() noexcept;

        /**
         * @brief Allocates and constructs a new array
         * @param size Number of elements to allocate
         * @return Pointer to allocated memory
         * @details Uses the vector's allocator to:
         * - Allocate raw memory
         * - Default-construct elements
         */
        TYPE* vectorArrayInit(u_integer size);

        /**
         * @brief Gets an element from the internal buffer with proper copy/move semantics
         * @param pos Position in the internal buffer
         * @return The element at the specified position
         */
        TYPE getElem(integer pos) const;

        /**
         * @brief Sets an element in the internal buffer with proper assignment semantics
         * @param pos Position in the internal buffer
         * @param e Element to set
         */
        void setElem(integer pos, const TYPE &e);

        /**
         * @brief Sets an element in a given buffer with proper assignment semantics
         * @param buf Target buffer
         * @param pos Position in the buffer
         * @param e Element to set
         */
        static void setBufElem(TYPE* buf, integer pos, const TYPE& e);

        /**
         * @brief Moves elements from the old buffer to the new buffer.
         * @param old_body The original array to move elements from.
         * @param inner_idx The starting index for moving.
         * @param len The number of elements to move.
         * @param new_body The new buffer to move elements to.
         * @param offset The offset to apply when moving the elements.
         */
        static void moveElements(TYPE* old_body, u_integer inner_idx,
                                 u_integer len, TYPE* new_body, integer offset);

        /**
         * @brief Converts an index to an inner buffer index.
         * @param index The index to convert.
         * @return The corresponding index in the internal buffer.
         */
        [[nodiscard]] u_integer toInnerIdx(integer index) const;

        /**
         * @brief Checks if a given increment would exceed the inner buffer size of the vector.
         * @param increment The increment to check.
         * @return True if the increment would exceed max size, false otherwise.
         */
        [[nodiscard]] bool outOfMaxSize(u_integer increment) const;

        /**
         * @brief Expands the vector's storage capacity
         * @param new_size New capacity (must be > current size)
         * @details Performs the following steps:
         * 1. Allocates new storage using the allocator
         * 2. Moves existing elements to new storage
         * 3. Destroys old elements and deallocates old storage
         * @exception Strong guarantee - original content preserved on failure
         */
        void grow(u_integer new_size);

        /**
         * @brief Adjusts the vector's internal buffer to accommodate an increment in size.
         * @param increment The number of elements to accommodate.
         */
        void adjust(u_integer increment);

        /**
         * @internal
         * @brief Internal base constructor for delegation purposes.
         * @param size The nominal element count for capacity calculation
         * @param alloc Allocator instance for memory management
         * @details This constructor:
         * - Only sets basic size/capacity parameters (size_, max_size, inner_begin)
         * - Does NOT allocate memory or initialize elements
         * - Serves as delegation target for other constructors
         *
         * @warning Not for direct use - the vector remains partially initialized after construction.
         *          Other constructors must complete initialization by:
         *          1. Allocating storage (body pointer)
         *          2. Constructing elements
         *          3. Setting proper inner_begin offset
         *
         * @note Capacity is pre-calculated as size * 4/3 to optimize for future growth,
         *       but actual memory allocation happens in delegated constructors.
         */
        explicit vector(u_integer size, ALLOC alloc = ALLOC{});

    public:

        // ==================== Iterator Class ====================

        /**
         * @class Iterator
         * @brief Random access iterator implementation for vector
         * @extends randomAccessIterator
         * @details Provides full random access capabilities including:
         * - Pointer arithmetic operations
         * - Element dereferencing
         * - Comparison operators
         */
        class Iterator final : public randomAccessIterator<TYPE, ALLOC> {
            /**
             * @brief Constructs an iterator for the vector.
             * @param ptr Pointer to the element in the vector that the iterator will point to.
             * @param container Pointer to the container (vector) the iterator belongs to.
             * @param pos The position of the iterator within the container.
             */
            explicit Iterator(TYPE* ptr, const vector* container, integer pos);

        public:
            friend vector;

            /**
             * @brief Copy constructor for the iterator.
             * @param other The iterator to copy.
             */
            Iterator(const Iterator& other);

            /**
             * @brief Assignment operator for the iterator.
             * @param other The iterator to assign from.
             * @return A reference to this iterator.
             */
            Iterator& operator=(const Iterator& other);

            /**
             * @brief Clones the iterator.
             * @return A new iterator pointing to the same position.
             */
            Iterator* clone() const override;

            /**
             * @brief Checks if the iterator is at the previous element relative to another iterator.
             * @param other The other iterator to compare to.
             * @return True if this iterator is at the previous element, false otherwise.
             */
            bool atPrev(const iterator<TYPE> *other) const override;

            /**
             * @brief Checks if the iterator is at the next element relative to another iterator.
             * @param other The other iterator to compare to.
             * @return True if this iterator is at the next element, false otherwise.
             */
            bool atNext(const iterator<TYPE> *other) const override;

            /**
             * @brief Gets the class name of the iterator.
             * @return The class name as a string.
             */
            [[nodiscard]] std::string className() const override;
        };

        // ==================== Constructors and Destructor ====================

        /**
         * @brief Constructs a vector with specified allocator
         * @param alloc Allocator instance to use (default: default-constructed ALLOC)
         * @details Initializes the vector with default capacity (16 elements).
         * The allocator will be used for all subsequent memory operations.
         */
        explicit vector(ALLOC alloc = ALLOC{});

        /**
         * @brief Constructs a vector with specified size and allocator, initializing elements with provided arguments.
         * @tparam ARGS Variadic template parameter pack for element construction arguments
         * @param size The initial number of elements in the vector
         * @param alloc Allocator instance to use (default: default-constructed ALLOC)
         * @param args Arguments to forward to element constructors
         * @details This constructor:
         * - Allocates storage for the specified number of elements
         * - Constructs each element using the provided arguments
         * - Maintains centered memory layout for efficient operations
         *
         * @note The vector's capacity will be automatically calculated to maintain optimal memory layout.
         * All elements will be initialized with the same provided arguments.
         */
        template<typename... ARGS>
        vector(u_integer size, ALLOC alloc, ARGS&&... args);

        /**
         * @brief Copy constructor for the vector.
         * @param other The vector to copy from.
         */
        vector(const vector& other);

        /**
         * @brief Constructs a vector from an initializer list.
         * @param list The initializer list to construct the vector from.
         */
        vector(const std::initializer_list<TYPE>& list);

        /**
         * @brief Constructs a vector from an arrayView
         * @param view The arrayView to copy elements from
         * @param alloc Allocator instance to use for memory management
         * @details This constructor creates a vector by copying elements from an arrayView.
         *          The size of the created vector will be equal to the count of the arrayView,
         *          and all elements will be copied from the view.
         *          This allows seamless conversion between arrayView and vector types.
         */
        explicit vector(arrayView<TYPE> view, ALLOC alloc = ALLOC{});

        /**
         * @brief Constructs a vector from an array.
         * @param arr The array to construct the vector from.
         */
        explicit vector(const array<TYPE>& arr);

        /**
         * @brief Assignment operator for the vector.
         * @param other The vector to assign from.
         * @return A reference to this vector.
         */
        vector& operator=(const vector& other);

        /**
         * @brief Move constructor for the vector.
         * @param other The vector to move from.
         */
        vector(vector&& other) noexcept;

        /**
         * @brief Move assignment operator for the vector.
         * @param other The vector to move from.
         * @return A reference to this vector.
         */
        vector& operator=(vector&& other) noexcept;

        /**
         * @brief Destructor for the vector.
         */
        ~vector() override;

        /**
         * @brief Swaps the contents of two vectors
         * @param other Vector to swap with
         * @details Exchanges the size, capacity, internal buffer, and optionally the allocator between two vectors.
         *          This operation is noexcept and provides strong exception guarantee.
         */
        void swap(vector& other) noexcept;

        // ==================== Capacity Methods ====================

        /**
         * @brief Gets the size of the vector.
         * @return The number of elements in the vector.
         */
        [[nodiscard]] u_integer size() const override;

        /**
         * @brief Gets the current capacity of the vector.
         * @return The maximum number of elements that can be stored without reallocation.
         */
        [[nodiscard]] u_integer capacity() const noexcept;

        // ==================== Element Access ====================

        /**
         * @brief Gets a reference to the first element in the vector.
         * @return A reference to the first element.
         */
        TYPE& data() const;

        /**
         * @brief Returns a view over the entire vector
         * @return arrayView providing access to the vector elements
         * @details The returned view provides a lightweight, non-owning interface
         *          to the vector's elements. Changes to the vector may invalidate
         *          the view if reallocation occurs.
         */
        arrayView<TYPE> view();

        /**
         * @brief Returns a slice view of the vector
         * @param start Starting index of the slice (inclusive)
         * @param end Ending index of the slice (exclusive)
         * @return arrayView covering the specified range
         * @throws outOfBoundError if the range is invalid
         * @details The returned view provides access to elements in the range [start, end).
         *          If start >= end, returns an empty view.
         */
        arrayView<TYPE> slice(u_integer start, u_integer end);

        /**
         * @brief Returns a const view over the entire vector
         * @return const arrayView providing read-only access to the vector elements
         */
        arrayView<const TYPE> view() const;

        /**
         * @brief Returns a const slice view of the vector
         * @param start Starting index of the slice (inclusive)
         * @param end Ending index of the slice (exclusive)
         * @return const arrayView covering the specified range
         * @throws outOfBoundError if the range is invalid
         */
        arrayView<const TYPE> slice(u_integer start, u_integer end) const;

        /**
         * @brief Gets an element at the specified index.
         * @param index The index of the element.
         * @return The element at the specified index.
         */
        TYPE get(integer index) const override;

        /**
         * @brief Gets a reference to the element at the specified index.
         * @param index The index of the element.
         * @return A reference to the element at the specified index.
         */
        TYPE& operator[](integer index) override;

        // const version
        using serial<TYPE, ALLOC>::operator[];

        /**
         * @brief Sets the element at the specified index.
         * @param index The index of the element to set.
         * @param e The value to set.
         */
        void set(integer index, const TYPE &e) override;

        // ==================== Search Operations ====================

        /**
         * @brief Finds the index of the first occurrence of the specified element.
         * @param e The element to find.
         * @return The index of the element, or the size of the vector if not found.
         */
        u_integer indexOf(const TYPE &e) const override;

        // ==================== Insertion Operations ====================

        /**
         * @brief Inserts an element at the beginning of the vector.
         * @param e The element to insert.
         */
        void pushBegin(const TYPE &e) override;

        /**
         * @brief Inserts an element at the specified index in the vector.
         * @param index The index to insert the element at.
         * @param e The element to insert.
         */
        void push(integer index, const TYPE &e) override;

        /**
         * @brief Inserts an element at the end of the vector.
         * @param e The element to insert.
         */
        void pushEnd(const TYPE &e) override;

        // ==================== Removal Operations ====================

        /**
         * @brief Removes and returns the first element in the vector.
         * @return The removed element.
         */
        TYPE popBegin() override;

        /**
         * @brief Removes and returns the element at the specified index.
         * @param index The index of the element to remove.
         * @return The removed element.
         */
        TYPE pop(integer index) override;

        /**
         * @brief Removes and returns the last element in the vector.
         * @return The removed element.
         */
        TYPE popEnd() override;

        // ==================== Iterator Methods ====================

        /**
         * @brief Gets an iterator to the beginning of the vector.
         * @return An iterator to the beginning.
         */
        Iterator* begins() const override;

        /**
         * @brief Gets an iterator to the end of the vector.
         * @return An iterator to the end.
         */
        Iterator* ends() const override;

        // ==================== Utility Methods ====================

        /**
         * @brief Gets the class name of the vector.
         * @return The class name as a string.
         */
        [[nodiscard]] std::string className() const override;

        template<typename T, typename... ARGS>
        friend vector<T> makeVector(u_integer size, ARGS&&... args);
    };

    // ==================== Factory Function ====================

    /**
     * @brief Factory function to create a vector with specified size and construction arguments.
     * @tparam T Element type of the vector
     * @tparam ARGS Variadic template parameter pack for construction arguments
     * @param size Number of elements to create
     * @param args Arguments to forward to element constructors
     * @return A new vector instance
     * @details Creates a vector with:
     * - Default allocator
     * - Elements constructed with provided arguments
     * - Automatic capacity management
     *
     * @example
     * // Creates a vector of 10 default-constructed integers
     * auto vec = makeVector<int>(10);
     *
     * // Creates a vector of 5 strings initialized with "hello"
     * auto strVec = makeVector<std::string>(5, "hello");
     */
    template<typename T, typename... ARGS>
    vector<T> makeVector(u_integer size, ARGS&&... args);
}

namespace std {
    template<typename TYPE, typename ALLOC>
    void swap(original::vector<TYPE, ALLOC>& lhs, original::vector<TYPE, ALLOC>& rhs) noexcept; // NOLINT
}

    template <typename TYPE, typename ALLOC>
    auto original::vector<TYPE, ALLOC>::vectorInit() -> void
    {
        this->size_ = 0;
        this->max_size = INNER_SIZE_INIT;
        this->inner_begin = (INNER_SIZE_INIT - 1)/2;
        this->body = vector::vectorArrayInit(INNER_SIZE_INIT);
    }

    template <typename TYPE, typename ALLOC>
    auto original::vector<TYPE, ALLOC>::vectorArrayDestroy() noexcept -> void
    {
        if (this->body) {
            for (u_integer i = 0; i < this->max_size; ++i) {
                this->destroy(&this->body[i]);
            }
            this->deallocate(this->body, this->max_size);
            this->body = nullptr;
        }
    }

    template <typename TYPE, typename ALLOC>
    auto original::vector<TYPE, ALLOC>::vectorArrayInit(const u_integer size) -> TYPE* {
        auto arr = this->allocate(size);
        for (u_integer i = 0; i < size; i++) {
            this->construct(&arr[i]);
        }
        return arr;
    }

    template <typename TYPE, typename ALLOC>
    TYPE original::vector<TYPE, ALLOC>::getElem(integer pos) const
    {
        if constexpr (std::is_copy_constructible_v<TYPE>) {
            return this->body[pos];
        } else if constexpr (std::is_move_constructible_v<TYPE>) {
            return std::move(this->body[pos]);
        } else {
            staticError<unSupportedMethodError, !std::is_copy_constructible_v<TYPE> && !std::is_move_constructible_v<TYPE>>::asserts();
            return TYPE{};
        }
    }

    template <typename TYPE, typename ALLOC>
    void original::vector<TYPE, ALLOC>::setElem(const integer pos, const TYPE& e)
    {
        setBufElem(this->body, pos, e);
    }

    template <typename TYPE, typename ALLOC>
    void original::vector<TYPE, ALLOC>::setBufElem(TYPE* buf, integer pos, const TYPE& e)
    {
        if constexpr (std::is_copy_assignable_v<TYPE>) {
            buf[pos] = e;
        } else if constexpr (std::is_move_assignable_v<TYPE>) {
            buf[pos] = std::move(const_cast<TYPE&>(e));
        } else {
            staticError<unSupportedMethodError, !std::is_copy_constructible_v<TYPE> && !std::is_move_constructible_v<TYPE>>::asserts();
        }
    }

    template <typename TYPE, typename ALLOC>
    auto original::vector<TYPE, ALLOC>::moveElements(TYPE* old_body, const u_integer inner_idx,
                                              const u_integer len, TYPE* new_body, const integer offset) -> void{
        if (offset > 0)
        {
            for (u_integer i = 0; i < len; i += 1)
            {
                setBufElem(new_body, inner_idx + offset + len - 1 - i, old_body[inner_idx + len - 1 - i]);
            }
        }else
        {
            for (u_integer i = 0; i < len; i += 1)
            {
                setBufElem(new_body, inner_idx + offset + i, old_body[inner_idx + i]);
            }
        }
    }

    template <typename TYPE, typename ALLOC>
    auto original::vector<TYPE, ALLOC>::toInnerIdx(integer index) const -> u_integer
    {
        return this->inner_begin + index;
    }

    template <typename TYPE, typename ALLOC>
    auto original::vector<TYPE, ALLOC>::outOfMaxSize(u_integer increment) const -> bool
    {
        return this->inner_begin + this->size() + increment > this->max_size - 1 || static_cast<integer>(this->inner_begin) - static_cast<integer>(increment) < 0;
    }

    template <typename TYPE, typename ALLOC>
    auto original::vector<TYPE, ALLOC>::grow(const u_integer new_size) -> void
    {
        TYPE* new_body = vector::vectorArrayInit(new_size);
        u_integer new_begin = (new_size - 1) / 4;
        const integer offset = static_cast<integer>(new_begin) - static_cast<integer>(this->inner_begin);
        vector::moveElements(this->body, this->inner_begin,
                             this->size(), new_body, offset);
        this->vectorArrayDestroy();
        this->body = new_body;
        this->max_size = new_size;
        this->inner_begin = new_begin;
    }

    template <typename TYPE, typename ALLOC>
    auto original::vector<TYPE, ALLOC>::adjust(u_integer increment) -> void {
        if (!this->outOfMaxSize(increment)) {
            return;
        }
        u_integer new_begin = (this->max_size - this->size() - increment) / 2;
        if (this->max_size >= this->size_ + increment && new_begin > 0) {
            const integer offset = static_cast<integer>(new_begin) - static_cast<integer>(this->inner_begin);
            vector::moveElements(this->body, this->inner_begin, this->size(),
                                 this->body, offset);
            this->inner_begin = new_begin;
        } else {
            const u_integer new_max_size = (this->size() + increment) * 2;
            this->grow(new_max_size);
        }
    }

    template <typename TYPE, typename ALLOC>
    original::vector<TYPE, ALLOC>::Iterator::Iterator(TYPE* ptr, const vector* container, integer pos)
        : randomAccessIterator<TYPE, ALLOC>(ptr, container, pos) {}

    template <typename TYPE, typename ALLOC>
    original::vector<TYPE, ALLOC>::Iterator::Iterator(const Iterator& other)
        : randomAccessIterator<TYPE, ALLOC>(nullptr, nullptr, 0)
    {
        this->operator=(other);
    }

    template <typename TYPE, typename ALLOC>
    auto original::vector<TYPE, ALLOC>::Iterator::operator=(const Iterator& other) -> Iterator&
    {
        if (this == &other) {
            return *this;
        }
        randomAccessIterator<TYPE, ALLOC>::operator=(other);
        return *this;
    }

    template <typename TYPE, typename ALLOC>
    auto original::vector<TYPE, ALLOC>::Iterator::clone() const -> Iterator* {
        return new Iterator(*this);
    }

    template <typename TYPE, typename ALLOC>
    auto original::vector<TYPE, ALLOC>::Iterator::atPrev(const iterator<TYPE> *other) const -> bool {
        auto other_it = dynamic_cast<const Iterator*>(other);
        return this->_ptr + 1 == other_it->_ptr;
    }

    template <typename TYPE, typename ALLOC>
    auto original::vector<TYPE, ALLOC>::Iterator::atNext(const iterator<TYPE> *other) const -> bool {
        auto other_it = dynamic_cast<const Iterator*>(other);
        return other_it->_ptr + 1 == this->_ptr;
    }

    template <typename TYPE, typename ALLOC>
    auto original::vector<TYPE, ALLOC>::Iterator::className() const -> std::string {
        return "vector::Iterator";
    }

    template <typename TYPE, typename ALLOC>
    original::vector<TYPE, ALLOC>::vector(ALLOC alloc)
        : baseList<TYPE, ALLOC>(std::move(alloc)), size_(), max_size(), inner_begin()
    {
        this->vectorInit();
    }

    template<typename TYPE, typename ALLOC>
    template<typename... ARGS>
    original::vector<TYPE, ALLOC>::vector(u_integer size, ALLOC alloc, ARGS&&... args)
    : vector(size, std::move(alloc)) {
        for (u_integer i = 0; i < this->max_size; ++i) {
            if (i >= this->inner_begin && i < this->inner_begin + this->size())
                this->construct(&this->body[i], std::forward<ARGS>(args)...);
            else
                this->construct(&this->body[i], TYPE{});
        }
    }

    template<typename TYPE, typename ALLOC>
    original::vector<TYPE, ALLOC>::vector(const u_integer size, ALLOC alloc)
        : baseList<TYPE, ALLOC>(std::move(alloc)), size_(size),
        max_size(size * 4 / 3), inner_begin(size / 3 >= 1 ? size / 3 - 1 : 0), body(this->allocate(this->max_size)) {
    }

    template <typename TYPE, typename ALLOC>
    original::vector<TYPE, ALLOC>::vector(const vector& other) : vector(){
        this->operator=(other);
    }

    template <typename TYPE, typename ALLOC>
    original::vector<TYPE, ALLOC>::vector(const std::initializer_list<TYPE>& list) : vector()
    {
        this->adjust(list.size());
        for (const TYPE& e: list)
        {
            this->setElem(this->inner_begin + this->size(), e);
            this->size_ += 1;
        }
    }

    template <typename TYPE, typename ALLOC>
    original::vector<TYPE, ALLOC>::vector(arrayView<TYPE> view, ALLOC alloc) : vector(view.count(), std::move(alloc))
    {
        for (u_integer i = 0; i < this->max_size; ++i) {
            if (i >= this->inner_begin && i < this->inner_begin + this->size())
                this->construct(&this->body[i], view[i - this->inner_begin]);
            else
                this->construct(&this->body[i], TYPE{});
        }
    }

    template <typename TYPE, typename ALLOC>
    auto original::vector<TYPE, ALLOC>::operator=(const vector& other) -> vector&
    {
        if (this == &other)
            return *this;

        this->vectorArrayDestroy();

        this->max_size = other.max_size;
        this->inner_begin = other.inner_begin;
        this->size_ = other.size_;
        this->body = vector::vectorArrayInit(this->max_size);
        for (u_integer i = 0; i < this->size(); ++i) {
            const TYPE& data = other.body[this->toInnerIdx(i)];
            this->setElem(this->toInnerIdx(i), data);
        }
        if constexpr (ALLOC::propagate_on_container_copy_assignment::value){
            this->allocator = other.allocator;
        }
        return *this;
    }

    template <typename TYPE, typename ALLOC>
    original::vector<TYPE, ALLOC>::vector(vector&& other) noexcept : vector()
    {
        this->operator=(std::move(other));
    }

    template <typename TYPE, typename ALLOC>
    auto original::vector<TYPE, ALLOC>::operator=(vector&& other) noexcept -> vector&
    {
        if (this == &other)
            return *this;

        this->vectorArrayDestroy();
        this->body = other.body;
        other.body = nullptr;
        this->max_size = other.max_size;
        this->inner_begin = other.inner_begin;
        this->size_ = other.size_;
        if constexpr (ALLOC::propagate_on_container_move_assignment::value){
            this->allocator = std::move(other.allocator);
        }
        other.vectorInit();
        return *this;
    }

    template <typename TYPE, typename ALLOC>
    original::vector<TYPE, ALLOC>::vector(const array<TYPE>& arr) : vector()
    {
        this->adjust(arr.size());
        for (u_integer i = 0; i < arr.size(); i += 1)
        {
            this->setElem(this->toInnerIdx(i), arr.get(i));
            this->size_ += 1;
        }
    }

    template <typename TYPE, typename ALLOC>
    auto original::vector<TYPE, ALLOC>::size() const -> u_integer
    {
        return this->size_;
    }

    template <typename TYPE, typename ALLOC>
    original::u_integer original::vector<TYPE, ALLOC>::capacity() const noexcept
    {
        return this->max_size;
    }

    template <typename TYPE, typename ALLOC>
    auto original::vector<TYPE, ALLOC>::data() const -> TYPE& {
        return this->body[this->toInnerIdx(0)];
    }

    template <typename TYPE, typename ALLOC>
    original::arrayView<TYPE> original::vector<TYPE, ALLOC>::view()
    {
        return arrayView<TYPE>{&this->data(), this->size()};
    }

    template <typename TYPE, typename ALLOC>
    original::arrayView<TYPE> original::vector<TYPE, ALLOC>::slice(u_integer start, u_integer end)
    {
        if (start > this->size() || end > this->size())
            throw outOfBoundError(
                printable::formatStrings("Slice range [", start, ":", end,
                                         "] out of bounds [0:", this->size(), "]."));

        if (start >= end)
            return arrayView<TYPE>{};

        return arrayView<TYPE>{&this->data() + start, end - start};
    }

    template <typename TYPE, typename ALLOC>
    original::arrayView<const TYPE> original::vector<TYPE, ALLOC>::view() const
    {
        return arrayView<const TYPE>{&this->data(), this->size()};
    }

    template <typename TYPE, typename ALLOC>
    original::arrayView<const TYPE>
    original::vector<TYPE, ALLOC>::slice(u_integer start, u_integer end) const
    {
        if (start > this->size() || end > this->size())
            throw outOfBoundError(
                printable::formatStrings("Slice range [", start, ":", end,
                                         "] out of bounds [0:", this->size(), "]."));

        if (start >= end)
            return arrayView<const TYPE>{};

        return arrayView<const TYPE>{&this->data() + start, end - start};
    }

    template <typename TYPE, typename ALLOC>
    auto original::vector<TYPE, ALLOC>::get(integer index) const -> TYPE
    {
        if (this->indexOutOfBound(index))
        {
            throw outOfBoundError("Index " + std::to_string(this->parseNegIndex(index)) +
                                  " out of bound max index " + std::to_string(this->size() - 1) + ".");
        }
        index = this->toInnerIdx(this->parseNegIndex(index));
        return this->getElem(index);
    }

    template <typename TYPE, typename ALLOC>
    auto original::vector<TYPE, ALLOC>::operator[](integer index) -> TYPE&
    {
        if (this->indexOutOfBound(index))
        {
            throw outOfBoundError("Index " + std::to_string(this->parseNegIndex(index)) +
                                  " out of bound max index " + std::to_string(this->size() - 1) + ".");
        }
        index = this->toInnerIdx(this->parseNegIndex(index));
        return this->body[index];
    }

    template <typename TYPE, typename ALLOC>
    auto original::vector<TYPE, ALLOC>::set(integer index, const TYPE &e) -> void
    {
        if (this->indexOutOfBound(index))
        {
            throw outOfBoundError("Index " + std::to_string(this->parseNegIndex(index)) +
                                  " out of bound max index " + std::to_string(this->size() - 1) + ".");
        }
        index = this->toInnerIdx(this->parseNegIndex(index));
        this->setElem(index, e);
    }

    template <typename TYPE, typename ALLOC>
    auto original::vector<TYPE, ALLOC>::indexOf(const TYPE &e) const -> u_integer
    {
        if constexpr (Comparable<TYPE>) {
            for (u_integer i = 0; i < this->size(); i += 1)
            {
                if (this->get(i) == e)
                {
                    return i;
                }
            }
            return this->size();
        } else {
            throw unSupportedMethodError("Comparison unsupported type");
        }
    }

    template <typename TYPE, typename ALLOC>
    auto original::vector<TYPE, ALLOC>::pushBegin(const TYPE &e) -> void
    {
        this->adjust(1);
        this->inner_begin -= 1;
        this->setElem(this->toInnerIdx(0), e);
        this->size_ += 1;
    }

    template <typename TYPE, typename ALLOC>
    auto original::vector<TYPE, ALLOC>::push(integer index, const TYPE &e) -> void
    {
        if (this->parseNegIndex(index) == this->size())
        {
            this->pushEnd(e);
        }else if (this->parseNegIndex(index) == 0)
        {
            this->pushBegin(e);
        }else
        {
            if (this->indexOutOfBound(index))
            {
                throw outOfBoundError("Index " + std::to_string(this->parseNegIndex(index)) +
                                      " out of bound max index " + std::to_string(this->size() - 1) + ".");
            }
            this->adjust(1);
            index = this->toInnerIdx(this->parseNegIndex(index));
            u_integer rel_idx = index - this->inner_begin;
            if (index - this->inner_begin <= (this->size() - 1) / 2)
            {
                vector::moveElements(this->body, this->inner_begin,
                                     rel_idx + 1, this->body, -1);
                this->inner_begin -= 1;
            }else
            {
                vector::moveElements(this->body, index,
                                     this->size() - rel_idx, this->body, 1);
            }
            this->setElem(this->toInnerIdx(rel_idx), e);
            this->size_ += 1;
        }
    }

    template <typename TYPE, typename ALLOC>
    auto original::vector<TYPE, ALLOC>::pushEnd(const TYPE &e) -> void
    {
        this->adjust(1);
        this->setElem(this->toInnerIdx(this->size()), e);
        this->size_ += 1;
    }

    template <typename TYPE, typename ALLOC>
    auto original::vector<TYPE, ALLOC>::popBegin() -> TYPE
    {
        if (this->size() == 0){
            throw noElementError();
        }
        TYPE res = this->get(0);
        this->inner_begin += 1;
        this->size_ -= 1;
        return res;
    }

    template <typename TYPE, typename ALLOC>
    auto original::vector<TYPE, ALLOC>::pop(integer index) -> TYPE
    {
        if (this->parseNegIndex(index) == 0)
        {
            return this->popBegin();
        }
        if (this->parseNegIndex(index) == this->size() - 1)
        {
            return this->popEnd();
        }
        if (this->indexOutOfBound(index)){
            throw outOfBoundError("Index " + std::to_string(this->parseNegIndex(index)) +
                                  " out of bound max index " + std::to_string(this->size() - 1) + ".");
        }
        TYPE res = this->get(index);
        index = this->toInnerIdx(this->parseNegIndex(index));
        u_integer rel_idx = index - this->inner_begin;
        if (index - this->inner_begin <= (this->size() - 1) / 2)
        {
            vector::moveElements(this->body, this->inner_begin,
                                 rel_idx, this->body, 1);
            this->inner_begin += 1;
        }else
        {
            vector::moveElements(this->body, index + 1,
                                 this->size() - 1 - rel_idx, this->body, -1);
        }
        this->size_ -= 1;
        return res;
    }

    template <typename TYPE, typename ALLOC>
    auto original::vector<TYPE, ALLOC>::popEnd() -> TYPE
    {
        if (this->size() == 0){
            throw noElementError();
        }
        TYPE res = this->get(this->size() - 1);
        this->size_ -= 1;
        return res;
    }

    template <typename TYPE, typename ALLOC>
    auto original::vector<TYPE, ALLOC>::begins() const -> Iterator* {
        return new Iterator(&this->body[this->toInnerIdx(0)], this, 0);
    }

    template <typename TYPE, typename ALLOC>
    auto original::vector<TYPE, ALLOC>::ends() const -> Iterator* {
        return new Iterator(&this->body[this->toInnerIdx(this->size() - 1)], this, this->size() - 1);
    }

    template <typename TYPE, typename ALLOC>
    auto original::vector<TYPE, ALLOC>::className() const -> std::string
    {
        return "vector";
    }

    template <typename TYPE, typename ALLOC>
    original::vector<TYPE, ALLOC>::~vector() {
        this->vectorArrayDestroy();
    }

    template <typename TYPE, typename ALLOC>
    void original::vector<TYPE, ALLOC>::swap(vector& other) noexcept
    {
        if (this == &other)
            return;

        std::swap(this->size_, other.size_);
        std::swap(this->max_size, other.max_size);
        std::swap(this->inner_begin, other.inner_begin);
        std::swap(this->body, other.body);
        if constexpr (ALLOC::propagate_on_container_swap::value) {
            std::swap(this->allocator, other.allocator);
        }
    }

    template<typename T, typename... ARGS>
    original::vector<T> original::makeVector(u_integer size, ARGS &&... args) {
        return original::vector<T>(size, original::allocator<T>{}, std::forward<ARGS>(args)...);
    }

    template <typename TYPE, typename ALLOC>
    void std::swap(original::vector<TYPE, ALLOC>& lhs, original::vector<TYPE, ALLOC>& rhs) noexcept // NOLINT
    {
        lhs.swap(rhs);
    }

#endif //VECTOR_H
