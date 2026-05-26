#ifndef AUTOPTR_H
#define AUTOPTR_H

#include "atomic.h"
#include "config.h"
#include "printable.h"
#include "comparable.h"
#include "hash.h"
#include "error.h"

/**
* @file autoPtr.h
* @brief Base class for reference-counted smart pointers
* @details Implements core functionality for automatic memory management
* using reference counting. Supports strong/weak references and custom deleters.
* Inherits printable and comparable interfaces for debugging and comparison.
*
* Key Features:
* - Automatic memory management via reference counting
* - Support for both strong and weak references
* - Customizable deletion policies via template parameter
* - Thread-safe reference counting operations
* - Integration with printable, comparable and hashable interfaces
*
* @note This is a base class meant to be inherited by concrete smart pointer implementations
* @see sharedPtr For a complete implementation using this base
* @see weakPtr For the weak reference counterpart
*/

namespace original
{
    // Forward declaration for refCountBase
    class refCountBase;

    // Forward declaration for refCount
    template <typename TYPE, typename DELETER>
    class refCount;

    /**
    * @class autoPtr
    * @tparam TYPE Managed object type
    * @tparam DERIVED CRTP pattern parameter for inheritance
    * @tparam DELETER Custom deleter policy type
    * @brief Base smart pointer with reference counting
    * @details Provides core resource management capabilities through:
    * - Strong/weak reference tracking
    * - Automatic cleanup when references reach zero
    * - Custom deletion policies via template parameter
    * - Exception-safe operations
    *
    * Memory Management Strategy:
    * - Each managed object has an associated refCount object
    * - Strong references control object lifetime
    * - Weak references allow observation without ownership
    * - Object destroyed when strong references reach zero
    * - refCount destroyed when both strong and weak references reach zero
    *
    * Thread Safety:
    * - Reference counting operations are atomic and thread-safe
    * - Managed object access requires external synchronization
    *
    * @extends printable For string representation capabilities
    * @extends comparable For comparison operations
    * @extends hashable For hashing support
    */
    template <typename TYPE, typename DERIVED, typename DELETER>
    class autoPtr : public printable,
                    public comparable<autoPtr<TYPE, DERIVED, DELETER>>,
                    public hashable<autoPtr<TYPE, DERIVED, DELETER>>
    {
        template <typename, typename, typename>
        friend class autoPtr;

    protected:
        atomic<refCountBase*> ref_count; ///< Reference counter object
        TYPE* alias_ptr; ///< Aliased pointer for type casting scenarios

        /**
        * @brief Construct from raw pointer
        * @param p Raw pointer to manage
        * @warning Ownership is transferred to autoPtr
        * @post ref_count points to new refCount object managing p
        */
        explicit autoPtr(TYPE* p);

        /**
        * @brief Increment strong reference count
        * @internal Reference set method
        * @note Thread-safe atomic operation
        */
        void addStrongRef() const;

        /**
        * @brief Increment weak reference count
        * @internal Reference set method
        * @note Thread-safe atomic operation
        */
        void addWeakRef() const;

        /**
        * @brief Decrement strong reference count
        * @internal Reference set method
        * @note Thread-safe atomic operation
        * @post May trigger object destruction if count reaches zero
        */
        void removeStrongRef() noexcept;

        /**
        * @brief Decrement weak reference count
        * @internal Reference set method
        * @note Thread-safe atomic operation
        * @post May trigger refCount destruction if both counts reach zero
        */
        void removeWeakRef() noexcept;

        /**
        * @brief Try to increment strong reference count only while object is alive
        * @return True if a strong reference was acquired
        */
        bool tryAddStrongRef() const noexcept;

        /**
        * @brief Release one control-block reference
        * @param current Reference counter object
        * @post Deletes the counter if this was the final strong/weak reference
        */
        static void releaseControlRef(const refCountBase* current) noexcept;

        /**
        * @brief Release ownership of the managed pointer
        * @return Raw pointer to the managed object
        * @note The caller becomes responsible for deleting the returned pointer
        * @post Reference counter retains weak references but no strong references
        * @warning After release, the autoPtr becomes empty and should not be used
        */
        TYPE* releasePtr() noexcept;

        /**
        * @brief Destroy reference counter
        * @note Only destroys counter when both ref counts reach zero
        * @internal Called during cleanup operations
        */
        void destroyRefCnt() noexcept;

        /**
        * @brief Detach this pointer from its reference counter
        * @internal Resource management method
        * @details Ownership release is handled by removeStrongRef()/removeWeakRef().
        */
        void clean() noexcept;

        /**
        * @brief Create new reference counter
        * @param p Pointer to manage (nullptr allowed)
        * @return Newly created counter object
        * @throws std::bad_alloc if memory allocation fails
        */
        static refCount<TYPE, DELETER>* newRefCount(TYPE* p = nullptr);

    public:
        /**
        * @brief Get strong reference count
        * @return Current number of strong references
        * @note Returns 0 if no reference counter exists
        */
        u_integer strongRefs() const;

        /**
        * @brief Get weak reference count
        * @return Current number of weak references
        * @note Returns 0 if no reference counter exists
        */
        u_integer weakRefs() const;

        /**
        * @brief Check active ownership
        * @return True if it has active references (strong or weak)
        * @note Different from operator bool() which checks for valid object
        */
        bool exist() const;

        /**
        * @brief Check resource validity
        * @return True if strong references == 0
        * @note Indicates the managed object has been destroyed
        */
        bool expired() const;

        /**
        * @brief Boolean conversion operator
        * @return True if has strong reference and managed pointer is non-null
        * @note Considers both reference count and pointer validity
        */
        explicit operator bool() const;

        /**
        * @brief Logical NOT operator
        * @return Opposite of operator bool()
        */
        bool operator!() const;

        /**
        * @brief Get managed pointer const version
        * @return Raw pointer to managed object
        * @throws nullPointerError if no active references
        * @note Returns alias_ptr if set, otherwise the actual managed pointer
        */
        const TYPE* get() const;

        /**
        * @brief Get managed pointer
        * @return Raw pointer to managed object
        * @throws nullPointerError if no active references
        * @note Returns alias_ptr if set, otherwise the actual managed pointer
        */
        TYPE* get();

        /**
        * @brief Const dereference operator
        * @return Reference to managed object
        * @throws nullPointerError if no managed object
        */
        virtual const TYPE& operator*() const;

        /**
        * @brief Const member access operator
        * @return Pointer to managed object
        * @throws nullPointerError if no managed object
        */
        virtual const TYPE* operator->() const;

        /**
        * @brief Const array access operator
        * @param index Array index
        * @return Reference to element
        * @throws nullPointerError if no managed array
        * @throws outOfBoundError if index is invalid
        */
        virtual const TYPE& operator[](u_integer index) const;

        // Mutable accessors
        /**
        * @brief Mutable dereference operator
        * @return Reference to managed object
        * @throws nullPointerError if no managed object
        */
        virtual TYPE& operator*();

        /**
        * @brief Mutable member access operator
        * @return Pointer to managed object
        * @throws nullPointerError if no managed object
        */
        virtual TYPE* operator->();

        /**
        * @brief Mutable array access operator
        * @param index Array index
        * @return Reference to element
        * @throws nullPointerError if no managed array
        * @throws outOfBoundError if index is invalid
        */
        virtual TYPE& operator[](u_integer index);

        /**
        * @brief Swaps the reference counters between two autoPtr instances
        * @param other Another autoPtr instance to swap with
        * @post After swapping:
        * - Current object acquires other's reference counter
        * - Other object acquires current object's reference counter
        * - Managed object ownership transfers implicitly
        * - alias_ptr values are also swapped
        * @warning This operation is not thread-safe. Do not call swap concurrently
        *          from multiple threads on the same autoPtr instances, as it may
        *          lead to data races and undefined behavior.
        */
        void swap(autoPtr& other) noexcept;

        /**
        * @brief Compare reference counters
        * @param other autoPtr to compare with
        * @return Difference between managed pointer addresses
        * @note Comparison is based on pointer values, not object contents
        */
        integer compareTo(const autoPtr& other) const override;

        /**
        * @brief Get class name string
        * @return Name of autoPtr
        */
        std::string className() const override;

        /**
        * @brief String representation formatter
        * @param enter Add newline if true
        * @return Formatted resource info including pointer address and reference counts
        */
        std::string toString(bool enter) const override;

        /**
        * @brief Compute hash value for the pointer
        * @return Hash of managed pointer address
        * @note Uses hash<TYPE> specialization
        * @warning Hash is based on pointer address, not object contents
        */
        [[nodiscard]] u_integer toHash() const noexcept override;

        /**
        * @brief Equality comparison
        * @param other autoPtr to compare with
        * @return True if both manage same pointer
        * @note Compares pointer addresses, not values
        */
        bool equals(const autoPtr& other) const noexcept override;

        /**
        * @brief Base destructor detaches after derived classes release their reference
        */
        ~autoPtr() override;

        template <typename T, typename DER, typename DEL>
        friend bool operator==(const autoPtr<T, DER, DEL>& ptr, const std::nullptr_t& null);

        template <typename T, typename DER, typename DEL>
        friend bool operator!=(const autoPtr<T, DER, DEL>& ptr, const std::nullptr_t& null);

        template <typename T, typename DER, typename DEL>
        friend bool operator==(const std::nullptr_t& null, const autoPtr<T, DER, DEL>& ptr);

        template <typename T, typename DER, typename DEL>
        friend bool operator!=(const std::nullptr_t& null, const autoPtr<T, DER, DEL>& ptr);
    };

