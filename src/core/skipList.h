#ifndef SKIPLIST_H
#define SKIPLIST_H
#include <random>
#include "comparator.h"
#include "vector.h"
#include "couple.h"

/**
 * @file skipList.h
 * @brief Skip List container implementation
 * @details Provides a probabilistic alternative to balanced trees with:
 * - Expected O(log n) search/insert/delete operations
 * - Multi-level linked list structure
 * - Custom comparator support
 * - STL-style allocator support
 *
 * Key Features:
 * - Probabilistic balancing with O(log n) expected performance
 * - Simpler implementation than balanced trees
 * - Sorted element storage
 * - Customizable comparison and allocation
 * - Full iterator support
 * - Exception safety (basic guarantee)
 */

namespace original {

    /**
     * @class skipList
     * @tparam K_TYPE Key type (must be comparable)
     * @tparam V_TYPE Value type
     * @tparam ALLOC Allocator type (default: allocator<K_TYPE>)
     * @tparam Compare Comparison function type (default: increaseComparator<K_TYPE>)
     * @brief Skip List container implementation
     * @details This class provides a probabilistic alternative to balanced trees with:
     * - Expected O(log n) search/insert/delete operations
     * - Multi-level linked list structure
     * - Custom comparator support
     * - STL-style allocator support
     */
    template<typename K_TYPE,
            typename V_TYPE,
            typename ALLOC = allocator<K_TYPE>,
            typename Compare = increaseComparator<K_TYPE>>
    class skipList {
    public:
        /**
         * @class skipListNode
         * @brief Internal node class for Skip List
         * @details Represents a single node in the list with:
         * - Key-value pair storage
         * - Vector of next pointers for multiple levels
         */
        class skipListNode {
            couple<const K_TYPE, V_TYPE> data_;  ///< Key-value pair storage
            vector<skipListNode*> next_;         ///< Vector of next pointers for each level

            using rebind_alloc_pointer = ALLOC::template rebind_alloc<skipListNode*>;
        public:
            friend class skipList;

            /**
             * @brief Constructs a new skipListNode
             * @param key Key to store
             * @param value Value to store
             * @param levels Number of levels for this node
             * @param next Initializer list of next pointers
             */
            explicit skipListNode(const K_TYPE& key = K_TYPE{}, const V_TYPE& value = V_TYPE{},
                                  u_integer levels = 1, std::initializer_list<skipListNode*> next = {});

            /**
             * @brief Gets key-value pair (non-const)
             * @return Reference to key-value pair
             */
            couple<const K_TYPE, V_TYPE>& getVal();

            /**
             * @brief Gets key-value pair (const)
             * @return Const reference to key-value pair
             */
            const couple<const K_TYPE, V_TYPE>& getVal() const;

            /**
             * @brief Gets key
             * @return Const reference to key
             */
            const K_TYPE& getKey() const;

            /**
             * @brief Gets value (const)
             * @return Const reference to value
             */
            const V_TYPE& getValue() const;

            /**
             * @brief Gets value (non-const)
             * @return Reference to value
             */
            V_TYPE& getValue();

            /**
             * @brief Gets number of levels for this node
             * @return Number of levels
             */
            [[nodiscard]] u_integer getLevels() const;

            /**
             * @brief Expands node to more levels
             * @param new_levels New total number of levels
             */
            void expandLevels(u_integer new_levels);

            /**
             * @brief Shrinks node to fewer levels
             * @param new_levels New total number of levels
             */
            void shrinkLevels(u_integer new_levels);

            /**
             * @brief Sets new value
             * @param value New value to set
             */
            void setValue(const V_TYPE& value);

            /**
             * @brief Gets next node at specified level
             * @param levels Level to query (1-based)
             * @return Pointer to next node at specified level
             */
            skipListNode* getPNext(u_integer levels) const;

            /**
             * @brief Sets next node at specified level
             * @param levels Level to modify (1-based)
             * @param next New next node to set
             */
            void setPNext(u_integer levels, skipListNode* next);

            /**
             * @brief Connects two nodes at specified level
             * @param levels Level to connect at (1-based)
             * @param prev Previous node
             * @param next Next node
             */
            static void connect(u_integer levels, skipListNode* prev, skipListNode* next);
        };

