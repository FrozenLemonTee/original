#ifndef PRIQUE_H
#define PRIQUE_H
#include <algorithms.h>
#include <comparator.h>
#include <vector.h>

namespace original
{
    template<typename TYPE,
    template <typename> typename SERIAL = vector,
    template <typename> typename Callback = increaseComparator>
    class prique : public printable
    {
        SERIAL<TYPE> serial_;
        Callback<TYPE> compare_;
        public:
            explicit prique(const SERIAL<TYPE>& serial = SERIAL<TYPE>{}, const Callback<TYPE>& compare = Callback<TYPE>{});
            prique(std::initializer_list<TYPE> lst, const Callback<TYPE>& compare = Callback<TYPE>{});
            prique(const prique& other);
            prique& operator=(const prique& other);
            bool operator==(const prique& other) const;
            [[nodiscard]] uint32_t size() const;
            [[nodiscard]] bool empty() const;
            void clear();
            void push(const TYPE& e);
            TYPE pop();
            TYPE top() const;
            [[nodiscard]] std::string className() const override;
            [[nodiscard]] std::string toString(bool enter) const override;
    };
}

    template <typename TYPE, template <typename> class SERIAL, template <typename> class Callback>
    original::prique<TYPE, SERIAL, Callback>::prique(const SERIAL<TYPE>& serial, const Callback<TYPE>& compare)
        : serial_(serial), compare_(compare)
    {
        algorithms::heapInit(this->serial_.first(), this->serial_.last(), this->compare_);
    }

    template <typename TYPE, template <typename> class SERIAL, template <typename> class Callback>
    original::prique<TYPE, SERIAL, Callback>::prique(std::initializer_list<TYPE> lst, const Callback<TYPE>& compare)
        : prique(SERIAL<TYPE>(lst), compare) {}

    template <typename TYPE, template <typename> class SERIAL, template <typename> class Callback>
    original::prique<TYPE, SERIAL, Callback>::prique(const prique& other)
        : serial_(other.serial_), compare_(other.compare_) {}

    template <typename TYPE, template <typename> class SERIAL, template <typename> class Callback>
    auto original::prique<TYPE, SERIAL, Callback>::operator=(const prique& other) -> prique&
    {
        if (this == &other) return *this;
        serial_ = other.serial_;
        compare_ = other.compare_;
        return *this;
    }

    template <typename TYPE, template <typename> class SERIAL, template <typename> class Callback>
    auto original::prique<TYPE, SERIAL, Callback>::operator==(const prique& other) const -> bool
    {
        return serial_ == other.serial_ && compare_ == other.compare_;
    }

    template <typename TYPE, template <typename> class SERIAL, template <typename> class Callback>
    auto original::prique<TYPE, SERIAL, Callback>::size() const -> uint32_t
    {
        return serial_.size();
    }

    template <typename TYPE, template <typename> class SERIAL, template <typename> class Callback>
    auto original::prique<TYPE, SERIAL, Callback>::empty() const -> bool
    {
        return serial_.empty();
    }

    template <typename TYPE, template <typename> class SERIAL, template <typename> class Callback>
    auto original::prique<TYPE, SERIAL, Callback>::clear() -> void
    {
        serial_.clear();
    }

    template <typename TYPE, template <typename> class SERIAL, template <typename> class Callback>
    auto original::prique<TYPE, SERIAL, Callback>::push(const TYPE& e) -> void
    {
        serial_.pushEnd(e);
        algorithms::heapAdjustUp(this->serial_.first(), this->serial_.last(), this->compare_);
    }

    template <typename TYPE, template <typename> class SERIAL, template <typename> class Callback>
    auto original::prique<TYPE, SERIAL, Callback>::pop() -> TYPE
    {
        if (this->empty()) throw noElementError();

        algorithms::swap(this->serial_.first(), this->serial_.last());
        TYPE res = serial_.popEnd();
        algorithms::heapAdjustDown(this->serial_.first(), this->serial_.last(), this->serial_.first(), compare_);
        return res;
    }

    template <typename TYPE, template <typename> class SERIAL, template <typename> class Callback>
    auto original::prique<TYPE, SERIAL, Callback>::top() const -> TYPE
    {
        return serial_.getBegin();
    }

    template <typename TYPE, template <typename> class SERIAL, template <typename> class Callback>
    auto original::prique<TYPE, SERIAL, Callback>::className() const -> std::string
    {
        return "prique";
    }

    template <typename TYPE, template <typename> class SERIAL, template <typename> class Callback>
    auto original::prique<TYPE, SERIAL, Callback>::toString(const bool enter) const -> std::string
    {
        std::stringstream ss;
        ss << this->className() << "(";
        bool first = true;
        for (const auto e : this->serial_)
        {
            if (!first) ss << ", ";
            ss << printable::formatString(e);
            first = false;
        }
        ss << ")";
        if (enter) ss << "\n";
        return ss.str();
    }


#endif //PRIQUE_H