    /**
    * @brief Equality comparison with nullptr
    * @tparam T Managed object type
    * @tparam DER CRTP derived class type
    * @tparam DEL Deleter policy type
    * @param ptr autoPtr to compare
    * @param null nullptr_t to compare against
    * @return true if the autoPtr is empty (no managed object)
    * @note Equivalent to checking !operator bool() of ptr
    */
    template <typename T, typename DER, typename DEL>
    bool operator==(const autoPtr<T, DER, DEL>& ptr, const std::nullptr_t& null);

    /**
    * @brief Inequality comparison with nullptr
    * @tparam T Managed object type
    * @tparam DER CRTP derived class type
    * @tparam DEL Deleter policy type
    * @param ptr autoPtr to compare
    * @param null nullptr_t to compare against
    * @return true if the autoPtr is not empty (has a managed object)
    * @note Equivalent to checking operator bool() of ptr
    */
    template <typename T, typename DER, typename DEL>
    bool operator!=(const autoPtr<T, DER, DEL>& ptr, const std::nullptr_t& null);

    /**
    * @brief Equality comparison with nullptr (reversed operands)
    * @tparam T Managed object type
    * @tparam DER CRTP derived class type
    * @tparam DEL Deleter policy type
    * @param ptr autoPtr to compare
    * @param null nullptr_t to compare against
    * @return true if the autoPtr is empty (no managed object)
    * @note Equivalent to checking !operator bool() of ptr
    */
    template <typename T, typename DER, typename DEL>
    bool operator==(const std::nullptr_t& null, const autoPtr<T, DER, DEL>& ptr);

