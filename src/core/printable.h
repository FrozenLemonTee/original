#ifndef PRINTABLE_H
#define PRINTABLE_H
#pragma once

#include <memory>
#include "format"
#include "types.h"
#include "sstream"

/**
 * @file printable.h
 * @brief Interface for polymorphic string formatting and output.
 * @details Defines the printable base class with comprehensive string conversion
 *          utilities, supporting automatic formatting of primitive types, enums,
 *          pointers, and custom class hierarchies. Includes integration with
 *          C++20 std::format.
 *
 * Key Features:
 * - Polymorphic string conversion for derived classes
 * - Automatic formatting of built-in types and pointers
 * - Enum value formatting with type names
 * - C-style string conversion with caching
 * - Integration with C++ streams and std::format
 * - Thread-safe formatting utilities
 */

namespace original {

/**
 * @class printable
 * @brief Base class providing polymorphic string conversion capabilities.
 * @details Inherit from this class to enable automatic string representation
 *          for output streams, C-style string access, and std::format support.
 *          Provides extensible formatting through template specializations.
 *
 * @note All derived classes automatically gain string conversion capabilities
 *       and can be used with std::cout, std::format, and other output mechanisms.
 */
class printable {
    mutable std::string cache_string_; ///< Buffer for C-string conversions

public:
    virtual ~printable() = 0;

    /**
     * @brief Gets the class name for type identification.
     * @return Class name as string.
     * @details Override in derived classes to provide accurate type names.
     *
     * @code{.cpp}
     * class MyClass : public printable {
     *     std::string className() const override { return "MyClass"; }
     * };
     * @endcode
     */
    [[nodiscard]] virtual std::string className() const;

    /**
     * @brief Generates formatted string representation.
     * @param enter Whether to append newline.
     * @return Formatted string in "ClassName(data)" format.
     *
     * @code{.cpp}
     * MyClass obj;
     * std::cout << obj.toString(true); // Outputs "printable(@0x7ffd) \n"
     * @endcode
     */
    [[nodiscard]] virtual std::string toString(bool enter) const;

    /**
     * @brief Explicit conversion to std::string.
     * @return String representation without newline.
     */
    explicit operator std::string() const;

    /**
     * @brief Explicit conversion to C-style string.
     * @return Null-terminated C-string (lifetime tied to object).
     * @note The returned pointer is valid until the next call to toCString()
     *       or destruction of the object.
     */
    explicit operator const char*() const;

    /**
     * @brief Direct C-string access with formatting control.
     * @param enter Append newline if true.
     * @return Managed C-string buffer.
     * @note The returned pointer is valid until the next call to toCString()
     *       or destruction of the object.
     */
    [[nodiscard]] const char* toCString(bool enter) const;

    // ----------------- Static Format Utilities -----------------

    /**
     * @brief Universal value-to-string conversion with type-specific formatting.
     * @tparam TYPE Input value type
     * @param t Value to format
     * @return Formatted string with type-specific handling
     * @details Automatically dispatches to appropriate formatting based on type:
     *          - Printable types: Uses operator<< or toString()
     *          - Enum types: Formats as "EnumName(value)" via formatEnum
     *          - String types: Wraps in quotes
     *          - Character types: Wraps in single quotes
     *          - Pointer types: Formats as hex address with @ prefix
     *          - Boolean types: Formats as "true"/"false"
     *          - nullptr_t: Formats as "nullptr"
     *          - Other types: Uses typeid and address formatting
     *
     * The function uses template specialization and concept constraints to
     * select the appropriate formatting strategy at compile time.
     *
     * Example outputs:
     * @code{.cpp}
     * printable::formatString(42);              // "42"
     * printable::formatString("hello");         // "\"hello\""
     * printable::formatString('A');             // "'A'"
     * printable::formatString(true);            // "true"
     * printable::formatString(nullptr);         // "nullptr"
     * printable::formatString((void*)0x1234);  // "@0x1234"
     *
     * enum Color { Red, Green };
     * printable::formatString(Color::Red);     // "Color(0)"
     * @endcode
     */
    template<typename TYPE>
    static std::string formatString(const TYPE& t);

    /**
     * @brief Specialization for types deriving from printable
     * @tparam TYPE Printable-derived type (constrained by Printable concept)
     * @param t Printable object to format
     * @return String representation using the object's toString() method
     * @details Uses the printable object's streaming operator or toString()
     *          method to generate the string representation. This ensures
     *          consistent formatting across all printable-derived types.
     */
    template<Printable TYPE>
    static std::string formatString(const TYPE& t);

