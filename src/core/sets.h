#ifndef SETS_H
#define SETS_H

#include "allocator.h"
#include "couple.h"
#include "hash.h"
#include "hashTable.h"
#include "set.h"
#include "ownerPtr.h"
#include "comparator.h"
#include "RBTree.h"
#include "skipList.h"


/**
 * @file sets.h
 * @brief Implementation of set containers with different underlying data structures
 * @details Provides three set implementations with different performance characteristics
 * and iteration capabilities:
 * 1. hashSet - Hash table based implementation (unordered, fastest average case)
 * 2. treeSet - Red-Black Tree based implementation (ordered, consistent performance)
 * 3. JSet - Skip List based implementation (ordered, probabilistic balance)
 *
 * Common Features:
 * - Unique element storage (no duplicates)
 * - Full iterator support with different capabilities
 * - Customizable comparison/hash functions
 * - Customizable allocators with propagation support
 * - Exception safety guarantees (basic guarantee for most operations)
 * - Polymorphic usage through set interface
 * - Integration with printable for string representation
 *
 * Performance Characteristics:
 * | Container | Insertion    | Lookup   | Deletion | Ordered | Memory Usage |
 * |-----------|--------------|----------|----------|---------|--------------|
 * | hashSet   | O(1) avg     | O(1)     | O(1)     | No      | Medium-High  |
 * | treeSet   | O(log n)     | O(log n) | O(log n) | Yes     | Low          |
 * | JSet      | O(log n) avg | O(log n) | O(log n) | Yes     | Medium       |
 *
 * Usage Guidelines:
 * - Use hashSet for maximum performance when order doesn't matter and elements are hashable
 * - Use treeSet for ordered traversal, range queries, and consistent worst-case performance
 * - Use JSet for concurrent scenarios (external synchronization) or when probabilistic balance is preferred
 *
 * Iterator Invalidation:
 * - hashSet: Iterators invalidate on rehash (insertion that causes capacity change)
 * - treeSet: Iterators invalidate on element removal that affects the current position
 * - JSet: Iterators invalidate on any structural modification
 *
 * Exception Safety:
 * - Basic guarantee: Container remains valid but unspecified state on exception
 * - Strong guarantee for some operations (no-throw if element operations are noexcept)
 * - Allocator-aware exception handling
 *
 * @see set.h For the base interface definition
 * @see hashTable.h For hashSet implementation details
 * @see RBTree.h For treeSet implementation details
 * @see skipList.h For JSet implementation details
 * @see printable.h For string formatting support
 */

namespace original {
    /**
     * @class hashSet
     * @tparam TYPE Element type (must be hashable)
     * @tparam HASH Hash function type (default: hash<TYPE>)
     * @tparam ALLOC Allocator type (default: allocator<couple<const TYPE, const bool>>)
     * @brief Hash table based implementation of the set interface
     * @details This class provides a concrete implementation of the set interface
     * using a hash table with separate chaining. It combines the functionality of:
     * - set (interface)
     * - hashTable (storage with bool values)
     * - iterable (iteration support)
     *
     * Performance Characteristics:
     * - Insertion: Average O(1), Worst O(n)
     * - Lookup: Average O(1), Worst O(n)
     * - Deletion: Average O(1), Worst O(n)
     *
     * The implementation guarantees:
     * - Unique elements (no duplicates)
     * - Type safety
     * - Exception safety (basic guarantee)
     * - Iterator validity unless modified
     */
    template <typename TYPE,
              typename HASH = hash<TYPE>,
              typename ALLOC = allocator<couple<const TYPE, const bool>>>
    class hashSet final : public hashTable<TYPE, const bool, ALLOC, HASH>,
                          public set<TYPE, ALLOC>,
                          public iterable<const TYPE>,
                          public printable{

        /**
         * @typedef hashNode
         * @brief Internal node type used for hash table storage
         */
        using hashNode = hashTable<TYPE, const bool, ALLOC, HASH>::hashNode;

        /**
         * @typedef rebind_alloc_pointer
         * @brief Rebound allocator type for pointer storage
         */
        using rebind_alloc_pointer = hashTable<TYPE, const bool, ALLOC, HASH>::rebind_alloc_pointer;

    public:
        /**
         * @class Iterator
         * @brief Forward iterator for hashSet
         * @details Provides iteration over hashSet elements while maintaining:
         * - Consistent traversal order
         * - Safe invalidation detection
         * - Const-correct access
         *
         * Iterator Characteristics:
         * - Forward iteration only (throws on reverse operations)
         * - Invalidates on rehash
         * - Lightweight copy semantics
         */
        class Iterator final : public hashTable<TYPE, const bool, ALLOC, HASH>::Iterator,
                             public baseIterator<const TYPE> {

            /**
             * @brief Constructs iterator pointing to specific bucket/node
             * @param buckets Pointer to buckets vector
             * @param bucket Current bucket index
             * @param node Current node pointer
             * @note Internal constructor, not meant for direct use
             */
            explicit Iterator(vector<hashNode*, rebind_alloc_pointer>* buckets = nullptr,
                              u_integer bucket = 0, hashNode* node = nullptr);

            /**
             * @brief Compares iterator pointers for equality
             * @param other Iterator to compare with
             * @return true if iterators point to same element
             * @internal
             */
            bool equalPtr(const iterator<const TYPE>* other) const override;

        public:
            friend class hashSet;

            /**
             * @brief Copy constructor
             * @param other Iterator to copy
             */
            Iterator(const Iterator& other);

            /**
             * @brief Copy assignment operator
             * @param other Iterator to copy
             * @return Reference to this iterator
             */
            Iterator& operator=(const Iterator& other);

            /**
             * @brief Creates a copy of this iterator
             * @return New iterator instance
             */
            Iterator* clone() const override;

            /**
             * @brief Gets iterator class name
             * @return "hashSet::Iterator"
             */
            [[nodiscard]] std::string className() const override;

            /**
             * @brief Advances iterator by steps
             * @param steps Number of positions to advance
             * @throw unSupportedMethodError if steps is negative
             */
            void operator+=(integer steps) const override;

            /**
             * @brief Not supported (throws unSupportedMethodError)
             */
            void operator-=(integer steps) const override;

            /**
             * @brief Not supported (throws unSupportedMethodError)
             */
            integer operator-(const iterator<const TYPE>& other) const override;

            /**
             * @brief Checks if more elements exist
             * @return true if more elements available
             */
            [[nodiscard]] bool hasNext() const override;

            /**
             * @brief Not supported (throws unSupportedMethodError)
             */
            [[nodiscard]] bool hasPrev() const override;

            /**
             * @brief Checks if other is previous to this
             * @param other Iterator to check
             * @return true if other is previous
             */
            bool atPrev(const iterator<const TYPE>* other) const override;

            /**
             * @brief Checks if other is next to this
             * @param other Iterator to check
             * @return true if other is next
             */
            bool atNext(const iterator<const TYPE>* other) const override;

            /**
             * @brief Moves to next element
             */
            void next() const override;

            /**
             * @brief Not supported (throws unSupportedMethodError)
             */
            void prev() const override;

            /**
             * @brief Not supported (throws unSupportedMethodError)
             */
            Iterator* getPrev() const override;

            /**
             * @brief Gets current element (non-const)
             * @return Reference to current element
             */
            const TYPE& get() override;

            /**
             * @brief Gets current element (const)
             * @return Copy of current element
             */
            const TYPE get() const override;

            /**
             * @brief Not supported (throws unSupportedMethodError)
             */
            void set(const TYPE& data) override;

            /**
             * @brief Checks if iterator is valid
             * @return true if iterator points to valid element
             */
            [[nodiscard]] bool isValid() const override;

            ~Iterator() override = default;
        };

        /**
         * @brief Constructs empty hashSet
         * @param hash Hash function to use
         * @param alloc Allocator to use
         */
        explicit hashSet(HASH hash = HASH{}, ALLOC alloc = ALLOC{});

        /**
         * @brief Copy constructor
         * @param other hashSet to copy
         * @details Performs deep copy of all elements and buckets
         * @note Allocator is copied if propagate_on_container_copy_assignment is true
         */
        hashSet(const hashSet& other);

        /**
         * @brief Copy assignment operator
         * @param other hashSet to copy
         * @return Reference to this hashSet
         * @details Performs deep copy of all elements and buckets
         * @note Allocator is copied if propagate_on_container_copy_assignment is true
         */
        hashSet& operator=(const hashSet& other);

        /**
         * @brief Move constructor
         * @param other hashSet to move from
         * @details Transfers ownership of resources from other
         * @note Leaves other in valid but unspecified state
         */
        hashSet(hashSet&& other) noexcept;

        /**
         * @brief Move assignment operator
         * @param other hashSet to move from
         * @return Reference to this hashSet
         * @details Transfers ownership of resources from other
         * @note Leaves other in valid but unspecified state
         * @note Allocator is moved if propagate_on_container_move_assignment is true
         */
        hashSet& operator=(hashSet&& other) noexcept;

        /**
         * @brief Swaps contents with another hashSet
         * @param other hashSet to swap with
         * @note No-throw guarantee if element swap and allocator swap are noexcept
         * @details Efficiently exchanges:
         * - Bucket arrays and their contents
         * - Size counters
         * - Hash function instances
         * - Allocators (if propagate_on_container_swap is true)
         *
         * Performance: O(1) - pointer swaps only
         * Iterator Invalidation: All iterators from both sets are invalidated
         *
         * Example usage:
         * @code{.cpp}
         * hashSet<int> set1, set2;
         * set1.add(1); set2.add(2);
         * set1.swap(set2);
         * // Now set1 contains 2, set2 contains 1
         * @endcode
         */
        void swap(hashSet& other) noexcept;

        /**
         * @brief Gets number of elements
         * @return Current size
         */
        [[nodiscard]] u_integer size() const override;

        /**
         * @brief Checks if element exists
         * @param e Element to check
         * @return true if element exists
         */
        bool contains(const TYPE &e) const override;

        /**
         * @brief Adds new element
         * @param e Element to add
         * @return true if added, false if element existed
         */
        bool add(const TYPE &e) override;

        /**
         * @brief Removes element
         * @param e Element to remove
         * @return true if removed, false if element didn't exist
         */
        bool remove(const TYPE &e) override;

        /**
         * @brief Gets begin iterator
         * @return New iterator at first element
         */
        Iterator* begins() const override;

        /**
         * @brief Gets end iterator
         * @return New iterator at position past last element
         */
        Iterator* ends() const override;

        /**
         * @brief Gets class name
         * @return "hashSet"
         */
        [[nodiscard]] std::string className() const override;

        /**
         * @brief Converts to string representation
         * @param enter Add newline if true
         * @return String representation
         */
        [[nodiscard]] std::string toString(bool enter) const override;

        ~hashSet() override;
    };

