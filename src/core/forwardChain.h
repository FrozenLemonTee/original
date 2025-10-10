#ifndef FORWARDCHAIN_H
#define FORWARDCHAIN_H

#include "singleDirectionIterator.h"
#include "array.h"
#include "baseList.h"

/**
 * @file forwardChain.h
 * @brief Non-cyclic singly linked list implementation
 * @details This file provides an implementation of a singly linked list with efficient front and back operations.
 * The class includes:
 * - Single-direction iterator support
 * - Basic list operations such as push, pop, get, and indexOf
 * - Memory management with custom node handling
 */


namespace original {

    /**
     * @class forwardChain
     * @tparam TYPE The type of elements stored in the forward chain
     * @tparam ALLOC Allocator type for memory management (default: allocator<TYPE>)
     * @brief A singly linked list implementation.
     * @extends baseList
     * @extends iterationStream
     * @details The forwardChain class implements a singly linked list where elements are stored in nodes.
     *          Each node points to the next node, and the list supports operations like push, pop, get, and indexOf.
     *          Uses the provided allocator for all memory management operations, including node allocation.
     */
    template <typename TYPE, typename ALLOC = allocator<TYPE>>
    class forwardChain final : public baseList<TYPE, ALLOC>, public iterationStream<TYPE, forwardChain<TYPE, ALLOC>>{

        /**
         * @class forwardChainNode
         * @brief Internal node structure for elements in forwardChain.
         * @extends wrapper
         * @details Represents a node in the linked list containing the element data and a pointer to the next node.
         *          Nodes are allocated using the rebound allocator for forwardChainNode type.
         */
        class forwardChainNode final : public wrapper<TYPE>{
            public:
                friend class iterator<TYPE>;
                friend class forwardChain;

                /**
                 * @brief Constructs a forwardChainNode with given data and next pointer.
                 * @param data The data to store in the node.
                 * @param next Pointer to the next node (default is nullptr).
                 */
                explicit forwardChainNode(const TYPE& data = TYPE{}, forwardChainNode* next = nullptr);
            private:
                TYPE data_;         ///< The data of the node
                forwardChainNode* next; ///< Pointer to the next node in the chain
            protected:

                /**
                 * @brief Copy constructor for forwardChainNode.
                 * @param other The node to copy from.
                 */
                forwardChainNode(const forwardChainNode& other);

                /**
                 * @brief Assignment operator for forwardChainNode.
                 * @param other The node to assign from.
                 * @return A reference to this node.
                 */
                forwardChainNode& operator=(const forwardChainNode& other);

                /**
                 * @brief Gets the value stored in the node.
                 * @return A reference to the value stored in the node.
                 */
                TYPE& getVal() override;

                /**
                 * @brief Gets the value stored in the node (const version).
                 * @return A const reference to the value stored in the node.
                 */
                const TYPE& getVal() const override;

                /**
                 * @brief Sets the value of the node.
                 * @param data The value to set the node to.
                 */
                void setVal(TYPE data) override;

                /**
                 * @brief Gets the pointer to the previous node (not supported in this class).
                 * @return Throws an exception.
                 * @throws unSupportedMethodError Always thrown as previous pointer is not supported
                 */
                forwardChainNode* getPPrev() const override;

                /**
                 * @brief Gets the pointer to the next node.
                 * @return A pointer to the next node.
                 */
                forwardChainNode* getPNext() const override;

                /**
                 * @brief Sets the pointer to the next node.
                 * @param new_next The new next node pointer.
                 */
                void setPNext(forwardChainNode* new_next);

                /**
                 * @brief Connects two nodes by adjusting their pointers.
                 * @param prev The previous node.
                 * @param next The next node.
                 * @details If prev is not nullptr, sets its next pointer to next
                 */
                static void connect(forwardChainNode* prev, forwardChainNode* next);
        };

        /**
         * @brief Rebound allocator type for node allocation
         * @details The allocator rebound to allocate forwardChainNode objects instead of TYPE elements.
         *          Used internally for all node allocations and de-allocations.
         */
        using rebind_alloc_node = ALLOC::template rebind_alloc<forwardChainNode>;

