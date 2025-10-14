#ifndef BLOCKSLIST_H
#define BLOCKSLIST_H

#include "baseList.h"
#include "couple.h"
#include "vector.h"

/**
 * @file blocksList.h
 * @brief A block-based list implementation.
 * @details This file provides the definition of the blocksList class, which is a list that stores elements
 *          in blocks. It efficiently handles operations such as insertion and removal at both ends.
 *          The class supports random access and iteration.
 */

namespace original {
    /**
     * @class blocksList
     * @tparam TYPE Type of elements stored in the blocksList
     * @tparam ALLOC Allocator type to use for memory management (default: allocator<TYPE>)
     * @brief A block-based list implementation.
     * @extends baseList
     * @extends iterationStream
     * @details The blocksList class is a container that stores elements in blocks.
     *          The class provides operations for insertion, deletion, and accessing elements
     *          from both ends. Memory management is handled through the specified allocator type,
     *          which is used for both block allocation and element construction/destruction.
     */
    template <typename TYPE, typename ALLOC = allocator<TYPE>>
    class blocksList final : public baseList<TYPE, ALLOC>, public iterationStream<TYPE, blocksList<TYPE, ALLOC>> {
        static constexpr u_integer BLOCK_MAX_SIZE = 16; ///< The maximum size of each block
        static constexpr u_integer POS_INIT = (BLOCK_MAX_SIZE - 1) / 2 + 1; ///< Initial position in a block

        /**
         * @brief Vector storing pointers to allocated blocks.
         * @details Uses a rebound allocator to manage the block pointers.
         *          The actual element storage is managed by the main allocator.
         */
        vector<TYPE*> map;

        u_integer size_; ///< Current size of the list
        u_integer first_; ///< Position of the first element
        u_integer last_; ///< Position of the last element
        u_integer first_block; ///< Block index of the first element
        u_integer last_block; ///< Block index of the last element


        /**
         * @brief Initializes a new block array.
         * @return Pointer to the newly allocated block
         * @details Allocates memory for a new block using the configured allocator
         *          and default-constructs all elements in the block.
         */
        TYPE* blockArrayInit();

        /**
         * @brief Initializes the blocksList with an initial block.
         * @details Uses the configured allocator to allocate the initial block
         *          and sets up the initial positions and indices.
         */
        void blocksListInit();

        /**
         * @brief Destroys the blocksList by deleting all blocks.
         * @details Uses the configured allocator to destroy all elements
         *          and deallocate all memory blocks.
         */
        void blocksListDestroy() noexcept;

        /**
         * @brief Converts the block and position to an absolute index.
         * @param block The block index.
         * @param pos The position within the block.
         * @return The absolute index.
         */
        [[nodiscard]] static u_integer innerIdxToAbsIdx(u_integer block, u_integer pos);

        /**
         * @brief Gets the absolute index of the first element.
         * @return The absolute index of the first element.
         */
        [[nodiscard]] u_integer firstAbsIdx() const;

        /**
         * @brief Gets the absolute index of the last element.
         * @return The absolute index of the last element.
         */
        [[nodiscard]] u_integer lastAbsIdx() const;

        /**
         * @brief Converts an absolute index to an outer index.
         * @param absIdx The absolute index.
         * @return The corresponding outer index.
         */
        [[nodiscard]] integer absIdxToOuterIdx(u_integer absIdx) const;

        /**
         * @brief Converts an outer index to an absolute index.
         * @param outerIdx The outer index.
         * @return The corresponding absolute index.
         */
        [[nodiscard]] u_integer outerIdxToAbsIdx(integer outerIdx) const;

        /**
         * @brief Converts an absolute index to a pair of block and position.
         * @param absIdx The absolute index.
         * @return A pair of block index and position within the block.
         */
        [[nodiscard]] static couple<u_integer, u_integer> absIdxToInnerIdx(u_integer absIdx);

        /**
         * @brief Computes the new block and position after applying an offset.
         * @param block The current block index.
         * @param pos The current position within the block.
         * @param offset The offset to apply.
         * @return A pair of the new block index and position.
         */
        [[nodiscard]] static couple<u_integer, u_integer> innerIdxOffset(u_integer block, u_integer pos, integer offset);

