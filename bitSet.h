#ifndef BITSET_H
#define BITSET_H
#include <array.h>
#include <couple.h>
#include <iterationStream.h>
#include <serial.h>


namespace original {
    class bitSet final : public serial<bool>, public iterationStream<bool>{
            using underlying_type = std::size_t;

            static constexpr int BLOCK_MAX_SIZE = sizeof(underlying_type) * 8;
            array<underlying_type> map;
            uint32_t size_;

            static bool getBitFromBlock(underlying_type block_value, int64_t bit);
            static underlying_type setBitFromBlock(underlying_type block_value, int64_t bit);
            static underlying_type clearBitFromBlock(underlying_type block_value, int64_t bit);
            bool getBit(int64_t bit, int64_t block) const;
            void setBit(int64_t bit, int64_t block);
            void clearBit(int64_t bit, int64_t block);
            void writeBit(int64_t bit, int64_t block, bool value);
            static couple<uint32_t, int64_t> toInnerIdx(int64_t index);
        public:
            class Iterator final : public iterator<bool> {
                    mutable int64_t cur_bit;
                    mutable int64_t cur_block;
                    mutable underlying_type* block_;
                    const bitSet* container_;

                    explicit Iterator(int64_t bit, int64_t block, underlying_type* block_p, const bitSet* container);
                    bool equalPtr(const iterator *other) const override;
                public:
                    friend class bitSet;
                    Iterator* clone() const override;
                    [[nodiscard]] bool hasNext() const override;
                    [[nodiscard]] bool hasPrev() const override;
                    bool atPrev(const iterator *other) const override;
                    bool atNext(const iterator *other) const override;
                    void next() const override;
                    void prev() const override;
                    Iterator* getPrev() override;
                    Iterator* getNext() override;
                    bool& get() override;
                    bool getElem() const override;
                    [[nodiscard]] std::string className() const override;
                    const bool& get() const override;
                    void set(const bool &data) override;
                    [[nodiscard]] bool isValid() const override;
            };

            explicit bitSet(uint32_t size);
            bitSet(std::initializer_list<bool> lst);
            bitSet(const bitSet& other);
            bitSet& operator=(const bitSet& other);
            bool operator==(const bitSet& other) const;
            [[nodiscard]] uint32_t size() const override;
            Iterator* begins() const override;
            Iterator* ends() const override;
            bool get(int64_t index) const override;
            bool& operator[](int64_t index) override;
            void set(int64_t index, const bool &e) override;
            uint32_t indexOf(const bool &e) const override;
            [[nodiscard]] std::string className() const override;

