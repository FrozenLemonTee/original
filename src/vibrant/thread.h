#ifndef THREAD_H
#define THREAD_H

#include "config.h"

#if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG
#include "pthread.h"
#elif ORIGINAL_COMPILER_MSVC
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#endif

#include "error.h"
#include "functional"
#include "hash.h"
#include "ownerPtr.h"
#include "zeit.h"


/**
 * @file thread.h
 * @brief Thread management utilities
 * @details Provides a layered threading abstraction with:
 * - Low-level POSIX thread wrapper (pThread) for GCC/Clang
 * - Low-level Windows thread wrapper (wThread) for MSVC
 * - High-level RAII thread management (thread)
 * - Exception-safe thread operations
 * - Flexible join/detach policies
 * - Cross-platform thread operations (sleep, ID retrieval)
 *
 * Platform Support:
 * - GCC/Clang: Uses pthread API (pThread implementation)
 * - MSVC: Uses Windows Thread API (wThread implementation)
 * - All platforms: High-level thread class with consistent interface
 */

namespace original {
    /**
     * @class threadBase
     * @brief Base class for thread implementations
     * @details Provides common thread functionality and interface
     *
     * Key Features:
     * - Manages thread joinable state
     * - Provides basic thread validity checks
     * - Non-copyable but movable
     * - Serves as base for both pThread and thread implementations
     *
     * @note This is an abstract base class and cannot be instantiated directly
     */
    template <typename DERIVED>
    class threadBase : public comparable<DERIVED>,
                       public hashable<DERIVED>,
                       public printable {
    protected:
        /**
         * @class threadData
         * @tparam Callback Type of the thread callback function
         * @brief Wrapper for thread execution data
         * @details Handles the storage and execution of thread callbacks,
         * including exception handling and resource management.
         */
        template<typename Callback>
        class threadData
        {
            Callback c;
        public:
            /**
             * @brief Construct thread data wrapper
             * @param c Callback to store
             * @post The callback is moved into the threadData object
             */
            explicit threadData(Callback c);

            /**
             * @brief Thread entry point wrapper
             * @param arg Pointer to threadData instance
             * @return Always nullptr
             * @throw sysError if callback throws an error
             * @note This static method serves as the bridge between C-style pthread
             *       callbacks and C++ callable objects.
             */
            static void* run(void* arg);
        };

        /**
         * @brief Check if thread is valid
         * @return true if thread is valid (has an associated execution context)
         * @note Pure virtual function to be implemented by derived classes
         */
        [[nodiscard]] virtual bool valid() const = 0;

        /**
         * @brief Get thread identifier
         * @return Unique identifier for the thread
         * @note Pure virtual function to be implemented by derived classes
         */
        [[nodiscard]] virtual ul_integer id() const = 0;
    public:

        /**
         * @brief Default constructor
         * @note Creates an invalid thread object
         */
        explicit threadBase() noexcept = default;

        /**
         * @brief Destructor
         * @note Terminates program if thread is joinable and not joined/detached
         */
        ~threadBase() noexcept override = default;

        threadBase(const threadBase&) = delete; ///< Deleted copy constructor
        threadBase& operator=(const threadBase&) = delete; ///< Deleted copy assignment

        threadBase(threadBase&& other) noexcept = default; ///< Default move constructor
        threadBase& operator=(threadBase&& other) noexcept = default; ///< Default move assignment

        /**
         * @brief Check if thread is valid
         * @return true if thread is valid
         */
        explicit operator bool() const;

        /**
         * @brief Check if thread is not valid
         * @return true if thread is not valid
         */
        bool operator!() const;

        /**
         * @brief Check if thread is joinable
         * @return true if thread is joinable
         * @note A thread is joinable if it represents an active thread of execution
         * @note Pure virtual function to be implemented by derived classes
         */
        [[nodiscard]] virtual bool joinable() const = 0;

        /**
         * @brief Wait for thread to complete execution
         * @throw sysError if join operation fails
         * @note Blocks the calling thread until this thread completes
         */
        virtual void join() = 0;

        /**
         * @brief Detach thread from handle
         * @throw sysError if detach operation fails
         * @note Allows thread to execute independently, resources automatically
         *       released when thread completes
         */
        virtual void detach() = 0;

        std::string className() const override;

        std::string toString(bool enter) const override;
    };

#if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG
    /**
     * @class pThread
     * @brief POSIX thread implementation for GCC and Clang compilers
     * @details Wrapper around pthread with RAII semantics. Provides low-level
     *          thread management using POSIX threads API.
     *
     * Platform-Specific Features:
     * - Uses pthread_create for thread creation
     * - Implements pthread_join for thread synchronization
     * - Uses pthread_detach for resource cleanup
     * - Provides pthread_t based thread identification
     *
     * @note This class is not thread-safe for concurrent operations on the same object
     * @note Implements the threadBase interface for POSIX threads
     * @note Only available when compiled with GCC or Clang
     *
     * Example usage:
     * @code
     * original::pThread thread([](){
     *     // Thread work
     * });
     * thread.join();
     * @endcode
     */
    class pThread final : public threadBase<pThread> {
        pthread_t handle; ///< Native thread handle
        bool is_joinable; ///< Flag indicating if thread can be joined