        using rebind_alloc_node = ALLOC::template rebind_alloc<skipListNode>;       ///< Rebound allocator for nodes
        using rebind_alloc_pointer = ALLOC::template rebind_alloc<skipListNode*>;  ///< Rebound allocator for pointers

        u_integer size_;                     ///< Number of elements
        skipListNode* head_;                 ///< Head node pointer
        Compare compare_;                     ///< Comparison function
        mutable rebind_alloc_node rebind_alloc{};  ///< Node allocator
        mutable std::mt19937 gen_{std::random_device{}()};  ///< Random number generator
        mutable std::uniform_real_distribution<> dis_{0.0, 1.0};  ///< Uniform distribution for level generation

        /**
         * @class Iterator
         * @brief Forward iterator for skipList
         * @details Provides iteration over list elements in sorted order
         */
        class Iterator {
        protected:
            mutable skipListNode* cur_;  ///< Current node pointer

            /**
             * @brief Constructs iterator
             * @param cur Current node pointer
             */
            explicit Iterator(skipListNode* cur = nullptr);

            /// Copy constructor
            Iterator(const Iterator& other);

            /// Copy assignment operator
            Iterator& operator=(const Iterator& other);

            /**
             * @brief Checks if more elements exist forward
             * @return true if more elements available
             */
            [[nodiscard]] bool hasNext() const;

            /**
             * @brief Moves to next element
             */
            void next() const;

            /**
             * @brief Creates a copy of this iterator
             * @return Pointer to new iterator
             */
            Iterator* clone() const;

            /**
             * @brief Advances iterator by steps
             * @param steps Number of positions to advance
             */
            void operator+=(integer steps) const;

            /**
             * @brief Calculates distance between iterators
             * @param other Iterator to compare with
             * @return Distance between iterators
             */
            integer operator-(const Iterator& other) const;

            /**
             * @brief Gets current element (non-const)
             * @return Reference to current key-value pair
             */
            couple<const K_TYPE, V_TYPE>& get();

            /**
             * @brief Gets current element (const)
             * @return Copy of current key-value pair
             */
            couple<const K_TYPE, V_TYPE> get() const;

            /**
             * @brief Checks if iterator is valid
             * @return true if iterator points to valid element
             */
            [[nodiscard]] bool isValid() const;

            /**
             * @brief Calculates distance between two iterators
             * @param start Starting iterator
             * @param end Ending iterator
             * @return Distance between iterators
             */
            static integer ptrDistance(const Iterator* start, const Iterator* end);
        };

        friend Iterator;

        /**
         * @brief Creates a new node with given parameters
         * @param key Key for new node
         * @param value Value for new node
         * @param levels Number of levels for new node
         * @param next Initializer list of next pointers
         * @return Pointer to newly created node
         * @details Uses allocator to construct node in place
         */
        skipListNode* createNode(const K_TYPE& key = K_TYPE{}, const V_TYPE& value = V_TYPE{},
                                 u_integer levels = 1, std::initializer_list<skipListNode*> next = {}) const;

        /**
         * @brief Destroys a node and deallocates memory
         * @param node Node to destroy
         * @details Uses allocator to destroy and deallocate node
         */
        void destroyNode(skipListNode* node) const;

        /**
         * @brief Compares priority between two nodes
         * @param cur First node to compare
         * @param next Second node to compare
         * @return true if cur has higher priority than next
         * @details Uses the list's comparison function
         */
        bool highPriority(skipListNode* cur, skipListNode* next) const;

        /**
         * @brief Compares priority between a key and a node
         * @param key Key to compare
         * @param next Node to compare
         * @return true if key has higher priority than node's key
         * @details Uses the list's comparison function
         */
        bool highPriority(const K_TYPE& key, skipListNode* next) const;

        /**
         * @brief Checks if key matches node's key
         * @param key Key to compare
         * @param next Node to compare
         * @return true if key matches node's key
         */
        static bool equal(const K_TYPE& key, skipListNode* next);

        /**
         * @brief Generates random number of levels for new node
         * @return Random number of levels (geometric distribution)
         */
        u_integer getRandomLevels() const;

        /**
         * @brief Gets current number of levels in list
         * @return Current maximum number of levels
         */
        u_integer getCurLevels() const;

        /**
         * @brief Expands list to more levels
         * @param new_levels New total number of levels
         */
        void expandCurLevels(u_integer new_levels);

