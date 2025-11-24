#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "allocator.h"
#include "couple.h"
#include "hash.h"
#include "vector.h"
#include "wrapper.h"


/**
 * @file hashTable.h
 * @brief Implementation of hashTable container
 * @details Provides a hash table implementation with:
 * - Average O(1) time complexity for basic operations
 * - Separate chaining collision resolution
 * - Customizable hash function and allocator
 * - Iterator support
 *
 * Key Features:
 * - Base class for hash-based containers
 * - Printable interface support
 * - Dynamic resizing based on load factor
 * - Predefined bucket sizes for optimal performance
 * - Exception-safe implementation
 */

namespace original {

    /**
     * @class hashTable
     * @tparam K_TYPE Key type (must be hashable)
     * @tparam V_TYPE Value type
     * @tparam ALLOC Allocator type (default: allocator<K_TYPE>)
     * @tparam HASH Hash function type (default: hash<K_TYPE>)
     * @brief Hash table implementation with separate chaining
     * @details This class provides a generic hash table implementation that serves as the
     * base for hash-based containers. It implements:
     * - Key-value pair storage
     * - Dynamic resizing
     * - Basic hash table operations
     *
     * Performance Characteristics:
     * - Insertion: Average O(1), Worst O(n)
     * - Lookup: Average O(1), Worst O(n)
     * - Deletion: Average O(1), Worst O(n)
     *
     * The implementation guarantees:
     * - Unique keys (no duplicates)
     * - Automatic resizing when load factor thresholds are crossed
     * - Exception safety (basic guarantee)
     */
    template<typename K_TYPE, typename V_TYPE, typename ALLOC = allocator<K_TYPE>, typename HASH = hash<K_TYPE>>
    class hashTable{
    public:

        /**
         * @class hashNode
         * @brief Internal node type for hash table storage
         * @details Wraps key-value pairs and maintains chain links for separate chaining.
         * Each node contains:
         * - A key-value pair (couple<const K_TYPE, V_TYPE>)
         * - A pointer to the next node in the chain
         *
         * The class provides:
         * - Key/value access methods
         * - Chain manipulation methods
         * - Proper memory management through wrapper interface
         */
        class hashNode final : public wrapper<couple<const K_TYPE, V_TYPE>> {
            couple<const K_TYPE, V_TYPE> data_;
            hashNode* next_;

        public:
            /**
             * @brief Constructs a new hash node
             * @param key Key to store (const reference)
             * @param value Value to associate
             * @param next Next node in chain (nullptr if end of chain)
             */
            explicit hashNode(const K_TYPE& key = K_TYPE{}, const V_TYPE& value = V_TYPE{}, hashNode* next = nullptr);

            /**
             * @brief Copy constructor
             * @param other Node to copy from
             */
            hashNode(const hashNode& other);

            /**
             * @brief Copy assignment operator
             * @param other Node to copy from
             * @return Reference to this node
             */
            hashNode& operator=(const hashNode& other);

            /**
             * @brief Gets key-value pair (non-const)
             * @return Reference to the contained pair
             */
            couple<const K_TYPE, V_TYPE>& getVal() override;

            /**
             * @brief Gets key-value pair (const)
             * @return Const reference to the contained pair
             */
            const couple<const K_TYPE, V_TYPE>& getVal() const override;

            /**
             * @brief Gets the key (const)
             * @return Const reference to the key
             */
            const K_TYPE& getKey() const;

            /**
             * @brief Gets the value (const)
             * @return Const reference to the value
             */
            const V_TYPE& getValue() const;

            /**
             * @brief Gets the value (non-const)
             * @return Reference to the value
             */
            V_TYPE& getValue();

            /**
             * @brief Not supported (throws unSupportedMethodError)
             */
            void setVal(couple<const K_TYPE, V_TYPE> data) override;

            /**
             * @brief Sets a new value
             * @param value New value to set
             */
            void setValue(const V_TYPE& value);

            /**
             * @brief Not supported (throws unSupportedMethodError)
             */
            hashNode* getPPrev() const override;