    /**
    * @brief Inequality comparison with nullptr (reversed operands)
    * @tparam T Managed object type
    * @tparam DER CRTP derived class type
    * @tparam DEL Deleter policy type
    * @param ptr autoPtr to compare
    * @param null nullptr_t to compare against
    * @return true if the autoPtr is not empty (has a managed object)
    * @note Equivalent to checking operator bool() of ptr
    */
    template <typename T, typename DER, typename DEL>
    bool operator!=(const std::nullptr_t& null, const autoPtr<T, DER, DEL>& ptr);

    /**
    * @class refCountBase
    * @brief Base class for reference counting metadata
    * @details Stores reference counts and provides interface for pointer management.
    *          This is an abstract base class that defines the reference counting interface.
    */
    class refCountBase
    {
        template <typename, typename, typename>
        friend class autoPtr;

    protected:
        mutable atomic<u_integer> strong_refs; ///< Strong reference counter
        mutable atomic<u_integer> weak_refs; ///< Weak reference counter
        mutable atomic<u_integer> total_refs; ///< Strong + weak pointer instances

        /**
        * @brief Construct refCountBase object
        * @post Initializes both reference counts to zero
        */
        refCountBase();

        /**
        * @brief Get managed pointer (const version)
        * @return Const pointer to managed object
        * @note Pure virtual function to be implemented by derived classes
        */
        virtual const void* getPtr() const noexcept = 0;

