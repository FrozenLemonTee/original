#ifndef VECTOR_H
#define VECTOR_H

#include "serial.h"
#include "iterable.h"
#include "array.h"

namespace original{
    template <typename TYPE>
    class vector final : public serial<TYPE>, public iterable<TYPE>, public printable{
        class vectorNode final : public wrapper<TYPE>{
            public:
                friend class iterator<TYPE>;
                friend class vector;
            private:
                TYPE data_;
            protected:
                explicit vectorNode(TYPE data);
                TYPE& getVal() override;
                void setVal(TYPE data) override;
                vectorNode* getPPrev() override;
                vectorNode* getPNext() override;
        };

        size_t size_;
        const size_t INNER_SIZE_INIT = 16;
        size_t max_size;
        size_t inner_begin;
        vectorNode** body;

        static vectorNode** vectorNodeArrayInit(size_t size);
        static void moveElements(vectorNode** old_body, size_t inner_idx,
                          size_t len, vectorNode** new_body, size_t offset);
        size_t toInnerIdx(int index);
        _GLIBCXX_NODISCARD bool outOfMaxSize(size_t increment) const;
        void grow(size_t new_size);
        void adjust(size_t increment);
    public:
        explicit vector();
        vector(std::initializer_list<TYPE> list);
        explicit vector(array<TYPE> arr);
        _GLIBCXX_NODISCARD size_t size() const override;
        TYPE get(int index) override;
        TYPE operator[](int index) override;
        void set(int index, TYPE e) override;
        size_t indexOf(TYPE e) override;
        void pushBegin(TYPE e) override;
        void push(int index, TYPE e) override;
        void pushEnd(TYPE e) override;
        TYPE popBegin() override;
        TYPE pop(int index) override;
        TYPE popEnd() override;
        iterator<TYPE>* begins() override;
        iterator<TYPE>* ends() override;
        _GLIBCXX_NODISCARD std::string toString(bool enter) override;
        ~vector() override;
    };
}

    template<typename TYPE>
    original::vector<TYPE>::vectorNode::vectorNode(TYPE data) : data_(data) {}

    template <typename TYPE>
    auto original::vector<TYPE>::vectorNode::getVal() -> TYPE&
    {
        return this->data_;
    }

    template <typename TYPE>
    auto original::vector<TYPE>::vectorNode::setVal(TYPE data) -> void
    {
        this->data_ = data;
    }

    template <typename TYPE>
    auto original::vector<TYPE>::vectorNode::getPPrev() -> vectorNode* {
        return this - 1;
    }

    template <typename TYPE>
    auto original::vector<TYPE>::vectorNode::getPNext() -> vectorNode* {
        return this + 1;
    }

    template <typename TYPE>
    auto original::vector<TYPE>::vectorNodeArrayInit(const size_t size) -> vectorNode**
    {
        auto** arr = new vectorNode*[size];
        for (int i = 0; i < size; i += 1)
        {
            arr[i] = nullptr;
        }
        return arr;
    }

    template <typename TYPE>
    auto original::vector<TYPE>::moveElements(vectorNode** old_body, const size_t inner_idx,
        const size_t len, vectorNode** new_body, const size_t offset) -> void{
        if (offset > 0)
        {
            for (int i = 0; i < len; i += 1)
            {
                new_body[inner_idx + offset + len - 1 - i] = old_body[inner_idx + len - 1 - i];
                if (old_body == new_body)
                    old_body[inner_idx + len - 1 - i] = nullptr;
            }
        }else
        {
            for (int i = 0; i < len; i += 1)
            {
                new_body[inner_idx + offset + i] = old_body[inner_idx + i];
                if (old_body == new_body)
                    old_body[inner_idx + i] = nullptr;
            }
        }
    }

    template <typename TYPE>
    auto original::vector<TYPE>::toInnerIdx(int index) -> size_t
    {
        return this->inner_begin + index;
    }

    template <typename TYPE>
    auto original::vector<TYPE>::outOfMaxSize(size_t increment) const -> bool
    {
        return this->inner_begin + this->size() - 1 + increment > this->max_size - 1 || this->inner_begin - increment < 1;
    }

    template <typename TYPE>
    auto original::vector<TYPE>::grow(const size_t new_size) -> void
    {
        vectorNode** new_body = vector::vectorNodeArrayInit(new_size);
        size_t new_begin = (new_size - 1) / 4;
        vector::moveElements(this->body, this->inner_begin,
                             this->size(), new_body, new_begin - this->inner_begin);
        delete this->body;
        this->body = new_body;
        this->max_size = new_size;
        this->inner_begin = new_begin;
    }

    template <typename TYPE>
    auto original::vector<TYPE>::adjust(size_t increment) -> void {
        if (!this->outOfMaxSize(increment)) {
            return;
        }
        if (this->max_size >= this->size_ + increment) {
            size_t new_begin = (this->max_size - this->size() - increment) / 2;
            vector::moveElements(this->body, this->inner_begin, this->size(), this->body,
                                 new_begin - this->inner_begin);
            this->inner_begin = new_begin;
        } else {
            const size_t new_max_size = (this->size() + increment) * 2;
            grow(new_max_size);
        }
    }

    template <typename TYPE>
    original::vector<TYPE>::vector() : size_(0), max_size(this->INNER_SIZE_INIT),
        inner_begin((this->INNER_SIZE_INIT - 1)/2), body(vector::vectorNodeArrayInit(this->INNER_SIZE_INIT)) {}

    template <typename TYPE>
    original::vector<TYPE>::vector(std::initializer_list<TYPE> list) : vector()
    {
        vector::adjust(list.size());
        for (TYPE e : list)
        {
            this->body[this->inner_begin + this->size()] = new vectorNode(e);
            this->size_ += 1;
        }
    }

    template <typename TYPE>
    original::vector<TYPE>::vector(array<TYPE> arr) : vector()
    {
        vector::adjust(arr.size());
        for (int i = 0; i < arr.size(); i += 1)
        {
            this->body[this->toInnerIdx(i)] = new vectorNode(arr.get(i));
            this->size_ += 1;
        }
    }