            /**
             * @brief Gets next node in chain
             * @return Pointer to next node (nullptr if end of chain)
             */
            hashNode* getPNext() const override;

            /**
             * @brief Sets next node in chain
             * @param new_next New next node to set
             */
            void setPNext(hashNode* new_next);

            /**
             * @brief Connects two nodes in chain
             * @param prev Previous node in chain
             * @param next Next node in chain
             * @note If prev is nullptr, does nothing
             */
            static void connect(hashNode* prev, hashNode* next);
        };

        /**
         * @typedef rebind_alloc_node
         * @brief Rebound allocator type for node storage
         */
        using rebind_alloc_node = ALLOC::template rebind_alloc<hashNode>;

        /**
         * @typedef rebind_alloc_pointer
         * @brief Rebound allocator type for pointer storage
         */
        using rebind_alloc_pointer = ALLOC::template rebind_alloc<hashNode*>;

        /**
         * @typedef buckets_type
         * @brief Type representing the hash table buckets container
         */
        using buckets_type = vector<hashNode*, rebind_alloc_pointer>;

        /**
         * @brief Minimum load factor before shrinking
         */
        static constexpr floating LOAD_FACTOR_MIN = 0.25;

        /**
         * @brief Maximum load factor before expanding
         */
        static constexpr floating LOAD_FACTOR_MAX = 0.75;

        /**
         * @brief Size of BUCKETS_SIZES
         * @ref BUCKETS_SIZES
         */
        static constexpr u_integer BUCKETS_SIZES_COUNT = 30;

        /**
         * @brief Predefined bucket sizes for hash table resizing
         * @details An array of prime numbers carefully selected for hash table bucket sizes.
         * These primes are used during table resizing to maintain optimal performance characteristics.
         *
         * Key Properties:
         * - All values are prime numbers to reduce hash collisions
         * - Each size is approximately double the previous (with some variance)
         * - Covers a wide range from small to very large tables
         * - Specifically chosen to avoid common modulo patterns
         *
         * Selection Criteria:
         * 1. Primes spaced roughly exponentially (growth factor ~1.8-2.2)
         * 2. Avoid primes close to powers of 2 to prevent clustering
         * 3. Sufficient gaps between sizes to justify resize operations
         * 4. Includes sizes suitable for both small and large datasets
         *
         * Performance Impact:
         * - Larger sizes reduce collisions but increase memory usage
         * - Smaller sizes conserve memory but may increase collisions
         * - The growth factor balances between resize frequency and memory overhead
         *
         * The sequence continues until reaching sizes suitable for maximum practical
         * in-memory hash tables (over 100 million buckets).
         *
         * @note The actual resize operation only occurs when the load factor
         * exceeds thresholds, not necessarily at every size transition.
         *
         * @see LOAD_FACTOR_MIN
         * @see LOAD_FACTOR_MAX
         */
        static constexpr u_integer BUCKETS_SIZES[] = {
                17,          29,          53,          97,          193,
                389,         769,         1543,        3079,        6151,
                12289,       24593,       49157,       98317,       196613,

                393241,      786433,      1572869,     3145739,     6291469,
                12582917,    25165843,    50331653,    100663319,   201326611,

                402653189,   805306457,   1610612741,  3221225473,  4294967291
        };

        u_integer size_;
        buckets_type buckets;
        HASH hash_;
        mutable rebind_alloc_node rebind_alloc{};

        /**
         * @class Iterator
         * @brief Forward iterator for hashTable
         * @details Provides forward iteration over hashTable elements with:
         * - Consistent traversal order (bucket by bucket, then chain order)
         * - Safe invalidation detection
         * - Const-correct access to elements
         *
         * Iterator Characteristics:
         * - Forward iteration only (throws on reverse operations)
         * - Invalidates on rehash operations
         * - Lightweight copy semantics
         * - STL-style iteration interface
         *
         * @note Iterators remain valid unless the table is rehashed
         */
        class Iterator {
        protected:
            mutable vector<hashNode*, rebind_alloc_pointer>* p_buckets;
            mutable u_integer cur_bucket;
            mutable hashNode* p_node;

