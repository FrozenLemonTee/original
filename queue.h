#ifndef QUEUE_H
#define QUEUE_H

#include "chain.h"
#include <string>

namespace original {
    template<typename TYPE, typename SERIAL = chain<TYPE>>
    class queue : public iterationStream<TYPE> {
        SERIAL serial_;
    public:
        explicit queue(const SERIAL& serial = SERIAL{});
        queue(const queue& other);
        queue& operator=(const queue& other);
        bool operator==(const queue& other) const;
        [[nodiscard]] uint32_t size() const;
        [[nodiscard]] bool empty() const;
        void clear();
        void push(const TYPE &e);
        TYPE pop();
        TYPE head() const;
        TYPE tail() const;
        iterator<TYPE>* begins() const override;
        iterator<TYPE>* ends() const override;
        [[nodiscard]] std::string className() const override;
    };
}

    template<typename TYPE, typename SERIAL>
    original::queue<TYPE, SERIAL>::queue(const SERIAL& serial) : serial_{serial} {}

    template<typename TYPE, typename SERIAL>
    original::queue<TYPE, SERIAL>::queue(const queue& other) : queue() {
        this->operator=(other);
    }

    template<typename TYPE, typename SERIAL>
    auto original::queue<TYPE, SERIAL>::operator=(const queue& other) -> queue& {
        if (this == &other) return *this;
        serial_ = other.serial_;
        return *this;
    }

    template<typename TYPE, typename SERIAL>
    auto original::queue<TYPE, SERIAL>::operator==(const queue &other) const -> bool {
        return serial_ == other.serial_;
    }

    template<typename TYPE, typename SERIAL>
    auto original::queue<TYPE, SERIAL>::size() const -> uint32_t {
        return serial_.size();
    }

    template<typename TYPE, typename SERIAL>
    auto original::queue<TYPE, SERIAL>::empty() const -> bool {
        return serial_.empty();
    }

    template<typename TYPE, typename SERIAL>
    auto original::queue<TYPE, SERIAL>::clear() -> void {
        serial_.clear();
    }

    template<typename TYPE, typename SERIAL>
    auto original::queue<TYPE, SERIAL>::push(const TYPE &e) -> void {
        serial_.pushEnd(e);
    }

    template<typename TYPE, typename SERIAL>
    auto original::queue<TYPE, SERIAL>::pop() -> TYPE {
        return serial_.popBegin();
    }

    template<typename TYPE, typename SERIAL>
    auto original::queue<TYPE, SERIAL>::head() const -> TYPE {
        return serial_.getBegin();
    }

    template<typename TYPE, typename SERIAL>
    auto original::queue<TYPE, SERIAL>::tail() const -> TYPE {
        return serial_.getEnd();
    }

    template<typename TYPE, typename SERIAL>
    auto original::queue<TYPE, SERIAL>::begins() const -> iterator<TYPE>* {
        return serial_.begins();
    }

    template<typename TYPE, typename SERIAL>
    auto original::queue<TYPE, SERIAL>::ends() const -> iterator<TYPE>* {
        return serial_.ends();
    }

    template <typename TYPE, typename SERIAL>
    std::string original::queue<TYPE, SERIAL>::className() const
    {
        return "queue";
    }

#endif //QUEUE_H
