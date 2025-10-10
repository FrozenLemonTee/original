#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "config.h"
#include "error.h"
#include "maths.h"
#include "type_traits"
#include "utility"

/**
* @file allocator.h
* @brief Memory allocation interface and implementations
* @details Provides:
* - Base allocator interface with CRTP support (allocatorBase)
* - Utility functions for raw memory operations (allocators class)
* - Default memory allocator implementation (allocator)
* - Object pool allocator implementation (objPoolAllocator)
*
* Memory Management Hierarchy:
* 1. Fundamental operations (allocators class):
*    - malloc/free: Raw memory operations using global new/delete
* 2. Base interface (allocatorBase):
*    - Type-safe allocation interface
*    - Object construction/destruction utilities
* 3. Implementations:
*    - allocator: Simple wrapper around allocators utilities
*    - objPoolAllocator: Advanced pool-based allocator with fallback to allocators
*
* @note All allocators maintain the following guarantees:
* - Type safety through templated interfaces
* - Exception safety (throws allocateError on failure)
* - Nullptr handling for zero-size allocations
*/
namespace original {

    /**
    * @class allocators
    * @brief Utility class providing static memory allocation/de-allocation functions
    * @details Contains static methods for raw memory operations that serve as the foundation
    *          for other allocator implementations in this namespace.
    */
    class allocators final {
    public:
        /**
        * @brief Allocates raw memory using global operator new
        * @tparam TYPE Type of objects to allocate
        * @param size Number of elements to allocate
        * @return Pointer to allocated memory
        * @throw allocateError When memory allocation fails
        * @note Returns nullptr if size is 0
        * @see free
        */
        template <typename TYPE>
        static TYPE* malloc(u_integer size);

        /**
        * @brief Deallocates memory using global operator delete
        * @tparam TYPE Type of objects being freed (auto deduced)
        * @param ptr Pointer to memory to free
        * @see malloc
        */
        template <typename TYPE>
        static void free(TYPE* ptr);
    };

    /**
    * @class allocatorBase
    * @tparam TYPE The type of objects to allocate
    * @tparam DERIVED CRTP template parameter for derived allocators
    * @brief Interface for other memory allocator implementations
    * @details Provides common interface and utilities for memory management:
    * - Type-safe allocation/de-allocation
    * - Object construction/destruction
    * - Rebinding support for container needs
    * - Propagation control for container operations
    *
    * Propagation traits:
    * - propagate_on_container_copy_assignment
    * - propagate_on_container_move_assignment
    * - propagate_on_container_swap
    * - propagate_on_container_merge
    */
    template<typename TYPE, template <typename> typename DERIVED>
    class allocatorBase{
    public:
        using propagate_on_container_copy_assignment = std::false_type; ///< No propagation on copy
        using propagate_on_container_move_assignment = std::false_type; ///< No propagation on move
        using propagate_on_container_swap = std::false_type; ///< No propagation on swap
        using propagate_on_container_merge = std::false_type; ///< No propagation on merge

        /**
        * @brief Rebinds allocator to different type
        * @tparam O_TYPE New type to allocate
        */
        template <typename O_TYPE>
        using rebind_alloc = DERIVED<O_TYPE>;

        /**
        * @brief Constructs a new allocatorBase instance
        * @details Performs compile-time validation of the allocated type:
        * - Ensures TYPE is not void
        * - Ensures TYPE is a complete type (size > 0)
        */
        constexpr allocatorBase();

        /**
        * @brief Allocates raw memory
        * @param size Number of elements to allocate
        * @return Pointer to allocated memory
        */
        virtual TYPE* allocate(u_integer size) = 0;

        /**
        * @brief Deallocates memory
        * @param ptr Pointer to memory to free
        * @param size Number of elements originally allocated
        */
        virtual void deallocate(TYPE* ptr, u_integer size) = 0;

        /**
        * @brief Constructs an object in allocated memory
        * @tparam O_TYPE Type of object to construct
        * @tparam Args Constructor argument types
        * @param o_ptr Pointer to allocated memory
        * @param args Arguments to forward to constructor
        */
        template<typename O_TYPE, typename... Args>
        void construct(O_TYPE* o_ptr, Args&&... args);

        /**
        * @brief Destroys an object without deallocating
        * @tparam O_TYPE Type of object to destroy
        * @param o_ptr Pointer to object to destroy
        */
        template<typename O_TYPE>
        static void destroy(O_TYPE* o_ptr);

        virtual ~allocatorBase() = 0; ///< Virtual destructor
    };

