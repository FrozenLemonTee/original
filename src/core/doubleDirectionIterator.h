#ifndef DOUBLEDIRECTIONITERATOR_H
#define DOUBLEDIRECTIONITERATOR_H
#include "stepIterator.h"

namespace original {
    template<typename TYPE>
    class doubleDirectionIterator : public stepIterator<TYPE> {
    protected:
        explicit doubleDirectionIterator(wrapper<TYPE>* ptr);
    public:
        doubleDirectionIterator(const doubleDirectionIterator& other);
        doubleDirectionIterator& operator=(const doubleDirectionIterator& other);
        doubleDirectionIterator* clone() const override;
        [[nodiscard]] bool hasPrev() const override;
        void prev() const override;
    };
}

    template<typename TYPE>
    original::doubleDirectionIterator<TYPE>::doubleDirectionIterator(wrapper<TYPE>* ptr)
        : stepIterator<TYPE>(ptr) {}

    template<typename TYPE>
    original::doubleDirectionIterator<TYPE>::doubleDirectionIterator(const doubleDirectionIterator &other)
        : stepIterator<TYPE>(nullptr) {
        this->operator=(other);
    }

    template<typename TYPE>
    original::doubleDirectionIterator<TYPE> & original::doubleDirectionIterator<TYPE>::operator=(
        const doubleDirectionIterator &other) {
        if (this == &other) {
            return *this;
        }
        this->stepIterator<TYPE>::operator=(other);
        return *this;
    }

    template<typename TYPE>
    bool original::doubleDirectionIterator<TYPE>::hasPrev() const {
        return this->isValid();
    }

    template<typename TYPE>
    void original::doubleDirectionIterator<TYPE>::prev() const {
        if (!this->isValid()) throw nullPointerError();
        this->_ptr = this->_ptr->getPPrev();
    }

    template<typename TYPE>
    auto original::doubleDirectionIterator<TYPE>::clone() const -> doubleDirectionIterator* {
        return new doubleDirectionIterator(*this);
    }

#endif //DOUBLEDIRECTIONITERATOR_H