        /**
        * @brief Get managed pointer
        * @return Pointer to managed object
        * @note Pure virtual function to be implemented by derived classes
        */
        virtual void* getPtr() noexcept = 0;

        /**
        * @brief Release ownership of the managed pointer
        * @return Raw pointer to the managed object
        * @note The caller becomes responsible for deleting the returned pointer
        * @note Pure virtual function to be implemented by derived classes
        */
        virtual void* releasePtr() noexcept = 0;

        /**
        * @brief Destroy managed pointer using deleter
        * @note Pure virtual function to be implemented by derived classes
        */
        virtual void destroyPtr() noexcept = 0;

        /**
        * @brief Virtual destructor for proper cleanup
        */
        virtual ~refCountBase() = default;
    };

    /**
    * @class refCount
    * @tparam TYPE Managed object type
    * @tparam DELETER Custom deleter policy type
    * @brief Reference counting metadata container
    * @details Stores:
    * - Managed pointer
    * - Strong/weak reference counts
    * - Deleter policy instance
    *
    * Lifecycle Management:
    * - Created when first autoPtr takes ownership of an object
    * - Destroyed when both strong and weak reference counts reach zero
    * - Manages object destruction through the provided deleter
    */
    template <typename TYPE, typename DELETER>
    class refCount final : public refCountBase
    {
        template <typename, typename, typename>
        friend class autoPtr;

        TYPE* ptr; ///< Managed raw pointer
        DELETER deleter; ///< Deleter policy instance

        /**
        * @brief Construct refCount object
        * @param p Pointer to manage (nullptr allowed)
        * @post Initializes reference counts and stores the pointer
        */
        explicit refCount(TYPE* p = nullptr);

        /**
        * @brief Get managed pointer (const version)
        * @return Const pointer to managed object
        */
        const void* getPtr() const noexcept override;

        /**
        * @brief Get managed pointer
        * @return Pointer to managed object
        * @note Handles const-correctness for the managed pointer
        */
        void* getPtr() noexcept override;

        /**
        * @brief Release ownership of the managed pointer
        * @return Raw pointer to the managed object
        * @post Sets internal pointer to nullptr
        * @note Caller becomes responsible for the returned pointer
        */
        void* releasePtr() noexcept override;

        /**
        * @brief Destroy managed pointer using deleter
        * @post Sets internal pointer to nullptr after destruction
        * @note Uses the provided deleter policy for destruction
        */
        void destroyPtr() noexcept override;

        /**
        * @brief Destructor ensures resource cleanup
        * @note Calls destroyPtr() to ensure proper cleanup
        */
        ~refCount() override;
    };
}

namespace std
{
    /**
    * @brief Specialization of std::swap for autoPtr
    * @tparam TYPE Managed object type
    * @tparam DERIVED CRTP derived class type
    * @tparam DELETER Deleter policy type
    * @param lhs First autoPtr to swap
    * @param rhs Second autoPtr to swap
    * @post Both autoPtr instances exchange their managed resources
    * @warning This operation is not thread-safe. Do not call std::swap concurrently
    *          from multiple threads on the same autoPtr instances.
    */
    template <typename TYPE, typename DERIVED, typename DELETER>
    void swap(original::autoPtr<TYPE, DERIVED, DELETER>& lhs, // NOLINT
              original::autoPtr<TYPE, DERIVED, DELETER>& rhs) noexcept;
}

template <typename TYPE, typename DERIVED, typename DELETER>
original::autoPtr<TYPE, DERIVED, DELETER>::autoPtr(TYPE* p)
    : ref_count(makeAtomic<refCountBase*>(newRefCount(p))), alias_ptr(nullptr)
{
}

template <typename TYPE, typename DERIVED, typename DELETER>
void original::autoPtr<TYPE, DERIVED, DELETER>::addStrongRef() const
{
    if (const refCountBase* current = *this->ref_count)
    {
        current->strong_refs += 1;
        current->total_refs += 1;
    }
}