    /**
     * @class treeSet
     * @tparam TYPE Element type (must be comparable)
     * @tparam Compare Comparison function type (default: increaseComparator<TYPE>)
     * @tparam ALLOC Allocator type (default: allocator<couple<const TYPE, const bool>>)
     * @brief Red-Black Tree based implementation of the set interface
     * @details This class provides a concrete implementation of the set interface
     * using a Red-Black Tree. It combines the functionality of:
     * - set (interface)
     * - RBTree (storage with bool values)
     * - iterable (iteration support)
     *
     * Performance Characteristics:
     * - Insertion: O(log n)
     * - Lookup: O(log n)
     * - Deletion: O(log n)
     * - Traversal: O(n)
     *
     * The implementation guarantees:
     * - Elements sorted according to comparator
     * - Unique elements (no duplicates)
     * - Type safety
     * - Exception safety (basic guarantee)
     * - Iterator validity unless modified
     */
    template <typename TYPE,
              typename Compare = increaseComparator<TYPE>,
              typename ALLOC = allocator<couple<const TYPE, const bool>>>
    class treeSet final : public RBTree<TYPE, const bool, ALLOC, Compare>,
                          public set<TYPE, ALLOC>,
                          public iterable<const TYPE>,
                          public printable {
        using RBTreeType = RBTree<TYPE, const bool, ALLOC, Compare>;

        /**
         * @typedef RBNode
         * @brief Internal node type used for Red-Black Tree storage
         */
        using RBNode = RBTreeType::RBNode;
    public:
        /**
         * @class Iterator
         * @brief Bidirectional iterator for treeSet
         * @details Provides iteration over treeSet elements while maintaining:
         * - Sorted traversal order (according to comparator)
         * - Safe invalidation detection
         * - Const-correct access
         *
         * Iterator Characteristics:
         * - Both forward and backward iteration
         * - Invalidates on tree modification
         * - Lightweight copy semantics
         */
        class Iterator final : public RBTreeType::Iterator,
                               public baseIterator<const TYPE>
        {
            /**
             * @brief Constructs iterator pointing to specific tree node
             * @param tree Pointer to owning tree
             * @param cur Current node pointer
             * @note Internal constructor, not meant for direct use
             */
            explicit Iterator(RBTreeType* tree, RBNode* cur);

            /**
             * @brief Compares iterator pointers for equality
             * @param other Iterator to compare with
             * @return true if iterators point to same element
             * @internal
             */
            bool equalPtr(const iterator<const TYPE>* other) const override;
        public:
            friend class treeSet;

            /**
             * @brief Copy constructor
             * @param other Iterator to copy
             */
            Iterator(const Iterator& other);

            /**
             * @brief Copy assignment operator
             * @param other Iterator to copy
             * @return Reference to this iterator
             */
            Iterator& operator=(const Iterator& other);

            /**
             * @brief Creates a copy of this iterator
             * @return New iterator instance
             */
            Iterator* clone() const override;

            /**
             * @brief Gets iterator class name
             * @return "treeSet::Iterator"
             */
            [[nodiscard]] std::string className() const override;

            /**
             * @brief Advances iterator by steps
             * @param steps Number of positions to advance
             */
            void operator+=(integer steps) const override;

            /**
             * @brief Rewinds iterator by steps
             * @param steps Number of positions to rewind
             */
            void operator-=(integer steps) const override;

            /**
             * @brief Not supported (throws unSupportedMethodError)
             */
            integer operator-(const iterator<const TYPE> &other) const override;

            /**
             * @brief Checks if more elements exist in forward direction
             * @return true if more elements available
             */
            [[nodiscard]] bool hasNext() const override;

            /**
             * @brief Checks if more elements exist in backward direction
             * @return true if more elements available
             */
            [[nodiscard]] bool hasPrev() const override;

            /**
             * @brief Checks if other is previous to this
             * @param other Iterator to check
             * @return true if other is previous
             */
            bool atPrev(const iterator<const TYPE>* other) const override;

            /**
             * @brief Checks if other is next to this
             * @param other Iterator to check
             * @return true if other is next
             */
            bool atNext(const iterator<const TYPE>* other) const override;

            /**
             * @brief Moves to next element
             */
            void next() const override;

            /**
             * @brief Moves to previous element
             */
            void prev() const override;

            /**
             * @brief Gets previous iterator
             * @return New iterator at previous position
             */
            Iterator* getPrev() const override;

            /**
             * @brief Gets current element (non-const)
             * @return Reference to current element
             */
            const TYPE& get() override;

            /**
             * @brief Gets current element (const)
             * @return Copy of current element
             */
            const TYPE get() const override;

            /**
             * @brief Not supported
             * @throw unSupportedMethodError
             */
            void set(const TYPE& data) override;

            /**
             * @brief Checks if iterator is valid
             * @return true if iterator points to valid element
             */
            [[nodiscard]] bool isValid() const override;

            ~Iterator() override = default;
        };

        friend class Iterator;

        /**
         * @brief Constructs empty treeSet
         * @param comp Comparison function to use
         * @param alloc Allocator to use
         */
        explicit treeSet(Compare comp = Compare{}, ALLOC alloc = ALLOC{});

        /**
         * @brief Copy constructor
         * @param other treeSet to copy
         * @details Performs deep copy of all elements and tree structure
         * @note Allocator is copied if propagate_on_container_copy_assignment is true
         */
        treeSet(const treeSet& other);

        /**
         * @brief Copy assignment operator
         * @param other treeSet to copy
         * @return Reference to this treeSet
         * @details Performs deep copy of all elements and tree structure
         * @note Allocator is copied if propagate_on_container_copy_assignment is true
         */
        treeSet& operator=(const treeSet& other);

        /**
         * @brief Move constructor
         * @param other treeSet to move from
         * @details Transfers ownership of resources from other
         * @note Leaves other in valid but unspecified state
         */
        treeSet(treeSet&& other) noexcept;

        /**
         * @brief Move assignment operator
         * @param other treeSet to move from
         * @return Reference to this treeSet
         * @details Transfers ownership of resources from other
         * @note Leaves other in valid but unspecified state
         * @note Allocator is moved if propagate_on_container_move_assignment is true
         */
        treeSet& operator=(treeSet&& other) noexcept;

        /**
         * @brief Swaps contents with another treeSet
         * @param other treeSet to swap with
         * @note No-throw guarantee if element swap and allocator swap are noexcept
         * @details Efficiently exchanges:
         * - Root nodes of the Red-Black Trees
         * - Size counters
         * - Comparison function instances
         * - Allocators (if propagate_on_container_swap is true)
         *
         * Performance: O(1) - pointer swaps only
         * Iterator Invalidation: All iterators from both sets are invalidated
         *
         * Example usage:
         * @code{.cpp}
         * treeSet<int> set1, set2;
         * set1.add(1); set2.add(2);
         * set1.swap(set2);
         * // Now set1 contains 2, set2 contains 1
         * @endcode
         */
        void swap(treeSet& other) noexcept;

        /**
         * @brief Gets number of elements
         * @return Current size
         */
        [[nodiscard]] u_integer size() const override;

        /**
         * @brief Checks if element exists
         * @param e Element to check
         * @return true if element exists
         */
        bool contains(const TYPE &e) const override;

        /**
         * @brief Adds new element
         * @param e Element to add
         * @return true if added, false if element existed
         */
        bool add(const TYPE &e) override;

        /**
         * @brief Removes element
         * @param e Element to remove
         * @return true if removed, false if element didn't exist
         */
        bool remove(const TYPE &e) override;

        /**
         * @brief Gets begin iterator
         * @return New iterator at first element (minimum element)
         */
        Iterator* begins() const override;

        /**
         * @brief Gets end iterator
         * @return New iterator at last element (maximum element)
         */
        Iterator* ends() const override;

        /**
         * @brief Gets class name
         * @return "treeSet"
         */
        [[nodiscard]] std::string className() const override;

        /**
         * @brief Converts to string representation
         * @param enter Add newline if true
         * @return String representation of elements
         */
        [[nodiscard]] std::string toString(bool enter) const override;

        /**
         * @brief Destructor
         * @details Cleans up all tree nodes and allocated memory
         */
        ~treeSet() override;
    };