            /**
             * @brief Finds the next non-empty bucket
             * @param buckets Reference to buckets vector
             * @param bucket Starting bucket index
             * @return Index of next non-empty bucket or buckets.size() if none
             * @internal
             */
            static u_integer findNextValidBucket(vector<hashNode *, rebind_alloc_pointer> *buckets,
                                              u_integer bucket);

            /**
             * @brief Finds the previous non-empty bucket
             * @param buckets Reference to buckets vector
             * @param bucket Starting bucket index
             * @return Index of previous non-empty bucket or buckets.size() if none
             * @internal
             */
            static u_integer findPrevValidBucket(vector<hashNode *, rebind_alloc_pointer> *buckets,
                                             u_integer bucket);

            /**
             * @brief Constructs an iterator pointing to specific position
             * @param buckets Pointer to buckets vector
             * @param bucket Current bucket index
             * @param node Current node pointer
             * @note Protected constructor for use by hashTable only
             */
            explicit Iterator(vector<hashNode*, rebind_alloc_pointer>* buckets = nullptr,
                              u_integer bucket = 0, hashNode* node = nullptr);

            /**
             * @brief Copy constructor
             * @param other Iterator to copy
             */
            Iterator(const Iterator& other);

            /**
             * @brief Copy assignment operator
             * @param other Iterator to copy from
             * @return Reference to this iterator
             */
            Iterator& operator=(const Iterator& other);
        public:
            /**
              * @brief Checks if more elements are available
              * @return true if more elements can be traversed
              */
            [[nodiscard]] bool hasNext() const;

            /**
             * @brief Advances to the next element
             * @throw outOfBoundError if already at end
             */
            void next() const;

            /**
             * @brief Advances iterator by steps positions
             * @param steps Number of positions to advance (must be >= 0)
             * @throw unSupportedMethodError if steps is negative
             */
            void operator+=(integer steps) const;

            /**
             * @brief Gets current key-value pair (non-const)
             * @return Reference to current pair
             * @throw outOfBoundError if iterator is invalid
             */
            couple<const K_TYPE, V_TYPE>& get();

            /**
             * @brief Gets current key-value pair (const)
             * @return Copy of current pair
             * @throw outOfBoundError if iterator is invalid
             */
            couple<const K_TYPE, V_TYPE> get() const;

            /**
             * @brief Checks if iterator points to valid element
             * @return true if iterator is valid
             */
            [[nodiscard]] bool isValid() const;
        };

        /**
         * @brief Creates a deep copy of the hash table's bucket array
         * @param old_buckets The original old_buckets array to copy
         * @return A new old_buckets array with cloned nodes
         *
         * @details
         * This function performs a deep copy of the entire old_buckets array used by the hash table,
         * including the internal linked list in each bucket (used for separate chaining).
         *
         * Copying strategy:
         * - For each bucket index, traverse the linked list of hash nodes
         * - Use `createNode()` to allocate a new node for each entry
         * - Preserve the order of elements within each bucket's chain
         *
         * This method is typically used in:
         * - Copy constructors
         * - Copy assignment operators
         *
         * The function ensures that:
         * - The original and copied hash tables do not share memory
         * - All keys and values are copied correctly
         * - Iterator validity is preserved for the new structure
         *
         * @note This function assumes the original old_buckets are well-formed (no cycles).
         * @note Uses allocator rebound to allocate memory for the new nodes.
         */
        buckets_type bucketsCopy(const buckets_type& old_buckets) const;

        /**
         * @brief Creates a new hash node
         * @param key Key for new node
         * @param value Value for new node
         * @param next Next node in chain
         * @return Pointer to newly created node
         * @note Uses the rebound allocator for memory management
         */
        hashNode* createNode(const K_TYPE& key = K_TYPE{}, const V_TYPE& value = V_TYPE{}, hashNode* next = nullptr) const;

        /**
         * @brief Destroys a hash node
         * @param node Node to destroy
         * @note Safely handles nullptr and uses rebound allocator
         */
        void destroyNode(hashNode* node) noexcept;

        /**
         * @brief Computes hash code for a key
         * @param key Key to hash
         * @return Computed bucket index
         */
        u_integer getHashCode(const K_TYPE& key) const;