            template<typename Callback = transform<bool>>
            void forEach(Callback operation = Callback{}) = delete;
    };
}

    inline auto original::bitSet::getBitFromBlock(const underlying_type block_value, const int64_t bit) -> bool {
        return block_value & 1 << bit;
    }

    inline auto original::bitSet::setBitFromBlock(const underlying_type block_value, const int64_t bit) -> underlying_type {
        return block_value | 1 << bit;
    }

    inline auto original::bitSet::clearBitFromBlock(const underlying_type block_value, const int64_t bit) -> underlying_type {
        return block_value & ~(1 << bit);
    }

    inline auto original::bitSet::getBit(const int64_t bit, const int64_t block) const -> bool {
        return getBitFromBlock(this->map.get(block), bit);
    }

    inline auto original::bitSet::setBit(const int64_t bit, const int64_t block) -> void {
        this->map.set(block, setBitFromBlock(this->map.get(block), bit));
    }

    inline auto original::bitSet::clearBit(const int64_t bit, const int64_t block) -> void {
        return this->map.set(block, clearBitFromBlock(this->map.get(block), bit));
    }

    inline void original::bitSet::writeBit(const int64_t bit, const int64_t block, const bool value) {
        value ? this->setBit(bit, block) : this->clearBit(bit, block);
    }

    inline auto original::bitSet::toInnerIdx(const int64_t index) -> couple<uint32_t, int64_t> {
        return {static_cast<unsigned>(index / BLOCK_MAX_SIZE), index % BLOCK_MAX_SIZE};
    }

    inline original::bitSet::Iterator::Iterator(const int64_t bit, const int64_t block, underlying_type* block_p, const bitSet* container)
        : cur_bit(bit), cur_block(block), block_(block_p), container_(container) {}

    inline auto original::bitSet::Iterator::equalPtr(const iterator *other) const -> bool {
        auto* other_it = dynamic_cast<const Iterator*>(other);
        return other_it != nullptr
            && this->cur_bit == other_it->cur_bit
            && this->cur_block == other_it->cur_block;
    }

    inline auto original::bitSet::Iterator::clone() const -> Iterator* {
        return new Iterator(*this);
    }

    inline auto original::bitSet::Iterator::hasNext() const -> bool {
        return this->cur_block < this->container_->map.size();
    }

    inline auto original::bitSet::Iterator::hasPrev() const -> bool {
        return this->cur_block >= 0;
    }

    inline auto original::bitSet::Iterator::atPrev(const iterator *other) const -> bool {
        auto* other_it = dynamic_cast<const Iterator*>(other);
        if (other_it == nullptr)
            return false;
        if (this->cur_block == other_it->cur_block)
            return this->cur_bit + 1 == other_it->cur_bit;
        return this->cur_block + 1 == other_it->cur_block
        && this->cur_bit == BLOCK_MAX_SIZE - 1
        && other_it->cur_bit == 0;
    }

    inline auto original::bitSet::Iterator::atNext(const iterator *other) const -> bool {
        auto* other_it = dynamic_cast<const Iterator*>(other);
        if (other_it == nullptr)
            return false;
        if (this->cur_block == other_it->cur_block)
            return this->cur_bit - 1 == other_it->cur_bit;
        return this->cur_block - 1 == other_it->cur_block
            && this->cur_bit == 0
            && other_it->cur_bit == BLOCK_MAX_SIZE - 1;
    }

    inline auto original::bitSet::Iterator::next() const -> void {
        if (this->cur_bit < BLOCK_MAX_SIZE - 1) {
            this->cur_bit += 1;
        }else {
            this->cur_bit = 0;
            this->cur_block += 1;
            this->block_ += 1;
        }
    }

    inline auto original::bitSet::Iterator::prev() const -> void {
        if (this->cur_bit > 0) {
            this->cur_bit -= 1;
        }else {
            this->cur_bit = BLOCK_MAX_SIZE - 1;
            this->cur_block -= 1;
            this->block_ -= 1;
        }
    }

    inline auto original::bitSet::Iterator::getPrev() -> Iterator* {
        if (!this->isValid()) throw outOfBoundError();
        auto* it = this->clone();
        it->prev();
        return it;
    }

    inline auto original::bitSet::Iterator::getNext() -> Iterator* {
        if (!this->isValid()) throw outOfBoundError();
        auto* it = this->clone();
        it->next();
        return it;
    }

    inline auto original::bitSet::Iterator::get() -> bool & {
        throw unSupportedMethodError();
    }

    inline bool original::bitSet::Iterator::getElem() const {
        if (!this->isValid()) throw outOfBoundError();
        return getBitFromBlock(*this->block_, this->cur_bit);
    }

    inline auto original::bitSet::Iterator::className() const -> std::string {
        return "bitSet::Iterator";
    }

    inline auto original::bitSet::Iterator::get() const -> const bool& {
        throw unSupportedMethodError();
    }

    inline void original::bitSet::Iterator::set(const bool &data) {
        if (!this->isValid()) throw outOfBoundError();
        *this->block_ = data ?
         setBitFromBlock(*this->block_, this->cur_bit) : clearBitFromBlock(*this->block_, this->cur_bit);
    }

    inline auto original::bitSet::Iterator::isValid() const -> bool {
        return this->cur_block < this->container_->map.size()
            && this->cur_bit + this->cur_block * BLOCK_MAX_SIZE < this->container_->size();
    }

    inline original::bitSet::bitSet(const uint32_t size)
        : map(array<underlying_type>((size + BLOCK_MAX_SIZE - 1) / BLOCK_MAX_SIZE)), size_(size) {}

    inline original::bitSet::bitSet(const std::initializer_list<bool> lst) : bitSet(lst.size()) {
        uint32_t i = 0;
        for (const auto& e : lst) {
            auto idx = toInnerIdx(i);
            this->writeBit(idx.second(), idx.first(), e);
            i += 1;
        }
    }

    inline original::bitSet::bitSet(const bitSet &other) : bitSet(other.size()) {
        this->operator=(other);
    }

    inline auto original::bitSet::operator=(const bitSet &other) -> bitSet& {
        if (this == &other) return *this;
        this->map = other.map;
        this->size_ = other.size_;
        return *this;
    }

    inline auto original::bitSet::operator==(const bitSet &other) const -> bool {
        if (this == &other) return true;
        return this->map == other.map && this->size_ == other.size_;
    }

    inline auto original::bitSet::size() const -> uint32_t {
        return this->size_;
    }

    inline auto original::bitSet::begins() const -> Iterator* {
        return new Iterator(0, 0, &this->map.data(), this);
    }

    inline auto original::bitSet::ends() const -> Iterator* {
        return new Iterator(BLOCK_MAX_SIZE - 1, this->map.size() - 1,
                    &this->map.data() + this->map.size() - 1, this);
    }

    inline auto original::bitSet::get(int64_t index) const -> bool {
        if (this->indexOutOfBound(index)) throw outOfBoundError();
        index = this->parseNegIndex(index);
        auto idx = toInnerIdx(index);
        return this->getBit(idx.second(), idx.first());
    }

    inline auto original::bitSet::operator[](int64_t index) -> bool& {
        throw unSupportedMethodError();
    }

    inline auto original::bitSet::set(int64_t index, const bool &e) -> void {
        if (this->indexOutOfBound(index)) throw outOfBoundError();
        index = this->parseNegIndex(index);
        auto idx = toInnerIdx(index);
        this->writeBit(idx.second(), idx.first(), e);
    }

    inline auto original::bitSet::indexOf(const bool &e) const -> uint32_t {
        for (uint32_t i = 0; i < BLOCK_MAX_SIZE; i++) {
            if (auto idx = toInnerIdx(i);
                e == this->getBit(idx.second(), idx.first())) {
                return i;
            }
        }
        return BLOCK_MAX_SIZE;
    }

    inline auto original::bitSet::className() const -> std::string {
        return "bitSet";
    }

#endif //BITSET_H