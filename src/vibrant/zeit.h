#ifndef ORIGINAL_ZEIT_H
#define ORIGINAL_ZEIT_H

#if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG
#include <ctime>
#elif ORIGINAL_COMPILER_MSVC
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <cmath>
#include "config.h"
#include "comparable.h"
#include "hash.h"
#include "printable.h"
#include "error.h"
#include <iomanip>

/**
 * @file zeit.h
 * @brief Time and date handling utilities
 * @details Provides classes for time duration, points in time, and UTC date/time handling.
 * Includes support for:
 * - Time durations with various units (nanoseconds to days)
 * - Time points representing moments in time
 * - UTC date/time with calendar operations
 * - Literals for time durations
 *
 * Key features:
 * - High precision time representation (nanoseconds)
 * - Calendar calculations (leap years, days in month, etc.)
 * - Platform-independent time operations
 * - Comparable, hashable, and printable interfaces
 */



namespace original {
    /**
     * @class time
     * @brief Namespace-like class containing time-related utilities
     * @details Contains duration, point, and UTCTime classes along with
     * unit definitions and conversion factors.
     */
    class time final {
    public:
        /// Type used for storing time values
        using time_val_type = integer;

    private:
        /// Base unit factor for time calculations
        static constexpr time_val_type UNIT_FACTOR_BASE = 1;

        /// Array of conversion factors for different time units
        static constexpr time_val_type UNIT_FACTOR[] {
                UNIT_FACTOR_BASE,
                UNIT_FACTOR_BASE * 1000,
                UNIT_FACTOR_BASE * 1000 * 1000,
                UNIT_FACTOR_BASE * 1000 * 1000 * 1000,
                UNIT_FACTOR_BASE * 1000 * 1000 * 1000 * 60,
                UNIT_FACTOR_BASE * 1000 * 1000 * 1000 * 60 * 60,
                UNIT_FACTOR_BASE * 1000 * 1000 * 1000 * 60 * 60 * 24,
        };

    public:
        /**
         * @enum unit
         * @brief Time units supported by the library
         */
        enum class unit {
            NANOSECOND,    ///< Nanoseconds (1e-9 seconds)
            MICROSECOND,   ///< Microseconds (1e-6 seconds)
            MILLISECOND,   ///< Milliseconds (1e-3 seconds)
            SECOND,       ///< Seconds
            MINUTE,       ///< Minutes (60 seconds)
            HOUR,         ///< Hours (3600 seconds)
            DAY,          ///< Days (86400 seconds)
        };

        /// Constant for nanosecond unit
        static constexpr auto NANOSECOND = unit::NANOSECOND;
        /// Constant for microsecond unit
        static constexpr auto MICROSECOND = unit::MICROSECOND;
        /// Constant for millisecond unit
        static constexpr auto MILLISECOND = unit::MILLISECOND;
        /// Constant for second unit
        static constexpr auto SECOND = unit::SECOND;
        /// Constant for minute unit
        static constexpr auto MINUTE = unit::MINUTE;
        /// Constant for hour unit
        static constexpr auto HOUR = unit::HOUR;
        /// Constant for day unit
        static constexpr auto DAY = unit::DAY;

        /// Conversion factor for nanoseconds
        static constexpr time_val_type FACTOR_NANOSECOND = UNIT_FACTOR[static_cast<ul_integer>(NANOSECOND)];
        /// Conversion factor for microseconds
        static constexpr time_val_type FACTOR_MICROSECOND = UNIT_FACTOR[static_cast<ul_integer>(MICROSECOND)];
        /// Conversion factor for milliseconds
        static constexpr time_val_type FACTOR_MILLISECOND = UNIT_FACTOR[static_cast<ul_integer>(MILLISECOND)];
        /// Conversion factor for seconds
        static constexpr time_val_type FACTOR_SECOND = UNIT_FACTOR[static_cast<ul_integer>(SECOND)];
        /// Conversion factor for minutes
        static constexpr time_val_type FACTOR_MINUTE = UNIT_FACTOR[static_cast<ul_integer>(MINUTE)];
        /// Conversion factor for hours
        static constexpr time_val_type FACTOR_HOUR = UNIT_FACTOR[static_cast<ul_integer>(HOUR)];
        /// Conversion factor for days
        static constexpr time_val_type FACTOR_DAY = UNIT_FACTOR[static_cast<ul_integer>(DAY)];

        /// Days in a leap year
        static constexpr integer DAYS_LEAP_YEAR = 366;
        /// Days in a common year
        static constexpr integer DAYS_COMMON_YEAR = 365;

        /// Epoch year (Unix epoch)
        static constexpr integer EPOCH_YEAR = 1970;
        /// Epoch month (Unix epoch)
        static constexpr integer EPOCH_MONTH = 1;
        /// Epoch day (Unix epoch)
        static constexpr integer EPOCH_DAY = 1;

        /// Forward declaration of point class
        class point;
        /// Forward declaration of UTCTime class
        class UTCTime;