    /**
     * @class JSet
     * @tparam TYPE Element type (must be comparable)
     * @tparam Compare Comparison function type (default: increaseComparator<TYPE>)
     * @tparam ALLOC Allocator type (default: allocator<couple<const TYPE, const bool>>)
     * @brief Skip List based implementation of the set interface
     * @details This class provides a concrete implementation of the set interface
     * using a probabilistic skip list. It combines the functionality of:
     * - set (interface)
     * - skipList (storage with bool values)
     * - iterable (iteration support)
     *
     * Performance Characteristics:
     * - Insertion: Average O(log n), Worst O(n)
     * - Lookup: Average O(log n), Worst O(n)
     * - Deletion: Average O(log n), Worst O(n)
     *
     * The implementation guarantees:
     * - Elements sorted according to comparator
     * - Unique elements (no duplicates)
     * - Type safety
     * - Exception safety (basic guarantee)
     * - Iterator validity unless modified
     */
    template <typename TYPE,
            typename Compare = increaseComparator<TYPE>,
            typename ALLOC = allocator<couple<const TYPE, const bool>>>
    class JSet final : public set<TYPE, ALLOC>,
                       public skipList<const TYPE, const bool, ALLOC, Compare>,
                       public iterable<const TYPE>,
                       public printable {
        using skipListType = skipList<const TYPE, const bool, ALLOC, Compare>;

        /**
         * @typedef skipListNode
         * @brief Internal node type used for Skip List storage
         */
        using skipListNode = skipListType::skipListNode;

    public:
        /**
         * @class Iterator
         * @brief Forward iterator for JSet
         * @details Provides iteration over JSet elements while maintaining:
         * - Sorted traversal order (according to comparator)
         * - Safe invalidation detection
         * - Const-correct access
         *
         * Iterator Characteristics:
         * - Forward iteration only (throws on reverse operations)
         * - Invalidates on container modification
         * - Lightweight copy semantics
         */
        class Iterator final : public skipListType::Iterator,
                               public baseIterator<const TYPE> {

            /**
             * @brief Compares iterator pointers for equality
             * @param other Iterator to compare with
             * @return true if iterators point to same element
             * @internal
             */
            bool equalPtr(const iterator<const TYPE>* other) const override;

        public:
            friend class JSet;

            /**
             * @brief Constructs iterator pointing to specific skip list node
             * @param cur Current node pointer
             * @note Internal constructor, not meant for direct use
             */
            explicit Iterator(skipListNode* cur);

            /**
             * @brief Copy constructor
             * @param other Iterator to copy
             */
            Iterator(const Iterator& other);

            /**
             * @brief Copy assignment operator
             * @param other Iterator to copy
             * @return Reference to this iterator
             */
            Iterator& operator=(const Iterator& other);

            /**
             * @brief Creates a copy of this iterator
             * @return New iterator instance
             */
            Iterator* clone() const override;

            /**
             * @brief Gets iterator class name
             * @return "JSet::Iterator"
             */
            [[nodiscard]] std::string className() const override;

            /**
             * @brief Advances iterator by steps
             * @param steps Number of positions to advance
             */
            void operator+=(integer steps) const override;

            /**
             * @brief Not supported (throws unSupportedMethodError)
             */
            void operator-=(integer steps) const override;

            /**
             * @brief Calculates distance between iterators
             * @param other Iterator to calculate distance to
             * @return Distance between iterators
             * @note Returns max/min integer values if iterators are not compatible
             */
            integer operator-(const iterator<const TYPE>& other) const override;

            /**
             * @brief Checks if more elements exist in forward direction
             * @return true if more elements available
             */
            [[nodiscard]] bool hasNext() const override;

            /**
             * @brief Not supported (throws unSupportedMethodError)
             */
            [[nodiscard]] bool hasPrev() const override;

            /**
             * @brief Checks if other is previous to this
             * @param other Iterator to check
             * @return true if other is previous
             */
            bool atPrev(const iterator<const TYPE>* other) const override;

            /**
             * @brief Checks if other is next to this
             * @param other Iterator to check
             * @return true if other is next
             */
            bool atNext(const iterator<const TYPE>* other) const override;

            /**
             * @brief Moves to next element
             */
            void next() const override;

            /**
             * @brief Not supported (throws unSupportedMethodError)
             */
            void prev() const override;

            /**
             * @brief Not supported (throws unSupportedMethodError)
             */
            Iterator* getPrev() const override;

            /**
             * @brief Gets current element (non-const)
             * @return Reference to current element
             */
            const TYPE& get() override;

            /**
             * @brief Gets current element (const)
             * @return Copy of current element
             */
            const TYPE get() const override;

            /**
             * @brief Not supported (throws unSupportedMethodError)
             */
            void set(const TYPE& data) override;

            /**
             * @brief Checks if iterator is valid
             * @return true if iterator points to valid element
             */
            [[nodiscard]] bool isValid() const override;

            ~Iterator() override = default;
        };

        friend class Iterator;

        /**
         * @brief Constructs empty JSet
         * @param comp Comparison function to use
         * @param alloc Allocator to use
         */
        explicit JSet(Compare comp = Compare{}, ALLOC alloc = ALLOC{});

        /**
         * @brief Copy constructor
         * @param other JSet to copy
         * @details Performs deep copy of all elements and skip list structure
         * @note Allocator is copied if propagate_on_container_copy_assignment is true
         */
        JSet(const JSet& other);

        /**
         * @brief Copy assignment operator
         * @param other JSet to copy
         * @return Reference to this JSet
         * @details Performs deep copy of all elements and skip list structure
         * @note Allocator is copied if propagate_on_container_copy_assignment is true
         */
        JSet& operator=(const JSet& other);

        /**
         * @brief Move constructor
         * @param other JSet to move from
         * @details Transfers ownership of resources from other
         * @note Leaves other in valid but unspecified state
         */
        JSet(JSet&& other) noexcept;

        /**
         * @brief Move assignment operator
         * @param other JSet to move from
         * @return Reference to this JSet
         * @details Transfers ownership of resources from other
         * @note Leaves other in valid but unspecified state
         * @note Allocator is moved if propagate_on_container_move_assignment is true
         */
        JSet& operator=(JSet&& other) noexcept;

        /**
         * @brief Swaps contents with another JSet
         * @param other JSet to swap with
         * @note No-throw guarantee if element swap and allocator swap are noexcept
         * @details Efficiently exchanges:
         * - Head nodes of the skip lists
         * - Size counters
         * - Comparison function instances
         * - Allocators (if propagate_on_container_swap is true)
         *
         * Performance: O(1) - pointer swaps only
         * Iterator Invalidation: All iterators from both sets are invalidated
         *
         * Example usage:
         * @code{.cpp}
         * JSet<int> set1, set2;
         * set1.add(1); set2.add(2);
         * set1.swap(set2);
         * // Now set1 contains 2, set2 contains 1
         * @endcode
         */
        void swap(JSet& other) noexcept;

        /**
         * @brief Gets number of elements
         * @return Current size
         */
        [[nodiscard]] u_integer size() const override;

        /**
         * @brief Checks if element exists
         * @param e Element to check
         * @return true if element exists
         */
        bool contains(const TYPE &e) const override;

        /**
         * @brief Adds new element
         * @param e Element to add
         * @return true if added, false if element existed
         */
        bool add(const TYPE &e) override;

        /**
         * @brief Removes element
         * @param e Element to remove
         * @return true if removed, false if element didn't exist
         * @note Average O(log n), worst case O(n) time complexity
         */
        bool remove(const TYPE &e) override;

        /**
         * @brief Gets begin iterator
         * @return New iterator at first element (minimum element)
         * @note O(log n) time complexity to find first element
         */
        Iterator* begins() const override;

        /**
         * @brief Gets end iterator
         * @return New iterator at last element (maximum element)
         * @note O(log n) time complexity to find last element
         */
        Iterator* ends() const override;

        /**
         * @brief Gets class name
         * @return "JSet"
         */
        [[nodiscard]] std::string className() const override;

        /**
         * @brief Converts to string representation
         * @param enter Add newline if true
         * @return String representation of elements
         * @note O(n) time complexity
         */
        [[nodiscard]] std::string toString(bool enter) const override;

        /**
         * @brief Destructor
         * @details Cleans up all skip list nodes and allocated memory
         * @note O(n) time complexity
         */
        ~JSet() override;
    };
}

