#ifndef OWNERPTR_H
#define OWNERPTR_H

#include <utility>
#include "autoPtr.h"
#include "deleter.h"


namespace original {
    template <typename TYPE, typename DELETER = deleter<TYPE>>
    class ownerPtr final : public autoPtr<TYPE, ownerPtr<TYPE, DELETER>, DELETER>{
    public:
        explicit ownerPtr(TYPE* p = nullptr);

        ownerPtr(const ownerPtr& other) = delete;

        ownerPtr& operator=(const ownerPtr& other) = delete;

        ownerPtr(ownerPtr&& other) noexcept;

        ownerPtr& operator=(ownerPtr&& other) noexcept;

        TYPE* unlock();

        [[nodiscard]] std::string className() const override;

        ~ownerPtr() override;

        template <typename T, typename DEL>
        friend ownerPtr<T, DEL> makeOwnerPtr();
    };

    template <typename TYPE, typename DELETER>
    class ownerPtr<TYPE[], DELETER> final : public autoPtr<TYPE[], ownerPtr<TYPE[], DELETER>, DELETER> {
    public:
        explicit ownerPtr(TYPE* p = nullptr);

        ownerPtr(const ownerPtr& other) = delete;

        ownerPtr& operator=(const ownerPtr& other) = delete;

        ownerPtr(ownerPtr&& other) noexcept;

        ownerPtr& operator=(ownerPtr&& other) noexcept;

        TYPE* unlock();

        [[nodiscard]] std::string className() const override;

        ~ownerPtr() override;

        template <typename T, typename DEL>
        friend ownerPtr<T[], DEL> makeOwnerPtr(u_integer size);
    };

    template <typename T, typename DEL = deleter<T>>
    ownerPtr<T, DEL> makeOwnerPtr();

    template <typename T, typename DEL = deleter<T[]>>
    ownerPtr<T[], DEL> makeOwnerPtr(u_integer size);


    template<typename TYPE, typename DELETER>
    ownerPtr<TYPE, DELETER>::ownerPtr(TYPE *p)
        : autoPtr<TYPE, ownerPtr, DELETER>(p) {
        this->addStrongRef();
    }

    template<typename TYPE, typename DELETER>
    ownerPtr<TYPE, DELETER>::ownerPtr(ownerPtr &&other) noexcept : ownerPtr(){
        this->operator=(std::move(other));
    }

    template<typename TYPE, typename DELETER>
    ownerPtr<TYPE, DELETER>& ownerPtr<TYPE, DELETER>::operator=(ownerPtr&& other) noexcept {
        if (this == &other)
            return *this;

        this->removeStrongRef();
        this->clean();
        this->ref_count = other.ref_count;
        other.ref_count = autoPtr<TYPE, ownerPtr, DELETER>::newRefCount();
        other.addStrongRef();
        return *this;
    }

    template<typename TYPE, typename DELETER>
    TYPE* ownerPtr<TYPE, DELETER>::unlock() {
        TYPE* p = this->getPtr();
        this->setPtr(nullptr);
        return p;
    }

    template<typename TYPE, typename DELETER>
    std::string ownerPtr<TYPE, DELETER>::className() const {
        return "ownerPtr";
    }

    template<typename TYPE, typename DELETER>
    ownerPtr<TYPE, DELETER>::~ownerPtr() {
        this->removeStrongRef();
    }

    template<typename TYPE, typename DELETER>
    ownerPtr<TYPE[], DELETER>::ownerPtr(TYPE* p)
        : autoPtr<TYPE[], ownerPtr, DELETER>(p) {
        this->addStrongRef();
    }

    template<typename TYPE, typename DELETER>
    ownerPtr<TYPE[], DELETER>::ownerPtr(ownerPtr &&other) noexcept : ownerPtr() {
        this->operator=(std::move(other));
    }

    template<typename TYPE, typename DELETER>
    ownerPtr<TYPE[], DELETER>& ownerPtr<TYPE[], DELETER>::operator=(ownerPtr &&other) noexcept {
        if (this == &other)
            return *this;

        this->removeStrongRef();
        this->clean();
        this->ref_count = other.ref_count;
        other.ref_count = autoPtr<TYPE[], ownerPtr, DELETER>::newRefCount();
        other.addStrongRef();
        return *this;
    }

    template<typename TYPE, typename DELETER>
    TYPE* ownerPtr<TYPE[], DELETER>::unlock() {
        TYPE* p = this->getPtr();
        this->setPtr(nullptr);
        return p;
    }

    template<typename TYPE, typename DELETER>
    std::string ownerPtr<TYPE[], DELETER>::className() const {
        return "ownerPtr";
    }

    template<typename TYPE, typename DELETER>
    ownerPtr<TYPE[], DELETER>::~ownerPtr() {
        this->removeStrongRef();
    }

    template<typename T, typename DEL>
    ownerPtr<T, DEL> makeOwnerPtr() {
        return ownerPtr<T, DEL>(new T{});
    }

    template<typename T, typename DEL>
    ownerPtr<T[], DEL> makeOwnerPtr(const u_integer size) {
        return ownerPtr<T[], DEL>(new T[size]);
    }
}

#endif //OWNERPTR_H