        /**
         * @class duration
         * @brief Represents a time duration with nanosecond precision
         * @extends comparable
         * @extends hashable
         * @extends printable
         * @details Supports arithmetic operations and conversions between
         * different time units.
         */
        class duration final
                      : public comparable<duration>,
                        public hashable<duration>,
                        public printable {
            time_val_type nano_seconds_; ///< Internal storage in nanoseconds

        public:
            friend class point;

            /// Zero duration constant
            static const duration ZERO;

            /**
             * @brief Constructs a duration with given value and unit
             * @param val Time value
             * @param unit Unit of time (default: MILLISECOND)
             */
            explicit duration(time_val_type val = 0, unit unit = MILLISECOND);

#if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG
            /**
             * @brief Constructs a duration from a POSIX timespec structure
             * @param ts timespec structure containing seconds and nanoseconds
             * @note Only available when compiled with GCC
             * @details Converts the timespec's tv_sec (seconds) and tv_nsec (nanoseconds)
             *          into a unified duration value in nanoseconds.
             */
            explicit duration(const timespec& ts);
#elif ORIGINAL_COMPILER_MSVC
            explicit duration(DWORD milliseconds);
#endif

            /// Default copy constructor
            duration(const duration& other) = default;

            /// Default copy assignment
            duration& operator=(const duration& other) = default;

            /**
             * @brief Move constructor
             * @param other Duration to move from
             */
            duration(duration&& other) noexcept;

            /**
             * @brief Move assignment
             * @param other Duration to move from
             * @return Reference to this duration
             */
            duration& operator=(duration&& other) noexcept;

            /**
             * @brief Gets the duration value in specified units
             * @param unit Unit to return value in (default: MILLISECOND)
             * @return Duration value in requested units
             */
            [[nodiscard]] time_val_type value(unit unit = MILLISECOND) const;

            /**
             * @brief Compares this duration to another
             * @param other Duration to compare with
             * @return Negative if this < other, 0 if equal, positive if this > other
             */
            [[nodiscard]] integer compareTo(const duration& other) const override;

            /**
             * @brief Computes hash value for this duration
             * @return Hash value
             */
            u_integer toHash() const noexcept override;

            /**
             * @brief Gets the class name
             * @return "time::duration"
             */
            std::string className() const override;

            /**
             * @brief Converts duration to string representation
             * @param enter Whether to include newline
             * @return String representation
             */
            std::string toString(bool enter) const override;

#if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG
            /**
             * @brief Converts duration to timespec (POSIX time structure)
             * @return timespec structure containing seconds and nanoseconds
             * @note Only available when compiled with GCC
             */
            explicit operator timespec() const;

            timespec toTimespec() const;
#elif ORIGINAL_COMPILER_MSVC
            explicit operator DWORD() const;

            DWORD toDWMilliseconds() const;
#endif

            /**
             * @brief Prefix increment (adds 1 nanosecond)
             * @return Reference to modified duration
             */
            duration& operator++();

            /**
             * @brief Postfix increment (adds 1 nanosecond)
             * @param postfix Dummy parameter for postfix
             * @return Copy of duration before increment
             */
            duration operator++(int postfix);

            /**
             * @brief Prefix decrement (subtracts 1 nanosecond)
             * @return Reference to modified duration
             */
            duration& operator--();

            /**
             * @brief Postfix decrement (subtracts 1 nanosecond)
             * @param postfix Dummy parameter for postfix
             * @return Copy of duration before decrement
             */
            duration operator--(int postfix);

            /**
             * @brief Adds another duration to this one
             * @param other Duration to add
             * @return Reference to modified duration
             */
            duration& operator+=(const duration& other);

            /**
             * @brief Subtracts another duration from this one
             * @param other Duration to subtract
             * @return Reference to modified duration
             */
            duration& operator-=(const duration& other);

            /**
             * @brief Multiplies duration by a factor
             * @param factor Multiplication factor
             * @return Reference to modified duration
             */
            duration& operator*=(time_val_type factor);

            /**
             * @brief Divides duration by a factor
             * @param factor Division factor
             * @return Reference to modified duration
             */
            duration& operator/=(time_val_type factor);

            /**
             * @brief Divides duration by another duration
             * @param other Duration to divide by
             * @return Reference to modified duration
             */
            duration& operator/=(const duration& other);

            /**
             * @brief Floating-point division by a factor
             * @param factor Division factor
             * @param unit Unit for factor (default: MILLISECOND)
             * @return Floating-point result
             */
            [[nodiscard]] floating div(time_val_type factor, unit unit = MILLISECOND) const;

            /**
             * @brief Floating-point division by another duration
             * @param other Duration to divide by
             * @return Floating-point result
             */
            [[nodiscard]] floating div(const duration& other) const;

            /**
             * @brief Negation operator
             * @param d Duration to negate
             * @return Negated duration
             */
            friend duration operator-(const duration& d);

            /**
             * @brief Addition operator
             * @param lhs Left operand
             * @param rhs Right operand
             * @return Sum of durations
             */
            friend duration operator+(const duration& lhs, const duration& rhs);

            /**
             * @brief Subtraction operator
             * @param lhs Left operand
             * @param rhs Right operand
             * @return Difference of durations
             */
            friend duration operator-(const duration& lhs, const duration& rhs);

            /**
             * @brief Multiplication operator (duration * factor)
             * @param d Duration to multiply
             * @param factor Multiplication factor
             * @return Product of duration and factor
             */
            friend duration operator*(const duration& d, time_val_type factor);

            /**
             * @brief Multiplication operator (factor * duration)
             * @param factor Multiplication factor
             * @param d Duration to multiply
             * @return Product of factor and duration
             */
            friend duration operator*(time_val_type factor, const duration& d);

            /**
             * @brief Division operator (duration / factor)
             * @param d Duration to divide
             * @param factor Division factor
             * @return Quotient of duration and factor
             */
            friend duration operator/(const duration& d, time_val_type factor);

            /**
             * @brief Division operator (duration / duration)
             * @param lhs Left operand
             * @param rhs Right operand
             * @return Quotient of durations
             */
            friend duration operator/(const duration& lhs, const duration& rhs);

            /**
             * @brief Absolute value of duration
             * @param d Duration to get absolute value of
             * @return Absolute duration
             */
            friend duration abs(const duration& d);
        };

        /**
         * @class point
         * @brief Represents a point in time with nanosecond precision
         * @extends comparable
         * @extends hashable
         * @extends printable
         * @details Represents time since epoch (1970-01-01 00:00:00 UTC)
         */
        class point final
                : public comparable<point>,
                  public hashable<point>,
                  public printable {
            duration nano_since_epoch_; ///< Duration since epoch

        public:
            friend class UTCTime;

            /**
             * @brief Gets current time point
             * @return Current time point
             */
            static point now();

            /**
             * @brief Constructs time point from value and unit
             * @param val Time value
             * @param unit Unit of time (default: MILLISECOND)
             */
            explicit point(time_val_type val = 0, unit unit = MILLISECOND);

            /**
             * @brief Constructs time point from duration
             * @param d Duration since epoch
             */
            explicit point(duration d);

#if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG
            /**
             * @brief Constructs a time point from a POSIX timespec structure.
             * @param ts The timespec structure containing seconds and nanoseconds since the epoch.
             * @note The conversion computes the total nanoseconds from ts.tv_sec and ts.tv_nsec.
             */
            explicit point(const timespec& ts);
#elif ORIGINAL_COMPILER_MSVC
            explicit point(DWORD milliseconds);
#endif

            /**
             * @brief Gets time value in specified units
             * @param unit Unit to return value in (default: MILLISECOND)
             * @return Time value in requested units
             */
            [[nodiscard]] time_val_type value(unit unit = MILLISECOND) const noexcept;

            /**
             * @brief Compares this time point to another
             * @param other Time point to compare with
             * @return Negative if this < other, 0 if equal, positive if this > other
             */
            [[nodiscard]] integer compareTo(const point& other) const override;

            /**
             * @brief Computes hash value for this time point
             * @return Hash value
             */
            u_integer toHash() const noexcept override;

            /**
             * @brief Gets the class name
             * @return "time::point"
             */
            std::string className() const override;

            /**
             * @brief Converts time point to string representation
             * @param enter Whether to include newline
             * @return String representation
             */
            std::string toString(bool enter) const override;

#if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG
            /**
             * @brief Converts this time point to a POSIX timespec structure.
             * @return A timespec representing this time point with seconds and nanoseconds since the epoch.
             * @note Nanoseconds are truncated if the internal value is not aligned to nanosecond precision.
             */
            explicit operator timespec() const;

            timespec toTimespec() const;
#elif ORIGINAL_COMPILER_MSVC
            explicit operator DWORD() const;

            DWORD toDWMilliseconds() const;
#endif

            /**
             * @brief Prefix increment (adds 1 nanosecond)
             * @return Reference to modified time point
             */
            point& operator++();

            /**
             * @brief Postfix increment (adds 1 nanosecond)
             * @param postfix Dummy parameter for postfix
             * @return Copy of time point before increment
             */
            point operator++(int postfix);

            /**
             * @brief Prefix decrement (subtracts 1 nanosecond)
             * @return Reference to modified time point
             */
            point& operator--();

            /**
             * @brief Postfix decrement (subtracts 1 nanosecond)
             * @param postfix Dummy parameter for postfix
             * @return Copy of time point before decrement
             */
            point operator--(int postfix);

            /**
             * @brief Adds duration to time point
             * @param d Duration to add
             * @return Reference to modified time point
             */
            point& operator+=(const duration& d);

            /**
             * @brief Subtracts duration from time point
             * @param d Duration to subtract
             * @return Reference to modified time point
             */
            point& operator-=(const duration& d);

            /**
             * @brief Adds duration to time point
             * @param p Time point
             * @param d Duration to add
             * @return New time point after addition
             */
            friend point operator+(const point& p, const duration& d);

            friend point operator+(const duration& d, const point& p);

            /**
             * @brief Subtracts duration from time point
             * @param p Time point
             * @param d Duration to subtract
             * @return New time point after subtraction
             */
            friend point operator-(const point& p, const duration& d);

            /**
             * @brief Computes duration between two time points
             * @param lhs Left operand
             * @param rhs Right operand
             * @return Duration between points
             */
            friend duration operator-(const point& lhs, const point& rhs);
        };