namespace std {
    /**
     * @brief std::swap specialization for hashSet
     * @tparam TYPE Element type
     * @tparam HASH Hash function type
     * @tparam ALLOC Allocator type
     * @param lhs First hashSet to swap
     * @param rhs Second hashSet to swap
     * @note No-throw guarantee if hashSet::swap is noexcept
     * @details Enables ADL-friendly swapping for use with standard algorithms
     * and containers. Delegates to hashSet::swap for actual implementation.
     *
     * Example usage with standard algorithms:
     * @code{.cpp}
     * hashSet<int> set1, set2;
     * std::swap(set1, set2);  // Uses this specialization
     * std::sort(/* ... * /, [](auto& a, auto& b) {
     *     std::swap(a, b);    // Also uses this specialization
     * });
     * @endcode
     */
    template <typename TYPE, typename HASH, typename ALLOC>
    void swap(original::hashSet<TYPE, HASH, ALLOC>& lhs, // NOLINT
              original::hashSet<TYPE, HASH, ALLOC>& rhs) noexcept;

    /**
     * @brief std::swap specialization for treeSet
     * @tparam TYPE Element type
     * @tparam COMPARE Comparison function type
     * @tparam ALLOC Allocator type
     * @param lhs First treeSet to swap
     * @param rhs Second treeSet to swap
     * @note No-throw guarantee if treeSet::swap is noexcept
     * @details Enables ADL-friendly swapping for use with standard algorithms
     * and containers. Delegates to treeSet::swap for actual implementation.
     *
     * Example usage with standard algorithms:
     * @code{.cpp}
     * treeSet<int> set1, set2;
     * std::swap(set1, set2);  // Uses this specialization
     * std::sort(/* ... * /, [](auto& a, auto& b) {
     *     std::swap(a, b);    // Also uses this specialization
     * });
     * @endcode
     */
    template <typename TYPE, typename COMPARE, typename ALLOC>
    void swap(original::treeSet<TYPE, COMPARE, ALLOC>& lhs, // NOLINT
              original::treeSet<TYPE, COMPARE, ALLOC>& rhs) noexcept;

    /**
     * @brief std::swap specialization for JSet
     * @tparam TYPE Element type
     * @tparam COMPARE Comparison function type
     * @tparam ALLOC Allocator type
     * @param lhs First JSet to swap
     * @param rhs Second JSet to swap
     * @note No-throw guarantee if JSet::swap is noexcept
     * @details Enables ADL-friendly swapping for use with standard algorithms
     * and containers. Delegates to JSet::swap for actual implementation.
     *
     * Example usage with standard algorithms:
     * @code{.cpp}
     * JSet<int> set1, set2;
     * std::swap(set1, set2);  // Uses this specialization
     * std::sort(/* ... * /, [](auto& a, auto& b) {
     *     std::swap(a, b);    // Also uses this specialization
     * });
     * @endcode
     */
    template <typename TYPE, typename COMPARE, typename ALLOC>
    void swap(original::JSet<TYPE, COMPARE, ALLOC>& lhs, // NOLINT
              original::JSet<TYPE, COMPARE, ALLOC>& rhs) noexcept;
}