        /**
         * @brief Gets current number of buckets
         * @return Current bucket count
         */
        u_integer getBucketCount() const;

        /**
         * @brief Gets bucket head for a key
         * @param key Key to lookup
         * @return Pointer to first node in bucket's chain
         */
        hashNode* getBucket(const K_TYPE& key) const;

        /**
         * @brief Calculates current load factor
         * @return Current elements/buckets ratio
         */
        floating loadFactor() const;

        /**
         * @brief Gets next appropriate bucket size for expansion
         * @return Next larger bucket size from predefined primes
         * @throw outOfBoundError if already at maximum size
         */
        u_integer getNextSize() const;

        /**
         * @brief Gets previous appropriate bucket size for shrinking
         * @return Next smaller bucket size from predefined primes
         */
        u_integer getPrevSize() const;

        /**
         * @brief Rehashes table to new bucket count
         * @param new_bucket_count New number of buckets
         * @details Rebuilds the hash table with new bucket count:
         * 1. Allocates new buckets vector
         * 2. Rehashes all elements
         * 3. Maintains element order within buckets
         * @note Invalidates all iterators
         */
        void rehash(u_integer new_bucket_count);

        /**
         * @brief Adjusts table size based on load factor
         * @details Checks current load factor and resizes if:
         * - loadFactor() >= LOAD_FACTOR_MAX: expands table
         * - loadFactor() <= LOAD_FACTOR_MIN: shrinks table
         */
        void adjust();

        /**
         * @brief Constructs empty hashTable
         * @param hash Hash function to use
         */
        explicit hashTable(HASH hash = HASH{});

        /**
         * @brief Finds node for given key
         * @param key Key to search for
         * @return Pointer to node if found, nullptr otherwise
         */
        hashNode* find(const K_TYPE& key) const;

        /**
         * @brief Modifies value for existing key
         * @param key Key to modify
         * @param value New value
         * @return true if key existed and was modified
         */
        bool modify(const K_TYPE& key, const V_TYPE& value);

        /**
         * @brief Inserts new key-value pair
         * @param key Key to insert
         * @param value Value to associate
         * @return true if inserted, false if key already existed
         * @note Automatically adjusts table size if needed
         */
        bool insert(const K_TYPE& key, const V_TYPE& value);

        /**
         * @brief Removes key-value pair
         * @param key Key to remove
         * @return true if key existed and was removed
         * @note Automatically adjusts table size if needed
         */
        bool erase(const K_TYPE& key);