        /**
         * @class UTCTime
         * @brief Represents a UTC calendar date and time
         * @extends comparable
         * @extends hashable
         * @extends printable
         * @details Provides calendar operations including:
         * - Leap year detection
         * - Days in month calculation
         * - Weekday calculation
         * - Conversion to/from time points
         */
        class UTCTime final
                : public comparable<UTCTime>,
                  public hashable<UTCTime>,
                  public printable {

            integer year_{};    ///< Year component
            integer month_{};   ///< Month component (1-12)
            integer day_{};     ///< Day component (1-31)
            integer hour_{};    ///< Hour component (0-23)
            integer minute_{};  ///< Minute component (0-59)
            integer second_{};  ///< Second component (0-59)

            /**
             * @enum calendar
             * @brief Calendar component types
             */
            enum class calendar : integer {
                YEAR,   ///< Year component
                MONTH,  ///< Month component
            };

            /**
             * @enum weekdays
             * @brief Days of the week
             */
            enum class weekdays : integer {
                SATURDAY, ///< Saturday
                SUNDAY,   ///< Sunday
                MONDAY,   ///< Monday
                TUESDAY,  ///< Tuesday
                WEDNESDAY,///< Wednesday
                THURSDAY, ///< Thursday
                FRIDAY,   ///< Friday
            };

            /// Days in each month (non-leap year)
            static constexpr integer DAYS_OF_MONTH[] {
                    31, 28, 31, 30, 31, 30,
                    31, 31, 30, 31, 30, 31
            };

            /**
             * @brief Checks if year is valid
             * @param year Year to check
             * @return true if valid, false otherwise
             */
            static constexpr bool isValidYear(integer year);

            /**
             * @brief Checks if month is valid
             * @param month Month to check
             * @return true if valid, false otherwise
             */
            static constexpr bool isValidMonth(integer month);

            /**
             * @brief Checks if day is valid
             * @param day Day to check
             * @return true if valid, false otherwise
             */
            static constexpr bool isValidDay(integer day);

            /**
             * @brief Sets all time components
             * @param year Year
             * @param month Month
             * @param day Day
             * @param hour Hour
             * @param minute Minute
             * @param second Second
             */
            void set(integer year, integer month, integer day,
                     integer hour, integer minute, integer second);

        public:
            friend time;

            /// Constant for Saturday
            static constexpr auto SATURDAY = weekdays::SATURDAY;
            /// Constant for Sunday
            static constexpr auto SUNDAY = weekdays::SUNDAY;
            /// Constant for Monday
            static constexpr auto MONDAY = weekdays::MONDAY;
            /// Constant for Tuesday
            static constexpr auto TUESDAY = weekdays::TUESDAY;
            /// Constant for Wednesday
            static constexpr auto WEDNESDAY = weekdays::WEDNESDAY;
            /// Constant for Thursday
            static constexpr auto THURSDAY = weekdays::THURSDAY;
            /// Constant for Friday
            static constexpr auto FRIDAY = weekdays::FRIDAY;

            /// Days in a week
            static constexpr integer DAYS_WEEK = 7;
            /// Months in a year
            static constexpr integer MONTHS_YEAR = std::size(DAYS_OF_MONTH);
            /// Years in a century
            static constexpr integer YEARS_CENTURY = 100;

            /// Epoch time constant (1970-01-01 00:00:00)
            static const UTCTime EPOCH;

            /**
             * @brief Gets current UTC time
             * @return Current UTC time
             */
            static UTCTime now();

            /**
             * @brief Returns the system's local timezone offset in hours from UTC.
             * @return Local timezone offset in hours.
             * @note This accounts for daylight saving time if applicable.
             */
            static integer localZonedOffset();

            /**
             * @brief Returns the current local time as UTCTime.
             * @return UTCTime representing local time.
             * @note This is computed by adding the local timezone offset to the current UTC time.
             */
            static UTCTime localNow();

            /**
             * @brief Checks if year is a leap year
             * @param year Year to check
             * @return true if leap year, false otherwise
             */
            static constexpr bool isLeapYear(integer year);

            /**
             * @brief Gets days in specified month
             * @param year Year (for leap year calculation)
             * @param month Month (1-12)
             * @return Number of days in month
             * @throws valueError if month is invalid
             */
            static constexpr integer daysOfMonth(integer year, integer month);

            /**
             * @brief Calculates weekday for given date
             * @param year Year
             * @param month Month (1-12)
             * @param day Day (1-31)
             * @return Weekday for date
             * @throws valueError if date is invalid
             */
            static constexpr weekdays weekday(integer year, integer month, integer day);

            /**
             * @brief Checks if year-month-day is valid
             * @param year Year
             * @param month Month (1-12)
             * @param day Day (1-31)
             * @return true if valid, false otherwise
             */
            static constexpr bool isValidYMD(integer year, integer month, integer day);

            /**
             * @brief Checks if hour-minute-second is valid
             * @param hour Hour (0-23)
             * @param minute Minute (0-59)
             * @param second Second (0-59)
             * @return true if valid, false otherwise
             */
            static constexpr bool isValidHMS(integer hour, integer minute, integer second);

            /**
             * @brief Checks if full date-time is valid
             * @param year Year
             * @param month Month (1-12)
             * @param day Day (1-31)
             * @param hour Hour (0-23)
             * @param minute Minute (0-59)
             * @param second Second (0-59)
             * @return true if valid, false otherwise
             */
            static constexpr bool isValid(integer year, integer month, integer day,
                                          integer hour, integer minute, integer second);

            /**
             * @brief Checks if this year is a leap year
             * @return true if leap year, false otherwise
             */
            [[nodiscard]] bool isLeapYear() const;

            /**
             * @brief Gets weekday for this date
             * @return Weekday
             */
            [[nodiscard]] weekdays weekday() const;

            /**
             * @brief Constructs a UTCTime object representing the epoch (1970-01-01 00:00:00 UTC)
             * @details Initializes all time components to zero, representing the Unix epoch.
             * @see UTCTime::EPOCH
             * @code
             * UTCTime t;  // Creates epoch time (1970-01-01 00:00:00)
             * @endcode
             */
            explicit UTCTime();

            /**
             * @brief Constructs a UTCTime object with specified date components (time set to 00:00:00)
             * @param year The year component
             * @param month The month component (1-12)
             * @param day The day component (1-31, must be valid for month/year)
             * @throws valueError if any component is invalid or date is impossible
             * @see isValidYMD(), daysOfMonth()
             * @code
             * UTCTime t(2025, 6, 28);  // Creates 2025-06-28 00:00:00
             * @endcode
             */
            explicit UTCTime(integer year, integer month, integer day);

            /**
             * @brief Constructs a UTCTime object with all date and time components
             * @param year The year component (must be >= 1970)
             * @param month The month component (1-12)
             * @param day The day component (1-31, must be valid for month/year)
             * @param hour The hour component (0-23)
             * @param minute The minute component (0-59)
             * @param second The second component (0-59)
             * @throws valueError if any component is invalid or datetime is impossible
             * @see isValid(), isValidYMD(), isValidHMS()
             * @code
             * UTCTime t(2025, 6, 28, 14, 30, 0);  // Creates 2025-06-28 14:30:00
             * @endcode
             */
            explicit UTCTime(integer year, integer month, integer day,
                             integer hour, integer minute, integer second);

            /**
             * @brief Constructs a UTCTime object from a time point
             * @param p The time point to convert from
             * @warning Conversion from time::point to UTCTime may lose precision
             * since UTCTime only stores whole seconds (sub-second components are truncated)
             * @code
             * time::point p = time::point::now();
             * UTCTime t(p);  // Creates UTCTime from current time (seconds precision)
             * @endcode
             */
            explicit UTCTime(const point& p);

            /// Default copy constructor
            UTCTime(const UTCTime& other) = default;

            /// Default copy assignment
            UTCTime& operator=(const UTCTime& other) = default;

            /**
             * @brief Move constructor
             * @param other UTCTime to move from
             */
            UTCTime(UTCTime&& other) noexcept;

            /**
             * @brief Move assignment
             * @param other UTCTime to move from
             * @return Reference to this UTCTime
             */
            UTCTime& operator=(UTCTime&& other) noexcept;

            /**
             * @brief Returns a new UTCTime object containing only the calendar date.
             * @return A new UTCTime object with time components cleared (00:00:00).
             * @see UTCTime::toString(), UTCTime::now()
             * @code
             * UTCTime t(2025, 6, 28, 11, 12, 30);
             * UTCTime d = t.date();
             * // d == (time::UTCTime 2025-06-28 00:00:00)
             * @endcode
             */
            UTCTime date() const;

            /**
             * @brief Gets time component value
             * @param unit Time unit to get
             * @return Value of requested component
             */
            integer value(unit unit) const;

            /**
             * @brief Gets calendar component value
             * @param calendar Calendar component to get
             * @return Value of requested component
             */
            integer value(calendar calendar) const;

            /**
             * @brief Converts to time::point
             * @return Time point representing this UTC time
             * @warning When converting back to time::point, the time point will only
             * have second-level precision (sub-second components will be zero)
             */
            explicit operator point() const;

            point toPoint() const;

            /**
             * @brief Compares this UTCTime to another
             * @param other UTCTime to compare with
             * @return Negative if this < other, 0 if equal, positive if this > other
             */
            integer compareTo(const UTCTime& other) const override;

            /**
             * @brief Computes hash value for this UTCTime
             * @return Hash value
             */
            u_integer toHash() const noexcept override;

            /**
             * @brief Gets the class name
             * @return "time::UTCTime"
             */
            std::string className() const override;

            /**
             * @brief Converts UTCTime to string representation (YYYY-MM-DD HH:MM:SS)
             * @param enter Whether to include newline
             * @return String representation
             */
            std::string toString(bool enter) const override;

            /**
             * @brief Adds duration to UTCTime
             * @param t UTCTime
             * @param d Duration to add
             * @return New UTCTime after addition
             */
            friend UTCTime operator+(const UTCTime& t, const duration& d);

            /**
             * @brief Adds duration to UTCTime (Reversed version)
             * @param d Duration to add
             * @param t UTCTime
             * @return New UTCTime after addition
             */
            friend UTCTime operator+(const duration& d, const UTCTime& t);

            /**
             * @brief Subtracts duration from UTCTime
             * @param t UTCTime
             * @param d Duration to subtract
             * @return New UTCTime after subtraction
             */
            friend UTCTime operator-(const UTCTime& t, const duration& d);

            /**
             * @brief Computes duration between two UTCTimes
             * @param lhs Left operand
             * @param rhs Right operand
             * @return Duration between UTCTimes
             */
            friend duration operator-(const UTCTime& lhs, const UTCTime& rhs);
        };