template<typename TYPE, typename HASH, typename ALLOC>
original::hashSet<TYPE, HASH, ALLOC>::Iterator::Iterator(vector<hashNode*, rebind_alloc_pointer> *buckets,
u_integer bucket, hashNode *node) : hashTable<TYPE, const bool, ALLOC, HASH>::Iterator(
    const_cast<vector<hashNode*, rebind_alloc_pointer>*>(buckets), bucket, node) {}

template<typename TYPE, typename HASH, typename ALLOC>
bool original::hashSet<TYPE, HASH, ALLOC>::Iterator::equalPtr(const iterator<const TYPE> *other) const {
    auto other_it = dynamic_cast<const Iterator*>(other);
    return other_it &&
           this->p_buckets == other_it->p_buckets &&
           this->cur_bucket == other_it->cur_bucket &&
           this->p_node == other_it->p_node;
}

template<typename TYPE, typename HASH, typename ALLOC>
original::hashSet<TYPE, HASH, ALLOC>::Iterator::Iterator(const Iterator &other) : Iterator() {
    this->operator=(other);
}

template<typename TYPE, typename HASH, typename ALLOC>
original::hashSet<TYPE, HASH, ALLOC>::Iterator&
original::hashSet<TYPE, HASH, ALLOC>::Iterator::operator=(const Iterator &other) {
    if (this == &other) {
        return *this;
    }

    hashTable<TYPE, const bool, ALLOC, HASH>::Iterator::operator=(other);
    return *this;
}

template<typename TYPE, typename HASH, typename ALLOC>
original::hashSet<TYPE, HASH, ALLOC>::Iterator*
original::hashSet<TYPE, HASH, ALLOC>::Iterator::clone() const {
    return new Iterator(*this);
}

template<typename TYPE, typename HASH, typename ALLOC>
std::string original::hashSet<TYPE, HASH, ALLOC>::Iterator::className() const {
    return "hashSet::Iterator";
}

template<typename TYPE, typename HASH, typename ALLOC>
void original::hashSet<TYPE, HASH, ALLOC>::Iterator::operator+=(integer steps) const {
    hashTable<TYPE, const bool, ALLOC, HASH>::Iterator::operator+=(steps);
}

template<typename TYPE, typename HASH, typename ALLOC>
void original::hashSet<TYPE, HASH, ALLOC>::Iterator::operator-=(integer) const {
    throw unSupportedMethodError();
}

template<typename TYPE, typename HASH, typename ALLOC>
original::integer
original::hashSet<TYPE, HASH, ALLOC>::Iterator::operator-(const iterator<const TYPE>&) const {
    throw unSupportedMethodError();
}

template<typename TYPE, typename HASH, typename ALLOC>
bool original::hashSet<TYPE, HASH, ALLOC>::Iterator::hasNext() const {
    return hashTable<TYPE, const bool, ALLOC, HASH>::Iterator::hasNext();
}

template<typename TYPE, typename HASH, typename ALLOC>
bool original::hashSet<TYPE, HASH, ALLOC>::Iterator::hasPrev() const {
    throw unSupportedMethodError();
}

template<typename TYPE, typename HASH, typename ALLOC>
bool original::hashSet<TYPE, HASH, ALLOC>::Iterator::atPrev(const iterator<const TYPE> *other) const {
    auto other_it = dynamic_cast<const Iterator*>(other);
    if (!other_it) {
        return false;
    }
    auto next = ownerPtr(this->clone());
    if (!next->isValid()){
        return false;
    }

    next->next();
    return next->equalPtr(other_it);
}

template<typename TYPE, typename HASH, typename ALLOC>
bool original::hashSet<TYPE, HASH, ALLOC>::Iterator::atNext(const iterator<const TYPE> *other) const {
    return other->atPrev(*this);
}

template<typename TYPE, typename HASH, typename ALLOC>
void original::hashSet<TYPE, HASH, ALLOC>::Iterator::next() const {
    hashTable<TYPE, const bool, ALLOC, HASH>::Iterator::next();
}

template<typename TYPE, typename HASH, typename ALLOC>
void original::hashSet<TYPE, HASH, ALLOC>::Iterator::prev() const {
    throw unSupportedMethodError();
}

template<typename TYPE, typename HASH, typename ALLOC>
original::hashSet<TYPE, HASH, ALLOC>::Iterator*
original::hashSet<TYPE, HASH, ALLOC>::Iterator::getPrev() const {
    throw unSupportedMethodError();
}

template<typename TYPE, typename HASH, typename ALLOC>
const TYPE& original::hashSet<TYPE, HASH, ALLOC>::Iterator::get() {
    return hashTable<TYPE, const bool, ALLOC, HASH>::Iterator::get().template get<0>();
}

template<typename TYPE, typename HASH, typename ALLOC>
const TYPE original::hashSet<TYPE, HASH, ALLOC>::Iterator::get() const {
    return hashTable<TYPE, const bool, ALLOC, HASH>::Iterator::get().template get<0>();
}

template<typename TYPE, typename HASH, typename ALLOC>
void original::hashSet<TYPE, HASH, ALLOC>::Iterator::set(const TYPE&) {
    throw unSupportedMethodError();
}

template<typename TYPE, typename HASH, typename ALLOC>
bool original::hashSet<TYPE, HASH, ALLOC>::Iterator::isValid() const {
    return hashTable<TYPE, const bool, ALLOC, HASH>::Iterator::isValid();
}

template<typename TYPE, typename HASH, typename ALLOC>
original::hashSet<TYPE, HASH, ALLOC>::hashSet(HASH hash, ALLOC alloc)
    : hashTable<TYPE, const bool, ALLOC, HASH>(std::move(hash)),
      set<TYPE, ALLOC>(std::move(alloc)) {}

template<typename TYPE, typename HASH, typename ALLOC>
original::hashSet<TYPE, HASH, ALLOC>::hashSet(const hashSet &other) : hashSet() {
    this->operator=(other);
}

template<typename TYPE, typename HASH, typename ALLOC>
original::hashSet<TYPE, HASH, ALLOC>&
original::hashSet<TYPE, HASH, ALLOC>::operator=(const hashSet &other) {
    if (this == &other) {
        return *this;
    }

    this->buckets = this->bucketsCopy(other.buckets);
    this->size_ = other.size_;
    this->hash_ = other.hash_;
    if constexpr(ALLOC::propagate_on_container_copy_assignment::value) {
        this->allocator = other.allocator;
        this->rebind_alloc = other.rebind_alloc;
    }
    return *this;
}

template<typename TYPE, typename HASH, typename ALLOC>
original::hashSet<TYPE, HASH, ALLOC>::hashSet(hashSet &&other) noexcept : hashSet() {
    this->operator=(std::move(other));
}

template<typename TYPE, typename HASH, typename ALLOC>
original::hashSet<TYPE, HASH, ALLOC>&
original::hashSet<TYPE, HASH, ALLOC>::operator=(hashSet &&other) noexcept {
    if (this == &other) {
        return *this;
    }

    this->buckets = std::move(other.buckets);
    this->size_ = other.size_;
    other.size_ = 0;
    this->hash_ = std::move(other.hash_);
    if constexpr(ALLOC::propagate_on_container_move_assignment::value) {
        this->allocator = std::move(other.allocator);
        this->rebind_alloc = std::move(other.rebind_alloc);
    }
    return *this;
}