        /**
         * @brief Check if thread is valid
         * @return true if thread handle is valid
         */
        [[nodiscard]] bool valid() const override;
    public:
        /**
         * @brief Construct empty (invalid) thread
         * @post Creates a thread object not associated with any execution
         */
        explicit pThread();

        /**
         * @brief Construct and start POSIX thread
         * @tparam Callback Callback function type
         * @tparam ARGS Argument types for callback
         * @param c Callback function to execute in new thread
         * @param args Arguments to forward to callback
         * @throw sysError if thread creation fails
         * @post New thread starts executing the callback with provided arguments
         */
        template<typename Callback, typename... ARGS>
        explicit pThread(Callback c, ARGS&&... args);

        /**
         * @brief Move constructor
         * @param other Thread to move from
         * @post Source thread becomes invalid
         */
        pThread(pThread&& other) noexcept;

        /**
         * @brief Move assignment
         * @param other Thread to move from
         * @return Reference to this object
         * @post Source thread becomes invalid
         */
        pThread& operator=(pThread&& other) noexcept;

        /**
         * @brief Get thread identifier
         * @return Unique identifier for the thread
         */
        [[nodiscard]] ul_integer id() const override;

        /**
         * @brief Check if thread is joinable
         * @return true if thread is joinable
         * @note Implementation of threadBase::joinable()
         */
        [[nodiscard]] bool joinable() const override;

        integer compareTo(const pThread &other) const override;

        u_integer toHash() const noexcept override;

        std::string className() const override;

        /**
         * @brief Wait for thread to complete
         * @note Terminates program if join fails
         * @note Blocks until the thread completes execution
         */
        void join() override;

        /**
         * @brief Detach thread (allow it to run independently)
         * @note Terminates program if detach fails
         * @note After detach, the thread object no longer represents the thread
         */
        void detach() override;

        /**
         * @brief Destructor
         * @note Terminates program if thread is joinable and not joined/detached
         */
        ~pThread() override;
    };
#elif ORIGINAL_COMPILER_MSVC
    /**
     * @class wThread
     * @brief Windows thread implementation for MSVC compiler
     * @details Wrapper around Windows Thread API with RAII semantics. Provides
     *          low-level thread management using Windows threading primitives.
     *
     * Platform-Specific Features:
     * - Uses CreateThread for thread creation
     * - Implements WaitForSingleObject for thread synchronization
     * - Uses CloseHandle for resource cleanup
     * - Provides HANDLE based thread identification
     *
     * @note This class is not thread-safe for concurrent operations on the same object
     * @note Implements the threadBase interface for Windows threads
     * @note Only available when compiled with MSVC
     *
     * Example usage:
     * @code
     * original::wThread thread([](){
     *     // Thread work
     * });
     * thread.join();
     * @endcode
     */
    class wThread final : public threadBase<wThread> {
        HANDLE handle;      ///< Windows thread handle
        bool is_joinable;   ///< Flag indicating if thread can be joined

        /**
         * @brief Check if thread is valid
         * @return true if thread handle is valid (not NULL)
         * @note Windows-specific validity check
         */
        [[nodiscard]] bool valid() const override;