        /// Constant for month calendar component
        static constexpr auto MONTH = UTCTime::calendar::MONTH;
        /// Constant for year calendar component
        static constexpr auto YEAR = UTCTime::calendar::YEAR;
    };

    /// Zero duration constant
    inline const time::duration time::duration::ZERO = duration{};

    time::duration nanoseconds(time::time_val_type val = 1);

    time::duration microseconds(time::time_val_type val = 1);

    time::duration milliseconds(time::time_val_type val = 1);

    time::duration seconds(time::time_val_type val = 1);

    time::duration minutes(time::time_val_type val = 1);

    time::duration hours(time::time_val_type val = 1);

    time::duration days(time::time_val_type val = 1);

    time::duration operator-(const time::duration& d);

    time::duration operator+(const time::duration& lhs, const time::duration& rhs);

    time::duration operator-(const time::duration& lhs, const time::duration& rhs);

    time::duration operator*(const time::duration& d, time::time_val_type factor);

    time::duration operator*(time::time_val_type factor, const time::duration& d);

    time::duration operator/(const time::duration& d, time::time_val_type factor);

    time::duration operator/(const time::duration& lhs, const time::duration& rhs);

    time::duration abs(const time::duration& d);

    time::point operator+(const time::point& p, const time::duration& d);

    time::point operator+(const time::duration& d, const time::point& p);