        /**
         * @brief Converts an outer index to a block and position pair.
         * @param outerIdx The outer index.
         * @return A pair of block index and position within the block.
         */
        [[nodiscard]] couple<u_integer, u_integer> outerIdxToInnerIdx(integer outerIdx) const;

        /**
         * @brief Converts a block and position to an outer index.
         * @param block The block index.
         * @param pos The position within the block.
         * @return The corresponding outer index.
         */
        [[nodiscard]] integer innerIdxToOuterIdx(u_integer block, u_integer pos) const;

        /**
         * @brief Gets the element at a given block and position.
         * @param block The block index.
         * @param pos The position within the block.
         * @return A reference to the element at the specified position.
         */
        TYPE& getElem(u_integer block, u_integer pos) const;

        /**
         * @brief Sets the element at a given block and position.
         * @param block The block index.
         * @param pos The position within the block.
         * @param e The value to set at the specified position.
         */
        void setElem(u_integer block, u_integer pos, const TYPE& e);

        /**
         * @brief Determines whether the blocksList needs to grow.
         * @param increment The number of elements to add.
         * @param is_first Flag indicating whether it is the first element being added.
         * @return True if growth is needed, false otherwise.
         */
        [[nodiscard]] bool growNeeded(u_integer increment, bool is_first) const;

        /**
         * @brief Moves elements from one position to another within the list.
         * @param start_block The block where the move starts.
         * @param start_pos The position where the move starts.
         * @param len The number of elements to move.
         * @param offset The offset to apply to the positions.
         */
        void moveElements(u_integer start_block, u_integer start_pos, u_integer len, integer offset);

        /**
         * @brief Adds a new block to the blocksList.
         * @param is_first Whether to add at beginning (true) or end (false)
         * @details Allocates a new block using the configured allocator
         *          and adds it to the appropriate end of the blocksList.
         */
        void addBlock(bool is_first);

        /**
         * @brief Adjusts the blocksList to accommodate new elements.
         * @param increment Number of elements to accommodate
         * @param is_first Whether adjustment is for beginning (true) or end (false)
         * @details Ensures sufficient blocks are allocated (using the configured allocator)
         *          to accommodate the specified number of new elements.
         */
        void adjust(u_integer increment, bool is_first);

    public:
        /**
         * @class Iterator
         * @brief Iterator for blocksList, supports forward and backward iteration.
         * @extends baseIterator
         * @details This iterator allows iterating through the blocksList with random access.
         */
        class Iterator final : public baseIterator<TYPE> {
            mutable integer cur_pos; ///< Current position within the block
            mutable integer cur_block; ///< Current block index
            mutable TYPE** data_; ///< Pointer to the block data
            const blocksList* container_; ///< Pointer to the containing blocksList

            /**
             * @brief Constructs an iterator for the blocksList.
             * @param pos The position of the element.
             * @param block The block index of the element.
             * @param data_ptr The pointer to the block data.
             * @param container The blocksList container.
             */
            explicit Iterator(integer pos, integer block, TYPE** data_ptr, const blocksList* container);

            /**
             * @brief Checks if two iterators point to the same element.
             * @param other The other iterator to compare.
             * @return True if the iterators point to the same element, false otherwise.
             */
            bool equalPtr(const iterator<TYPE>* other) const override;

        public:
            friend blocksList;

            /**
             * @brief Copy constructor for the Iterator.
             * @param other The iterator to copy.
             */
            Iterator(const Iterator& other);

            /**
             * @brief Assignment operator for the Iterator.
             * @param other The iterator to assign.
             * @return A reference to this iterator.
             */
            Iterator& operator=(const Iterator& other);

            /**
             * @brief Clones the iterator.
             * @return A new iterator pointing to the same element.
             */
            Iterator* clone() const override;

            /**
             * @brief Checks if there is a next element.
             * @return True if there is a next element, false otherwise.
             */
            [[nodiscard]] bool hasNext() const override;

            /**
             * @brief Checks if there is a previous element.
             * @return True if there is a previous element, false otherwise.
             */
            [[nodiscard]] bool hasPrev() const override;

            /**
             * @brief Moves the iterator to the next element.
             */
            void next() const override;

            /**
             * @brief Moves the iterator to the previous element.
             */
            void prev() const override;

