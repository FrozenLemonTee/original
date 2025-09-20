#ifndef CHAIN_H
#define CHAIN_H
#pragma once

#include "doubleDirectionIterator.h"
#include "array.h"
#include "baseList.h"
#include "iterationStream.h"

#include <iostream>

/**
 * @file chain.h
 * @brief Non-cyclic doubly linked list implementation
 * @details Provides a bidirectional linked list container with efficient front/back operations.
 * Features include:
 * - Constant time insertion/removal at both ends
 * - Linear time random access
 * - Safe element ownership through RAII
 */

namespace original {
    /**
     * @class chain
     * @tparam TYPE Type of elements stored in the list
     * @tparam ALLOC Allocator type for memory management (default is allocator<TYPE>)
     * @brief Non-cyclic doubly linked list container
     * @extends baseList
     * @extends iterationStream
     * @details Implements a classic doubly linked list with:
     * - Sentinel nodes for boundary management
     * - Bidirectional traversal capabilities
     * - Index-based element access (O(n) complexity)
     * - Deep copy semantics
     * - Custom memory allocation through allocator
     */
    template <typename TYPE, typename ALLOC = allocator<TYPE>>
    class chain final : public baseList<TYPE, ALLOC>, public iterationStream<TYPE, chain<TYPE, ALLOC>>{
        /**
         * @class chainNode
         * @brief Internal node structure for chain elements
         * @extends wrapper
         * @details Represents a single list node containing:
         * - Data payload
         * - Previous/next node pointers
         * - Value access/modification methods
         */
        class chainNode final : public wrapper<TYPE>{
            public:
                friend class iterator<TYPE>;
                friend class chain;

                /**
                 * @brief Constructs a chainNode with given data, previous, and next pointers.
                 * @param data The value to store in the node.
                 * @param prev Pointer to the previous node (default is nullptr).
                 * @param next Pointer to the next node (default is nullptr).
                 */
                explicit chainNode(const TYPE& data = TYPE{}, chainNode* prev = nullptr, chainNode* next = nullptr);
            private:
                TYPE data_;         ///< Element storage
                chainNode* prev;    ///< Pointer to previous node
                chainNode* next;    ///< Pointer to next node
            protected:

                /**
                * @brief Copy constructor for chainNode.
                * @param other The node to copy.
                */
                chainNode(const chainNode& other);

                /**
                * @brief Assignment operator for chainNode.
                * @param other The node to assign.
                * @return A reference to this node.
                */
                chainNode& operator=(const chainNode& other);

                /**
                * @brief Gets the reference of the node value.
                * @return The value stored in the node.
                */
                TYPE& getVal() override;

                /**
                * @brief Gets the value of the node (const version).
                * @return The constant value stored in the node.
                */
                const TYPE& getVal() const override;

                /**
                * @brief Sets the value of the node.
                * @param data The value to store in the node.
                */
                void setVal(TYPE data) override;

                /**
                * @brief Gets the pointer to the previous node.
                * @return A pointer to the previous node.
                */
                chainNode* getPPrev() const override;

                /**
                * @brief Gets the pointer to the next node.
                * @return A pointer to the next node.
                */
                chainNode* getPNext() const override;

                /**
                 * @brief Sets the pointer to the previous node.
                 * @param new_prev The new previous node pointer.
                 */
                void setPPrev(chainNode* new_prev);

                /**
                 * @brief Sets the pointer to the next node.
                 * @param new_next The new next node pointer.
                 */
                void setPNext(chainNode* new_next);

                /**
                * @brief Connects two nodes
                * @param prev Previous node to connect
                * @param next Next node to connect
                */
                static void connect(chainNode* prev, chainNode* next);
        };

        /**
         * @brief Rebound allocator type for chainNode allocation
         * @details Used internally to allocate and deallocate chainNode objects
         */
        using rebind_alloc_node = ALLOC::template rebind_alloc<chainNode>;