    time::point operator-(const time::point& p, const time::duration& d);

    time::duration operator-(const time::point& lhs, const time::point& rhs);

    /// Epoch time constant (1970-01-01 00:00:00)
    inline const time::UTCTime time::UTCTime::EPOCH = UTCTime{};

    time::UTCTime operator+(const time::UTCTime& t, const time::duration& d);

    time::UTCTime operator+(const time::duration& d, const time::UTCTime& t);

    time::UTCTime operator-(const time::UTCTime& t, const time::duration& d);

    time::duration operator-(const time::UTCTime& lhs, const time::UTCTime& rhs);

    /**
     * @namespace literals
     * @brief User-defined literals for time durations
     */
    namespace literals {
        /**
         * @brief Creates nanosecond duration from integer literal
         * @param val Value in nanoseconds
         * @return Duration object
         */
        inline time::duration operator""_ns(const unsigned long long val) {
            return time::duration(static_cast<time::time_val_type>(val), time::NANOSECOND);
        }

        /**
         * @brief Creates microsecond duration from integer literal
         * @param val Value in microseconds
         * @return Duration object
         */
        inline time::duration operator""_us(const unsigned long long val) {
            return time::duration(static_cast<time::time_val_type>(val), time::MICROSECOND);
        }

        /**
         * @brief Creates millisecond duration from integer literal
         * @param val Value in milliseconds
         * @return Duration object
         */
        inline time::duration operator""_ms(const unsigned long long val) {
            return time::duration(static_cast<time::time_val_type>(val), time::MILLISECOND);
        }

        /**
         * @brief Creates second duration from integer literal
         * @param val Value in seconds
         * @return Duration object
         */
        inline time::duration operator""_s(const unsigned long long val) {
            return time::duration(static_cast<time::time_val_type>(val), time::SECOND);
        }

        /**
         * @brief Creates minute duration from integer literal
         * @param val Value in minutes
         * @return Duration object
         */
        inline time::duration operator""_min(const unsigned long long val) {
            return time::duration(static_cast<time::time_val_type>(val), time::MINUTE);
        }

        /**
         * @brief Creates hour duration from integer literal
         * @param val Value in hours
         * @return Duration object
         */
        inline time::duration operator""_h(const unsigned long long val) {
            return time::duration(static_cast<time::time_val_type>(val), time::HOUR);
        }

        /**
         * @brief Creates day duration from integer literal
         * @param val Value in days
         * @return Duration object
         */
        inline time::duration operator""_d(const unsigned long long val) {
            return time::duration(static_cast<time::time_val_type>(val), time::DAY);
        }

        /**
         * @brief Creates nanosecond duration from floating-point literal
         * @param val Value in nanoseconds
         * @return Duration object
         */
        inline time::duration operator""_ns(const long double val) {
            return time::duration(std::llround(val * time::FACTOR_NANOSECOND), time::NANOSECOND);
        }

        /**
         * @brief Creates microsecond duration from floating-point literal
         * @param val Value in microseconds
         * @return Duration object
         */
        inline time::duration operator""_us(const long double val) {
            return time::duration(std::llround(val * time::FACTOR_MICROSECOND), time::NANOSECOND);
        }

        /**
         * @brief Creates millisecond duration from floating-point literal
         * @param val Value in milliseconds
         * @return Duration object
         */
        inline time::duration operator""_ms(const long double val) {
            return time::duration(std::llround(val * time::FACTOR_MILLISECOND), time::NANOSECOND);
        }

        /**
         * @brief Creates second duration from floating-point literal
         * @param val Value in seconds
         * @return Duration object
         */
        inline time::duration operator""_s(const long double val) {
            return time::duration(std::llround(val * time::FACTOR_SECOND), time::NANOSECOND);
        }

        /**
         * @brief Creates minute duration from floating-point literal
         * @param val Value in minutes
         * @return Duration object
         */
        inline time::duration operator""_min(const long double val) {
            return time::duration(std::llround(val * time::FACTOR_MINUTE), time::NANOSECOND);
        }

        /**
         * @brief Creates hour duration from floating-point literal
         * @param val Value in hours
         * @return Duration object
         */
        inline time::duration operator""_h(const long double val) {
            return time::duration(std::llround(val * time::FACTOR_HOUR), time::NANOSECOND);
        }

        /**
         * @brief Creates day duration from floating-point literal
         * @param val Value in days
         * @return Duration object
         */
        inline time::duration operator""_d(const long double val) {
            return time::duration(std::llround(val * time::FACTOR_DAY), time::NANOSECOND);
        }
    }
}

inline original::time::duration::duration(const time_val_type val, const unit unit) {
    switch (unit) {
        case unit::DAY:
            this->nano_seconds_ = FACTOR_DAY * val;
            break;
        case unit::HOUR:
            this->nano_seconds_ = FACTOR_HOUR * val;
            break;
        case unit::MINUTE:
            this->nano_seconds_ = FACTOR_MINUTE * val;
            break;
        case unit::SECOND:
            this->nano_seconds_ = FACTOR_SECOND * val;
            break;
        case unit::MILLISECOND:
            this->nano_seconds_ = FACTOR_MILLISECOND * val;
            break;
        case unit::MICROSECOND:
            this->nano_seconds_ = FACTOR_MICROSECOND * val;
            break;
        case unit::NANOSECOND:
            this->nano_seconds_ = FACTOR_NANOSECOND * val;
            break;
        default:
            throw valueError("Invalid time unit specified for duration construction");
    }
}

#if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG
inline original::time::duration::duration(const timespec& ts)
    : nano_seconds_(ts.tv_sec * FACTOR_SECOND + ts.tv_nsec) {}
#elif ORIGINAL_COMPILER_MSVC
inline original::time::duration::duration(const DWORD milliseconds)
    : nano_seconds_(static_cast<time_val_type>(milliseconds) * FACTOR_MILLISECOND) {}
#endif

inline original::time::duration::duration(duration&& other) noexcept : duration() {
    this->operator=(std::move(other));
}

inline original::time::duration&
original::time::duration::operator=(duration&& other) noexcept {
    if (this == &other)
        return *this;

    this->nano_seconds_ = other.nano_seconds_;
    other.nano_seconds_ = 0;
    return *this;
}