            /**
             * @brief Advances the iterator by the specified number of steps.
             * @param steps The number of steps to advance.
             */
            void operator+=(integer steps) const override;

            /**
             * @brief Moves the iterator backward by the specified number of steps.
             * @param steps The number of steps to move backward.
             */
            void operator-=(integer steps) const override;

            /**
             * @brief Computes the distance between two iterators.
             * @param other The other iterator to compare.
             * @return The distance between the two iterators.
             */
            integer operator-(const iterator<TYPE>& other) const override;

            /**
             * @brief Gets the previous iterator.
             * @return A new iterator pointing to the previous element.
             */
            Iterator* getPrev() const override;

            /**
             * @brief Gets the next iterator.
             * @return A new iterator pointing to the next element.
             */
            Iterator* getNext() const override;

            /**
             * @brief Gets the element pointed to by the iterator.
             * @return A reference to the element.
             */
            TYPE& get() override;

            /**
             * @brief Gets the element pointed to by the iterator (const version).
             * @return A const reference to the element.
             */
            TYPE get() const override;

            /**
             * @brief Sets the value of the element pointed to by the iterator.
             * @param data The value to set.
             */
            void set(const TYPE& data) override;

            /**
             * @brief Checks if the iterator is valid.
             * @return True if the iterator is valid, false otherwise.
             */
            [[nodiscard]] bool isValid() const override;

            /**
             * @brief Checks if the iterator is at the previous element relative to another iterator.
             * @param other The other iterator to compare.
             * @return True if the iterator is at the previous element, false otherwise.
             */
            bool atPrev(const iterator<TYPE>* other) const override;

            /**
             * @brief Checks if the iterator is at the next element relative to another iterator.
             * @param other The other iterator to compare.
             * @return True if the iterator is at the next element, false otherwise.
             */
            bool atNext(const iterator<TYPE>* other) const override;

            /**
             * @brief Gets the class name of the iterator.
             * @return The class name as a string.
             */
            [[nodiscard]] std::string className() const override;
        };

        friend Iterator;

        /**
         * @brief Constructs an empty blocksList.
         * @param alloc Allocator instance to use for memory management
         * @details Initializes an empty blocksList with one initial block allocated
         *          using the provided allocator.
         */
        explicit blocksList(ALLOC alloc = ALLOC{});

        /**
         * @brief Constructs a blocksList from an initializer list.
         * @param lst The initializer list to construct the blocksList from.
         */
        blocksList(const std::initializer_list<TYPE>& lst);

        /**
         * @brief Constructs a blocksList from an array.
         * @param arr The array to construct the blocksList from.
         */
        explicit blocksList(const array<TYPE>& arr);

        /**
         * @brief Copy constructor.
         * @param other The blocksList to copy from.
         * @details Creates a deep copy of another blocksList.
         *          If ALLOC::propagate_on_container_copy_assignment is true, the allocator is also copied.
         */
        blocksList(const blocksList& other);

        /**
         * @brief Copy assignment operator.
         * @param other The blocksList to assign from.
         * @return A reference to this blocksList.
         * @details Performs a deep copy of another blocksList.
         *          If ALLOC::propagate_on_container_copy_assignment is true, the allocator is also copied.
         */
        blocksList& operator=(const blocksList& other);

        /**
         * @brief Move constructor.
         * @param other The blocksList to move from.
         * @details Moves the contents of another blocksList into this one.
         *          If ALLOC::propagate_on_container_move_assignment is true, the allocator is also moved.
         */
        blocksList(blocksList&& other) noexcept;

        /**
         * @brief Move assignment operator.
         * @param other The blocksList to move from.
         * @return A reference to this blocksList.
         * @details Moves the contents of another blocksList into this one.
         *          If ALLOC::propagate_on_container_move_assignment is true, the allocator is also moved.
         */
        blocksList& operator=(blocksList&& other) noexcept;

        /**
         * @brief Swaps the contents of this blocksList with another.
         * @param other The blocksList to swap with.
         * @details Exchanges the contents and allocators (if propagate_on_container_swap is true)
         *          of this blocksList with another.
         */
        void swap(blocksList& other) noexcept;