        u_integer size_;         ///< Current element count
        chainNode* begin_;      ///< Pointer to first element node
        chainNode* end_;        ///< Pointer to end sentinel node
        rebind_alloc_node rebind_alloc{};


        /**
         * @brief Finds the node at the given index.
         * @param index The index of the node to find.
         * @return The node at the given index.
         */
        chainNode* findNode(integer index) const;

        /**
         * @brief Creates a new node using the rebound allocator
         * @param value The value to store in the new node
         * @param prev Pointer to the previous node (default is nullptr)
         * @param next Pointer to the next node (default is nullptr)
         * @return Pointer to the newly created node
         */
        chainNode* createNode(const TYPE& value = TYPE{}, chainNode* prev = nullptr, chainNode* next = nullptr);

        /**
         * @brief Destroys a node using the rebound allocator
         * @param node Pointer to the node to destroy
         * @note No-throw guarantee
         */
        void destroyNode(chainNode* node) noexcept;

        /**
         * @brief Initializes the chain with sentinel nodes.
         */
        void chainInit();

        /**
         * @brief Adds a node to the beginning of the chain.
         * @param node The node to add at the beginning.
         */
        void firstAdd(chainNode* node);

        /**
         * @brief Deletes the last node of the chain.
         * @return The last node that was deleted.
         */
        chainNode* lastDelete();

        /**
         * @brief Destroys the chain by deleting all nodes.
         */
        void chainDestroy();
    public:

        /**
         * @class Iterator
         * @brief Bidirectional iterator implementation for chain
         * @extends doubleDirectionIterator
         * @details Provides:
         * - Forward/backward traversal
         * - Clone capability
         * - Position comparison checks
         */
        class Iterator final : public doubleDirectionIterator<TYPE>
        {

            /**
             * @brief Constructs an Iterator from a given chainNode pointer.
             * @param ptr Pointer to the chainNode the iterator will point to.
             */
            explicit Iterator(chainNode* ptr);
        public:
            friend chain;

            /**
             * @brief Copy constructor for Iterator.
             * @param other The iterator to copy.
             */
            Iterator(const Iterator& other);

            /**
             * @brief Assignment operator for Iterator.
             * @param other The iterator to assign.
             * @return A reference to this iterator.
             */
            Iterator& operator=(const Iterator& other);

            /**
             * @brief Clones the iterator.
             * @return A new iterator pointing to the same position.
             */
            Iterator* clone() const override;

            /**
             * @brief Checks if the iterator is at the previous position relative to another iterator.
             * @param other The iterator to compare with.
             * @return True if this iterator is at the previous position, false otherwise.
             */
            bool atPrev(const iterator<TYPE> *other) const override;

            /**
             * @brief Checks if the iterator is at the next position relative to another iterator.
             * @param other The iterator to compare with.
             * @return True if this iterator is at the next position, false otherwise.
             */
            bool atNext(const iterator<TYPE> *other) const override;

            /**
             * @brief Gets the class name of the iterator.
             * @return The class name as a string.
             */
            [[nodiscard]] std::string className() const override;
        };

        /**
         * @brief Constructs an empty chain with specified allocator
         * @param alloc Allocator instance to use (has default-constructed ALLOC)
         */
        explicit chain(ALLOC alloc = ALLOC{});

        /**
         * @brief Copy constructs a chain with optional allocator
         * @param other The chain to copy from
         * @note Allocator is copied if ALLOC::propagate_on_container_copy_assignment is true
         */
        chain(const chain& other);

        /**
         * @brief Constructs a chain from an initializer list.
         * @param list The initializer list to construct the chain from.
         */
        chain(const std::initializer_list<TYPE>& list);

        /**
         * @brief Constructs a chain from an array.
         * @param arr The array to construct the chain from.
         */
        explicit chain(const array<TYPE>& arr);

        /**
         * @brief Copy assignment operator with allocator propagation
         * @param other The chain to copy from
         * @return Reference to this chain
         * @note Allocator is copied if ALLOC::propagate_on_container_copy_assignment is true
         */
        chain& operator=(const chain& other);