inline original::time::time_val_type
original::time::duration::value(const unit unit) const {
    time_val_type val = this->nano_seconds_;
    switch (unit) {
        case unit::DAY:
            val /= FACTOR_DAY;
            break;
        case unit::HOUR:
            val /= FACTOR_HOUR;
            break;
        case unit::MINUTE:
            val /= FACTOR_MINUTE;
            break;
        case unit::SECOND:
            val /= FACTOR_SECOND;
            break;
        case unit::MILLISECOND:
            val /= FACTOR_MILLISECOND;
            break;
        case unit::MICROSECOND:
            val /= FACTOR_MICROSECOND;
            break;
        case unit::NANOSECOND:
            val /= FACTOR_NANOSECOND;
            break;
        default:
            throw valueError("Invalid time unit specified for duration value conversion");
    }
    return val;
}

inline original::integer
original::time::duration::compareTo(const duration& other) const {
    if (this->nano_seconds_ == other.nano_seconds_)
        return 0;
    return this->nano_seconds_ > other.nano_seconds_ ? 1 : -1;
}

inline original::u_integer
original::time::duration::toHash() const noexcept {
    return hash<time_val_type>::hashFunc(this->nano_seconds_);
}

inline std::string original::time::duration::className() const {
    return "time::duration";
}

inline std::string original::time::duration::toString(const bool enter) const {
    std::stringstream ss;
    ss << "(" << this->className() << " " << this->nano_seconds_ << "ns)";
    if (enter)
        ss << "\n";
    return ss.str();
}

#if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG
inline original::time::duration::operator timespec() const
{
    auto nanoseconds = this->value(NANOSECOND);
    const auto seconds = nanoseconds / FACTOR_SECOND;
    nanoseconds %= FACTOR_SECOND;
    return timespec{seconds, static_cast<long>(nanoseconds)};  //NOLINT: The remaining nanosecond value is within the range of type long.
}

inline timespec original::time::duration::toTimespec() const
{
    return static_cast<timespec>(*this);
}
#elif ORIGINAL_COMPILER_MSVC
inline original::time::duration::operator DWORD() const
{
    return static_cast<DWORD>(this->value());
}

inline DWORD original::time::duration::toDWMilliseconds() const
{
    return static_cast<DWORD>(*this);
}
#endif

inline original::time::duration&
original::time::duration::operator++() {
    this->nano_seconds_ += 1;
    return *this;
}

inline original::time::duration
original::time::duration::operator++(int) {
    duration res{*this};
    this->nano_seconds_ += 1;
    return res;
}

inline original::time::duration&
original::time::duration::operator--() {
    this->nano_seconds_ -= 1;
    return *this;
}

inline original::time::duration
original::time::duration::operator--(int) {
    duration res{*this};
    this->nano_seconds_ -= 1;
    return res;
}

inline original::time::duration&
original::time::duration::operator+=(const duration& other) {
    this->nano_seconds_ += other.nano_seconds_;
    return *this;
}

inline original::time::duration&
original::time::duration::operator-=(const duration& other) {
    this->nano_seconds_ -= other.nano_seconds_;
    return *this;
}

inline original::time::duration&
original::time::duration::operator*=(const time_val_type factor) {
    this->nano_seconds_ *= factor;
    return *this;
}

inline original::time::duration&
original::time::duration::operator/=(const time_val_type factor) {
    this->nano_seconds_ /= factor;
    return *this;
}

inline original::time::duration&
original::time::duration::operator/=(const duration& other) {
    this->nano_seconds_ /= other.nano_seconds_;
    return *this;
}

inline original::floating
original::time::duration::div(const time_val_type factor, const unit unit) const {
    return this->div(duration{factor, unit});
}

inline original::floating
original::time::duration::div(const duration& other) const {
    return static_cast<floating>(this->nano_seconds_) / static_cast<floating>(other.nano_seconds_);
}

inline original::time::duration
original::nanoseconds(const time::time_val_type val) {
    return time::duration{val, time::NANOSECOND};
}

inline original::time::duration
original::microseconds(const time::time_val_type val) {
    return time::duration{val, time::MICROSECOND};
}

inline original::time::duration
original::milliseconds(const time::time_val_type val) {
    return time::duration{val, time::MILLISECOND};
}

inline original::time::duration
original::seconds(const time::time_val_type val) {
    return time::duration{val, time::SECOND};
}

inline original::time::duration
original::minutes(const time::time_val_type val) {
    return time::duration{val, time::MINUTE};
}

inline original::time::duration
original::hours(const time::time_val_type val) {
    return time::duration{val, time::HOUR};
}

inline original::time::duration
original::days(const time::time_val_type val) {
    return time::duration{val, time::DAY};
}

inline original::time::duration
original::operator-(const time::duration& d) {
    time::duration res{d};
    res.nano_seconds_ = -res.nano_seconds_;
    return res;
}

inline original::time::duration
original::operator+(const time::duration& lhs, const time::duration& rhs) {
    time::duration d{lhs};
    d += rhs;
    return d;
}

inline original::time::duration
original::operator-(const time::duration& lhs, const time::duration& rhs) {
    time::duration d{lhs};
    d -= rhs;
    return d;
}

inline original::time::duration
original::operator*(const time::duration &d, const time::time_val_type factor) {
    time::duration res{d};
    res *= factor;
    return res;
}

inline original::time::duration
original::operator*(const time::time_val_type factor, const time::duration& d) {
    return d * factor;
}

inline original::time::duration
original::operator/(const time::duration &d, const time::time_val_type factor) {
    time::duration res{d};
    res /= factor;
    return res;
}

inline original::time::duration
original::operator/(const time::duration& lhs, const time::duration& rhs) {
    time::duration d{lhs};
    d /= rhs;
    return d;
}

inline original::time::duration
original::abs(const time::duration& d) {
    time::time_val_type val = d.nano_seconds_;
    val = val < 0 ? -val : val;
    return time::duration{val, time::NANOSECOND};
}

inline original::time::point
original::time::point::now() {
#if ORIGINAL_COMPILER_GCC || (ORIGINAL_COMPILER_CLANG && ORIGINAL_PLATFORM_LINUX)
    timespec ts{};
    clock_gettime(CLOCK_REALTIME, &ts);
    return point{ts};
#elif ORIGINAL_PLATFORM_APPLE
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    time_val_type ns = tv.tv_sec * FACTOR_SECOND + tv.tv_usec * FACTOR_MICROSECOND;
    return point(ns, NANOSECOND);
#elif ORIGINAL_COMPILER_MSVC
    // Other implements not complete
    return point();
#endif
}

inline original::time::point::point(const time_val_type val, const unit unit)
    : nano_since_epoch_(val, unit) {}

inline original::time::point::point(duration d)
    : nano_since_epoch_(std::move(d)) {}

#if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG
inline original::time::point::point(const timespec& ts)
    : nano_since_epoch_(ts) {}
