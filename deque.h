#ifndef DEQUE_H
#define DEQUE_H

#include "chain.h"
#include <string>

namespace original{
    template<typename TYPE, typename SERIAL = chain<TYPE>>
    class deque : public iterationStream<TYPE>{
        SERIAL serial_;
    public:
        explicit deque(const SERIAL& serial = SERIAL{});
        deque(const deque& other);
        deque& operator=(const deque& other);
        bool operator==(const deque& other) const;
        [[nodiscard]] uint32_t size() const;
        [[nodiscard]] bool empty() const;
        void clear();
        void pushBegin(const TYPE &e);
        void pushEnd(const TYPE &e);
        TYPE popBegin();
        TYPE popEnd();
        TYPE head() const;
        TYPE tail() const;
        iterator<TYPE>* begins() const override;
        iterator<TYPE>* ends() const override;
        [[nodiscard]] std::string className() const override;
    };
}

    template<typename TYPE, typename SERIAL>
    original::deque<TYPE, SERIAL>::deque(const SERIAL& serial) : serial_{serial} {}

    template<typename TYPE, typename SERIAL>
    original::deque<TYPE, SERIAL>::deque(const deque& other) : deque() {
        this->operator=(other);
    }

    template<typename TYPE, typename SERIAL>
    auto original::deque<TYPE, SERIAL>::operator=(const deque& other) -> deque& {
        if (this == &other) return *this;
        serial_ = other.serial_;
        return *this;
    }

    template<typename TYPE, typename SERIAL>
    auto original::deque<TYPE, SERIAL>::operator==(const deque &other) const -> bool {
        return serial_ == other.serial_;
    }

    template<typename TYPE, typename SERIAL>
    auto original::deque<TYPE, SERIAL>::size() const -> uint32_t {
        return serial_.size();
    }

    template<typename TYPE, typename SERIAL>
    auto original::deque<TYPE, SERIAL>::empty() const -> bool {
        return serial_.empty();
    }

    template<typename TYPE, typename SERIAL>
    auto original::deque<TYPE, SERIAL>::clear() -> void {
        serial_.clear();
    }

    template<typename TYPE, typename SERIAL>
    auto original::deque<TYPE, SERIAL>::pushBegin(const TYPE &e) -> void {
        serial_.pushBegin(e);
    }

    template<typename TYPE, typename SERIAL>
    auto original::deque<TYPE, SERIAL>::pushEnd(const TYPE &e) -> void {
        serial_.pushEnd(e);
    }

    template<typename TYPE, typename SERIAL>
    auto original::deque<TYPE, SERIAL>::popBegin() -> TYPE {
        return serial_.popBegin();
    }

    template<typename TYPE, typename SERIAL>
    auto original::deque<TYPE, SERIAL>::popEnd() -> TYPE {
        return serial_.popEnd();
    }

    template<typename TYPE, typename SERIAL>
    auto original::deque<TYPE, SERIAL>::head() const -> TYPE {
        return serial_.getBegin();
    }

    template<typename TYPE, typename SERIAL>
    auto original::deque<TYPE, SERIAL>::tail() const -> TYPE {
        return serial_.getEnd();
    }

    template<typename TYPE, typename SERIAL>
    auto original::deque<TYPE, SERIAL>::begins() const -> iterator<TYPE>* {
        return serial_.begins();
    }

    template<typename TYPE, typename SERIAL>
    auto original::deque<TYPE, SERIAL>::ends() const -> iterator<TYPE>* {
        return serial_.ends();
    }

    template <typename TYPE, typename SERIAL>
    std::string original::deque<TYPE, SERIAL>::className() const
    {
        return "deque";
    }

#endif //DEQUE_H