        u_integer size_;         ///< The number of elements in the chain
        forwardChainNode* begin_; ///< Pointer to the first node in the chain (sentinel node)
        rebind_alloc_node rebind_alloc{}; ///< Rebound allocator for node memory management


        /**
         * @brief Gets the first node (after the sentinel).
         * @return A pointer to the first node in the chain.
         */
        forwardChainNode* beginNode() const;

        /**
         * @brief Finds the node at the specified index.
         * @param index The index of the node to find.
         * @return A pointer to the node at the specified index.
         * @details Performs linear search from beginning of chain
         */
        forwardChainNode* findNode(integer index) const;

        /**
         * @brief Creates a new node using the rebound allocator
         * @param value The value to store in the new node
         * @param next Pointer to the next node (default: nullptr)
         * @return Pointer to the newly created node
         * @details Uses the rebound allocator to allocate memory and construct the node
         * @throws std::bad_alloc if memory allocation fails
         */
        forwardChainNode* createNode(const TYPE& value = TYPE{}, forwardChainNode* next = nullptr);

        /**
         * @brief Destroys a node using the rebound allocator
         * @param node Pointer to the node to destroy
         * @note No-throw guarantee
         * @details Uses the rebound allocator to destroy and deallocate the node
         */
        void destroyNode(forwardChainNode* node) noexcept;

        /**
         * @brief Initializes the chain with a sentinel node.
         * @details Creates an empty chain with sentinel node and zero size
         */
        void chainInit();

        /**
         * @brief Adds a node to the beginning of the chain.
         * @param node The node to add to the beginning.
         * @details Connects the node after the sentinel and increments size
         */
        void firstAdd(forwardChainNode* node);

        /**
         * @brief Deletes the last node of the chain.
         * @return The last node that was deleted.
         * @details Used when chain has only one element
         */
        forwardChainNode* lastDelete();

        /**
         * @brief Destroys the chain by deleting all nodes.
         * @details Iterates through all nodes and destroys them using destroyNode
         */
        void chainDestroy();
    public:

        /**
         * @class Iterator
         * @brief Iterator for forwardChain, supports single-direction traversal
         * @extends singleDirectionIterator
         * @details Allows forward iteration through the forwardChain with operations like cloning, comparison, etc.
         *          Provides read and write access to elements.
         */
        class Iterator final : public singleDirectionIterator<TYPE>
        {
            /**
             * @brief Constructs an Iterator from a given forwardChainNode pointer.
             * @param ptr Pointer to the forwardChainNode the iterator will point to.
             */
            explicit Iterator(forwardChainNode* ptr);
        public:
            friend forwardChain;

            /**
            * @brief Copy constructor for Iterator.
            * @param other The iterator to copy from.
            */
            Iterator(const Iterator& other);

            /**
             * @brief Assignment operator for Iterator.
             * @param other The iterator to assign from.
             * @return A reference to this iterator.
             */
            Iterator& operator=(const Iterator& other);

            /**
             * @brief Clones the iterator.
             * @return A new iterator pointing to the same position.
             * @throws std::bad_alloc if memory allocation fails
             */
            Iterator* clone() const override;

            /**
             * @brief Checks if this iterator is at the previous element relative to another iterator.
             * @param other The iterator to compare to.
             * @return True if this iterator is at the previous element, false otherwise.
             */
            bool atPrev(const iterator<TYPE> *other) const override;

            /**
             * @brief Checks if this iterator is at the next element relative to another iterator.
             * @param other The iterator to compare to.
             * @return True if this iterator is at the next element, false otherwise.
             */
            bool atNext(const iterator<TYPE> *other) const override;

            /**
             * @brief Gets the class name of the iterator.
             * @return The class name as a string.
             */
            [[nodiscard]] std::string className() const override;
        };