#elif ORIGINAL_COMPILER_MSVC
inline original::time::point::point(const DWORD milliseconds)
    : nano_since_epoch_(milliseconds) {}
#endif

inline original::time::time_val_type
original::time::point::value(const unit unit) const noexcept {
    return this->nano_since_epoch_.value(unit);
}

inline original::integer
original::time::point::compareTo(const point& other) const {
    return this->nano_since_epoch_.compareTo(other.nano_since_epoch_);
}

inline original::u_integer
original::time::point::toHash() const noexcept {
    return this->nano_since_epoch_.toHash();
}

inline std::string original::time::point::className() const {
    return "time::point";
}

inline std::string
original::time::point::toString(const bool enter) const {
    std::stringstream ss;
    ss << "(" << this->className() << " " << this->nano_since_epoch_.nano_seconds_ << ")";
    if (enter)
        ss << "\n";
    return ss.str();
}

#if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG
inline original::time::point::operator timespec() const
{
    return static_cast<timespec>(this->nano_since_epoch_);
}

inline timespec original::time::point::toTimespec() const
{
    return static_cast<timespec>(*this);
}
#elif ORIGINAL_COMPILER_MSVC
inline original::time::point::operator DWORD() const
{
    return static_cast<DWORD>(this->nano_since_epoch_);
}

inline DWORD original::time::point::toDWMilliseconds() const
{
    return static_cast<DWORD>(*this);
}
#endif

inline original::time::point&
original::time::point::operator++() {
    ++this->nano_since_epoch_;
    return *this;
}

inline original::time::point
original::time::point::operator++(int) {
    point res{*this};
    ++this->nano_since_epoch_;
    return res;
}

inline original::time::point&
original::time::point::operator--() {
    --this->nano_since_epoch_;
    return *this;
}

inline original::time::point
original::time::point::operator--(int) {
    point res{*this};
    --this->nano_since_epoch_;
    return res;
}

inline original::time::point&
original::time::point::operator+=(const duration& d) {
    this->nano_since_epoch_ += d;
    return *this;
}

inline original::time::point&
original::time::point::operator-=(const duration& d) {
    this->nano_since_epoch_ -= d;
    return *this;
}

inline original::time::point
original::operator-(const time::point &p, const time::duration &d) {
    time::point res{p};
    res -= d;
    return res;
}

inline original::time::duration
original::operator-(const time::point &lhs, const time::point &rhs) {
    return lhs.nano_since_epoch_ - rhs.nano_since_epoch_;
}

inline original::time::point
original::operator+(const time::point &p, const time::duration &d) {
    time::point res{p};
    res += d;
    return res;
}

inline original::time::point
original::operator+(const time::duration &d, const time::point &p) {
    return p + d;
}

constexpr bool
original::time::UTCTime::isValidYear(const integer year) {
    return 0 <= year;
}

constexpr bool
original::time::UTCTime::isValidMonth(const integer month) {
    return 1 <= month && month <= 12;
}

constexpr bool
original::time::UTCTime::isValidDay(const integer day) {
    return 1 <= day && day <= 31;
}

inline void original::time::UTCTime::set(const integer year, const integer month, const integer day,
                                         const integer hour, const integer minute, const integer second) {
    this->year_ = year;
    this->month_ = month;
    this->day_ = day;
    this->hour_ = hour;
    this->minute_ = minute;
    this->second_ = second;
}

inline original::time::UTCTime
original::time::UTCTime::now() {
    return UTCTime{point::now()};
}

inline original::integer
original::time::UTCTime::localZonedOffset() {
    integer offset_seconds = 0;
#if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG
    integer t = std::time(nullptr);
    tm local_tm{};
#if ORIGINAL_PLATFORM_WINDOWS
    localtime_s(&local_tm, &t);
#else
    localtime_r(&t, &local_tm);
#endif

    tm utc_tm{};
#if ORIGINAL_PLATFORM_WINDOWS
    gmtime_s(&utc_tm, &t);
#else
    gmtime_r(&t, &utc_tm);
#endif

    offset_seconds = static_cast<integer>(difftime(mktime(&local_tm), mktime(&utc_tm)));
#else
    TIME_ZONE_INFORMATION tz_info;
    DWORD result = GetTimeZoneInformation(&tz_info);
    if (result != TIME_ZONE_ID_INVALID) {
        offset_seconds = -tz_info.Bias * (FACTOR_MINUTE / FACTOR_SECOND);
        if (result == TIME_ZONE_ID_DAYLIGHT) {
            offset_seconds -= tz_info.DaylightBias * (FACTOR_MINUTE / FACTOR_SECOND);
        }
    }
#endif
    return offset_seconds / (FACTOR_HOUR / FACTOR_SECOND);
}

inline original::time::UTCTime
original::time::UTCTime::localNow() {
    return now() + duration{localZonedOffset(), HOUR};
}