        /**
         * @brief Shrinks list to fewer levels
         * @param new_levels New total number of levels
         */
        void shrinkCurLevels(u_integer new_levels);

        /**
         * @brief Creates a deep copy of the list
         * @return Pointer to head of copied list
         */
        skipListNode* listCopy() const;

        /**
         * @brief Finds last node in list
         * @return Pointer to last node
         */
        skipListNode* findLastNode() const;

        /**
         * @brief Constructs skipList with given comparison function
         * @param compare Comparison function to use
         */
        explicit skipList(Compare compare = Compare{});

        /**
         * @brief Finds node with given key
         * @param key Key to search for
         * @return Pointer to found node, or nullptr if not found
         * @details Uses multi-level search for efficiency
         */
        skipListNode* find(const K_TYPE& key) const;

        /**
         * @brief Modifies value for existing key
         * @param key Key to modify
         * @param value New value to set
         * @return true if key was found and modified
         */
        bool modify(const K_TYPE& key, const V_TYPE& value);

        /**
         * @brief Inserts new key-value pair
         * @param key Key to insert
         * @param value Value to insert
         * @return true if inserted, false if key already existed
         * @details Automatically adjusts node levels probabilistically
         */
        bool insert(const K_TYPE& key, const V_TYPE& value);

        /**
         * @brief Erases node with given key
         * @param key Key to erase
         * @return true if key was found and erased
         * @details Automatically adjusts list levels if needed
         */
        bool erase(const K_TYPE& key);

        /**
         * @brief Destroys entire list and deallocates all nodes
         * @details Uses sequential traversal to destroy all nodes
         */
        void listDestroy() noexcept;

        /**
         * @brief Destructor
         * @details Cleans up all list nodes and allocated memory
         */
        ~skipList();
    };
}

template <typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::skipListNode::skipListNode(const K_TYPE& key, const V_TYPE& value,
    u_integer levels, std::initializer_list<skipListNode*> next)
    : data_({key, value}), next_(vector<skipListNode*>(levels, rebind_alloc_pointer{}, nullptr)) {
    if (next.size() != 0 && static_cast<u_integer>(next.size()) != levels) {
        throw outOfBoundError();
    }
    u_integer i = 0;
    for (auto&& ptr: next) {
        this->next_[i] = ptr;
        i += 1;
    }
}

template <typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
original::couple<const K_TYPE, V_TYPE>&
original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::skipListNode::getVal() {
    return this->data_;
}

template <typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
const original::couple<const K_TYPE, V_TYPE>&
original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::skipListNode::getVal() const {
    return this->data_;
}

template <typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
const K_TYPE&
original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::skipListNode::getKey() const {
    return this->data_.template get<0>();
}

template <typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
const V_TYPE&
original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::skipListNode::getValue() const {
    return this->data_.template get<1>();
}

template <typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
V_TYPE&
original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::skipListNode::getValue() {
    return this->data_.template get<1>();
}