        /**
         * @brief Move constructs a chain with optional allocator
         * @param other The chain to move from
         * @note Allocator is moved if ALLOC::propagate_on_container_move_assignment is true
         */
        chain(chain&& other) noexcept;

        /**
         * @brief Move assignment operator with allocator propagation
         * @param other Chain to move from
         * @return Reference to this chain
         * @note Allocator is moved if ALLOC::propagate_on_container_move_assignment is true
         */
        chain& operator=(chain&& other) noexcept;

        /**
        * @brief Concatenates another chain to this one
        * @param other The chain to concatenate
        * @return Reference to this chain after merge
        */
        chain& operator+=(chain& other);

        /**
         * @brief Gets the size of the chain.
         * @return The number of elements in the chain.
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
         * @return The index of the element, or the size of the chain if not found.
         */
        u_integer indexOf(const TYPE &e) const override;

        /**
         * @brief Pushes an element to the beginning of the chain.
         * @param e The element to push to the beginning.
         */
        void pushBegin(const TYPE &e) override;

        /**
         * @brief Pushes an element at the specified index in the chain.
         * @param index The index at which to insert the element.
         * @param e The element to push.
         */
        void push(integer index, const TYPE &e) override;

        /**
         * @brief Pushes an element to the end of the chain.
         * @param e The element to push to the end.
         */
        void pushEnd(const TYPE &e) override;

        /**
         * @brief Pops an element from the beginning of the chain.
         * @return The element that was popped.
         */
        TYPE popBegin() override;

        /**
         * @brief Pops an element at the specified index in the chain.
         * @param index The index of the element to pop.
         * @return The element that was popped.
         */
        TYPE pop(integer index) override;

        /**
         * @brief Pops an element from the end of the chain.
         * @return The element that was popped.
         */
        TYPE popEnd() override;

        /**
         * @brief Gets an iterator to the beginning of the chain.
         * @return An iterator to the beginning of the chain.
         */
        Iterator* begins() const override;

        /**
         * @brief Gets an iterator to the end of the chain.
         * @return An iterator to the end of the chain.
         */
        Iterator* ends() const override;

        /**
         * @brief Gets the class name of the chain.
         * @return The class name as a string.
         */
        [[nodiscard]] std::string className() const override;