        /**
         * @brief Constructs an empty forwardChain with specified allocator
         * @param alloc Allocator instance to use (default: default-constructed ALLOC)
         * @details Initializes the chain with a sentinel node using the provided allocator
         */
        explicit forwardChain(ALLOC alloc = ALLOC{});

        /**
         * @brief Copy constructs a forwardChain with allocator propagation
         * @param other The forwardChain to copy from
         * @note The allocator is copied if ALLOC::propagate_on_container_copy_assignment is true
         */
        forwardChain(const forwardChain& other);

        /**
         * @brief Constructs a forwardChain from an initializer list.
         * @param list The initializer list to construct the forwardChain from.
         */
        forwardChain(std::initializer_list<TYPE> list);

        /**
         * @brief Constructs a forwardChain from an array.
         * @param arr The array to construct the forwardChain from.
         */
        explicit forwardChain(const array<TYPE>& arr);

        /**
         * @brief Assignment operator for forwardChain.
         * @param other The forwardChain to assign from.
         * @return A reference to this forwardChain.
         * @note The allocator is copied if ALLOC::propagate_on_container_copy_assignment is true
         */
        forwardChain& operator=(const forwardChain& other);

        /**
         * @brief Move constructs a forwardChain with allocator propagation
         * @param other The forwardChain to move from
         * @note The allocator is moved if ALLOC::propagate_on_container_move_assignment is true
         * @note noexcept guarantees exception safety during move
         */
        forwardChain(forwardChain&& other) noexcept;

        /**
         * @brief Move assignment operator for forwardChain.
         * @param other The forwardChain to move from.
         * @return A reference to this forwardChain.
         * @note The allocator is moved if ALLOC::propagate_on_container_move_assignment is true
         */
        forwardChain& operator=(forwardChain&& other) noexcept;

        /**
         * @brief Gets the size of the forwardChain.
         * @return The number of elements in the forwardChain.
         */
        [[nodiscard]] u_integer size() const override;

        /**
         * @brief Gets the element at the specified index.
         * @param index The index of the element to retrieve.
         * @return The element at the specified index.
         */
        TYPE get(integer index) const override;

        /**
         * @brief Gets a reference to the element at the specified index.
         * @param index The index of the element to retrieve.
         * @return A reference to the element at the specified index.
         */
        TYPE& operator[](integer index) override;

        /**
         * @brief Sets the element at the specified index.
         * @param index The index of the element to set.
         * @param e The value to set the element to.
         */
        void set(integer index, const TYPE &e) override;

        /**
         * @brief Finds the index of the first occurrence of the specified element.
         * @param e The element to search for.
         * @return The index of the element, or the size of the forwardChain if not found.
         */
        u_integer indexOf(const TYPE &e) const override;

        /**
         * @brief Inserts element at the beginning of the chain
         * @param e Element to insert
         * @details Uses the chain's allocator to construct the new node
         */
        void pushBegin(const TYPE &e) override;

        /**
         * @brief Pushes an element at the specified index in the forwardChain.
         * @param index The index at which to insert the element.
         * @param e The element to push.
         * @details Uses the chain's allocator to construct the new node
         */
        void push(integer index, const TYPE &e) override;

        /**
         * @brief Inserts element at the end of the chain
         * @param e Element to insert
         * @details Uses the chain's allocator to construct the new node
         */
        void pushEnd(const TYPE &e) override;

        /**
         * @brief Removes and returns the first element
         * @return The removed element
         * @pre Chain must not be empty
         * @note Uses the chain's allocator to destroy the removed node
         */
        TYPE popBegin() override;

        /**
         * @brief Pops an element at the specified index in the forwardChain.
         * @param index The index of the element to pop.
         * @return The element that was popped.
         * @note Uses the chain's allocator to destroy the removed node
         */
        TYPE pop(integer index) override;

        /**
         * @brief Removes and returns the last element
         * @return The removed element
         * @pre Chain must not be empty
         * @note Uses the chain's allocator to destroy the removed node
         */
        TYPE popEnd() override;