    public:
        /**
         * @brief Construct empty (invalid) thread
         * @post Creates a thread object not associated with any execution
         * @note Windows-specific initialization
         */
        explicit wThread();

        /**
         * @brief Construct and start Windows thread
         * @tparam Callback Callback function type
         * @tparam ARGS Argument types for callback
         * @param c Callback function to execute in new thread
         * @param args Arguments to forward to callback
         * @throw sysError if thread creation fails
         * @post New thread starts executing the callback with provided arguments
         * @note Uses CreateThread Windows API internally
         */
        template<typename Callback, typename... ARGS>
        explicit wThread(Callback c, ARGS&&... args);

        /**
         * @brief Move constructor
         * @param other Thread to move from
         * @post Source thread becomes invalid, ownership transferred
         * @note Windows-specific handle transfer
         */
        wThread(wThread&& other) noexcept;

        /**
         * @brief Move assignment
         * @param other Thread to move from
         * @return Reference to this object
         * @post Source thread becomes invalid, ownership transferred
         * @note Windows-specific handle transfer and cleanup
         */
        wThread& operator=(wThread&& other) noexcept;

        /**
         * @brief Get thread identifier
         * @return Unique identifier for the thread (based on HANDLE)
         * @note Windows-specific thread identification
         */
        [[nodiscard]] ul_integer id() const override;

        /**
         * @brief Check if thread is joinable
         * @return true if thread is joinable
         * @note Windows-specific joinable state check
         */
        [[nodiscard]] bool joinable() const override;

        /**
         * @brief Compare this thread with another
         * @param other Thread to compare with
         * @return Comparison result based on thread IDs
         * @note Windows-specific thread comparison
         */
        integer compareTo(const wThread &other) const override;

        /**
         * @brief Compute hash value for this thread
         * @return Hash value based on thread handle
         * @note Windows-specific hash computation
         */
        u_integer toHash() const noexcept override;

        /**
         * @brief Get class name
         * @return "wThread" string
         */
        std::string className() const override;

        /**
         * @brief Wait for thread to complete
         * @throw sysError if wait operation fails
         * @note Uses WaitForSingleObject with INFINITE timeout
         * @note Windows-specific thread synchronization
         */
        void join() override;

        /**
         * @brief Detach thread (allow it to run independently)
         * @throw sysError if detach operation fails
         * @note Uses CloseHandle to release thread resources
         * @note Windows-specific thread detachment
         */
        void detach() override;

        /**
         * @brief Destructor
         * @note Automatically detaches if thread is still joinable
         * @note Windows-specific resource cleanup
         */
        ~wThread() override;
    };
#endif

    /**
     * @class thread
     * @brief High-level cross-platform thread wrapper
     * @details Manages thread lifetime with automatic join/detach. Provides
     *          RAII semantics for thread management with configurable join policy.
     *          Automatically selects the appropriate underlying implementation:
     *          - pThread for GCC/Clang on Linux/macOS
     *          - wThread for MSVC on Windows
     *
     * Key Features:
     * - Wraps platform-specific thread implementation with unified interface
     * - Configurable join policy (AUTO_JOIN or AUTO_DETACH)
     * - Cross-platform thread operations (sleep, ID retrieval)
     * - Automatic cleanup based on join policy
     *
     * Platform Abstraction:
     * - GCC/Clang: Delegates to pThread (POSIX threads)
     * - MSVC: Delegates to wThread (Windows threads)
     * - All platforms: Consistent high-level interface
     *
     * Example usage:
     * @code
     * // Cross-platform thread creation
     * original::thread t([](){
     *     // thread work - same on all platforms
     * }, original::thread::AUTO_DETACH);
     * @endcode
     *
     * @see original::pThread (GCC/Clang)
     * @see original::wThread (MSVC)
     * @see original::threadBase
     */
    class thread final : public threadBase<thread> {
        #if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG
                pThread thread_; ///< POSIX thread implementation (GCC/Clang)
        #elif ORIGINAL_COMPILER_MSVC
                wThread thread_; ///< Windows thread implementation (MSVC)
        #endif
        bool will_join;  ///< Join policy flag