template<typename TYPE, typename HASH, typename ALLOC>
void original::hashSet<TYPE, HASH, ALLOC>::swap(hashSet& other) noexcept
{
    if (this == &other)
        return;

    std::swap(this->size_, other.size_);
    std::swap(this->buckets, other.buckets);
    std::swap(this->hash_, other.hash_);
    if constexpr (ALLOC::propagate_on_container_swap::value) {
        std::swap(this->allocator, other.allocator);
        std::swap(this->rebind_alloc, other.rebind_alloc);
    }
}

template<typename TYPE, typename HASH, typename ALLOC>
original::u_integer original::hashSet<TYPE, HASH, ALLOC>::size() const {
    return this->size_;
}

template<typename TYPE, typename HASH, typename ALLOC>
bool original::hashSet<TYPE, HASH, ALLOC>::contains(const TYPE &e) const {
    return this->find(e);
}

template<typename TYPE, typename HASH, typename ALLOC>
bool original::hashSet<TYPE, HASH, ALLOC>::add(const TYPE &e) {
    return this->insert(e, true);
}

template<typename TYPE, typename HASH, typename ALLOC>
bool original::hashSet<TYPE, HASH, ALLOC>::remove(const TYPE &e) {
    return this->erase(e);
}

template<typename TYPE, typename HASH, typename ALLOC>
original::hashSet<TYPE, HASH, ALLOC>::Iterator*
original::hashSet<TYPE, HASH, ALLOC>::begins() const {
    auto p_buckets = const_cast<vector<hashNode*, rebind_alloc_pointer>*>(&this->buckets);
    if (this->buckets[0]) {
        return new Iterator(p_buckets, 0, this->buckets[0]);
    }
    auto bucket = Iterator::findNextValidBucket(p_buckets, 0);
    return new Iterator(p_buckets, bucket, p_buckets->get(bucket));
}

template<typename TYPE, typename HASH, typename ALLOC>
original::hashSet<TYPE, HASH, ALLOC>::Iterator*
original::hashSet<TYPE, HASH, ALLOC>::ends() const {
    auto p_buckets = const_cast<vector<hashNode*, rebind_alloc_pointer>*>(&this->buckets);
    auto bucket = Iterator::findPrevValidBucket(p_buckets, this->buckets.size());
    auto node = this->buckets[bucket];
    while (node && node->getPNext()) {
        node = node->getPNext();
    }
    return new Iterator(p_buckets, bucket, node);
}

template<typename TYPE, typename HASH, typename ALLOC>
std::string original::hashSet<TYPE, HASH, ALLOC>::className() const {
    return "hashSet";
}

template<typename TYPE, typename HASH, typename ALLOC>
std::string original::hashSet<TYPE, HASH, ALLOC>::toString(const bool enter) const {
    std::stringstream ss;
    ss << this->className();
    ss << "(";
    bool first = true;
    for (auto it = this->begin(); it != this->end(); it.next()){
        if (!first){
            ss << ", ";
        }
        ss << printable::formatString(it.get());
        first = false;
    }
    ss << ")";
    if (enter)
        ss << "\n";
    return ss.str();
}

template<typename TYPE, typename HASH, typename ALLOC>
original::hashSet<TYPE, HASH, ALLOC>::~hashSet() = default;

template <typename TYPE, typename Compare, typename ALLOC>
original::treeSet<TYPE, Compare, ALLOC>::Iterator::Iterator(RBTreeType* tree, RBNode* cur)
    : RBTreeType::Iterator(tree, cur)  {}

template <typename TYPE, typename Compare, typename ALLOC>
bool original::treeSet<TYPE, Compare, ALLOC>::Iterator::equalPtr(const iterator<const TYPE>* other) const
{
    auto other_it = dynamic_cast<const Iterator*>(other);
    return other_it &&
           this->tree_ == other_it->tree_ &&
           this->cur_ == other_it->cur_;
}

template <typename TYPE, typename Compare, typename ALLOC>
original::treeSet<TYPE, Compare, ALLOC>::Iterator::Iterator(const Iterator& other)
    : Iterator(nullptr, nullptr)
{
    this->operator=(other);
}

template <typename TYPE, typename Compare, typename ALLOC>
original::treeSet<TYPE, Compare, ALLOC>::Iterator&
original::treeSet<TYPE, Compare, ALLOC>::Iterator::operator=(const Iterator& other)
{
    if (this == &other) {
        return *this;
    }

    this->tree_ = other.tree_;
    this->cur_ = other.cur_;
    return *this;
}

template <typename TYPE, typename Compare, typename ALLOC>
original::treeSet<TYPE, Compare, ALLOC>::Iterator*
original::treeSet<TYPE, Compare, ALLOC>::Iterator::clone() const
{
    return new Iterator(*this);
}

template <typename TYPE, typename Compare, typename ALLOC>
std::string original::treeSet<TYPE, Compare, ALLOC>::Iterator::className() const
{
    return "treeSet::Iterator";
}

template <typename TYPE, typename Compare, typename ALLOC>
void original::treeSet<TYPE, Compare, ALLOC>::Iterator::operator+=(integer steps) const
{
    RBTreeType::Iterator::operator+=(steps);
}

template <typename TYPE, typename Compare, typename ALLOC>
void original::treeSet<TYPE, Compare, ALLOC>::Iterator::operator-=(integer steps) const
{
    RBTreeType::Iterator::operator-=(steps);
}

template <typename TYPE, typename Compare, typename ALLOC>
original::integer
original::treeSet<TYPE, Compare, ALLOC>::Iterator::operator-(const iterator<const TYPE>&) const
{
    throw unSupportedMethodError();
}

template <typename TYPE, typename Compare, typename ALLOC>
bool original::treeSet<TYPE, Compare, ALLOC>::Iterator::hasNext() const
{
    return RBTreeType::Iterator::hasNext();
}

template <typename TYPE, typename Compare, typename ALLOC>
bool original::treeSet<TYPE, Compare, ALLOC>::Iterator::hasPrev() const
{
    return RBTreeType::Iterator::hasPrev();
}

template <typename TYPE, typename Compare, typename ALLOC>
bool original::treeSet<TYPE, Compare, ALLOC>::Iterator::atPrev(const iterator<const TYPE>* other) const
{
    auto other_it = dynamic_cast<const Iterator*>(other);
    if (!other_it) {
        return false;
    }
    auto next = ownerPtr(this->clone());
    if (!next->isValid()){
        return false;
    }

    next->next();
    return next->equalPtr(other_it);
}

template <typename TYPE, typename Compare, typename ALLOC>
bool original::treeSet<TYPE, Compare, ALLOC>::Iterator::atNext(const iterator<const TYPE>* other) const
{
    return other->atNext(*this);
}

template <typename TYPE, typename Compare, typename ALLOC>
void original::treeSet<TYPE, Compare, ALLOC>::Iterator::next() const
{
    RBTreeType::Iterator::next();
}

template <typename TYPE, typename Compare, typename ALLOC>
void original::treeSet<TYPE, Compare, ALLOC>::Iterator::prev() const
{
    RBTreeType::Iterator::prev();
}

template <typename TYPE, typename Compare, typename ALLOC>
original::treeSet<TYPE, Compare, ALLOC>::Iterator*
original::treeSet<TYPE, Compare, ALLOC>::Iterator::getPrev() const
{
    auto it = this->clone();
    it->prev();
    return it;
}

template <typename TYPE, typename Compare, typename ALLOC>
const TYPE& original::treeSet<TYPE, Compare, ALLOC>::Iterator::get()
{
    return RBTreeType::Iterator::get().template get<0>();
}

template <typename TYPE, typename Compare, typename ALLOC>
const TYPE original::treeSet<TYPE, Compare, ALLOC>::Iterator::get() const
{
    return RBTreeType::Iterator::get().template get<0>();
}