template <typename TYPE>
    auto original::vector<TYPE>::size() const -> size_t
    {
        return this->size_;
    }

    template <typename TYPE>
    auto original::vector<TYPE>::get(int index) -> TYPE
    {
        if (this->indexOutOfBound(index))
        {
            throw indexError();
        }
        index = this->toInnerIdx(this->parseNegIndex(index));
        return this->body[index]->getVal();
    }

    template <typename TYPE>
    auto original::vector<TYPE>::operator[](const int index) -> TYPE
    {
        return this->get(index);
    }

    template <typename TYPE>
    auto original::vector<TYPE>::set(int index, TYPE e) -> void
    {
        if (this->indexOutOfBound(index))
        {
            throw indexError();
        }
        index = this->toInnerIdx(this->parseNegIndex(index));
        this->body[index]->setVal(e);
    }

    template <typename TYPE>
    auto original::vector<TYPE>::indexOf(TYPE e) -> size_t
    {
        for (int i = 0; i < this->size(); i += 1)
        {
            if (this->get(i) == e)
            {
                return i;
            }
        }
        return this->size();
    }

    template <typename TYPE>
    auto original::vector<TYPE>::pushBegin(TYPE e) -> void
    {
        adjust(1);
        this->inner_begin -= 1;
        this->body[this->toInnerIdx(0)] = new vectorNode(e);
        this->size_ += 1;
    }

    template <typename TYPE>
    auto original::vector<TYPE>::push(int index, TYPE e) -> void
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
                throw indexError();
            }
            adjust(1);
            index = this->toInnerIdx(this->parseNegIndex(index));
            if (index <= (this->size() - 1) / 2)
            {
                vector::moveElements(this->body, this->inner_begin, index + 1, this->body, -1);
                this->inner_begin -= 1;
            }else
            {
                vector::moveElements(this->body, index,
                    this->size() - 1 - index, this->body, 1);
            }
            this->body[index] = new vectorNode(e);
            this->size_ += 1;
        }
    }

    template <typename TYPE>
    auto original::vector<TYPE>::pushEnd(TYPE e) -> void
    {
        adjust(1);
        this->body[this->toInnerIdx(this->size())] = new vectorNode(e);
        this->size_ += 1;
    }

    template <typename TYPE>
    auto original::vector<TYPE>::popBegin() -> TYPE
    {
        if (this->size() == 0){
            throw noElementError();
        }
        size_t index = this->toInnerIdx(0);
        TYPE res = this->get(0);
        delete this->body[index];
        this->body[index] = nullptr;
        this->inner_begin += 1;
        this->size_ -= 1;
        return res;
    }

    template <typename TYPE>
    auto original::vector<TYPE>::pop(int index) -> TYPE
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
            throw indexError();
        }
        TYPE res = this->get(index);
        index = this->toInnerIdx(this->parseNegIndex(index));
        delete this->body[index];
        this->body[index] = nullptr;
        if (index <= (this->size() - 1) / 2)
        {
            vector::moveElements(this->body, this->inner_begin,
                index + 1, this->body, 1);
            this->inner_begin += 1;
        }else
        {
            vector::moveElements(this->body, index + 1,
                this->size() - 2 - index, this->body, -1);
        }
        this->size_ -= 1;
        return res;
    }

    template <typename TYPE>
    auto original::vector<TYPE>::popEnd() -> TYPE
    {
        if (this->size() == 0){
            throw noElementError();
        }
        size_t index = this->toInnerIdx(this->size() - 1);
        TYPE res = this->get(this->size() - 1);
        delete this->body[index];
        this->body[index] = nullptr;
        this->size_ -= 1;
        return res;
    }

    template <typename TYPE>
    auto original::vector<TYPE>::begins() -> iterator<TYPE>*
    {
        size_t index = this->toInnerIdx(0);
        return new iterator<TYPE>(this->body[index]);
    }

    template <typename TYPE>
    auto original::vector<TYPE>::ends() -> iterator<TYPE>*
    {
        size_t index = this->toInnerIdx(this->size() - 1);
        return new iterator<TYPE>(this->body[index]);
    }

    template <typename TYPE>
    std::string original::vector<TYPE>::toString(const bool enter)
    {
        std::stringstream ss;
        ss << "vector" << "(";
        for (int i = 0; i < this->size(); i += 1)
        {
            size_t index = this->toInnerIdx(i);
            if (this->body[index] - this->body[index - 1] == 1) // 这是一段测试结点地址之间是否连续的代码
            {
                ss << "<->";
            }
            ss << this->body[index]->toString(false);
            if (i < this->size() - 1)
            {
                ss << "," << " ";
            }
        }
        ss << ")";
        if (enter)
        {
            ss << "\n";
        }
        return ss.str();
    }

    template <typename TYPE>
    original::vector<TYPE>::~vector() {
        for (size_t i = 0; i < this->max_size; ++i) {
            if (this->body[i] != nullptr) {
                delete this->body[i];
            }
        }
        delete[] this->body;
    }

#endif //VECTOR_H
