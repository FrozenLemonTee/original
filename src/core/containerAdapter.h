#ifndef CONTAINERADAPTER_H
#define CONTAINERADAPTER_H
#include "baseList.h"
#include "types.h"
#include "printable.h"
#include "comparable.h"


namespace original {
    /**
     * @file containerAdapter.h
     * @brief Base class for container adapters with common interfaces
     * @details Provides unified foundation for stack, queue, deque, priority_queue and other
     * container adapters that need to work with different underlying container types.
     * Implements core functionality required by standard container adapters.
     */

    /**
     * @class containerAdapter
     * @tparam TYPE The type of elements stored in the container
     * @tparam SERIAL The underlying serial container type
     * @tparam ALLOC The allocator template to use for memory management
     * @brief Adapter class that provides unified interface for various container types
     * @extends printable
     * @extends container
     * @details This class template adapts different serial container types (like vectors, lists)
     * to provide consistent interfaces including size checking, element existence verification,
     * and formatted string output. Inherits both printable interface and container interface.
     *
     * The allocator template is propagated to both the base container class and the underlying
     * serial container for consistent memory management.
     * The adapter provides full comparison support through the comparable interface,
     * with lexicographical ordering based on the underlying container's elements.
     * All comparison operations (==, !=, <, >, <=, >=) are available.
     *
     * @note The SERIAL template parameter is constrained by C++20 requires clause to ensure
     * it derives from baseList<TYPE>. This guarantees the availability of required container
     * interfaces like size(), clear(), and contains() methods.
     *
     * @tparam SERIAL Must satisfy:
     * @code
     * ExtendsOf<baseList<TYPE, ALLOC<TYPE>>, SERIAL<TYPE, ALLOC<TYPE>>>
     * @endcode
     * Which enforces SERIAL to be a subclass of baseList<TYPE, ALLOC<TYPE>>.
     */
    template <typename TYPE,
              template <typename, typename> typename SERIAL,
              template <typename> typename ALLOC>
    requires ExtendsOf<baseList<TYPE, ALLOC<TYPE>>, SERIAL<TYPE, ALLOC<TYPE>>>
    class containerAdapter
            : public printable,
              public container<TYPE, ALLOC<TYPE>>,
              public comparable<containerAdapter<TYPE, SERIAL, ALLOC>>{
    protected:
        /**
         * @brief The underlying container instance
         * @details Holds the actual container elements and manages memory using the provided allocator.
         * All operations are delegated to this container instance.
         */
        SERIAL<TYPE, ALLOC<TYPE>> serial_;

        /**
         * @brief Constructs a container adapter with specified underlying container
         * @param serial The underlying container instance to adapt
         * @details The constructor initializes the adapter with the provided serial container.
         * The allocator from the serial container will be used for all memory operations.
         *
         * @note The allocator is propagated from the serial container to ensure consistent
         * memory management throughout the adapter's lifetime.
         */
        explicit containerAdapter(const SERIAL<TYPE, ALLOC<TYPE>>& serial);

    public:
        /**
         * @brief Returns the number of elements in the adapter
         * @return u_integer Current number of stored elements
         * @details Directly delegates to underlying container's size() method.
         * Memory usage depends on the underlying container's allocator.
         */
        [[nodiscard]] u_integer size() const override;

        /**
         * @brief Removes all elements from the adapter
         * @details Clears content through underlying container's clear() method.
         * All elements are properly destroyed using the container's allocator.
         */
        void clear();

        /**
         * @brief Checks for element existence in the adapter
         * @param e Element to search for
         * @return bool True if element exists, false otherwise
         * @details Uses underlying container's contains() method for verification.
         * The search operation respects the memory layout determined by the allocator.
         */
        bool contains(const TYPE &e) const override;

        /**
         * @brief Compares two container adapters lexicographically
         * @param other The container adapter to compare against
         * @return integer
         *         - Negative value if this adapter is "less than" other
         *         - Zero if both adapters are equivalent
         *         - Positive value if this adapter is "greater than" other
         * @details The comparison is performed by delegating to the underlying serial container's
         *          compareTo method. This provides lexicographical comparison of elements in the
         *          containers, similar to standard container comparison semantics.
         *
         *          The comparison follows these rules:
         *          1. If sizes differ, the adapter with fewer elements is considered "less"
         *          2. For adapters of equal size, elements are compared pairwise until a mismatch is found
         *          3. If all elements are equal, the adapters are considered equivalent
         */
        integer compareTo(const containerAdapter &other) const override;

        /**
         * @brief Gets class name identifier for type information
         * @return std::string Fixed identifier "containerAdapter"
         * @note Derived classes should override this with specific names (e.g. "stack")
         */
        [[nodiscard]] std::string className() const override;

        /**
         * @brief Generates formatted string representation
         * @param enter Add newline at end when true
         * @return std::string Formatted elements in "containerAdapter(e1, e2,...)" format
         * @details Provides uniform formatting across all container adapters
         */
        [[nodiscard]] std::string toString(bool enter) const override;

        /// @brief Virtual destructor for proper polymorphic cleanup
        ~containerAdapter() override = default;
    };