    /**
     * @brief Specialization for enum types with type-safe formatting
     * @tparam TYPE Enum type (constrained by EnumType concept)
     * @param t Enum value to format
     * @return String in "EnumName(value)" format
     * @details Extracts the underlying integer value and combines it with
     *          the type name for readable enum representation. Uses
     *          std::underlying_type_t for safe value extraction.
     */
    template<EnumType TYPE>
    static std::string formatString(const TYPE& t);

    /**
     * @brief Pointer-specific formatting with null safety
     * @tparam TYPE Pointer type
     * @param ptr Pointer to format
     * @return String with hex address prefixed by @, or "nullptr" for null
     * @details Safely handles null pointers by returning "nullptr" and
     *          formats valid pointers as hexadecimal addresses with @ prefix
     *          for easy identification.
     *
     * @code{.cpp}
     * int* p = nullptr;
     * printable::formatString(p);          // "nullptr"
     *
     * int x = 42;
     * printable::formatString(&x);         // "@0x7ffeeb4a4c5c"
     * @endcode
     */
    template <typename TYPE>
    static std::string formatString(TYPE* const& ptr);

    /**
     * @brief C-string cache for temporary usage with static storage
     * @tparam TYPE Input type
     * @param t Value to format
     * @return Static C-string buffer containing formatted result
     * @warning Not thread-safe due to static buffer - avoid in multi-threaded contexts
     * @note The returned pointer points to static storage that will be
     *       overwritten on subsequent calls. For thread-safe usage or
     *       persistent strings, use formatString() and store the result.
     *
     * @code{.cpp}
     * // Single-threaded usage only:
     * const char* str = printable::formatCString(42);
     * printf("%s\n", str);  // Safe if no concurrent calls
     * @endcode
     */
    template<typename TYPE>
    static const char* formatCString(const TYPE& t);

    /**
     * @brief Enum formatting utility with underlying value extraction
     * @tparam TYPE Enum type
     * @param t Enum value
     * @return String in "EnumName(value)" format
     * @details Formats enum values by combining the type name from typeid
     *          with the underlying integer value. Provides consistent
     *          enum representation across the codebase.
     *
     * @code{.cpp}
     * enum Color { Red = 1, Green = 2 };
     * printable::formatEnum(Color::Red);  // "Color(1)"
     * printable::formatEnum(Color::Green); // "Color(2)"
     * @endcode
     */
    template<typename TYPE>
    static std::string formatEnum(const TYPE& t);

    friend std::ostream& operator<<(std::ostream& os, const printable& p);
};

/**
 * @brief Stream insertion operator for printable objects
 * @param os Output stream
 * @param p Printable object
 * @return Modified output stream
 * @details Enables direct streaming of printable objects to output streams.
 *          Uses the object's toString(false) method to generate the output
 *          without trailing newline.
 *
 * @code{.cpp}
 * MyClass obj;
 * std::cout << obj;                    // Outputs "MyClass(custom_data)"
 * std::cout << obj << std::endl;       // Adds newline separately
 * @endcode
 */
std::ostream& operator<<(std::ostream& os, const printable& p);

} // namespace original

/**
 * @namespace std
 * @brief Standard namespace extensions for printable integration
 */
namespace std {
    /**
     * @brief std::to_string overload for printable-derived types
     * @tparam T Type derived from printable
     * @param t Printable object to convert
     * @return String representation using printable::formatString
     * @details Provides integration with standard library functions that
     *          use std::to_string, enabling seamless use of printable
     *          types in standard contexts.
     *
     * @code{.cpp}
     * MyClass obj;
     * std::string s = std::to_string(obj);  // Uses printable formatting
     * @endcode
     */
    template<typename T>
    requires original::ExtendsOf<original::printable, T>
    std::string to_string(const T& t); // NOLINT