    /**
    * @class allocator
    * @tparam TYPE Type of objects to allocate
    * @brief Default memory allocator using allocators utilities
    * @details Implements standard-compliant allocator using:
    * - allocators::malloc for allocation
    * - allocators::free for de-allocation
    * - Placement new for construction
    * - Explicit destructor calls
    * @extends allocatorBase
    */
    template<typename TYPE>
    class allocator : public allocatorBase<TYPE, allocator> {
    public:
        using typename allocatorBase<TYPE, allocator>::propagate_on_container_copy_assignment;
        using typename allocatorBase<TYPE, allocator>::propagate_on_container_move_assignment;
        using typename allocatorBase<TYPE, allocator>::propagate_on_container_swap;
        using typename allocatorBase<TYPE, allocator>::propagate_on_container_merge;

        /**
        * @brief Allocates memory using global operator new
        * @param size Number of elements to allocate
        * @return Pointer to allocated memory
        * @throw allocateError When memory allocation fails
        * @note Returns nullptr if size is 0
        */
        TYPE* allocate(u_integer size) override;

        /**
        * @brief Deallocates memory using global operator delete
        * @param ptr Pointer to memory to free
        * @param size Number of elements (unused in this implementation)
        */
        void deallocate(TYPE* ptr, u_integer size) override;
    };

    /**
    * @class objPoolAllocator
    * @tparam TYPE Type of objects to allocate
    * @brief Object pool allocator for efficient fixed-size memory management
    * @details Implements a memory allocator using an object pool pattern with these characteristics:
    * - Organizes memory into power-of-two sized chunks (1, 2, 4, 8, 16, 32, ... bytes)
    * - Maintains separate free lists for each chunk size
    * - Allocates memory in blocks (chunks) to reduce fragmentation
    * - Falls back to allocators::malloc/ allocators::free for allocations too large for the pool
    *
    * Memory Management Approach:
    * 1. The allocator maintains multiple free lists, one for each size class (power-of-two)
    * 2. When an allocation request comes in:
    *    - Finds the smallest size class that can satisfy the request
    *    - If no free chunks available, allocates a new block of chunks
    *    - For requests too large for the largest size class, uses global new
    * 3. When memory is deallocated:
    *    - Returns the chunk to its appropriate free list
    *    - Large allocations (outside pool) are freed immediately
    *
    * @note For large allocations, uses allocators::malloc/ allocators::free
    * @extends allocatorBase
    * @see allocator For the default allocator implementation
    */
    template<typename TYPE>
    class objPoolAllocator final : public allocatorBase<TYPE, objPoolAllocator>
    {
        /**
        * @class freeChunk
        * @brief Internal structure representing a free memory chunk
        * @details Used to maintain the free list as a linked list
        */
        class freeChunk
        {
        public:
            freeChunk* next = nullptr; ///< Pointer to next free chunk
        };

        /**
        * @class allocatedChunks
        * @brief Tracks blocks of allocated memory for later de-allocation
        */
        class allocatedChunks {
        public:
            void* chunks = nullptr;       ///< Pointer to allocated memory block
            allocatedChunks* next = nullptr; ///< Next block in the list
        };

        /**
        * @brief The fundamental chunk size used for memory management
        * @details Determines the minimum allocation unit size in the pool.
        * The actual size is calculated as the maximum between:
        * - sizeof(TYPE): To ensure each chunk can hold at least one object of the templated type
        * - sizeof(freeChunk*): To ensure proper free list maintenance (must be able to store a pointer)
        */
        static constexpr u_integer CHUNK_SIZE =  sizeof(TYPE) > sizeof(freeChunk*) ? sizeof(TYPE) : sizeof(freeChunk*);

        u_integer size_class_count;      ///< Number of size classes (indices 0 to size_class_count-1)
        u_integer chunk_count_init;      ///< Initial chunk count for each size class
        u_integer* chunk_count;          ///< Array of chunk counts per size class
        freeChunk** free_list_head;      ///< Array of free list heads (one per size class)
        allocatedChunks* allocated_list_head; ///< List of all allocated blocks
        u_integer* chunks_available;     ///< Array of available chunks per size class

        /**
        * @brief Initializes the memory pool structures
        * @details Allocates and initializes:
        * - chunk_count array
        * - free_list_head array
        * - chunks_available array
        * - allocated_list_head
        * @note Uses allocators::malloc for all allocations
        */
        void poolInit();

        /**
        * @brief Calculates the appropriate size class index for a request
        * @param size Requested allocation size
        * @return Index of the smallest size class that can satisfy the request
        */
        [[nodiscard]] static constexpr u_integer getChunkIndex(u_integer size);