        /**
         * @brief Destroys hashTable
         * @details Cleans up all nodes and buckets
         */
        virtual ~hashTable();
    };
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::hashNode::hashNode(const K_TYPE& key, const V_TYPE& value, hashNode* next)
    : data_({key, value}), next_(next) {}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::hashNode::hashNode(const hashNode& other) : hashNode() {
    this->operator=(other);
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::hashNode&
original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::hashNode::operator=(const hashNode& other) {
    if (this == &other)
        return *this;
    this->data_ = other.data_;
    this->next_ = other.next_;
    return *this;
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
original::couple<const K_TYPE, V_TYPE>&
original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::hashNode::getVal() {
    return this->data_;
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
const original::couple<const K_TYPE, V_TYPE>&
original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::hashNode::getVal() const {
    return this->data_;
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
const K_TYPE& original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::hashNode::getKey() const {
    return this->getVal().first();
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
const V_TYPE& original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::hashNode::getValue() const {
    return this->getVal().second();
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
V_TYPE& original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::hashNode::getValue() {
    return this->getVal().second();
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
void original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::hashNode::setVal(couple<const K_TYPE, V_TYPE>) {
    throw unSupportedMethodError();
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
void original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::hashNode::setValue(const V_TYPE &value) {
    this->data_.template set<1>(value);
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::hashNode*
original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::hashNode::getPPrev() const {
    throw unSupportedMethodError();
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::hashNode*
original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::hashNode::getPNext() const {
    return this->next_;
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
void original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::hashNode::setPNext(hashNode *new_next) {
    this->next_ = new_next;
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
void original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::hashNode::connect(hashNode *prev, hashNode *next) {
    if (prev != nullptr) prev->setPNext(next);
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
original::u_integer
original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::Iterator::findNextValidBucket(
    vector<hashNode *, rebind_alloc_pointer> *buckets, const u_integer bucket) {
    for (u_integer i = bucket + 1; i < buckets->size(); i++) {
        if ((*buckets)[i])
            return i;
    }
    return buckets->size();
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
original::u_integer
original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::Iterator::findPrevValidBucket(
    vector<hashNode*, rebind_alloc_pointer> *buckets, const u_integer bucket) {
    if (bucket == 0) return buckets->size();
    for (u_integer i = bucket - 1; i > 0; i--) {
        if ((*buckets)[i])
            return i;
    }
    if ((*buckets)[0]) {
        return 0;
    }
    return buckets->size();
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::Iterator::Iterator(
    vector<hashNode *, rebind_alloc_pointer> *buckets, const u_integer bucket, hashNode *node)
    : p_buckets(buckets), cur_bucket(bucket), p_node(node) {}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::Iterator::Iterator(const Iterator &other) : Iterator() {
    this->operator=(other);
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::Iterator&
original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::Iterator::operator=(const Iterator &other) {
    if (this == &other)
        return *this;

    this->p_buckets = other.p_buckets;
    this->p_node = other.p_node;
    return *this;
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
bool original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::Iterator::hasNext() const {
    if (this->p_node && this->p_node->getPNext()) {
        return true;
    }

    return Iterator::findNextValidBucket(this->p_buckets, this->cur_bucket) != this->p_buckets->size();
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
void original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::Iterator::next() const {
    if (!this->isValid()) {
        throw outOfBoundError();
    }

    if (this->p_node->getPNext()) {
        this->p_node = this->p_node->getPNext();
        return;
    }

    if (auto next_bucket = Iterator::findNextValidBucket(this->p_buckets, this->cur_bucket);
        next_bucket != this->p_buckets->size()) {
        this->cur_bucket = next_bucket;
        this->p_node = this->p_buckets->get(next_bucket);
        return;
    }

    this->cur_bucket = this->p_buckets->size();
    this->p_node = nullptr;
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
void original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::Iterator::operator+=(const integer steps) const {
    if (steps < 0) {
        throw unSupportedMethodError();
    }

    for (integer i = 0; i < steps; ++i) {
        this->next();
    }
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
original::couple<const K_TYPE, V_TYPE>&
original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::Iterator::get() {
    if (!this->isValid()) {
        throw outOfBoundError();
    }

    return this->p_node->getVal();
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
original::couple<const K_TYPE, V_TYPE>
original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::Iterator::get() const {
    if (!this->isValid()) {
        throw outOfBoundError();
    }

    return this->p_node->getVal();
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
bool original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::Iterator::isValid() const {
    return this->p_node;
}

template <typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::buckets_type
original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::bucketsCopy(const buckets_type& old_buckets) const
{
    buckets_type new_buckets = buckets_type(old_buckets.size(), rebind_alloc_pointer{}, nullptr);
    for (u_integer i = 0; i < old_buckets.size(); ++i) {
        hashNode* old_node = old_buckets[i];
        hashNode* prev_new_node = nullptr;

        while (old_node) {
            hashNode* new_node = this->createNode(old_node->getKey(), old_node->getValue());

            if (!prev_new_node) {
                new_buckets[i] = new_node;
            } else {
                prev_new_node->setPNext(new_node);
            }

            prev_new_node = new_node;
            old_node = old_node->getPNext();
        }
    }
    return new_buckets;
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::hashNode*
original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::createNode(const K_TYPE& key, const V_TYPE& value, hashNode* next) const {
    auto node = this->rebind_alloc.allocate(1);
    this->rebind_alloc.construct(node, key, value, next);
    return node;
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
void original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::destroyNode(hashNode* node) noexcept {
    this->rebind_alloc.destroy(node);
    this->rebind_alloc.deallocate(node, 1);
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
original::u_integer
original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::getHashCode(const K_TYPE &key) const {
    return this->hash_(key) % this->getBucketCount();
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
original::u_integer
original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::getBucketCount() const {
    return this->buckets.size();
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::hashNode*
original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::getBucket(const K_TYPE &key) const {
    u_integer code = this->getHashCode(key);
    return this->buckets[code];
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
original::floating
original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::loadFactor() const {
    return static_cast<floating>(this->size_) / this->getBucketCount();
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
original::u_integer original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::getNextSize() const {
    for (u_integer i : BUCKETS_SIZES){
        if (this->getBucketCount() < i){
            return i;
        }
    }
    throw outOfBoundError();
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
original::u_integer original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::getPrevSize() const {
    const u_integer current = this->getBucketCount();
    for (u_integer i = BUCKETS_SIZES_COUNT - 1; i > 0; --i){
        if (BUCKETS_SIZES[i] < current){
            return BUCKETS_SIZES[i];
        }
    }
    return BUCKETS_SIZES[0];
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
void original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::rehash(u_integer new_bucket_count) {
    if (new_bucket_count == this->getBucketCount())
        return;

    auto new_buckets = buckets_type(new_bucket_count, rebind_alloc_pointer{}, nullptr);

    for (hashNode*& old_head : this->buckets) {
        while (old_head) {
            hashNode* cur = old_head;
            old_head = old_head->getPNext();

            cur->setPNext(nullptr);
            auto code = this->hash_(cur->getKey()) % new_bucket_count;

            cur->setPNext(new_buckets[code]);
            new_buckets[code] = cur;
        }
    }

    this->buckets = std::move(new_buckets);
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
void original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::adjust() {
    if (this->loadFactor() <= LOAD_FACTOR_MIN){
        this->rehash(this->getPrevSize());
    } else if (this->loadFactor() >= LOAD_FACTOR_MAX){
        this->rehash(this->getNextSize());
    }
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::hashTable(HASH hash)
    : size_(0), hash_(std::move(hash)) {
    this->buckets = vector<hashNode*, rebind_alloc_pointer>(BUCKETS_SIZES[0], rebind_alloc_pointer{}, nullptr);
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::hashNode*
original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::find(const K_TYPE& key) const {
    if (this->size_ == 0)
        return nullptr;

    for (auto cur = this->getBucket(key); cur; cur = cur->getPNext()){
        if (cur->getKey() == key)
            return cur;
    }
    return nullptr;
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
bool original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::modify(const K_TYPE &key, const V_TYPE &value) {
    if (auto cur = this->find(key)){
        cur->setValue(value);
        return true;
    }
    return false;
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
bool original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::insert(const K_TYPE &key, const V_TYPE &value) {
    this->adjust();

    auto cur = this->getBucket(key);
    if (!cur){
        this->buckets[this->getHashCode(key)] = this->createNode(key, value);
    } else{
        if (cur->getKey() == key)
            return false;

        for (; cur->getPNext(); cur = cur->getPNext()){
            if (cur->getKey() == key)
                return false;
        }
        hashNode::connect(cur, this->createNode(key, value));
    }

    this->size_ += 1;
    return true;
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
bool original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::erase(const K_TYPE &key) {
    if (this->size_ == 0)
        return false;

    this->adjust();

    u_integer code = this->getHashCode(key);
    hashNode* cur = this->buckets[code];
    hashNode* prev = nullptr;

    while (cur){
        if (cur->getKey() == key) {
            if (prev) {
                hashNode::connect(prev, cur->getPNext());
            } else {
                this->buckets[code] = cur->getPNext();
            }
            this->destroyNode(cur);
            this->size_ -= 1;
            return true;
        }
        prev = cur;
        cur = cur->getPNext();
    }

    return false;
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename HASH>
original::hashTable<K_TYPE, V_TYPE, ALLOC, HASH>::~hashTable() {
    for (hashNode*& bucket: this->buckets) {
        while (bucket){
            auto next = bucket->getPNext();
            this->destroyNode(bucket);
            bucket = next;
        }
    }
}

#endif //HASHTABLE_H