        /**
         * @enum joinPolicy
         * @brief Defines thread cleanup policy on destruction
         * @details Controls whether a thread should be joined or detached automatically
         * when the thread object is destroyed.
         *
         * @var AUTO_JOIN
         * The thread is joined in the destructor (blocking cleanup).
         *
         * @var AUTO_DETACH
         * The thread is detached in the destructor (non-blocking cleanup).
         */
        enum class joinPolicy {
            AUTO_JOIN,   ///< Join the thread automatically on destruction
            AUTO_DETACH, ///< Detach the thread automatically on destruction
        };

        [[nodiscard]] bool valid() const override;
    public:

        /**
         * @brief Get the current thread's identifier
         * @return Unique identifier for the current thread
         * @details Platform-specific implementation:
         * - GCC/Clang: Uses pthread_self() and converts to numeric ID
         * - MSVC: Uses GetCurrentThreadId() and converts to numeric ID
         * @note The returned ID format is platform-dependent but unique within process
         * @code
         * auto my_id = original::thread::thisId(); // Works on all platforms
         * @endcode
         */
        static ul_integer thisId();

        /**
         * @brief Puts the current thread to sleep for a specified duration
         * @param d Duration to sleep
         * @details Platform-specific implementation:
         * - GCC/Clang: Uses clock_nanosleep with CLOCK_REALTIME and handles EINTR
         * - MSVC: Uses Sleep() with millisecond precision
         * @note Features:
         * - Negative durations result in no sleep
         * - Handles EINTR interruptions automatically (POSIX)
         * - High precision sleep on POSIX, millisecond precision on Windows
         * @throw sysError if sleep operation fails on POSIX systems
         * @note Windows implementation does not throw exceptions for sleep failures
         * @code
         * // Sleep for 1 second - works on all platforms
         * original::thread::sleep(original::seconds(1));
         * @endcode
         */
        static inline void sleep(const time::duration& d);

        /// @brief Alias for joinPolicy::AUTO_JOIN
        static constexpr auto AUTO_JOIN = joinPolicy::AUTO_JOIN;

        /// @brief Alias for joinPolicy::AUTO_DETACH
        static constexpr auto AUTO_DETACH = joinPolicy::AUTO_DETACH;

        /**
         * @brief Construct empty thread
         * @post Creates an invalid thread object (not associated with any execution)
         */
        explicit thread();

        /**
         * @brief Construct and start thread with callback (AUTO_JOIN policy)
         * @tparam Callback Callable type
         * @tparam ARGS Argument types
         * @param c Callable to execute in thread
         * @param args Arguments forwarded to the callable
         * @post Thread starts and will be joined on destruction
         */
        template<typename Callback, typename... ARGS>
        explicit thread(Callback c, ARGS&&... args);

        /**
         * @brief Construct and start a thread with the given callback and join policy
         * @tparam Callback Type of the callable object
         * @tparam ARGS Types of arguments to pass to the callable
         * @param c Callable object to run in the new thread
         * @param policy Join policy (AUTO_JOIN or AUTO_DETACH)
         * @param args Arguments forwarded to the callable
         * @post Starts a new thread and applies the specified join policy
         *
         * @see joinPolicy
         */
        template<typename Callback, typename... ARGS>
        explicit thread(Callback c, joinPolicy policy, ARGS&&... args);

#if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG
        /**
         * @brief Construct a thread from an existing pThread with a join policy
         * @param p_thread The POSIX thread wrapper to take ownership of
         * @param policy Join policy (AUTO_JOIN or AUTO_DETACH)
         * @post Takes ownership of the thread and applies the specified join policy
         * @note Only available when compiled with GCC or Clang
         * @note Provides interoperability with low-level pThread objects
         */
        explicit thread(pThread p_thread, joinPolicy policy = AUTO_JOIN);
#elif ORIGINAL_COMPILER_MSVC
        /**
         * @brief Construct a thread from an existing wThread with a join policy
         * @param w_thread The Windows thread wrapper to take ownership of
         * @param policy Join policy (AUTO_JOIN or AUTO_DETACH)
         * @post Takes ownership of the thread and applies the specified join policy
         * @note Only available when compiled with MSVC
         * @note Provides interoperability with low-level wThread objects
         */
        explicit thread(wThread w_thread, joinPolicy policy = AUTO_JOIN);
#endif