        /**
         * @brief Gets the element at the specified index.
         * @param index The index of the element to retrieve.
         * @return The element at the specified index.
         */
        TYPE get(integer index) const override;

        /**
         * @brief Gets the size of the blocksList.
         * @return The size of the blocksList.
         */
        [[nodiscard]] u_integer size() const override;

        /**
         * @brief Gets an iterator to the beginning of the blocksList.
         * @return An iterator to the beginning of the blocksList.
         */
        Iterator* begins() const override;

        /**
         * @brief Gets an iterator to the end of the blocksList.
         * @return An iterator to the end of the blocksList.
         */
        Iterator* ends() const override;

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
        void set(integer index, const TYPE& e) override;

        /**
         * @brief Finds the index of the first occurrence of the specified element.
         * @param e The element to search for.
         * @return The index of the element, or the size of the blocksList if not found.
         */
        u_integer indexOf(const TYPE& e) const override;

        /**
         * @brief Pushes an element to the specified index in the blocksList.
         * @param index The index at which to insert the element.
         * @param e The element to insert.
         */
        void push(integer index, const TYPE& e) override;

        /**
         * @brief Pops the element at the specified index in the blocksList.
         * @param index The index of the element to pop.
         * @return The element that was popped.
         */
        TYPE pop(integer index) override;

        /**
         * @brief Pushes an element to the beginning of the blocksList.
         * @param e The element to push.
         */
        void pushBegin(const TYPE& e) override;

        /**
         * @brief Pops the element from the beginning of the blocksList.
         * @return The element that was popped.
         */
        TYPE popBegin() override;

        /**
         * @brief Pushes an element to the end of the blocksList.
         * @param e The element to push.
         */
        void pushEnd(const TYPE& e) override;

        /**
         * @brief Pops the element from the end of the blocksList.
         * @return The element that was popped.
         */
        TYPE popEnd() override;

        /**
         * @brief Gets the class name of the blocksList.
         * @return The class name as a string.
         */
        [[nodiscard]] std::string className() const override;

        /**
         * @brief Destructor for the blocksList.
         */
        ~blocksList() override;
    };
}// namespace original