template <typename TYPE, typename Compare, typename ALLOC>
void original::treeSet<TYPE, Compare, ALLOC>::Iterator::set(const TYPE&)
{
    throw unSupportedMethodError();
}

template <typename TYPE, typename Compare, typename ALLOC>
bool original::treeSet<TYPE, Compare, ALLOC>::Iterator::isValid() const
{
    return RBTreeType::Iterator::isValid();
}

template<typename TYPE, typename Compare, typename ALLOC>
original::treeSet<TYPE, Compare, ALLOC>::treeSet(Compare comp, ALLOC alloc)
    : RBTreeType(std::move(comp)),
      set<TYPE, ALLOC>(std::move(alloc)) {}

template<typename TYPE, typename Compare, typename ALLOC>
original::treeSet<TYPE, Compare, ALLOC>::treeSet(const treeSet& other) : treeSet() {
    this->operator=(other);
}

template<typename TYPE, typename Compare, typename ALLOC>
original::treeSet<TYPE, Compare, ALLOC>&
original::treeSet<TYPE, Compare, ALLOC>::operator=(const treeSet& other) {
    if (this == &other){
        return *this;
    }

    this->destroyTree();
    this->root_ = other.treeCopy();
    this->size_ = other.size_;
    this->compare_ = other.compare_;
    if constexpr(ALLOC::propagate_on_container_copy_assignment::value) {
        this->allocator = other.allocator;
        this->rebind_alloc = other.rebind_alloc;
    }
    return *this;
}

template<typename TYPE, typename Compare, typename ALLOC>
original::treeSet<TYPE, Compare, ALLOC>::treeSet(treeSet&& other) noexcept : treeSet() {
    this->operator=(std::move(other));
}

template<typename TYPE, typename Compare, typename ALLOC>
original::treeSet<TYPE, Compare, ALLOC>&
original::treeSet<TYPE, Compare, ALLOC>::operator=(treeSet&& other) noexcept {
    if (this == &other){
        return *this;
    }

    this->destroyTree();
    this->root_ = other.root_;
    other.root_ = nullptr;
    this->size_ = other.size_;
    other.size_ = 0;
    this->compare_ = std::move(other.compare_);
    if constexpr(ALLOC::propagate_on_container_move_assignment::value) {
        this->allocator = std::move(other.allocator);
        this->rebind_alloc = std::move(other.rebind_alloc);
    }
    return *this;
}

template<typename TYPE, typename Compare, typename ALLOC>
void original::treeSet<TYPE, Compare, ALLOC>::swap(treeSet& other) noexcept
{
    if (this == &other)
        return;

    std::swap(this->root_, other.root_);
    std::swap(this->size_, other.size_);
    std::swap(this->compare_, other.compare_);
    if constexpr (ALLOC::propagate_on_container_swap::value) {
        std::swap(this->allocator, other.allocator);
        std::swap(this->rebind_alloc, other.rebind_alloc);
    }
}

template<typename TYPE, typename Compare, typename ALLOC>
original::u_integer original::treeSet<TYPE, Compare, ALLOC>::size() const {
    return this->size_;
}

template<typename TYPE, typename Compare, typename ALLOC>
bool original::treeSet<TYPE, Compare, ALLOC>::contains(const TYPE &e) const {
    return this->find(e);
}

template<typename TYPE, typename Compare, typename ALLOC>
bool original::treeSet<TYPE, Compare, ALLOC>::add(const TYPE &e) {
    return this->insert(e, true);
}

template<typename TYPE, typename Compare, typename ALLOC>
bool original::treeSet<TYPE, Compare, ALLOC>::remove(const TYPE &e) {
    return this->erase(e);
}

template <typename TYPE, typename Compare, typename ALLOC>
original::treeSet<TYPE, Compare, ALLOC>::Iterator*
original::treeSet<TYPE, Compare, ALLOC>::begins() const
{
    return new Iterator(const_cast<treeSet*>(this), this->getMinNode());
}

template <typename TYPE, typename Compare, typename ALLOC>
original::treeSet<TYPE, Compare, ALLOC>::Iterator*
original::treeSet<TYPE, Compare, ALLOC>::ends() const
{
    return new Iterator(const_cast<treeSet*>(this), this->getMaxNode());
}

template<typename TYPE, typename Compare, typename ALLOC>
std::string original::treeSet<TYPE, Compare, ALLOC>::className() const {
    return "treeSet";
}

template<typename TYPE, typename Compare, typename ALLOC>
std::string original::treeSet<TYPE, Compare, ALLOC>::toString(const bool enter) const {
    std::stringstream ss;
    ss << this->className();
    ss << "(";
    bool first = true;
    for (auto it = this->begin(); it != this->end(); it.next()){
        if (!first){
            ss << ", ";
        }
        ss << printable::formatString(it.get());
        first = false;
    }
    ss << ")";
    if (enter)
        ss << "\n";
    return ss.str();
}

template<typename TYPE, typename Compare, typename ALLOC>
original::treeSet<TYPE, Compare, ALLOC>::~treeSet() = default;

template<typename TYPE, typename Compare, typename ALLOC>
bool original::JSet<TYPE, Compare, ALLOC>::Iterator::equalPtr(
        const iterator<const TYPE>* other) const {
    auto other_it = dynamic_cast<const Iterator*>(other);
    return other_it && this->cur_ == other_it->cur_;
}

template<typename TYPE, typename Compare, typename ALLOC>
original::JSet<TYPE, Compare, ALLOC>::Iterator::Iterator(skipListNode* cur)
        : skipListType::Iterator(cur) {}

template<typename TYPE, typename Compare, typename ALLOC>
original::JSet<TYPE, Compare, ALLOC>::Iterator::Iterator(const Iterator& other)
        : skipListType::Iterator(other) {}

template<typename TYPE, typename Compare, typename ALLOC>
original::JSet<TYPE, Compare, ALLOC>::Iterator&
original::JSet<TYPE, Compare, ALLOC>::Iterator::operator=(const Iterator& other) {
    skipListType::Iterator::operator=(other);
    return *this;
}

template<typename TYPE, typename Compare, typename ALLOC>
original::JSet<TYPE, Compare, ALLOC>::Iterator*
original::JSet<TYPE, Compare, ALLOC>::Iterator::clone() const {
    return new Iterator(*this);
}

template<typename TYPE, typename Compare, typename ALLOC>
std::string
original::JSet<TYPE, Compare, ALLOC>::Iterator::className() const {
    return "JSet::Iterator";
}

template<typename TYPE, typename Compare, typename ALLOC>
void original::JSet<TYPE, Compare, ALLOC>::Iterator::operator+=(integer steps) const {
    skipListType::Iterator::operator+=(steps);
}

template<typename TYPE, typename Compare, typename ALLOC>
void original::JSet<TYPE, Compare, ALLOC>::Iterator::operator-=(integer) const {
    throw unSupportedMethodError();
}

template<typename TYPE, typename Compare, typename ALLOC>
original::integer
original::JSet<TYPE, Compare, ALLOC>::Iterator::operator-(
        const iterator<const TYPE>& other) const {
    auto other_it = dynamic_cast<const Iterator*>(&other);
    if (other_it == nullptr)
        return this > &other ?
               (std::numeric_limits<integer>::max)() :
               (std::numeric_limits<integer>::min)();
    return skipListType::Iterator::operator-(*other_it);
}

template<typename TYPE, typename Compare, typename ALLOC>
bool original::JSet<TYPE, Compare, ALLOC>::Iterator::hasNext() const {
    return skipListType::Iterator::hasNext();
}