        thread(const thread&) = delete; ///< Deleted copy constructor
        thread& operator=(const thread&) = delete; ///< Deleted copy assignment

        /**
         * @brief Move constructor (defaults to AUTO_JOIN)
         * @param other Thread to move from
         * @post Ownership transferred; will join on destruction
         */
        thread(thread&& other) noexcept;

        /**
         * @brief Move constructor with specified join policy
         * @param other Thread to move from
         * @param policy Join policy (AUTO_JOIN or AUTO_DETACH)
         * @post Ownership transferred and join behavior follows policy
         */
        thread(thread&& other, joinPolicy policy) noexcept;

        /**
         * @brief Move assignment
         * @param other Thread to move from
         * @return Reference to this object
         * @post Ownership and join policy transferred
         */
        thread& operator=(thread&& other) noexcept;

        /**
         * @brief Get thread identifier
         * @return Unique identifier for the thread
         */
        [[nodiscard]] ul_integer id() const override;

        /**
         * @brief Check if thread is joinable
         * @return true if thread is joinable
         * @note Implementation of threadBase::joinable()
         */
        [[nodiscard]] bool joinable() const override;

        [[nodiscard]] integer compareTo(const thread &other) const override;

        [[nodiscard]] u_integer toHash() const noexcept override;

        [[nodiscard]] std::string className() const override;

        /**
         * @brief Wait for thread to complete
         * @note Terminates program if join fails
         * @note Blocks until the thread completes execution
         */
        void join() override;

        /**
         * @brief Detach thread (allow it to run independently)
         * @note Terminates program if detach fails
         * @note After detach, the thread object no longer represents the thread
         */
        void detach() override;

        /**
         * @brief Destructor
         * @note Automatically joins or detaches based on will_join policy
         * @note Terminates program if join/detach operation fails
         */
        ~thread() override;
    };
}


template <typename DERIVED>
template <typename Callback>
original::threadBase<DERIVED>::threadData<Callback>::threadData(Callback c)
    : c(std::move(c)) {}

template <typename DERIVED>
template <typename Callback>
void* original::threadBase<DERIVED>::threadData<Callback>::run(void* arg)
{
    auto self = ownerPtr<threadData>(static_cast<threadData*>(arg));
    try {
        self->c();
    }catch (const error& e) {
        throw sysError("Thread callback execution failed with message: " + e.message());
    }
    return nullptr;
}

template <typename DERIVED>
original::threadBase<DERIVED>::operator bool() const
{
    return this->valid();
}

template <typename DERIVED>
bool original::threadBase<DERIVED>::operator!() const
{
    return !this->valid();
}

template<typename DERIVED>
std::string original::threadBase<DERIVED>::className() const {
    return "threadBase";
}

template<typename DERIVED>
std::string original::threadBase<DERIVED>::toString(bool enter) const {
    std::stringstream ss;
    ss << "(" << this->className() << " ";
    ss << "#" << this->id();
    ss << ")";
    if (enter)
        ss << "\n";
    return ss.str();
}
#if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG
inline original::pThread::pThread() : handle(), is_joinable() {}

template<typename Callback, typename... ARGS>
original::pThread::pThread(Callback c, ARGS&&... args) : handle(), is_joinable(true)
{
    auto bound_lambda =
    [func = std::forward<Callback>(c), ...lambda_args = std::forward<ARGS>(args)]() mutable {
        std::invoke(std::move(func), std::move(lambda_args)...);
    };

    using bound_callback = decltype(bound_lambda);
    using bound_thread_data = threadData<bound_callback>;

    auto task = new bound_thread_data(std::move(bound_lambda));

    if (const int code = pthread_create(&this->handle, nullptr, &bound_thread_data::run, task); code != 0)
    {
        delete task;
        throw sysError("Failed to create thread (pthread_create returned " + formatString(code) + ")");
    }
}

inline bool original::pThread::valid() const
{
    return this->handle != pthread_t{};
}

inline original::pThread::pThread(pThread&& other) noexcept
    : pThread() {
    this->operator=(std::move(other));
}