namespace std {
    /**
     * @brief Specialization of std::swap for original::blocksList
     * @tparam TYPE Element type
     * @tparam ALLOC Allocator type
     * @param lhs Left blocksList
     * @param rhs Right blocksList
     */
    template<typename TYPE, typename ALLOC>
    void swap(original::blocksList<TYPE, ALLOC>& lhs, original::blocksList<TYPE, ALLOC>& rhs) noexcept; // NOLINT
}

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::blocksListInit() -> void
    {
        this->map = vector({this->blockArrayInit()});
        this->size_ = 0;
        this->first_ = POS_INIT + 1;
        this->last_ = POS_INIT;
        this->first_block = this->map.size() / 2;
        this->last_block = this->map.size() / 2;
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::blocksListDestroy() noexcept -> void
    {
        for (auto* block : this->map) {
            for (u_integer i = 0; i < BLOCK_MAX_SIZE; ++i) {
                this->destroy(&block[i]);
            }
            this->deallocate(block, BLOCK_MAX_SIZE);
        }
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::blockArrayInit() -> TYPE* {
        auto arr = this->allocate(BLOCK_MAX_SIZE);
        for (u_integer i = 0; i < BLOCK_MAX_SIZE; i++) {
            this->construct(&arr[i]);
        }
        return arr;
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::innerIdxToAbsIdx(const u_integer block, const u_integer pos) -> u_integer
    {
        return block * BLOCK_MAX_SIZE + pos;
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::firstAbsIdx() const -> u_integer
    {
        return innerIdxToAbsIdx(this->first_block, this->first_);
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::lastAbsIdx() const -> u_integer
    {
        return innerIdxToAbsIdx(this->last_block, this->last_);
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::absIdxToOuterIdx(const u_integer absIdx) const -> integer
    {
        return absIdx - this->firstAbsIdx();
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::outerIdxToAbsIdx(const integer outerIdx) const -> u_integer
    {
        return this->firstAbsIdx() + outerIdx;
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::absIdxToInnerIdx(const u_integer absIdx) -> couple<u_integer, u_integer>
    {
        return {absIdx / BLOCK_MAX_SIZE, absIdx % BLOCK_MAX_SIZE};
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::innerIdxOffset(const u_integer block, const u_integer pos,
                                                    const integer offset) -> couple<u_integer, u_integer>
    {
        return absIdxToInnerIdx(static_cast<u_integer>(static_cast<integer>(innerIdxToAbsIdx(block, pos)) + offset));
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::outerIdxToInnerIdx(const integer outerIdx) const -> couple<u_integer, u_integer>
    {
        return absIdxToInnerIdx(this->outerIdxToAbsIdx(outerIdx));
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::innerIdxToOuterIdx(const u_integer block, const u_integer pos) const -> integer
    {
        return this->absIdxToOuterIdx(innerIdxToAbsIdx(block, pos));
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::getElem(u_integer block, u_integer pos) const -> TYPE&
    {
        return this->map.get(block)[pos];
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::setElem(u_integer block, u_integer pos, const TYPE& e) -> void
    {
        this->map.get(block)[pos] = e;
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::growNeeded(const u_integer increment, bool is_first) const -> bool
    {
        return is_first ? firstAbsIdx() < increment
        : lastAbsIdx() + increment > innerIdxToAbsIdx(this->map.size() - 1, BLOCK_MAX_SIZE - 1);
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::moveElements(const u_integer start_block, const u_integer start_pos,
                                                  const u_integer len, const integer offset) -> void
    {
        if (offset > 0)
        {
            for (u_integer i = 0; i < len; i++)
            {
                auto idx = innerIdxOffset(start_block, start_pos, len - 1 - i);
                auto idx_offset = innerIdxOffset(start_block, start_pos, len - 1 - i + offset);
                this->setElem(idx_offset.first(), idx_offset.second(),
                              this->getElem(idx.first(), idx.second()));
            }
        }else
        {
            for (u_integer i = 0; i < len; i++)
            {
                auto idx = innerIdxOffset(start_block, start_pos, i);
                auto idx_offset = innerIdxOffset(start_block, start_pos, i + offset);
                this->setElem(idx_offset.first(), idx_offset.second(),
                              this->getElem(idx.first(), idx.second()));
            }
        }
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::addBlock(bool is_first) -> void
    {
        auto* new_block = this->blockArrayInit();
        is_first ? this->map.pushBegin(new_block) : this->map.pushEnd(new_block);
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::adjust(const u_integer increment, const bool is_first) -> void
    {
        if (this->growNeeded(increment, is_first)){
            u_integer new_blocks_cnt = increment / BLOCK_MAX_SIZE + 1;
            for (u_integer i = 0; i < new_blocks_cnt; ++i) {
                this->addBlock(is_first);
            }
            if (is_first){
                this->first_block += new_blocks_cnt;
                this->last_block += new_blocks_cnt;
            }
        }
    }

    template <typename TYPE, typename ALLOC>
    original::blocksList<TYPE, ALLOC>::Iterator::Iterator(const integer pos, const integer block, TYPE** data_ptr, const blocksList* container)
        : cur_pos(pos), cur_block(block), data_(data_ptr), container_(container) {}

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::Iterator::equalPtr(const iterator<TYPE>* other) const -> bool
    {
        auto* other_it = dynamic_cast<const Iterator*>(other);
        return other_it != nullptr
               && this->cur_pos == other_it->cur_pos
               && this->cur_block == other_it->cur_block
               && this->data_ == other_it->data_
               && this->container_ == other_it->container_;
    }

    template <typename TYPE, typename ALLOC>
    original::blocksList<TYPE, ALLOC>::Iterator::Iterator(const Iterator& other) : Iterator(0, 0, nullptr, nullptr)
    {
        this->operator=(other);
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::Iterator::operator=(const Iterator& other) -> Iterator&
    {
        if (this == &other)
            return *this;

        this->cur_pos = other.cur_pos;
        this->cur_block = other.cur_block;
        this->data_ = other.data_;
        this->container_ = other.container_;
        return *this;
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::Iterator::clone() const -> Iterator*
    {
        return new Iterator(*this);
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::Iterator::hasNext() const -> bool
    {
        return blocksList::innerIdxToAbsIdx(this->cur_block, this->cur_pos) < this->container_->lastAbsIdx();
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::Iterator::hasPrev() const -> bool
    {
        return blocksList::innerIdxToAbsIdx(this->cur_block, this->cur_pos) > this->container_->firstAbsIdx();
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::Iterator::next() const -> void
    {
        this->operator+=(1);
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::Iterator::prev() const -> void
    {
        this->operator-=(1);
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::Iterator::operator+=(const integer steps) const -> void
    {
        auto new_idx = innerIdxOffset(this->cur_block, this->cur_pos, steps);
        this->cur_block = new_idx.first();
        this->cur_pos = new_idx.second();
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::Iterator::operator-=(const integer steps) const -> void
    {
        auto new_idx = innerIdxOffset(this->cur_block, this->cur_pos, -steps);
        this->cur_block = new_idx.first();
        this->cur_pos = new_idx.second();
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::Iterator::operator-(const iterator<TYPE>& other) const -> integer
    {
        auto* other_it = dynamic_cast<const Iterator*>(&other);
        if (other_it == nullptr)
            return this > &other ?
                (std::numeric_limits<integer>::max)() :
                (std::numeric_limits<integer>::min)();
        if (this->container_ != other_it->container_)
            return this->container_ > other_it->container_ ?
                (std::numeric_limits<integer>::max)() :
                (std::numeric_limits<integer>::min)();

        return static_cast<integer>(innerIdxToAbsIdx(this->cur_block, this->cur_pos)) -
               static_cast<integer>(innerIdxToAbsIdx(other_it->cur_block, other_it->cur_pos));
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::Iterator::getPrev() const -> Iterator*
    {
        if (!this->isValid()) throw outOfBoundError();
        auto* it = this->clone();
        it->prev();
        return it;
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::Iterator::getNext() const -> Iterator*
    {
        if (!this->isValid()) throw outOfBoundError();
        auto* it = this->clone();
        it->next();
        return it;
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::Iterator::get() -> TYPE&
    {
        if (!this->isValid()) throw outOfBoundError();
        return this->data_[this->cur_block][this->cur_pos];
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::Iterator::get() const -> TYPE
    {
        if (!this->isValid()) throw outOfBoundError();
        return this->data_[this->cur_block][this->cur_pos];
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::Iterator::set(const TYPE& data) -> void
    {
        if (!this->isValid()) throw outOfBoundError();
        this->data_[this->cur_block][this->cur_pos] = data;
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::Iterator::isValid() const -> bool
    {
        return this->container_->innerIdxToOuterIdx(this->cur_block, this->cur_pos) >= 0 &&
               this->container_->innerIdxToOuterIdx(this->cur_block, this->cur_pos) < this->container_->size();
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::Iterator::atPrev(const iterator<TYPE>* other) const -> bool
    {
        auto* other_it = dynamic_cast<const Iterator*>(other);
        if (other_it == nullptr)
            return false;
        return this->operator-(*other_it) == -1;
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::Iterator::atNext(const iterator<TYPE>* other) const -> bool
    {
        auto* other_it = dynamic_cast<const Iterator*>(other);
        if (other_it == nullptr)
            return false;
        return this->operator-(*other_it) == 1;
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::Iterator::className() const -> std::string {
        return "blocksList::Iterator";
    }

    template <typename TYPE, typename ALLOC>
    original::blocksList<TYPE, ALLOC>::blocksList(ALLOC alloc)
        : baseList<TYPE, ALLOC>(std::move(alloc)), map(), size_(), first_(), last_(), first_block(), last_block()
    {
        this->blocksListInit();
    }

    template <typename TYPE, typename ALLOC>
    original::blocksList<TYPE, ALLOC>::blocksList(const std::initializer_list<TYPE>& lst) : blocksList() {
        this->adjust(lst.size(), false);
        for (const auto& e : lst) {
            auto new_idx = innerIdxOffset(this->last_block, this->last_, 1);
            this->last_block = new_idx.first();
            this->last_ = new_idx.second();
            this->setElem(this->last_block, this->last_, e);
            this->size_ += 1;
        }
    }

    template <typename TYPE, typename ALLOC>
    original::blocksList<TYPE, ALLOC>::blocksList(const array<TYPE>& arr) : blocksList() {
        this->adjust(arr.size(), false);
        for (const auto& e : arr) {
            auto new_idx = innerIdxOffset(this->last_block, this->last_, 1);
            this->last_block = new_idx.first();
            this->last_ = new_idx.second();
            this->setElem(this->last_block, this->last_, e);
            this->size_ += 1;
        }
    }

    template <typename TYPE, typename ALLOC>
    original::blocksList<TYPE, ALLOC>::blocksList(const blocksList& other) : blocksList() {
        this->operator=(other);
    }

    template <typename TYPE, typename ALLOC>
    original::blocksList<TYPE, ALLOC>& original::blocksList<TYPE, ALLOC>::operator=(const blocksList& other) {
        if (this == &other) return *this;

        this->blocksListDestroy();
        this->map = vector<TYPE*>{};

        for (integer i = 0; i < other.map.size(); ++i) {
            auto* block = this->blockArrayInit();
            for (u_integer j = 0; j < BLOCK_MAX_SIZE; ++j) {
                block[j] = other.getElem(i, j);
            }
            this->map.pushEnd(block);
        }

        this->first_ = other.first_;
        this->last_ = other.last_;
        this->size_ = other.size_;
        this->first_block = other.first_block;
        this->last_block = other.last_block;
        if constexpr (ALLOC::propagate_on_container_copy_assignment::value){
            this->allocator = other.allocator;
        }
        return *this;
    }

    template <typename TYPE, typename ALLOC>
    original::blocksList<TYPE, ALLOC>::blocksList(blocksList&& other) noexcept : blocksList()
    {
        this->operator=(std::move(other));
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::operator=(blocksList&& other) noexcept -> blocksList&
    {
        if (this == &other)
            return *this;

        this->blocksListDestroy();

        this->map = std::move(other.map);
        this->first_ = other.first_;
        this->last_ = other.last_;
        this->first_block = other.first_block;
        this->last_block = other.last_block;
        this->size_ = other.size_;
        if constexpr (ALLOC::propagate_on_container_move_assignment::value){
            this->allocator = std::move(other.allocator);
        }
        other.blocksListInit();
        return *this;
    }

    template <typename TYPE, typename ALLOC>
    void original::blocksList<TYPE, ALLOC>::swap(blocksList& other) noexcept
    {
        if (this == &other)
            return;

        std::swap(this->map, other.map);
        std::swap(this->size_, other.size_);
        std::swap(this->first_, other.first_);
        std::swap(this->last_, other.last_);
        std::swap(this->first_block, other.first_block);
        std::swap(this->last_block, other.last_block);
        if constexpr (ALLOC::propagate_on_container_swap::value) {
            std::swap(this->allocator, other.allocator);
        }
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::get(integer index) const -> TYPE {
        if (this->indexOutOfBound(this->parseNegIndex(index))) throw outOfBoundError();
        index = this->parseNegIndex(index);
        auto inner_idx = this->outerIdxToInnerIdx(index);
        return this->getElem(inner_idx.first(), inner_idx.second());
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::size() const -> u_integer
    {
        return this->size_;
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::begins() const -> Iterator* {
        return new Iterator(this->first_, this->first_block, &this->map.data(), this);
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::ends() const -> Iterator* {
        return new Iterator(this->last_, this->last_block, &this->map.data(), this);
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::operator[](integer index) -> TYPE& {
        if (this->indexOutOfBound(this->parseNegIndex(index))) throw outOfBoundError();
        index = this->parseNegIndex(index);
        auto inner_idx = this->outerIdxToInnerIdx(index);
        return this->getElem(inner_idx.first(), inner_idx.second());
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::set(integer index, const TYPE &e) -> void {
        if (this->indexOutOfBound(index)) throw outOfBoundError();
        index = this->parseNegIndex(index);
        auto inner_idx = this->outerIdxToInnerIdx(index);
        this->setElem(inner_idx.first(), inner_idx.second(), e);
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::indexOf(const TYPE &e) const -> u_integer {
        for (u_integer i = 0; i < this->size(); ++i) {
            if (auto idx = this->outerIdxToInnerIdx(i);
                this->getElem(idx.first(), idx.second()) == e)
                    return i;
        }
        return this->size();
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::push(integer index, const TYPE& e) -> void
    {
        if (this->parseNegIndex(index) == this->size())
        {
            this->pushEnd(e);
        } else if (this->parseNegIndex(index) == 0)
        {
            this->pushBegin(e);
        } else{
            if (this->indexOutOfBound(index))
                throw outOfBoundError();

            index = this->parseNegIndex(index);
            const bool is_first = index <= (this->size() - 1) / 2;
            this->adjust(1, is_first);
            if (is_first){
                this->moveElements(this->first_block, this->first_, index + 1, -1);
                auto new_idx = innerIdxOffset(this->first_block, this->first_, -1);
                this->first_block = new_idx.first();
                this->first_ = new_idx.second();
            } else{
                auto idx = outerIdxToInnerIdx(index);
                this->moveElements(idx.first(), idx.second(), this->size() - index, 1);
                auto new_idx = innerIdxOffset(this->last_block, this->last_, 1);
                this->last_block = new_idx.first();
                this->last_ = new_idx.second();
            }
            this->size_ += 1;
            auto idx = outerIdxToInnerIdx(index);
            this->setElem(idx.first(), idx.second(), e);
        }
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::pop(integer index) -> TYPE
    {
        if (this->parseNegIndex(index) == 0)
            return this->popBegin();
        if (this->parseNegIndex(index) == this->size() - 1)
            return this->popEnd();
        if (this->indexOutOfBound(index))
            throw outOfBoundError();

        index = this->parseNegIndex(index);
        auto idx = outerIdxToInnerIdx(index);
        TYPE res = this->getElem(idx.first(), idx.second());
        if (index <= (this->size() - 1) / 2){
            moveElements(this->first_block, this->first_, index, 1);
            auto new_idx = innerIdxOffset(this->first_block, this->first_, 1);
            this->first_block = new_idx.first();
            this->first_ = new_idx.second();
        } else{
            auto idx_offset = innerIdxOffset(idx.first(), idx.second(), 1);
            moveElements(idx_offset.first(), idx_offset.second(), this->size() - 1 - index, -1);
            auto new_idx = innerIdxOffset(this->last_block, this->last_, -1);
            this->last_block = new_idx.first();
            this->last_ = new_idx.second();
        }
        this->size_ -= 1;
        return res;
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::pushBegin(const TYPE& e) -> void
    {
        this->adjust(1, true);
        auto new_idx = innerIdxOffset(this->first_block, this->first_, -1);
        this->first_block = new_idx.first();
        this->first_ = new_idx.second();
        this->setElem(this->first_block, this->first_, e);
        this->size_ += 1;
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::popBegin() -> TYPE
    {
        if (this->empty()) throw noElementError();

        TYPE res = this->getElem(this->first_block, this->first_);
        auto new_idx = innerIdxOffset(this->first_block, this->first_, 1);
        this->first_block = new_idx.first();
        this->first_ = new_idx.second();
        this->size_ -= 1;
        return res;
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::pushEnd(const TYPE& e) -> void
    {
        this->adjust(1, false);
        auto new_idx = innerIdxOffset(this->last_block, this->last_, 1);
        this->last_block = new_idx.first();
        this->last_ = new_idx.second();
        this->setElem(this->last_block, this->last_, e);
        this->size_ += 1;
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::popEnd() -> TYPE
    {
        if (this->empty()) throw noElementError();

        TYPE res = this->getElem(this->last_block, this->last_);
        auto new_idx = innerIdxOffset(this->last_block, this->last_, -1);
        this->last_block = new_idx.first();
        this->last_ = new_idx.second();
        this->size_ -= 1;
        return res;
    }

    template <typename TYPE, typename ALLOC>
    auto original::blocksList<TYPE, ALLOC>::className() const -> std::string {
        return "blocksList";
    }

    template <typename TYPE, typename ALLOC>
    original::blocksList<TYPE, ALLOC>::~blocksList() {
        this->blocksListDestroy();
    }

    template <typename TYPE, typename ALLOC>
    void std::swap(original::blocksList<TYPE, ALLOC>& lhs, original::blocksList<TYPE, ALLOC>& rhs) noexcept // NOLINT
    {
        lhs.swap(rhs);
    }

#endif //BLOCKSLIST_H