template <typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
original::u_integer original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::skipListNode::getLevels() const {
    return this->next_.size();
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
void original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::skipListNode::expandLevels(u_integer new_levels) {
    if (this->getLevels() >= new_levels){
        return;
    }
    auto increment = new_levels - this->getLevels();
    for (u_integer i = 0; i < increment; ++i) {
        this->next_.pushEnd(nullptr);
    }
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
void original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::skipListNode::shrinkLevels(u_integer new_levels) {
    if (new_levels >= this->getLevels() || new_levels == 0){
        return;
    }
    auto decrement = this->getLevels() - new_levels;
    for (u_integer i = 0; i < decrement; ++i) {
        this->next_.popEnd();
    }
}

template <typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
void original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::skipListNode::setValue(const V_TYPE& value) {
    this->data_.template set<1>(value);
}

template <typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
 original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::skipListNode*
original::skipList<K_TYPE, V_TYPE, ALLOC,Compare>::skipListNode::getPNext(const u_integer levels) const {
    return this->next_[levels - 1];
}

template <typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
void original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::skipListNode::setPNext(const u_integer levels, skipListNode* next)
{
    this->next_[levels - 1] = next;
}

template <typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
void original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::skipListNode::connect(const u_integer levels, skipListNode* prev, skipListNode* next)
{
    if (prev) {
        prev->setPNext(levels, next);
    }
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::Iterator::Iterator(skipListNode* cur)
    : cur_(cur) {}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::Iterator::Iterator(const Iterator& other) {
    this->operator=(other);
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::Iterator&
original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::Iterator::operator=(const Iterator& other) {
    if (this == &other){
        return *this;
    }

    this->cur_ = other.cur_;
    return *this;
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
bool original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::Iterator::hasNext() const {
    return this->cur_->getPNext(1);
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
void original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::Iterator::next() const {
    this->cur_ = this->cur_->getPNext(1);
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::Iterator*
original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::Iterator::clone() const {
    return new Iterator(*this);
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
void original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::Iterator::operator+=(original::integer steps) const {
    if (steps < 0){
        throw unSupportedMethodError();
    }

    for (integer i = 0; i < steps; ++i) {
        this->next();
    }
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
original::integer
original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::Iterator::operator-(const Iterator& other) const {
    if (const integer pos_dis = ptrDistance(&other, this);
            pos_dis != (std::numeric_limits<integer>::max)()) return pos_dis;
    if (const integer neg_dis = ptrDistance(this, &other);
            neg_dis != (std::numeric_limits<integer>::max)()) return -neg_dis;
    return this->cur_ > other.cur_ ?
           (std::numeric_limits<integer>::max)() :
           (std::numeric_limits<integer>::min)();
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
original::couple<const K_TYPE, V_TYPE>&
original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::Iterator::get() {
    if (!this->isValid()){
        throw outOfBoundError();
    }

    return this->cur_->getVal();
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
original::couple<const K_TYPE, V_TYPE>
original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::Iterator::get() const {
    if (!this->isValid()){
        throw outOfBoundError();
    }

    return this->cur_->getVal();
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
bool original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::Iterator::isValid() const {
    return this->cur_;
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
original::integer
original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::Iterator::ptrDistance(const Iterator* start, const Iterator* end) {
    auto s = ownerPtr(start->clone());
    auto e = ownerPtr(end->clone());
    integer dis = 0;
    while (s->isValid()){
        if (s->cur_ == e->cur_){
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

template <typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::skipListNode*
original::skipList<K_TYPE, V_TYPE, ALLOC,Compare>::createNode(const K_TYPE& key, const V_TYPE& value, u_integer levels,
    std::initializer_list<skipListNode*> next) const {
    auto node = this->rebind_alloc.allocate(1);
    this->rebind_alloc.construct(node, key, value, levels, next);
    return node;
}

template <typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
void original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::destroyNode(skipListNode* node) const {
    this->rebind_alloc.destroy(node);
    this->rebind_alloc.deallocate(node, 1);
}

template <typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
bool original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::highPriority(skipListNode* cur, skipListNode* next) const
{
    if (!cur){
        return false;
    }
    return this->highPriority(cur->getKey(), next);
}

template <typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
bool original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::highPriority(const K_TYPE& key, skipListNode* next) const
{
    if (!next){
        return true;
    }
    return this->compare_(key, next->getKey());
}

template <typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
bool original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::equal(const K_TYPE& key, skipListNode* next)
{
    if (!next) {
        return false;
    }

    return key == next->getKey();
}

template <typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
original::u_integer original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::getRandomLevels() const
{
    constexpr floating p = 0.5;
    u_integer levels = 1;
    while (this->dis_(this->gen_) < p) {
        levels += 1;
    }
    return levels;
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
original::u_integer
original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::getCurLevels() const {
    return this->head_->getLevels();
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
void original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::expandCurLevels(u_integer new_levels) {
    this->head_->expandLevels(new_levels);
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
void original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::shrinkCurLevels(u_integer new_levels) {
    this->head_->shrinkLevels(new_levels);
}

template <typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::skipListNode*
original::skipList<K_TYPE, V_TYPE, ALLOC,Compare>::listCopy() const {
    auto copied_head =
    this->createNode(this->head_->getKey(), this->head_->getValue(), this->getCurLevels());

    vector<skipListNode*> copied_curs{this->getCurLevels(), rebind_alloc_pointer{}, copied_head};
    auto src_cur = this->head_;
    while (src_cur->getPNext(1)){
        auto src_next = src_cur->getPNext(1);
        auto copied_next = this->createNode(src_next->getKey(), src_next->getValue(), src_next->getLevels());
        for (u_integer i = 0; i < src_next->getLevels(); ++i) {
            skipListNode::connect(i + 1, copied_curs[i], copied_next);
            copied_curs[i] = copied_next;
        }
        src_cur = src_next;
    }

    return copied_head;
}

template<typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::skipListNode*
original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::findLastNode() const {
    auto cur = this->head_;
    while (cur->getPNext(1)){
        cur = cur->getPNext(1);
    }
    return cur;
}

template <typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::skipList(Compare compare)
    : size_(0), head_(this->createNode()), compare_(std::move(compare)) {}

template <typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::skipListNode*
original::skipList<K_TYPE, V_TYPE, ALLOC,Compare>::find(const K_TYPE& key) const
{
    if (this->size_ == 0){
        return nullptr;
    }

    const u_integer levels = this->getCurLevels();
    auto cur_p = this->head_;
    skipListNode* next_p;
    for (u_integer i = levels; i > 0; --i) {
        next_p = cur_p->getPNext(i);
        while (next_p){
            if (equal(key, next_p)){
                return next_p;
            }
            if (this->highPriority(key, next_p)){
                break;
            }
            cur_p = next_p;
            next_p = next_p->getPNext(i);
        }
    }
    return equal(key, next_p) ? next_p : nullptr;
}

template <typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
bool original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::modify(const K_TYPE& key, const V_TYPE& value)
{
    auto cur_p = this->find(key);
    if (!cur_p){
        return false;
    }

    cur_p->setValue(value);
    return true;
}

template <typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
bool original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::insert(const K_TYPE& key, const V_TYPE& value)
{
    u_integer new_levels = this->getRandomLevels();
    if (new_levels > this->getCurLevels()) {
        this->expandCurLevels(new_levels);
    }

    vector<skipListNode*> update(new_levels, rebind_alloc_pointer{}, nullptr);
    skipListNode* cur = this->head_;

    for (u_integer i = this->getCurLevels(); i > 0; --i) {
        while (cur->getPNext(i) && !this->highPriority(key, cur->getPNext(i))) {
            cur = cur->getPNext(i);
        }
        if (i <= new_levels) {
            update[i - 1] = cur;

            if (equal(key, cur) && cur != this->head_) {
                return false;
            }
        }
    }

    auto new_node = this->createNode(key, value, new_levels);
    for (u_integer i = 0; i < new_levels; ++i) {
        auto new_next = update[i]->getPNext(i + 1);
        skipListNode::connect(i + 1, new_node, new_next);
        skipListNode::connect(i + 1, update[i], new_node);
    }

    this->size_ += 1;
    return true;
}

template <typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
bool original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::erase(const K_TYPE& key)
{
    auto cur_p = this->find(key);
    if (!cur_p){
        return false;
    }

    auto cur_levels = cur_p->getLevels();
    vector<skipListNode*> prev_nodes{cur_levels, rebind_alloc_pointer{}, this->head_};
    vector<skipListNode*> next_nodes{cur_levels, rebind_alloc_pointer{}, nullptr};
    for (u_integer i = 0; i < cur_levels; ++i) {
        next_nodes[i] = cur_p->getPNext(i + 1);
        while (true){
            skipListNode* cur_node = prev_nodes[i];
            skipListNode* next_node = cur_node->getPNext(i + 1);
            if (!next_node || !this->highPriority(next_node, cur_p)){
                break;
            }
            prev_nodes[i] = next_node;
        }
    }
    for (u_integer i = 0; i < cur_levels; ++i) {
        skipListNode::connect(i + 1, prev_nodes[i], next_nodes[i]);
    }
    this->destroyNode(cur_p);

    u_integer decrement = 0;
    for (u_integer i = this->getCurLevels(); i > 0; --i) {
        if (this->head_->getPNext(i)){
            break;
        }
        decrement += 1;
    }
    if (decrement > 0){
        this->shrinkCurLevels(this->getCurLevels() - decrement);
    }
    this->size_ -= 1;
    return true;
}

template <typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
void original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::listDestroy() noexcept
{
    auto cur = this->head_;
    while (cur) {
        auto next = cur->getPNext(1);
        this->destroyNode(cur);
        cur = next;
    }
}

template <typename K_TYPE, typename V_TYPE, typename ALLOC, typename Compare>
original::skipList<K_TYPE, V_TYPE, ALLOC, Compare>::~skipList()
{
    this->listDestroy();
}

#endif //SKIPLIST_H