inline original::pThread& original::pThread::operator=(pThread&& other) noexcept
{
    if (this == &other) {
        return *this;
    }

    if (this->is_joinable && this->valid()) {
        pthread_detach(this->handle);
    }

    this->handle = other.handle;
    other.handle = {};
    this->is_joinable = other.is_joinable;
    other.is_joinable = false;
    return *this;
}

inline original::ul_integer original::pThread::id() const {
    ul_integer id = 0;
    std::memcpy(&id, &this->handle, sizeof(pthread_t));
    return id;
}

inline bool original::pThread::joinable() const
{
    return this->is_joinable;
}

inline original::integer
original::pThread::compareTo(const pThread& other) const {
    if (this->id() != other.id())
        return this->id() > other.id() ? 1 : -1;
    return 0;
}

inline original::u_integer
original::pThread::toHash() const noexcept {
    return hash<pThread>::hashFunc(this->id());
}

inline std::string original::pThread::className() const {
    return "pThread";
}

inline void original::pThread::join() {
    if (this->is_joinable){
        if (const int code = pthread_join(this->handle, nullptr);
            code != 0){
            throw sysError("Failed to join thread (pthread_join returned " + formatString(code) + ")");
        }
        this->is_joinable = false;
        this->handle = {};
    }
}

inline void original::pThread::detach() {
    if (this->is_joinable){
        if (const int code = pthread_detach(this->handle);
            code != 0){
            throw sysError("Failed to detach thread (pthread_detach returned " + formatString(code) + ")");
        }
        this->is_joinable = false;
        this->handle = {};
    }
}

inline original::pThread::~pThread()
{
    if (this->is_joinable) {
        try {
            this->detach();
        } catch (...) {
            std::cerr << "Fatal error in pThread destructor" << std::endl;
            std::terminate();
        }
    }
}
#elif ORIGINAL_COMPILER_MSVC
inline bool original::wThread::valid() const
{
    return this->handle != HANDLE{};
}

inline original::wThread::wThread() : handle(), is_joinable() {}

template <typename Callback, typename ... ARGS>
original::wThread::wThread(Callback c, ARGS&&... args) : handle(), is_joinable(true)
{
    auto bound_lambda =
    [func = std::forward<Callback>(c), ...lambda_args = std::forward<ARGS>(args)]() mutable {
        std::invoke(std::move(func), std::move(lambda_args)...);
    };

    using bound_callback = decltype(bound_lambda);
    using bound_thread_data = threadData<bound_callback>;

    auto task = new bound_thread_data(std::move(bound_lambda));

    static auto threadEntry = [](LPVOID param) -> DWORD {
        bound_thread_data::run(param);
        return 0;
    };

    this->handle = CreateThread(nullptr, 0, threadEntry, task, 0, nullptr);
    if (this->handle == nullptr) {
        delete task;
        throw sysError("Failed to create thread (CreateThread returned null)");
    }
}

inline original::wThread::wThread(wThread&& other) noexcept : wThread()
{
    this->operator=(std::move(other));
}

inline original::wThread& original::wThread::operator=(wThread&& other) noexcept
{
    if (this == &other) {
        return *this;
    }

    if (this->is_joinable && this->valid()) {
        CloseHandle(this->handle);
    }

    this->handle = other.handle;
    other.handle = {};
    this->is_joinable = other.is_joinable;
    other.is_joinable = false;
    return *this;
}

inline original::ul_integer original::wThread::id() const
{
    ul_integer id = 0;
    std::memcpy(&id, &this->handle, sizeof(HANDLE));
    return id;
}

inline bool original::wThread::joinable() const
{
    return this->is_joinable;
}

inline original::integer original::wThread::compareTo(const wThread& other) const
{
    if (this->id() != other.id())
        return this->id() > other.id() ? 1 : -1;
    return 0;
}

inline original::u_integer original::wThread::toHash() const noexcept
{
    return hash<HANDLE>::hashFunc(this->id());
}

inline std::string original::wThread::className() const
{
    return "wThread";
}

inline void original::wThread::join()
{
    WaitForSingleObject(this->handle, INFINITE);
    this->is_joinable = false;
    this->handle = {};
}

inline void original::wThread::detach()
{
    CloseHandle(this->handle);
    this->is_joinable = false;
    this->handle = {};
}