template<typename TYPE, typename Compare, typename ALLOC>
bool original::JSet<TYPE, Compare, ALLOC>::Iterator::hasPrev() const {
    throw unSupportedMethodError();
}

template<typename TYPE, typename Compare, typename ALLOC>
bool original::JSet<TYPE, Compare, ALLOC>::Iterator::atPrev(
        const iterator<const TYPE>* other) const {
    const auto other_it = dynamic_cast<const Iterator*>(other);
    if (!other_it){
        return false;
    }
    auto cloned_it = ownerPtr(other_it->clone());
    return this->equalPtr(cloned_it.get());
}

template<typename TYPE, typename Compare, typename ALLOC>
bool original::JSet<TYPE, Compare, ALLOC>::Iterator::atNext(
        const original::iterator<const TYPE>* other) const {
    return other->atPrev(*this);
}

template<typename TYPE, typename Compare, typename ALLOC>
void original::JSet<TYPE, Compare, ALLOC>::Iterator::next() const {
    skipListType::Iterator::next();
}

template<typename TYPE, typename Compare, typename ALLOC>
void original::JSet<TYPE, Compare, ALLOC>::Iterator::prev() const {
    throw unSupportedMethodError();
}

template<typename TYPE, typename Compare, typename ALLOC>
original::JSet<TYPE, Compare, ALLOC>::Iterator*
original::JSet<TYPE, Compare, ALLOC>::Iterator::getPrev() const {
    throw unSupportedMethodError();
}

template<typename TYPE, typename Compare, typename ALLOC>
const TYPE& original::JSet<TYPE, Compare, ALLOC>::Iterator::get() {
    return skipListType::Iterator::get().template get<0>();
}

template<typename TYPE, typename Compare, typename ALLOC>
const TYPE original::JSet<TYPE, Compare, ALLOC>::Iterator::get() const {
    return skipListType::Iterator::get().template get<0>();
}

template<typename TYPE, typename Compare, typename ALLOC>
void original::JSet<TYPE, Compare, ALLOC>::Iterator::set(const TYPE&) {
    throw unSupportedMethodError();
}

template<typename TYPE, typename Compare, typename ALLOC>
bool original::JSet<TYPE, Compare, ALLOC>::Iterator::isValid() const {
    return skipListType::Iterator::isValid();
}

template<typename TYPE, typename Compare, typename ALLOC>
original::JSet<TYPE, Compare, ALLOC>::JSet(Compare comp, ALLOC alloc)
        : set<TYPE, ALLOC>(std::move(alloc)),
          skipListType(std::move(comp)) {}

template<typename TYPE, typename Compare, typename ALLOC>
original::JSet<TYPE, Compare, ALLOC>::JSet(const JSet& other) : JSet() {
    this->operator=(other);
}

template<typename TYPE, typename Compare, typename ALLOC>
original::JSet<TYPE, Compare, ALLOC>&
original::JSet<TYPE, Compare, ALLOC>::operator=(const JSet& other) {
    if (this == &other){
        return *this;
    }

    this->listDestroy();
    this->head_ = other.listCopy();
    this->size_ = other.size_;
    this->compare_ = other.compare_;
    if constexpr(ALLOC::propagate_on_container_copy_assignment::value) {
        this->allocator = other.allocator;
        this->rebind_alloc = other.rebind_alloc;
    }
    return *this;
}

template<typename TYPE, typename Compare, typename ALLOC>
original::JSet<TYPE, Compare, ALLOC>::JSet(JSet&& other) noexcept : JSet() {
    this->operator=(std::move(other));
}

template<typename TYPE, typename Compare, typename ALLOC>
original::JSet<TYPE, Compare, ALLOC>&
original::JSet<TYPE, Compare, ALLOC>::operator=(JSet&& other) noexcept {
    if (this == &other){
        return *this;
    }

    this->listDestroy();
    this->head_ = other.head_;
    other.head_ = other.createNode();
    this->size_ = other.size_;
    other.size_ = 0;
    this->compare_ = std::move(other.compare_);
    if constexpr(ALLOC::propagate_on_container_move_assignment::value) {
        this->allocator = std::move(other.allocator);
        this->rebind_alloc = std::move(other.rebind_alloc);
    }
    return *this;
}

template<typename TYPE, typename Compare, typename ALLOC>
void original::JSet<TYPE, Compare, ALLOC>::swap(JSet& other) noexcept
{
    if (this == &other)
        return;

    std::swap(this->size_, other.size_);
    std::swap(this->head_, other.head_);
    std::swap(this->compare_, other.compare_);
    if constexpr (ALLOC::propagate_on_container_swap::value) {
        std::swap(this->allocator, other.allocator);
        std::swap(this->rebind_alloc, other.rebind_alloc);
    }
}

template<typename TYPE, typename Compare, typename ALLOC>
original::u_integer original::JSet<TYPE, Compare, ALLOC>::size() const {
    return this->size_;
}

template<typename TYPE, typename Compare, typename ALLOC>
bool original::JSet<TYPE, Compare, ALLOC>::contains(const TYPE &e) const {
    return this->find(e);
}

template<typename TYPE, typename Compare, typename ALLOC>
bool original::JSet<TYPE, Compare, ALLOC>::add(const TYPE &e) {
    return this->insert(e, true);
}

template<typename TYPE, typename Compare, typename ALLOC>
bool original::JSet<TYPE, Compare, ALLOC>::remove(const TYPE &e) {
    return this->erase(e);
}

template<typename TYPE, typename Compare, typename ALLOC>
original::JSet<TYPE, Compare, ALLOC>::Iterator*
original::JSet<TYPE, Compare, ALLOC>::begins() const {
    return new Iterator(this->head_->getPNext(1));
}

template<typename TYPE, typename Compare, typename ALLOC>
original::JSet<TYPE, Compare, ALLOC>::Iterator*
original::JSet<TYPE, Compare, ALLOC>::ends() const {
    return new Iterator(this->findLastNode());
}

template<typename TYPE, typename Compare, typename ALLOC>
std::string original::JSet<TYPE, Compare, ALLOC>::className() const {
    return "JSet";
}

template<typename TYPE, typename Compare, typename ALLOC>
std::string original::JSet<TYPE, Compare, ALLOC>::toString(bool enter) const {
    std::stringstream ss;
    ss << this->className();
    ss << "(";
    bool first = true;
    for (auto it = this->begin(); it != this->end(); it.next()){
        if (!first){
            ss << ", ";
        }
        ss << printable::formatString(it.get());
        first = false;
    }
    ss << ")";
    if (enter)
        ss << "\n";
    return ss.str();
}

template<typename TYPE, typename Compare, typename ALLOC>
original::JSet<TYPE, Compare, ALLOC>::~JSet() = default;

template <typename TYPE, typename HASH, typename ALLOC>
void std::swap(original::hashSet<TYPE, HASH, ALLOC>& lhs, // NOLINT
               original::hashSet<TYPE, HASH, ALLOC>& rhs) noexcept
{
    lhs.swap(rhs);
}

template <typename TYPE, typename COMPARE, typename ALLOC>
void std::swap(original::treeSet<TYPE, COMPARE, ALLOC>& lhs, // NOLINT
               original::treeSet<TYPE, COMPARE, ALLOC>& rhs) noexcept
{
    lhs.swap(rhs);
}

template <typename TYPE, typename COMPARE, typename ALLOC>
void std::swap(original::JSet<TYPE, COMPARE, ALLOC>& lhs, // NOLINT
               original::JSet<TYPE, COMPARE, ALLOC>& rhs) noexcept
{
    lhs.swap(rhs);
}

#endif //SETS_H