        /**
         * @brief Destructor for the chain.
         */
        ~chain() override;
    };
}

    template <typename TYPE, typename ALLOC>
    original::chain<TYPE, ALLOC>::chainNode::chainNode(const TYPE& data, chainNode* prev, chainNode* next)
    : data_(data), prev(prev), next(next) {}

    template <typename TYPE, typename ALLOC>
    original::chain<TYPE, ALLOC>::chainNode::chainNode(const chainNode& other)
            : data_(other.data_), prev(other.prev), next(other.next) {}

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::chainNode::operator=(const chainNode& other) -> chainNode& {
        if (this != &other) {
            data_ = other.data_;
            prev = other.prev;
            next = other.next;
        }
        return *this;
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::chainNode::getVal() -> TYPE&
    {
        return this->data_;
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::chainNode::getVal() const -> const TYPE&
    {
        return this->data_;
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::chainNode::setVal(TYPE data) -> void
    {
        this->data_ = data;
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::chainNode::getPPrev() const -> chainNode* {
        return this->prev;
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::chainNode::getPNext() const -> chainNode* {
        return this->next;
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::chainNode::setPPrev(chainNode* new_prev) -> void {
        this->prev = new_prev;
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::chainNode::setPNext(chainNode* new_next) -> void {
        this->next = new_next;
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::chainNode::connect(chainNode* prev, chainNode* next) -> void
    {
        if (prev != nullptr) prev->setPNext(next);
        if (next != nullptr) next->setPPrev(prev);
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::findNode(integer index) const -> chainNode* {
        const bool reverse_visit = index <= this->size() / 2 ? 0 : 1;
        chainNode* cur;
        if (!reverse_visit){
            cur = this->begin_;
            for(u_integer i = 0; i < index; i++)
            {
                cur = cur->getPNext();
            }
        } else{
            cur = this->end_;
            for(u_integer i = this->size() - 1; i > index; i -= 1)
            {
                cur = cur->getPPrev();
            }
        }
        return cur;
    }

    template<typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::createNode(const TYPE &value, chainNode* prev, chainNode* next) -> chainNode* {
        auto node = this->rebind_alloc.allocate(1);
        this->rebind_alloc.construct(node, value, prev, next);
        return node;
    }

    template<typename TYPE, typename ALLOC>
    void original::chain<TYPE, ALLOC>::destroyNode(chainNode *node) noexcept {
        this->rebind_alloc.destroy(node);
        this->rebind_alloc.deallocate(node, 1);
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::chainInit() -> void
    {
        auto pivot = this->createNode();
        this->size_ = 0;
        this->begin_ = pivot->getPNext();
        this->end_ = pivot;
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::firstAdd(chainNode* node) -> void
    {
        chainNode::connect(this->end_, node);
        this->begin_ = node;
        this->end_ = node;
        this->size_ += 1;
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::lastDelete() -> chainNode*
    {
        auto last = this->end_;
        this->destroyNode(last->getPPrev());
        chainNode::connect(nullptr, last);
        this->chainInit();
        return last;
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::chainDestroy() -> void
    {
        auto current = this->end_;
        while (current) {
            auto prev = current->getPPrev();
            this->destroyNode(current);
            current = prev;
        }
    }

    template <typename TYPE, typename ALLOC>
    original::chain<TYPE, ALLOC>::Iterator::Iterator(chainNode* ptr)
        : doubleDirectionIterator<TYPE>::doubleDirectionIterator(ptr) {}

    template <typename TYPE, typename ALLOC>
    original::chain<TYPE, ALLOC>::Iterator::Iterator(const Iterator& other)
        : doubleDirectionIterator<TYPE>::doubleDirectionIterator(nullptr) {
        this->operator=(other);
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::Iterator::operator=(const Iterator& other) -> Iterator& {
        if (this == &other) return *this;
        doubleDirectionIterator<TYPE>::operator=(other);
        return *this;
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::Iterator::clone() const -> Iterator* {
        return new Iterator(*this);
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::Iterator::atPrev(const iterator<TYPE> *other) const -> bool {
        auto other_it = dynamic_cast<const Iterator*>(other);
        return other_it != nullptr && this->_ptr->getPNext() == other_it->_ptr;
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::Iterator::atNext(const iterator<TYPE> *other) const -> bool {
        auto other_it = dynamic_cast<const Iterator*>(other);
        return other_it != nullptr && other_it->_ptr->getPNext() == this->_ptr;
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::Iterator::className() const -> std::string {
        return "chain::Iterator";
    }

    template <typename TYPE, typename ALLOC>
    original::chain<TYPE, ALLOC>::chain(ALLOC alloc) : baseList<TYPE, ALLOC>(std::move(alloc)), size_(0), rebind_alloc(std::move(rebind_alloc_node{}))
    {
        chainInit();
    }

    template <typename TYPE, typename ALLOC>
    original::chain<TYPE, ALLOC>::chain(const chain& other) : chain(){
    std::cout << "[DEBUG] " << "allocator_ (this): " << &this->allocator << std::endl;
    std::cout << "[DEBUG] " << "allocator_ (other): " << &other.allocator << std::endl;
        this->operator=(other);
    }

    template <typename TYPE, typename ALLOC>
    original::chain<TYPE, ALLOC>::chain(const std::initializer_list<TYPE>& list)
        : chain() {
        for (const auto& e : list) {
            auto cur_node = this->createNode(e);
            if (this->size() == 0)
            {
                this->firstAdd(cur_node);
            }else
            {
                chainNode::connect(this->end_, cur_node);
                this->end_ = cur_node;
                size_ += 1;
            }
        }
    }

    template <typename TYPE, typename ALLOC>
    original::chain<TYPE, ALLOC>::chain(const array<TYPE>& arr)
        : chain() {
        for (u_integer i = 0; i < arr.size(); i++) {
            auto cur_node = this->createNode(arr.get(i));
            if (this->size() == 0)
            {
                this->firstAdd(cur_node);
            }else
            {
                chainNode::connect(this->end_, cur_node);
                this->end_ = cur_node;
                size_ += 1;
            }
        }
    }

    template <typename TYPE, typename ALLOC>
    original::chain<TYPE, ALLOC>& original::chain<TYPE, ALLOC>::operator=(const chain& other){
        std::cout << "[DEBUG] copy ctor this=" << this
              << " allocator=" << &this->allocator
              << " from other=" << &other
              << " allocator=" << &other.allocator << std::endl;
        if (this == &other) return *this;
        this->chainDestroy();
        this->size_ = other.size_;
        if (this->size() != 0){
            auto other_ = other.begin_->getPPrev();
            auto pivot = this->createNode(other_->getVal());
            other_ = other_->getPNext();
            chainNode::connect(pivot, this->createNode(other_->getVal()));
            this->begin_ = pivot->getPNext();
            auto this_ = this->begin_;
            while (other_ != other.end_){
                other_ = other_->getPNext();
                chainNode::connect(this_, this->createNode(other_->getVal()));
                this_ = this_->getPNext();
            }
            this->end_ = this_;
        } else{
            this->chainInit();
        }
        if constexpr (ALLOC::propagate_on_container_copy_assignment::value){
            this->allocator = other.allocator;
            this->rebind_alloc = other.rebind_alloc;
        }
        std::cout << "[DEBUG] chain copy ctor end this=" << this << std::endl;
        return *this;
    }

    template <typename TYPE, typename ALLOC>
    original::chain<TYPE, ALLOC>::chain(chain&& other) noexcept : chain()
    {
        this->operator=(std::move(other));
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::operator=(chain&& other) noexcept -> chain&
    {
        if (this == &other)
            return *this;

        this->chainDestroy();
        this->begin_ = other.begin_;
        this->end_ = other.end_;
        this->size_ = other.size_;
        if constexpr (ALLOC::propagate_on_container_move_assignment::value){
            this->allocator = std::move(other.allocator);
            this->rebind_alloc = std::move(other.rebind_alloc);
        }
        other.chainInit();
        return *this;
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::operator+=(chain& other) -> chain&
    {
        if (this == &other || other.empty()) return *this;

        this->size_ += other.size_;
        other.destroyNode(other.begin_->getPPrev());
        chainNode::connect(this->end_, other.begin_);
        this->end_ = other.end_;
        if constexpr (ALLOC::propagate_on_container_merge::value) {
            this->allocator += other.allocator;
            this->rebind_alloc += other.rebind_alloc;
        }
        other.chainInit();
        return *this;
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::size() const -> u_integer
    {
        return this->size_;
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::className() const -> std::string
    {
        return "chain";
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::get(integer index) const -> TYPE
    {
        if (this->indexOutOfBound(index)){
            throw outOfBoundError("chain::get: Index " + printable::formatString(index) +
                                 " out of bounds for chain of size " + printable::formatString(this->size()));
        }
        chainNode* cur = this->findNode(this->parseNegIndex(index));
        return cur->getVal();
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::operator[](const integer index) -> TYPE&
    {
        if (this->indexOutOfBound(index)){
            throw outOfBoundError("chain::operator[]: Index " + printable::formatString(index) +
                                 " out of bounds for chain of size " + printable::formatString(this->size()));
        }
        chainNode* cur = this->findNode(this->parseNegIndex(index));
        return cur->getVal();
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::set(integer index, const TYPE &e) -> void
    {
        if (this->indexOutOfBound(index)){
            throw outOfBoundError("chain::set: Index " + printable::formatString(index) +
                                 " out of bounds for chain of size " + printable::formatString(this->size()));
        }
        auto cur = this->findNode(this->parseNegIndex(index));
        cur->setVal(e);
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::indexOf(const TYPE &e) const -> u_integer {
        u_integer i = 0;
        for (chainNode* current = this->begin_; current != nullptr; current = current->getPNext()) {
            if (current->getVal() == e) {
                return i;
            }
            i += 1;
        }
        return this->size();
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::pushBegin(const TYPE &e) -> void
    {
        auto new_node = this->createNode(e);
        if (this->size() == 0){
            this->firstAdd(new_node);
        } else{
            auto pivot = this->begin_->getPPrev();
            chainNode::connect(new_node, this->begin_);
            chainNode::connect(pivot, new_node);
            this->begin_ = new_node;
            this->size_ += 1;
        }
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::push(integer index, const TYPE &e) -> void
    {
        index = this->parseNegIndex(index);
        if (index == 0){
            this->pushBegin(e);
        } else if (index == this->size()){
            this->pushEnd(e);
        } else{
            if (this->indexOutOfBound(index)){
                throw outOfBoundError("chain::push: Index " + printable::formatString(index) +
                                     " out of bounds for chain of size " + printable::formatString(this->size()));
            }
            auto new_node = this->createNode(e);
            auto cur = this->findNode(index);
            auto prev = cur->getPPrev();
            chainNode::connect(prev, new_node);
            chainNode::connect(new_node, cur);
            this->size_ += 1;
        }
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::pushEnd(const TYPE &e) -> void
    {
        auto new_node = this->createNode(e);
        if (this->size() == 0){
            this->firstAdd(new_node);
        } else{
            chainNode::connect(this->end_, new_node);
            this->end_ = new_node;
            this->size_ += 1;
        }
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::popBegin() -> TYPE
    {
        TYPE res;
        if (this->size() == 0){
            throw noElementError("chain::popBegin: Cannot pop from empty chain");
        }
        if (this->size() == 1){
            auto del = this->lastDelete();
            res = del->getVal();
            this->destroyNode(del);
        } else{
            res = this->begin_->getVal();
            auto new_begin = this->begin_->getPNext();
            auto pivot = this->begin_->getPPrev();
            this->destroyNode(this->begin_);
            this->begin_ = new_begin;
            chainNode::connect(pivot, this->begin_);
            this->size_ -= 1;
        }
        return res;
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::pop(integer index) -> TYPE
    {
        index = this->parseNegIndex(index);
        if (index == 0){
            return this->popBegin();
        }
        if (index == this->size() - 1){
            return this->popEnd();
        }
        if (this->indexOutOfBound(index)){
            throw outOfBoundError("chain::pop: Index " + printable::formatString(index) +
                                 " out of bounds for chain of size " + printable::formatString(this->size()));
        }
        TYPE res;
        chainNode* cur = this->findNode(index);
        res = cur->getVal();
        auto prev = cur->getPPrev();
        auto next = cur->getPNext();
        chainNode::connect(prev, next);
        this->destroyNode(cur);
        this->size_ -= 1;
        return res;
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::popEnd() -> TYPE
    {
        TYPE res;
        if (this->size() == 0){
            throw noElementError("chain::popEnd: Cannot pop from empty chain");
        }
        if (this->size() == 1){
            auto del = this->lastDelete();
            res = del->getVal();
            this->destroyNode(del);
        } else{
            res = this->end_->getVal();
            auto new_end = this->end_->getPPrev();
            this->destroyNode(this->end_);
            this->end_ = new_end;
            chainNode::connect(this->end_, nullptr);
            this->size_ -= 1;
        }
        return res;
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::begins() const -> Iterator* {
        return new Iterator(this->begin_);
    }

    template <typename TYPE, typename ALLOC>
    auto original::chain<TYPE, ALLOC>::ends() const -> Iterator* {
        return new Iterator(this->end_);
    }

    template <typename TYPE, typename ALLOC>
    original::chain<TYPE, ALLOC>::~chain() {
        this->chainDestroy();
    }

#endif //CHAIN_H