        /**
        * @brief Allocates a new block of chunks for a size class
        * @param num_element Number of chunks to allocate
        * @param index Size class index
        * @throw allocateError If memory allocation fails
        */
        void chunkAllocate(u_integer num_element, u_integer index);

        /**
        * @brief Releases all allocated memory using allocators::free
        * @details Safely deallocates:
        * - All allocated memory blocks
        * - Internal management arrays
        * - Pool data structures
        * @note Called during destruction and move assignment
        */
        void release() noexcept;

    public:
        using typename allocatorBase<TYPE, objPoolAllocator>::propagate_on_container_copy_assignment;
        using propagate_on_container_move_assignment = std::true_type; ///< Allows propagation on move
        using propagate_on_container_swap = std::true_type; ///< Allows propagation on swap
        using propagate_on_container_merge = std::true_type; ///< Allows propagation on merge

        /**
        * @brief Constructs a new object pool allocator
        * @param size_class_count Number of size classes to manage (default=8 → indices 0-7)
        * @param count Initial number of chunks per size class (default 4)
        */
        explicit objPoolAllocator(u_integer size_class_count = 8, u_integer count = 4);

        objPoolAllocator(const objPoolAllocator&) = delete; ///< Copy construction disabled
        objPoolAllocator& operator=(const objPoolAllocator&) = delete; ///< Copy assignment disabled

        /**
        * @brief Merges another pool allocator into this one
        * @param other The allocator to merge
        * @return Reference to this allocator
        * @details Merges:
        * - Size class configurations (takes maximum)
        * - Free lists (concatenates)
        * - Allocated blocks (combines)
        * - Allocator resources (since propagate_on_container_merge is true)
        */
        objPoolAllocator& operator+=(objPoolAllocator& other);

        /**
        * @brief Move constructor
        * @param other Allocator to move from
        * @note Transfers ownership of all resources and leaves source in initialized state
        */
        objPoolAllocator(objPoolAllocator&& other) noexcept;

        /**
        * @brief Move assignment operator
        * @param other Allocator to move from
        * @return Reference to this allocator
        * @note Releases current resources before taking ownership of new ones
        *       Leaves source in initialized state via poolInit()
        */
        objPoolAllocator& operator=(objPoolAllocator&& other) noexcept;

        /**
        * @brief Swaps the contents of two allocators
        * @param other Allocator to swap with
        */
        void swap(objPoolAllocator& other) noexcept;

        /**
        * @brief Allocates memory from the pool
        * @param size Number of elements to allocate
        * @return Pointer to allocated memory
        * @throw allocateError When memory allocation fails
        * @note For large allocations, falls back to global operator new
        */
        TYPE* allocate(u_integer size) override;

        /**
        * @brief Returns memory to the pool
        * @param ptr Pointer to memory to free
        * @param size Number of elements originally allocated
        * @note For large allocations, uses global operator delete
        */
        void deallocate(TYPE* ptr, u_integer size) override;

        /**
        * @brief Destructor - releases all allocated memory
        */
        ~objPoolAllocator() override;
    };
}

namespace std {
    /**
    * @brief Specialization of std::swap for objPoolAllocator
    * @tparam TYPE Type of objects allocated
    * @param lhs First allocator to swap
    * @param rhs Second allocator to swap
    */
    template<typename TYPE>
    void swap(original::objPoolAllocator<TYPE>& lhs, original::objPoolAllocator<TYPE>& rhs) noexcept; // NOLINT
}

template<typename TYPE>
TYPE* original::allocators::malloc(const u_integer size) {
    if (size == 0) {
        return nullptr;
    }

    try {
        return static_cast<TYPE*>(operator new(size * sizeof(TYPE)));
    } catch (const std::bad_alloc&) {
        throw allocateError();
    }
}

template<typename TYPE>
void original::allocators::free(TYPE* ptr) {
    ::operator delete(ptr);
}

template <typename TYPE, template <typename> typename DERIVED>
constexpr original::allocatorBase<TYPE, DERIVED>::allocatorBase()
{
    staticError<allocateError, sizeof(TYPE) == 0 || std::is_void_v<TYPE>>::asserts();
}

template<typename TYPE, template <typename> typename DERIVED>
original::allocatorBase<TYPE, DERIVED>::~allocatorBase() = default;

template<typename TYPE, template <typename> typename DERIVED>
template<typename O_TYPE, typename... Args>
void original::allocatorBase<TYPE, DERIVED>::construct(O_TYPE *o_ptr, Args &&... args) {
    new (o_ptr) O_TYPE{std::forward<Args>(args)...};
}

