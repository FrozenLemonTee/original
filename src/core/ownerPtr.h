#ifndef OWNERPTR_H
#define OWNERPTR_H

#include <utility>
#include "meta.h"
#include "autoPtr.h"
#include "deleter.h"

/**
* @file ownerPtr.h
* @brief Exclusive-ownership smart pointer implementation
* @details Provides move-only semantics for dynamic object ownership management.
* Enforces single ownership through deleted copy operations and supports custom deleters.
* Inherits reference counting infrastructure from autoPtr base class.
*
* Key Features:
* - Move-only semantics ensuring unique ownership
* - Customizable deletion policies
* - Type-safe pointer casting operations
* - Factory functions for safe object creation
* - RAII-compliant resource management
*/

namespace original {
    /**
    * @class ownerPtr
    * @tparam TYPE Managed object type
    * @tparam DELETER Deletion policy type (default: deleter<TYPE>)
    * @brief Unique ownership smart pointer with move semantics
    * @details Key features:
    * - Explicit ownership transfer through move operations
    * - Prohibited copy operations for ownership uniqueness
    * - RAII-compliant automatic resource management
    * - Factory functions for safe object creation
    * @extends autoPtr
    */
    template <typename TYPE, typename DELETER = deleter<TYPE>>
    class ownerPtr final : public autoPtr<TYPE, ownerPtr<TYPE, DELETER>, DELETER>, public moveOnlyMeta {
        template<typename, typename> friend class ownerPtr;
    public:
        /**
        * @brief Construct from raw pointer
        * @param p Pointer to take ownership of
        * @warning Transfers ownership responsibility to ownerPtr
        */
        explicit ownerPtr(TYPE* p = std::nullptr_t{});

        

        /**
        * @brief Move constructor
        * @param other Source ownerPtr to transfer ownership from
        * @post other becomes empty
        */
        ownerPtr(ownerPtr&& other) noexcept;

        /**
        * @brief Move assignment operator
        * @param other Source ownerPtr to transfer ownership from
        * @return Reference to this ownerPtr
        * @post other becomes empty
        */
        ownerPtr& operator=(ownerPtr&& other) noexcept;

        /**
        * @brief Performs static cast and transfers ownership
        * @tparam U Target type for static cast
        * @tparam DEL Rebound deleter type for target type
        * @return ownerPtr<U, DEL> with ownership transferred
        * @note Uses static_cast for type conversion
        */
        template<typename U, typename DEL = DELETER::template rebound_deleter<U>>
        ownerPtr<U, DEL> staticCastMoveTo();

        /**
        * @brief Performs dynamic cast and transfers ownership
        * @tparam U Target type for dynamic cast
        * @tparam DEL Rebound deleter type for target type
        * @return ownerPtr<U, DEL> with ownership transferred if cast succeeds, empty ownerPtr otherwise
        * @note Uses dynamic_cast for type conversion, returns empty pointer if cast fails
        */
        template<typename U, typename DEL = DELETER::template rebound_deleter<U>>
        ownerPtr<U, DEL> dynamicCastMoveTo();

        /**
        * @brief Performs const cast and transfers ownership
        * @tparam U Target type for const cast
        * @tparam DEL Rebound deleter type for target type
        * @return ownerPtr<U, DEL> with ownership transferred
        * @note Uses const_cast for type conversion
        */
        template<typename U, typename DEL = DELETER::template rebound_deleter<U>>
        ownerPtr<U, DEL> constCastMoveTo();

        /**
        * @brief Release ownership of managed object
        * @return Raw pointer to released resource
        * @post ownerPtr no longer manages the resource
        * @warning Caller assumes ownership responsibility
        */
        TYPE* unlock();

        /**
        * @brief Get class name identifier
        * @return Class name of ownerPtr
        */
        [[nodiscard]] std::string className() const override;

        /**
        * @brief Destructor releases owned resources
        */
        ~ownerPtr() override;

        // Factory friend functions
        template <typename T, typename DEL, typename... Args>
        friend ownerPtr<T, DEL> makeOwnerPtr(Args&&... args);

        template <typename T, typename DEL, typename... Args>
        friend ownerPtr<T, DEL> makeOwnerPtrArray(u_integer size, Args&&... args);
    };