template <typename TYPE, typename DERIVED, typename DELETER>
void original::autoPtr<TYPE, DERIVED, DELETER>::addWeakRef() const
{
    if (const refCountBase* current = *this->ref_count)
    {
        current->weak_refs += 1;
        current->total_refs += 1;
    }
}

template <typename TYPE, typename DERIVED, typename DELETER>
void original::autoPtr<TYPE, DERIVED, DELETER>::removeStrongRef() noexcept
{
    refCountBase* current = *this->ref_count;
    if (!current)
    {
        return;
    }

    u_integer refs = *current->strong_refs;
    while (refs != 0)
    {
        if (const u_integer desired = refs - 1; current->strong_refs.exchangeCmp(refs, desired))
        {
            if (desired == 0)
            {
                current->destroyPtr();
            }
            this->ref_count = nullptr;
            this->alias_ptr = nullptr;
            releaseControlRef(current);
            return;
        }
    }

    this->ref_count = nullptr;
    this->alias_ptr = nullptr;
}

template <typename TYPE, typename DERIVED, typename DELETER>
void original::autoPtr<TYPE, DERIVED, DELETER>::removeWeakRef() noexcept
{
    const refCountBase* current = *this->ref_count;
    if (!current)
    {
        return;
    }

    u_integer refs = *current->weak_refs;
    while (refs != 0)
    {
        if (const u_integer desired = refs - 1; current->weak_refs.exchangeCmp(refs, desired))
        {
            this->ref_count = nullptr;
            this->alias_ptr = nullptr;
            releaseControlRef(current);
            return;
        }
    }

    this->ref_count = nullptr;
    this->alias_ptr = nullptr;
}

template <typename TYPE, typename DERIVED, typename DELETER>
bool original::autoPtr<TYPE, DERIVED, DELETER>::tryAddStrongRef() const noexcept
{
    const refCountBase* current = *this->ref_count;
    if (!current)
    {
        return false;
    }

    u_integer refs = *current->strong_refs;
    while (refs != 0)
    {
        if (const u_integer desired = refs + 1; current->strong_refs.exchangeCmp(refs, desired))
        {
            current->total_refs += 1;
            return true;
        }
    }
    return false;
}

template <typename TYPE, typename DERIVED, typename DELETER>
void original::autoPtr<TYPE, DERIVED, DELETER>::releaseControlRef(const refCountBase* current) noexcept
{
    if (!current)
    {
        return;
    }

    u_integer refs = *current->total_refs;
    while (refs != 0)
    {
        if (const u_integer desired = refs - 1; current->total_refs.exchangeCmp(refs, desired))
        {
            if (desired == 0)
            {
                delete current;
            }
            return;
        }
    }
}

template <typename TYPE, typename DERIVED, typename DELETER>
TYPE* original::autoPtr<TYPE, DERIVED, DELETER>::releasePtr() noexcept
{
    refCountBase* current = *this->ref_count;
    if (!current) return nullptr;
    return static_cast<TYPE*>(current->releasePtr());
}

template <typename TYPE, typename DERIVED, typename DELETER>
void original::autoPtr<TYPE, DERIVED, DELETER>::destroyRefCnt() noexcept
{
    const refCountBase* current = *this->ref_count;
    if (!current) return;
    this->ref_count = nullptr;
    delete current;
}

template <typename TYPE, typename DERIVED, typename DELETER>
void original::autoPtr<TYPE, DERIVED, DELETER>::clean() noexcept
{
    this->ref_count = nullptr;
    this->alias_ptr = nullptr;
}

template <typename TYPE, typename DERIVED, typename DELETER>
original::refCount<TYPE, DELETER>* original::autoPtr<TYPE, DERIVED, DELETER>::newRefCount(TYPE* p)
{
    return new refCount<TYPE, DELETER>(p);
}

template <typename TYPE, typename DERIVED, typename DELETER>
original::u_integer original::autoPtr<TYPE, DERIVED, DELETER>::strongRefs() const
{
    const refCountBase* current = *this->ref_count;
    if (!current) return 0;
    return *current->strong_refs;
}