template<typename TYPE, template <typename> typename DERIVED>
template<typename O_TYPE>
void original::allocatorBase<TYPE, DERIVED>::destroy(O_TYPE *o_ptr) {
    o_ptr->~O_TYPE();
}

template<typename TYPE>
TYPE* original::allocator<TYPE>::allocate(const u_integer size) {
    return allocators::malloc<TYPE>(size);
}

template<typename TYPE>
void original::allocator<TYPE>::deallocate(TYPE* ptr, u_integer) {
    allocators::free(ptr);
}

template<typename TYPE>
constexpr original::u_integer
original::objPoolAllocator<TYPE>::getChunkIndex(const u_integer size) {
    u_integer index = 0;
    while (static_cast<u_integer>(1) << index < size) {
        index += 1;
    }
    return index;
}

template<typename TYPE>
void original::objPoolAllocator<TYPE>::poolInit() {
    this->chunk_count = allocators::malloc<u_integer>(this->size_class_count);
    this->free_list_head = allocators::malloc<freeChunk*>(this->size_class_count);
    this->chunks_available = allocators::malloc<u_integer>(this->size_class_count);
    this->allocated_list_head = nullptr;

    for (u_integer i = 0; i < this->size_class_count; i++) {
        this->chunk_count[i] = this->chunk_count_init;
        this->free_list_head[i] = nullptr;
        this->chunks_available[i] = 0;
    }
}

template <typename TYPE>
void original::objPoolAllocator<TYPE>::chunkAllocate(const u_integer num_element, u_integer index)
{
    const u_integer block_size = (1 << index) * CHUNK_SIZE;
    auto new_free_chunk = allocators::malloc<byte>(num_element * block_size);
    auto new_allocated_chunk = allocators::malloc<allocatedChunks>(1);

    new_allocated_chunk->chunks = new_free_chunk;
    new_allocated_chunk->next = this->allocated_list_head;
    this->allocated_list_head = new_allocated_chunk;

    for (u_integer i = 0; i < num_element; i++)
    {
        auto cur_ptr = reinterpret_cast<freeChunk*>(new_free_chunk + i * block_size);
        cur_ptr->next = this->free_list_head[index];
        this->free_list_head[index] = cur_ptr;
    }
    this->chunks_available[index] += num_element;
    this->chunk_count[index] = this->chunk_count[index] + (this->chunk_count[index] >> 1);
}

template<typename TYPE>
void original::objPoolAllocator<TYPE>::release() noexcept {
    while (this->allocated_list_head) {
        auto next_chunk = this->allocated_list_head->next;
        allocators::free(this->allocated_list_head->chunks);
        allocators::free(this->allocated_list_head);
        this->allocated_list_head = next_chunk;
    }

    if (this->chunk_count) {
        allocators::free(this->chunk_count);
        this->chunk_count = nullptr;
    }
    if (this->free_list_head) {
        allocators::free(this->free_list_head);
        this->free_list_head = nullptr;
    }
    if (this->chunks_available) {
        allocators::free(this->chunks_available);
        this->chunks_available = nullptr;
    }
}

template <typename TYPE>
original::objPoolAllocator<TYPE>::objPoolAllocator(const u_integer size_class_count, const u_integer count)
    : size_class_count(size_class_count), chunk_count_init(count), chunk_count(nullptr),
      free_list_head(nullptr), allocated_list_head(nullptr), chunks_available(nullptr) {
    this->poolInit();
}