constexpr bool
original::time::UTCTime::isLeapYear(const integer year) {
    if (!isValidYear(year))
        return false;
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

constexpr original::integer
original::time::UTCTime::daysOfMonth(const integer year, const integer month) {
    if (!isValidYear(year) || !isValidMonth(month))
        throw valueError();

    if (month == 2 && isLeapYear(year)){
        return DAYS_OF_MONTH[month - 1] + 1;
    }
    return DAYS_OF_MONTH[month - 1];
}

constexpr original::time::UTCTime::weekdays
original::time::UTCTime::weekday(const integer year, const integer month, const integer day) {
    if (!isValidYMD(year, month, day))
        throw valueError("Invalid date for weekday calculation: " +
                        formatString(year) + "-" + formatString(month) + "-" + formatString(day));

    integer corrected_year, corrected_month;
    if (month == 1 || month == 2){
        corrected_year = year - 1;
        corrected_month = MONTHS_YEAR + month;
    } else {
        corrected_year = year;
        corrected_month = month;
    }
    const integer century = corrected_year / YEARS_CENTURY;
    const integer years_in_century = corrected_year % YEARS_CENTURY;

    return static_cast<weekdays>((
               day +
               13 * (corrected_month + 1) / 5 +
               years_in_century +
               years_in_century / 4 +
               century / 4 +
               5 * century
           ) % DAYS_WEEK);
}

constexpr bool
original::time::UTCTime::isValidYMD(const integer year, const integer month, const integer day) {
    if (!isValidYear(year) || !isValidMonth(month) || !isValidDay(day))
        return false;

    return day <= daysOfMonth(year, month);
}

constexpr bool
original::time::UTCTime::isValidHMS(const integer hour, const integer minute, const integer second) {
    return 0 <= hour && hour <= 23 &&
           0 <= minute && minute <= 59 &&
           0 <= second && second <= 59;
}

constexpr bool
original::time::UTCTime::isValid(const integer year, const integer month, const integer day,
                                 const integer hour, const integer minute, const integer second) {
    return isValidYMD(year, month, day) && isValidHMS(hour, minute, second);
}

inline bool original::time::UTCTime::isLeapYear() const {
    return isLeapYear(this->year_);
}

inline original::time::UTCTime::weekdays
original::time::UTCTime::weekday() const {
    return weekday(this->year_, this->month_, this->day_);
}

inline original::time::UTCTime::UTCTime()
    : UTCTime(EPOCH_YEAR, EPOCH_MONTH, EPOCH_DAY, 0, 0, 0) {}

inline original::time::UTCTime::UTCTime(const integer year, const integer month, const integer day)
    : UTCTime(year, month, day, 0, 0, 0) {}

inline original::time::UTCTime::UTCTime(const integer year, const integer month, const integer day,
                                        const integer hour, const integer minute, const integer second) {
    if (!isValid(year, month, day, hour, minute, second))
        throw valueError("Invalid UTC time parameters: " +
                        formatString(year) + "-" + formatString(month) + "-" + formatString(day) + " " +
                        formatString(hour) + ":" + formatString(minute) + ":" + formatString(second));

    this->set(year, month, day, hour, minute, second);
}

inline original::time::UTCTime::UTCTime(const point& p) {
    const auto nano_seconds = p.value(NANOSECOND);

    integer second = nano_seconds / FACTOR_SECOND;

    integer minute = second / (FACTOR_MINUTE / FACTOR_SECOND);
    second %= (FACTOR_MINUTE / FACTOR_SECOND);

    integer hour = minute / (FACTOR_HOUR / FACTOR_MINUTE);
    minute %= (FACTOR_HOUR / FACTOR_MINUTE);

    integer day = hour / (FACTOR_DAY / FACTOR_HOUR);
    hour %= (FACTOR_DAY / FACTOR_HOUR);

    integer year = EPOCH_YEAR;
    while (true){
        const integer days_in_year = isLeapYear(year) ? DAYS_LEAP_YEAR : DAYS_COMMON_YEAR;
        if (day < days_in_year){
            break;
        }

        day -= days_in_year;
        year += 1;
    }

    integer month = 1;
    while (true){
        const integer dim = daysOfMonth(year, month);
        if (day < dim){
            break;
        }

        day -= dim;
        month += 1;
    }

    day += 1;

    this->set(year, month, day, hour, minute, second);
}

inline original::time::UTCTime::UTCTime(UTCTime&& other) noexcept : UTCTime() {
    this->operator=(std::move(other));
}

inline original::time::UTCTime&
original::time::UTCTime::operator=(UTCTime&& other) noexcept {
    if (this == &other)
        return *this;

    this->set(other.year_, other.month_, other.day_,
              other.hour_, other.minute_, other.second_);
    other.set(EPOCH_YEAR, EPOCH_MONTH, EPOCH_DAY, 0, 0, 0);
    return *this;
}

inline original::time::UTCTime original::time::UTCTime::date() const
{
    return UTCTime{this->year_, this->month_, this->day_};
}

inline original::integer
original::time::UTCTime::value(const unit unit) const {
    switch (unit) {
        case SECOND:
            return this->second_;
        case MINUTE:
            return this->minute_;
        case HOUR:
            return this->hour_;
        case DAY:
            return this->day_;
        default:
            throw valueError("Invalid time unit for UTCTime value access");
    }
}

inline original::integer
original::time::UTCTime::value(const calendar calendar) const {
    switch (calendar) {
        case MONTH:
            return this->month_;
        case YEAR:
            return this->year_;
        default:
            throw valueError("Invalid calendar component for UTCTime value access");
    }
}

inline original::time::UTCTime::operator point() const {
    time_val_type total_days = 0;

    for (integer year = EPOCH_YEAR; year < this->year_; ++year) {
        total_days += isLeapYear(year) ? DAYS_LEAP_YEAR : DAYS_COMMON_YEAR;
    }

    for (integer month = 1; month < this->month_; ++month) {
        total_days += daysOfMonth(this->year_, month);
    }

    total_days += this->day_ - 1;

    time_val_type total_seconds = total_days * (FACTOR_DAY / FACTOR_SECOND);
    total_seconds += this->hour_ * (FACTOR_HOUR / FACTOR_SECOND);
    total_seconds += this->minute_ * (FACTOR_MINUTE / FACTOR_SECOND);
    total_seconds += this->second_;

    return point{total_seconds, SECOND};
}

inline original::time::point original::time::UTCTime::toPoint() const
{
    return static_cast<point>(*this);
}

inline original::integer
original::time::UTCTime::compareTo(const UTCTime& other) const {
    if (this->year_ != other.year_)
        return this->year_ > other.year_ ? 1 : -1;
    if (this->month_ != other.month_)
        return this->month_ > other.month_ ? 1 : -1;
    if (this->day_ != other.day_)
        return this->day_ > other.day_ ? 1 : -1;
    if (this->hour_ != other.hour_)
        return this->hour_ > other.hour_ ? 1 : -1;
    if (this->minute_ != other.minute_)
        return this->minute_ > other.minute_ ? 1 : -1;
    if (this->second_ != other.second_)
        return this->second_ > other.second_ ? 1 : -1;
    return 0;
}

inline original::u_integer
original::time::UTCTime::toHash() const noexcept {
    u_integer seed = 0;
    hash<UTCTime>::hashCombine(seed, this->year_, this->month_, this->day_,
                               this-> hour_, this->minute_, this->second_);
    return seed;
}

inline std::string original::time::UTCTime::className() const {
    return "time::UTCTime";
}

inline std::string original::time::UTCTime::toString(const bool enter) const {
    std::stringstream ss;
    ss << "(" << this->className() << " "
       << this->year_ << "-"
       << std::setw(2) << std::setfill('0') << this->month_ << "-"
       << std::setw(2) << std::setfill('0') << this->day_ << " "
       << std::setw(2) << std::setfill('0') << this->hour_ << ":"
       << std::setw(2) << std::setfill('0') << this->minute_ << ":"
       << std::setw(2) << std::setfill('0') << this->second_ << ")";
    if (enter)
        ss << "\n";
    return ss.str();
}

inline original::time::UTCTime
original::operator-(const time::UTCTime& t, const time::duration& d) {
    return time::UTCTime{static_cast<time::point>(t) - d};
}

inline original::time::duration
original::operator-(const time::UTCTime& lhs, const time::UTCTime& rhs) {
    return static_cast<time::point>(lhs) - static_cast<time::point>(rhs);
}

inline original::time::UTCTime
original::operator+(const time::UTCTime& t, const time::duration& d) {
    return time::UTCTime{static_cast<time::point>(t) + d};
}

inline original::time::UTCTime
original::operator+(const time::duration &d, const time::UTCTime &t) {
    return t + d;
}

#endif //ORIGINAL_ZEIT_H