    template <typename TYPE,
              template <typename, typename> typename SERIAL,
              template <typename> typename ALLOC>
    requires ExtendsOf<baseList<TYPE, ALLOC<TYPE>>, SERIAL<TYPE, ALLOC<TYPE>>>
    containerAdapter<TYPE, SERIAL, ALLOC>::containerAdapter(const SERIAL<TYPE, ALLOC<TYPE>>& serial)
            : serial_(serial) {}

    template <typename TYPE,
              template <typename, typename> typename SERIAL,
              template <typename> typename ALLOC>
    requires ExtendsOf<baseList<TYPE, ALLOC<TYPE>>, SERIAL<TYPE, ALLOC<TYPE>>>
    auto containerAdapter<TYPE, SERIAL, ALLOC>::size() const -> u_integer
    {
        return serial_.size();
    }

    template <typename TYPE,
              template <typename, typename> typename SERIAL,
              template <typename> typename ALLOC>
    requires ExtendsOf<baseList<TYPE, ALLOC<TYPE>>, SERIAL<TYPE, ALLOC<TYPE>>>
    auto containerAdapter<TYPE, SERIAL, ALLOC>::clear() -> void {
        serial_.clear();
    }

    template <typename TYPE,
              template <typename, typename> typename SERIAL,
              template <typename> typename ALLOC>
    requires ExtendsOf<baseList<TYPE, ALLOC<TYPE>>, SERIAL<TYPE, ALLOC<TYPE>>>
    auto containerAdapter<TYPE, SERIAL, ALLOC>::contains(const TYPE &e) const -> bool {
        return serial_.contains(e);
    }

    template <typename TYPE,
              template <typename, typename> typename SERIAL,
              template <typename> typename ALLOC>
    requires ExtendsOf<baseList<TYPE, ALLOC<TYPE>>, SERIAL<TYPE, ALLOC<TYPE>>>
    auto containerAdapter<TYPE, SERIAL, ALLOC>::compareTo(const containerAdapter& other) const -> integer
    {
        return serial_.compareTo(other.serial_);
    }

    template <typename TYPE,
              template <typename, typename> typename SERIAL,
              template <typename> typename ALLOC>
    requires ExtendsOf<baseList<TYPE, ALLOC<TYPE>>, SERIAL<TYPE, ALLOC<TYPE>>>
    auto containerAdapter<TYPE, SERIAL, ALLOC>::className() const -> std::string {
        return "containerAdapter";
    }

    template <typename TYPE,
              template <typename, typename> typename SERIAL,
              template <typename> typename ALLOC>
    requires ExtendsOf<baseList<TYPE, ALLOC<TYPE>>, SERIAL<TYPE, ALLOC<TYPE>>>
    auto containerAdapter<TYPE, SERIAL, ALLOC>::toString(const bool enter) const -> std::string {
        std::stringstream ss;
        ss << this->className() << "(";
        bool first = true;
        for (const auto& e : this->serial_)
        {
            if (!first) ss << ", ";
            ss << printable::formatString(e);
            first = false;
        }
        ss << ")";
        if (enter) ss << "\n";
        return ss.str();
    }
}

#endif //CONTAINERADAPTER_H