template <typename TYPE, typename DERIVED, typename DELETER>
original::u_integer original::autoPtr<TYPE, DERIVED, DELETER>::weakRefs() const
{
    const refCountBase* current = *this->ref_count;
    if (!current) return 0;
    return *current->weak_refs;
}

template <typename TYPE, typename DERIVED, typename DELETER>
bool original::autoPtr<TYPE, DERIVED, DELETER>::exist() const
{
    const refCountBase* current = *this->ref_count;
    if (!current) return false;
    return *current->strong_refs > 0 || *current->weak_refs > 0;
}

template <typename TYPE, typename DERIVED, typename DELETER>
bool original::autoPtr<TYPE, DERIVED, DELETER>::expired() const
{
    const refCountBase* current = *this->ref_count;
    if (!current) return true;
    return *current->strong_refs == 0;
}

template <typename TYPE, typename DERIVED, typename DELETER>
original::autoPtr<TYPE, DERIVED, DELETER>::operator bool() const
{
    refCountBase* current = *this->ref_count;
    if (!current) return false;
    if (*current->strong_refs == 0) return false;
    const void* p = current->getPtr();
    return p != nullptr || this->alias_ptr != nullptr;
}

template <typename TYPE, typename DERIVED, typename DELETER>
bool original::autoPtr<TYPE, DERIVED, DELETER>::operator!() const
{
    return !this->operator bool();
}

template <typename TYPE, typename DERIVED, typename DELETER>
const TYPE* original::autoPtr<TYPE, DERIVED, DELETER>::get() const
{
    if (!this->exist())
    {
        throw nullPointerError();
    }
    if (this->alias_ptr)
    {
        return this->alias_ptr;
    }
    refCountBase* current = *this->ref_count;
    return static_cast<TYPE*>(current->getPtr());
}

template <typename TYPE, typename DERIVED, typename DELETER>
TYPE* original::autoPtr<TYPE, DERIVED, DELETER>::get()
{
    if (!this->exist())
    {
        throw nullPointerError();
    }
    if (this->alias_ptr)
    {
        return this->alias_ptr;
    }
    refCountBase* current = *this->ref_count;
    return static_cast<TYPE*>(current->getPtr());
}

template <typename TYPE, typename DERIVED, typename DELETER>
const TYPE& original::autoPtr<TYPE, DERIVED, DELETER>::operator*() const
{
    const auto ptr = this->get();
    if (!ptr)
        throw nullPointerError();
    return *ptr;
}

template <typename TYPE, typename DERIVED, typename DELETER>
const TYPE*
original::autoPtr<TYPE, DERIVED, DELETER>::operator->() const
{
    const auto ptr = this->get();
    if (!ptr)
        throw nullPointerError();
    return ptr;
}

template <typename TYPE, typename DERIVED, typename DELETER>
const TYPE& original::autoPtr<TYPE, DERIVED, DELETER>::operator[](u_integer index) const
{
    const auto ptr = this->get();
    if (!ptr)
        throw nullPointerError();
    return ptr[index];
}

template <typename TYPE, typename DERIVED, typename DELETER>
TYPE& original::autoPtr<TYPE, DERIVED, DELETER>::operator*()
{
    auto ptr = this->get();
    if (!ptr)
        throw nullPointerError();
    return *ptr;
}

template <typename TYPE, typename DERIVED, typename DELETER>
TYPE*
original::autoPtr<TYPE, DERIVED, DELETER>::operator->()
{
    auto ptr = this->get();
    if (!ptr)
        throw nullPointerError();
    return ptr;
}

template <typename TYPE, typename DERIVED, typename DELETER>
TYPE& original::autoPtr<TYPE, DERIVED, DELETER>::operator[](u_integer index)
{
    auto ptr = this->get();
    if (!ptr)
        throw nullPointerError();
    return ptr[index];
}

template <typename TYPE, typename DERIVED, typename DELETER>
void original::autoPtr<TYPE, DERIVED, DELETER>::swap(autoPtr& other) noexcept
{
    if (this == &other)
        return;

    refCountBase* a = this->ref_count.exchange(nullptr);
    refCountBase* b = other.ref_count.exchange(a);
    std::swap(this->alias_ptr, other.alias_ptr);
    this->ref_count = b;
}