inline original::wThread::~wThread()
{
    if (this->is_joinable) {
        try {
            this->detach();
        } catch (...) {
            std::cerr << "Fatal error in wThread destructor" << std::endl;
            std::terminate();
        }
    }
}
#endif

inline bool original::thread::valid() const
{
    return this->thread_.operator bool();
}

inline original::ul_integer
original::thread::thisId() {
    ul_integer id = 0;
#if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG
    auto handle = pthread_self();
    std::memcpy(&id, &handle, sizeof(pthread_t));
#elif ORIGINAL_COMPILER_MSVC
    const auto handle = GetCurrentThreadId();
    std::memcpy(&id, &handle, sizeof(HANDLE));
#endif
    return id;
}

inline void original::thread::sleep(const time::duration& d)
{
    if (d.value() < 0)
        return;

#if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG
    const timespec ts = d.toTimespec();
    errno = 0;

    #if ORIGINAL_PLATFORM_MACOS || ORIGINAL_PLATFORM_WINDOWS
        timespec rem = ts;
        int code;

        do {
            code = nanosleep(&rem, &rem);
        } while (code == -1 && errno == EINTR);

        if (code != 0)
        throw sysError("Failed to sleep thread (nanosleep returned " +
                       formatString(code) + ", errno: " + formatString(errno) + ").");
    #endif

#elif ORIGINAL_PLATFORM_LINUX
    int code = clock_nanosleep(CLOCK_REALTIME, 0, &ts, nullptr);
    if (code != 0 && (errno == EINVAL || errno == ENOSYS)) {
        code = clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, nullptr);
    }
    if (code != 0)
        throw sysError("Failed to sleep thread (clock_nanosleep returned " +
                       formatString(code) + ", errno: " + formatString(errno) + ").");

#elif ORIGINAL_COMPILER_MSVC
    Sleep(d.toDWMilliseconds());
#endif

}

inline original::thread::thread()
    : will_join(true) {}

template <typename Callback, typename ... ARGS>
original::thread::thread(Callback c, ARGS&&... args)
    : thread_(std::forward<Callback>(c), std::forward<ARGS>(args)...), will_join(true) {}

template <typename Callback, typename ... ARGS>
original::thread::thread(Callback c, const joinPolicy policy, ARGS&&... args)
    : thread_(std::forward<Callback>(c), std::forward<ARGS>(args)...), will_join(policy == AUTO_JOIN) {}

#if ORIGINAL_COMPILER_GCC || ORIGINAL_COMPILER_CLANG
inline original::thread::thread(pThread p_thread, const joinPolicy policy)
    : thread_(std::move(p_thread)), will_join(policy == AUTO_JOIN) {}
#elif ORIGINAL_COMPILER_MSVC
inline original::thread::thread(wThread w_thread, const joinPolicy policy)
    : thread_(std::move(w_thread)), will_join(policy == AUTO_JOIN) {}
#endif

inline original::thread::thread(thread&& other) noexcept
    : thread_(std::move(other.thread_)), will_join(true) {}

inline original::thread::thread(thread&& other, const joinPolicy policy) noexcept
    : thread_(std::move(other.thread_)), will_join(policy == AUTO_JOIN) {}

inline original::thread& original::thread::operator=(thread&& other) noexcept {
    if (this == &other) {
        return *this;
    }

    this->thread_ = std::move(other.thread_);
    this->will_join = other.will_join;
    other.will_join = false;
    return *this;
}

inline original::integer
original::thread::compareTo(const thread& other) const {
    return this->thread_.compareTo(other.thread_);
}

inline original::u_integer
original::thread::toHash() const noexcept {
    return this->thread_.toHash();
}

inline std::string original::thread::className() const {
    return "thread";
}

inline void original::thread::join()
{
    this->thread_.join();
}

inline void original::thread::detach()
{
    this->thread_.detach();
}

inline original::thread::~thread()
{
    try {
        this->will_join ? this->thread_.join() : this->thread_.detach();
    } catch (const sysError& e) {
        std::cerr << "Fatal error in thread destructor: " << e.what() << std::endl;
        std::terminate();
    }
}

inline original::ul_integer original::thread::id() const {
    return this->thread_.id();
}

inline bool original::thread::joinable() const
{
    return this->thread_.joinable();
}

#endif //THREAD_H