        /**
         * @brief Gets an iterator to the beginning of the forwardChain.
         * @return An iterator to the beginning of the forwardChain.
         */
        Iterator* begins() const override;

        /**
         * @brief Gets an iterator to the end of the forwardChain.
         * @return An iterator to the end of the forwardChain.
         */
        Iterator* ends() const override;

        /**
         * @brief Gets the class name of the forwardChain.
         * @return The class name as a string.
         */
        [[nodiscard]] std::string className() const override;

        /**
         * @brief Destructor for the forwardChain.
         */
        ~forwardChain() override;
    };
}

    template <typename TYPE, typename ALLOC>
    original::forwardChain<TYPE, ALLOC>::forwardChainNode::forwardChainNode(const TYPE& data, forwardChainNode* next)
        : data_(data), next(next) {}

    template <typename TYPE, typename ALLOC>
    original::forwardChain<TYPE, ALLOC>::forwardChainNode::forwardChainNode(const forwardChainNode &other)
        : data_(other.data_), next(other.next) {}

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::forwardChainNode::operator=(
        const forwardChainNode &other) -> forwardChainNode & {
        if (this != &other) {
            data_ = other.data_;
            next = other.next;
        }
        return *this;
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::forwardChainNode::getVal() -> TYPE& {
        return this->data_;
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::forwardChainNode::getVal() const -> const TYPE& {
        return this->data_;
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::forwardChainNode::setVal(TYPE data) -> void {
        this->data_ = data;
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::forwardChainNode::getPPrev() const -> forwardChainNode* {
        throw unSupportedMethodError();
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::forwardChainNode::getPNext() const -> forwardChainNode* {
        return this->next;
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::forwardChainNode::setPNext(forwardChainNode *new_next) -> void {
        this->next = new_next;
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::forwardChainNode::connect(
        forwardChainNode *prev, forwardChainNode *next) -> void {
        if (prev != nullptr) prev->setPNext(next);
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::beginNode() const -> forwardChainNode*
    {
        return this->begin_->getPNext();
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::findNode(const integer index) const -> forwardChainNode* {
        if (this->size() == 0) return this->begin_;
        auto cur = this->beginNode();
        for(u_integer i = 0; i < index; i++)
        {
            cur = cur->getPNext();
        }
        return cur;
    }

    template<typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::createNode(const TYPE &value, forwardChainNode *next) -> forwardChainNode* {
        auto node = this->rebind_alloc.allocate(1);
        this->rebind_alloc.construct(node, value, next);
        return node;
    }

    template<typename TYPE, typename ALLOC>
    void original::forwardChain<TYPE, ALLOC>::destroyNode(forwardChainNode *node) noexcept {
        this->rebind_alloc.destroy(node);
        this->rebind_alloc.deallocate(node, 1);
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::chainInit() -> void
    {
        auto pivot = this->createNode();
        this->size_ = 0;
        this->begin_ = pivot;
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::firstAdd(forwardChainNode* node) -> void
    {
        forwardChainNode::connect(this->findNode(0), node);
        this->size_ += 1;
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::lastDelete() -> forwardChainNode*
    {
        auto last = this->beginNode();
        this->destroyNode(this->begin_);
        this->chainInit();
        return last;
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::chainDestroy() -> void
    {
        auto cur = this->begin_;
        while (cur)
        {
            auto next = cur->getPNext();
            this->destroyNode(cur);
            cur = next;
        }
    }

    template <typename TYPE, typename ALLOC>
    original::forwardChain<TYPE, ALLOC>::Iterator::Iterator(forwardChainNode *ptr)
        : singleDirectionIterator<TYPE>(ptr) {}

    template <typename TYPE, typename ALLOC>
    original::forwardChain<TYPE, ALLOC>::Iterator::Iterator(const Iterator &other)
        : singleDirectionIterator<TYPE>(nullptr) {
        this->operator=(other);
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::Iterator::operator=(const Iterator &other) -> Iterator & {
        if (this == &other) return *this;
        singleDirectionIterator<TYPE>::operator=(other);
        return *this;
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::Iterator::clone() const -> Iterator* {
        return new Iterator(*this);
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::Iterator::atPrev(const iterator<TYPE> *other) const -> bool {
        auto other_it = dynamic_cast<const Iterator*>(other);
        return other_it != nullptr && this->_ptr->getPNext() == other_it->_ptr;
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::Iterator::atNext(const iterator<TYPE> *other) const -> bool {
        auto other_it = dynamic_cast<const Iterator*>(other);
        return other_it != nullptr && other_it->_ptr->getPNext() == this->_ptr;
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::Iterator::className() const -> std::string {
        return "forwardChain::Iterator";
    }

    template <typename TYPE, typename ALLOC>
    original::forwardChain<TYPE, ALLOC>::forwardChain(ALLOC alloc)
    : baseList<TYPE, ALLOC>(std::move(alloc)),
      size_(0),
      begin_(nullptr),
      rebind_alloc(std::move(rebind_alloc_node{}))
    {
        this->chainInit();
    }

    template <typename TYPE, typename ALLOC>
    original::forwardChain<TYPE, ALLOC>::forwardChain(const forwardChain &other) : forwardChain() {
        this->operator=(other);
    }

    template <typename TYPE, typename ALLOC>
    original::forwardChain<TYPE, ALLOC>::forwardChain(std::initializer_list<TYPE> list) : forwardChain() {
        for (auto e: list) {
            auto cur_node = this->createNode(e);
            if (this->size() == 0)
            {
                this->firstAdd(cur_node);
            }else
            {
                auto end = this->findNode(this->size() - 1);
                forwardChainNode::connect(end, cur_node);
                this->size_ += 1;
            }
        }
    }

    template <typename TYPE, typename ALLOC>
    original::forwardChain<TYPE, ALLOC>::forwardChain(const array<TYPE>& arr) : forwardChain() {
        for (u_integer i = 0; i < arr.size(); i++) {
            auto cur_node = this->createNode(arr.get(i));
            if (this->size() == 0)
            {
                this->firstAdd(cur_node);
            }else
            {
                auto end = this->findNode(this->size() - 1);
                forwardChainNode::connect(end, cur_node);
                this->size_ += 1;
            }
        }
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::operator=(const forwardChain &other) -> forwardChain& {
        if (this == &other) return *this;
        this->chainDestroy();
        this->size_ = other.size_;
        if (this->size() != 0){
            auto other_ = other.begin_;
            this->begin_ = this->createNode(other_->getVal());
            auto this_ = this->begin_;
            while (other_->getPNext() != nullptr){
                other_ = other_->getPNext();
                forwardChainNode::connect(this_, this->createNode(other_->getVal()));
                this_ = this_->getPNext();
            }
        } else{
            this->chainInit();
        }
        if constexpr (ALLOC::propagate_on_container_copy_assignment::value){
            this->allocator = other.allocator;
            this->rebind_alloc = other.rebind_alloc;
        }
        return *this;
    }

    template <typename TYPE, typename ALLOC>
    original::forwardChain<TYPE, ALLOC>::forwardChain(forwardChain &&other) noexcept : forwardChain() {
        this->operator=(std::move(other));
    }

    template <typename TYPE, typename ALLOC>
    original::forwardChain<TYPE, ALLOC> & original::forwardChain<TYPE, ALLOC>::operator=(forwardChain &&other) noexcept {
        if (this == &other)
            return *this;

        this->chainDestroy();
        this->begin_ = other.begin_;
        this->size_ = other.size_;
        if constexpr (ALLOC::propagate_on_container_move_assignment::value){
            this->allocator = std::move(other.allocator);
            this->rebind_alloc = std::move(other.rebind_alloc);
        }
        other.chainInit();
        return *this;
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::size() const -> u_integer
    {
        return this->size_;
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::get(integer index) const -> TYPE {
        if (this->indexOutOfBound(index)){
            throw outOfBoundError();
        }
        auto cur = this->findNode(this->parseNegIndex(index));
        return cur->getVal();
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::operator[](integer index) -> TYPE& {
        if (this->indexOutOfBound(index)){
            throw outOfBoundError();
        }
        auto cur = this->findNode(this->parseNegIndex(index));
        return cur->getVal();
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::set(integer index, const TYPE &e) -> void {
        if (this->indexOutOfBound(index)){
            throw outOfBoundError();
        }
        auto cur = this->findNode(this->parseNegIndex(index));
        cur->setVal(e);
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::indexOf(const TYPE &e) const -> u_integer {
        u_integer i = 0;
        for (auto current = this->begin_; current != nullptr; current = current->getPNext()) {
            if (current->getVal() == e) {
                return i;
            }
            i += 1;
        }
        return this->size();
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::pushBegin(const TYPE &e) -> void {
        auto new_node = this->createNode(e);
        if (this->size() == 0){
            this->firstAdd(new_node);
        } else{
            auto next = this->beginNode();
            forwardChainNode::connect(this->begin_, new_node);
            forwardChainNode::connect(new_node, next);
            this->size_ += 1;
        }
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::push(integer index, const TYPE &e) -> void {
        index = this->parseNegIndex(index);
        if (index == 0){
            this->pushBegin(e);
        } else if (index == this->size()){
            this->pushEnd(e);
        } else{
            if (this->indexOutOfBound(index)){
                throw outOfBoundError();
            }
            auto new_node = this->createNode(e);
            auto prev = this->findNode(index - 1);
            auto cur = prev->getPNext();
            forwardChainNode::connect(prev, new_node);
            forwardChainNode::connect(new_node, cur);
            this->size_ += 1;
        }
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::pushEnd(const TYPE &e) -> void {
        auto new_node = this->createNode(e);
        if (this->size() == 0){
            this->firstAdd(new_node);
        } else{
            auto end = this->findNode(this->size() - 1);
            forwardChainNode::connect(end, new_node);
            this->size_ += 1;
        }
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::popBegin() -> TYPE {
        TYPE res;
        if (this->size() == 0){
            throw noElementError();
        }

        res = this->beginNode()->getVal();
        if (this->size() == 1){
            this->destroyNode(this->lastDelete());
        } else{
            auto del = this->beginNode();
            auto new_begin = del->getPNext();
            this->destroyNode(del);
            forwardChainNode::connect(this->begin_, new_begin);
            this->size_ -= 1;
        }
        return res;
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::pop(integer index) -> TYPE {
        index = this->parseNegIndex(index);
        if (index == 0){
            return this->popBegin();
        }
        if (index == this->size() - 1){
            return this->popEnd();
        }
        if (this->indexOutOfBound(index)){
            throw outOfBoundError();
        }
        TYPE res;
        auto prev = this->findNode(index - 1);
        auto cur = prev->getPNext();
        res = cur->getVal();
        auto next = cur->getPNext();
        forwardChainNode::connect(prev, next);
        this->destroyNode(cur);
        this->size_ -= 1;
        return res;
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::popEnd() -> TYPE {
        TYPE res;
        if (this->size() == 0){
            throw noElementError();
        }
        if (this->size() == 1){
            res = this->beginNode()->getVal();
            this->destroyNode(this->lastDelete());
        } else{
            auto new_end = this->findNode(this->size() - 2);
            auto end = new_end->getPNext();
            res = end->getVal();
            this->destroyNode(end);
            forwardChainNode::connect(new_end, nullptr);
            this->size_ -= 1;
        }
        return res;
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::begins() const -> Iterator* {
        return new Iterator(this->beginNode());
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::ends() const -> Iterator* {
        return new Iterator(this->findNode(this->size() - 1));
    }

    template <typename TYPE, typename ALLOC>
    auto original::forwardChain<TYPE, ALLOC>::className() const -> std::string {
        return "forwardChain";
    }

    template <typename TYPE, typename ALLOC>
    original::forwardChain<TYPE, ALLOC>::~forwardChain() {
        this->chainDestroy();
    }

#endif //FORWARDCHAIN_H