template <typename TYPE, typename DERIVED, typename DELETER>
original::integer original::autoPtr<TYPE, DERIVED, DELETER>::compareTo(const autoPtr& other) const
{
    return this->get() - other.get();
}

template <typename TYPE, typename DERIVED, typename DELETER>
std::string original::autoPtr<TYPE, DERIVED, DELETER>::className() const
{
    return "autoPtr";
}

template <typename TYPE, typename DERIVED, typename DELETER>
std::string original::autoPtr<TYPE, DERIVED, DELETER>::toString(const bool enter) const
{
    std::stringstream ss;
    ss << this->className() << "(";
    ss << formatString(this->get());
    ss << ")";
    if (enter)
        ss << "\n";
    return ss.str();
}

template <typename TYPE, typename DERIVED, typename DELETER>
original::u_integer original::autoPtr<TYPE, DERIVED, DELETER>::toHash() const noexcept
{
    return hash<TYPE>::hashFunc(this->get());
}

template <typename TYPE, typename DERIVED, typename DELETER>
bool original::autoPtr<TYPE, DERIVED, DELETER>::equals(const autoPtr& other) const noexcept
{
    return *this == other;
}

template <typename TYPE, typename DERIVED, typename DELETER>
original::autoPtr<TYPE, DERIVED, DELETER>::~autoPtr()
{
    this->ref_count = nullptr;
    this->alias_ptr = nullptr;
}

template <typename T, typename DER, typename DEL>
bool original::operator==(const autoPtr<T, DER, DEL>& ptr, const std::nullptr_t&)
{
    return !ptr.operator bool();
}

template <typename T, typename DER, typename DEL>
bool original::operator!=(const autoPtr<T, DER, DEL>& ptr, const std::nullptr_t&)
{
    return ptr.operator bool();
}

template <typename T, typename DER, typename DEL>
bool original::operator==(const std::nullptr_t&, const autoPtr<T, DER, DEL>& ptr)
{
    return !ptr.operator bool();
}

template <typename T, typename DER, typename DEL>
bool original::operator!=(const std::nullptr_t&, const autoPtr<T, DER, DEL>& ptr)
{
    return ptr.operator bool();
}

inline original::refCountBase::refCountBase()
    : strong_refs(makeAtomic<u_integer>(0)),
      weak_refs(makeAtomic<u_integer>(0)),
      total_refs(makeAtomic<u_integer>(0))
{
}

template <typename TYPE, typename DELETER>
original::refCount<TYPE, DELETER>::refCount(TYPE* p)
    : ptr(p)
{
}

template <typename TYPE, typename DELETER>
const void* original::refCount<TYPE, DELETER>::getPtr() const noexcept
{
    return this->ptr;
}

template <typename TYPE, typename DELETER>
void* original::refCount<TYPE, DELETER>::getPtr() noexcept
{
    if constexpr (std::is_const_v<TYPE>)
    {
        return const_cast<std::remove_const_t<TYPE>*>(this->ptr);
    }
    else
    {
        return this->ptr;
    }
}

template <typename TYPE, typename DELETER>
void* original::refCount<TYPE, DELETER>::releasePtr() noexcept
{
    if constexpr (std::is_const_v<TYPE>)
    {
        auto p = const_cast<std::remove_const_t<TYPE>*>(this->ptr);
        this->ptr = nullptr;
        return p;
    }
    else
    {
        auto p = this->ptr;
        this->ptr = nullptr;
        return p;
    }
}

template <typename TYPE, typename DELETER>
void original::refCount<TYPE, DELETER>::destroyPtr() noexcept
{
    TYPE* tmp = this->ptr;
    this->ptr = nullptr;
    this->deleter(tmp);
}

template <typename TYPE, typename DELETER>
original::refCount<TYPE, DELETER>::~refCount()
{
    this->destroyPtr();
}

template <typename TYPE, typename DERIVED, typename DELETER>
void std::swap(original::autoPtr<TYPE, DERIVED, DELETER>& lhs, // NOLINT
               original::autoPtr<TYPE, DERIVED, DELETER>& rhs) noexcept
{
    lhs.swap(rhs);
}

#endif //AUTOPTR_H