template<typename TYPE>
original::objPoolAllocator<TYPE>& original::objPoolAllocator<TYPE>::operator+=(objPoolAllocator& other) {
    if (this == &other)
        return *this;

    auto size_class_count_max = maximum(this->size_class_count, other.size_class_count);
    auto chunk_count_init_max = maximum(this->chunk_count_init, other.chunk_count_init);
    auto chunk_count_max = allocators::malloc<u_integer>(size_class_count_max);
    auto free_list_head_max = allocators::malloc<freeChunk*>(size_class_count_max);
    auto chunks_available_max = allocators::malloc<u_integer>(size_class_count_max);

    for (u_integer i = 0; i < size_class_count_max; i++) {
        if (i < this->size_class_count && i < other.size_class_count) {
            chunk_count_max[i] = maximum(this->chunk_count[i], other.chunk_count[i]);
            chunks_available_max[i] = this->chunks_available[i] + other.chunks_available[i];
            auto cur_list_head_i = other.free_list_head[i];
            if (cur_list_head_i) {
                while (cur_list_head_i->next) {
                    cur_list_head_i = cur_list_head_i->next;
                }
                cur_list_head_i->next = this->free_list_head[i];
                free_list_head_max[i] = other.free_list_head[i];
            }else {
                free_list_head_max[i] = this->free_list_head[i];
            }
        }else if (i < this->size_class_count) {
            chunk_count_max[i] = this->chunk_count[i];
            chunks_available_max[i] = this->chunks_available[i];
            free_list_head_max[i] = this->free_list_head[i];
        }else {
            chunk_count_max[i] = other.chunk_count[i];
            chunks_available_max[i] = other.chunks_available[i];
            free_list_head_max[i] = other.free_list_head[i];
        }
    }

    auto allocated_list_head_max = other.allocated_list_head;
    auto cur_allocated_list_head = allocated_list_head_max;
    if (cur_allocated_list_head) {
        while (cur_allocated_list_head->next) {
            cur_allocated_list_head = cur_allocated_list_head->next;
        }
        cur_allocated_list_head->next = this->allocated_list_head;
    }else {
        allocated_list_head_max = this->allocated_list_head;
    }
    this->allocated_list_head = nullptr;
    other.allocated_list_head = nullptr;

    this->release();
    other.release();
    other.poolInit();

    this->size_class_count = size_class_count_max;
    this->chunk_count_init = chunk_count_init_max;
    this->chunk_count = chunk_count_max;
    this->free_list_head = free_list_head_max;
    this->chunks_available = chunks_available_max;
    this->allocated_list_head = allocated_list_head_max;

    return *this;
}

template<typename TYPE>
original::objPoolAllocator<TYPE>::objPoolAllocator(objPoolAllocator&& other) noexcept : objPoolAllocator() {
    this->operator=(std::move(other));
}

template<typename TYPE>
original::objPoolAllocator<TYPE>& original::objPoolAllocator<TYPE>::operator=(objPoolAllocator&& other) noexcept {
    if (this == &other)
        return *this;

    this->release();

    this->size_class_count = other.size_class_count;
    this->chunk_count_init = other.chunk_count_init;
    this->chunk_count = other.chunk_count;
    this->free_list_head = other.free_list_head;
    this->allocated_list_head = other.allocated_list_head;
    this->chunks_available = other.chunks_available;

    other.poolInit();
    return *this;
}

template <typename TYPE>
void original::objPoolAllocator<TYPE>::swap(objPoolAllocator& other) noexcept
{
    if (this == &other)
        return;

    std::swap(this->size_class_count, other.size_class_count);
    std::swap(this->chunk_count_init, other.chunk_count_init);
    std::swap(this->chunk_count, other.chunk_count);
    std::swap(this->free_list_head, other.free_list_head);
    std::swap(this->allocated_list_head, other.allocated_list_head);
    std::swap(this->chunks_available, other.chunks_available);
}

template <typename TYPE>
TYPE* original::objPoolAllocator<TYPE>::allocate(const u_integer size)
{
    if (size == 0) {
        return nullptr;
    }

    u_integer index = getChunkIndex(size);

    if (index >= this->size_class_count) {
        return allocators::malloc<TYPE>(size);
    }

    if (!this->free_list_head[index] || this->chunks_available[index] * (static_cast<u_integer>(1) << index) < size) {
        this->chunkAllocate(maximum<u_integer>(1, this->chunk_count[index]), index);
    }

    auto cur_ptr = this->free_list_head[index];
    this->free_list_head[index] = this->free_list_head[index]->next;
    this->chunks_available[index] -= 1;
    return reinterpret_cast<TYPE*>(cur_ptr);
}

template <typename TYPE>
void original::objPoolAllocator<TYPE>::deallocate(TYPE* ptr, const u_integer size)
{
    if (size == 0) {
        return;
    }

    u_integer index = getChunkIndex(size);

    if (index >= this->size_class_count) {
        allocators::free(ptr);
        return;
    }

    auto p = reinterpret_cast<freeChunk*>(ptr);
    p->next = this->free_list_head[index];
    this->free_list_head[index] = p;
    this->chunks_available[index] += 1;
}

template<typename TYPE>
original::objPoolAllocator<TYPE>::~objPoolAllocator() {
    this->release();
}

template <typename TYPE>
void std::swap(original::objPoolAllocator<TYPE>& lhs, original::objPoolAllocator<TYPE>& rhs) noexcept // NOLINT
{
    lhs.swap(rhs);
}

#endif //ALLOCATOR_H