    /**
     * @brief std::formatter specialization for printable types
     * @tparam T Type derived from printable
     * @details Enables use of printable-derived types with C++20 std::format.
     *          Uses the object's string conversion operators for consistent
     *          formatting across all output mechanisms.
     *
     * Example usage:
     * @code{.cpp}
     * MyClass obj;
     * auto s1 = std::format("Object: {}", obj);       // Basic formatting
     * auto s2 = std::format("Object: {:<20}", obj);   // With format spec
     * auto s3 = std::format("Object: {:.10}", obj);   // With precision
     * @endcode
     *
     * @note The formatter delegates to std::string formatter after
     *       converting the printable object to string.
     */
    template<typename T>
    requires original::ExtendsOf<original::printable, T>
    struct formatter<T> { // NOLINT

        /**
         * @brief Parses the format specification for printable types
         * @param ctx Format parse context
         * @return Iterator past the parsed format specification
         * @details Currently accepts any format specification and delegates
         *          parsing to the underlying string formatter.
         */
        static constexpr auto parse(format_parse_context& ctx);

        /**
         * @brief Formats the printable object using its string conversion
         * @param p Printable object to format
         * @param ctx Format context
         * @return Iterator past the formatted output
         * @details Converts the printable object to string using its
         *          explicit conversion operator, then delegates to the
         *          std::string formatter for actual formatting.
         */
        static auto format(const T& p, format_context& ctx);
    };
}

// ----------------- Definitions of printable.h -----------------

inline original::printable::~printable() = default;

inline auto original::printable::className() const -> std::string
{
    return "printable";
}

inline auto original::printable::toString(const bool enter) const -> std::string
{
    std::stringstream ss;
    ss << this->className() << "(" << formatString(this) << ")";
    if (enter) ss << "\n";
    return ss.str();
}

inline original::printable::operator std::string() const {
    return this->toString(false);
}

inline original::printable::operator const char*() const {
    return this->toCString(false);
}

inline auto original::printable::toCString(const bool enter) const -> const char*
{
    this->cache_string_ = this->toString(enter);
    return this->cache_string_.c_str();
}

template<typename TYPE>
auto original::printable::formatString(const TYPE& t) -> std::string
{
    std::stringstream ss;
    ss << typeid(t).name() << "("  << formatString(&t) << ")";
    return ss.str();
}

template<original::Printable TYPE>
std::string original::printable::formatString(const TYPE& t)
{
    std::stringstream ss;
    ss << t;
    return ss.str();
}

template <original::EnumType TYPE>
std::string original::printable::formatString(const TYPE& t)
{
    return formatEnum(t);
}

template <typename TYPE>
auto original::printable::formatCString(const TYPE& t) -> const char*
{
    static auto result =
            std::make_unique<std::string>(formatString<TYPE>(t));
    return result->c_str();
}

template <typename TYPE>
auto original::printable::formatEnum(const TYPE& t) -> std::string
{
    const std::string enum_name = typeid(t).name();
    const int enum_value = static_cast<std::underlying_type_t<TYPE>>(t);
    return enum_name + "(" + std::to_string(enum_value) + ")";
}

template<>
inline auto original::printable::formatString<std::nullptr_t>(const std::nullptr_t&) -> std::string
{
    return "nullptr";
}

template<>
inline auto original::printable::formatString<std::string>(const std::string& t) -> std::string
{
    return "\"" + t + "\"";
}

template<>
inline auto original::printable::formatString<char>(const char& t) -> std::string
{
    return "'" + std::string(1, t) + "'";
}

template<>
inline auto original::printable::formatString<bool>(const bool& t) -> std::string
{
    return t ? "true" : "false";
}

template <typename TYPE>
auto original::printable::formatString(TYPE* const& ptr) -> std::string
{
    if (!ptr)
        return "nullptr";

    std::stringstream ss;
    ss << "@" << ptr;
    return ss.str();
}

template<>
inline auto original::printable::formatString<const char>(const char* const &ptr) -> std::string {
    return formatString<std::string>(ptr);
}

inline std::ostream& original::operator<<(std::ostream& os, const printable& p){
    os << p.toString(false);
    return os;
}

template<typename T>
requires original::ExtendsOf<original::printable, T>
std::string std::to_string(const T& t) // NOLINT
{
    return original::printable::formatString(t);
}

template<typename T>
requires original::ExtendsOf<original::printable, T>
constexpr auto std::formatter<T>::parse(std::format_parse_context &ctx) {
    return ctx.begin();
}

template<typename T>
requires original::ExtendsOf<original::printable, T>
auto std::formatter<T>::format(const T &p, std::format_context &ctx) {
    return formatter<std::string>().format(static_cast<std::string>(p), ctx);
}

#endif // PRINTABLE_H