    /**
    * @brief Creates a new ownerPtr managing a dynamically allocated object
    * @tparam T Type of object to create and manage
    * @tparam DEL Deleter policy type (default: deleter<T>)
    * @tparam Args Argument types for object construction
    * @param args Arguments to forward to T's constructor
    * @return ownerPtr<T, DEL> owning the newly created object
    * @note Provides exception-safe object creation
    * @warning The returned ownerPtr has exclusive ownership of the object
    *
    * @code
    * // Create an ownerPtr managing a new MyClass constructed with args
    * auto ptr = makeOwnerPtr<MyClass>(arg1, arg2);
    * @endcode
    */
    template <typename T, typename DEL = deleter<T>, typename... Args>
    ownerPtr<T, DEL> makeOwnerPtr(Args&&... args);

    /**
    * @brief Creates a new ownerPtr managing a dynamically allocated array
    * @tparam T Type of array elements to create
    * @tparam DEL Deleter policy type (default: deleter<T[]>)
    * @tparam Args Argument types for array element initialization
    * @param size Number of elements in the array
    * @param args Arguments to forward to each element's constructor
    * @return ownerPtr<T, DEL> owning the newly created array
    * @note Provides exception-safe array creation
    * @warning The returned ownerPtr has exclusive ownership of the array
    *
    * @code
    * // Create an ownerPtr managing a new MyClass[10] array
    * auto arr = makeOwnerPtrArray<MyClass>(10);
    * @endcode
    */
    template <typename T, typename DEL = deleter<T[]>, typename... Args>
    ownerPtr<T, DEL> makeOwnerPtrArray(u_integer size, Args&&... args);


    // ----------------- Definitions of ownerPtr.h -----------------


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
        this->ref_count = *other.ref_count;
        other.ref_count = autoPtr<TYPE, ownerPtr, DELETER>::newRefCount();
        other.addStrongRef();
        this->alias_ptr = other.alias_ptr;
        other.alias_ptr = nullptr;
        return *this;
    }

    template <typename TYPE, typename DELETER>
    template<typename U, typename DEL>
    ownerPtr<U, DEL>
    ownerPtr<TYPE, DELETER>::staticCastMoveTo() {
        return ownerPtr<U, DEL>{
            static_cast<U*>(this->unlock())
        };
    }

    template <typename TYPE, typename DELETER>
    template<typename U, typename DEL>
    ownerPtr<U, DEL>
    ownerPtr<TYPE, DELETER>::dynamicCastMoveTo() {
        auto p = dynamic_cast<U*>(this->get());
        if (p == nullptr) {
            return ownerPtr<U, DEL>{};
        }
        this->unlock();
        return ownerPtr<U, DEL>{p};
    }

    template <typename TYPE, typename DELETER>
    template<typename U, typename DEL>
    ownerPtr<U, DEL>
    ownerPtr<TYPE, DELETER>::constCastMoveTo() {
        return ownerPtr<U, DEL>{
            const_cast<U*>(this->unlock())
        };
    }

    template<typename TYPE, typename DELETER>
    TYPE* ownerPtr<TYPE, DELETER>::unlock() {
        return this->releasePtr();
    }

    template<typename TYPE, typename DELETER>
    std::string ownerPtr<TYPE, DELETER>::className() const {
        return "ownerPtr";
    }

    template<typename TYPE, typename DELETER>
    ownerPtr<TYPE, DELETER>::~ownerPtr() {
        this->removeStrongRef();
    }

    template<typename T, typename DEL, typename ... Args>
    ownerPtr<T, DEL> makeOwnerPtr(Args &&...args) {
        return ownerPtr<T, DEL>(new T{std::forward<Args>(args)...});
    }

    template<typename T, typename DEL, typename ... Args>
    ownerPtr<T, DEL> makeOwnerPtrArray(const u_integer size, Args&& ...args) {
        auto owner_ptr = ownerPtr<T, DEL>(new T[size]);
        for (u_integer i = 0; i < size; i++)
        {
            owner_ptr[i] = T(std::forward<Args>(args)...);
        }
        return owner_ptr;
    }
}

#endif //OWNERPTR_H
