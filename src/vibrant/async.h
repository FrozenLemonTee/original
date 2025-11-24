#ifndef ORIGINAL_ASYNC_H
#define ORIGINAL_ASYNC_H

#include "atomic.h"
#include "condition.h"
#include "optional.h"
#include "refCntPtr.h"
#include "thread.h"
#include <exception>
#include <functional>
#include <utility>

namespace original {

    /**
     * @class async
     * @brief Asynchronous programming utilities with future/promise pattern
     * @details Provides a thread-safe implementation of the future/promise pattern
     * for asynchronous computation. Supports both value-returning and void functions.
     */
    class async {
        // ==================== Async Wrapper (Internal) ====================

        /**
         * @class asyncWrapper
         * @brief Internal wrapper for asynchronous result storage
         * @tparam TYPE The result type of the asynchronous computation
         * @details Manages the state and synchronization of asynchronous results
         */
        template<typename TYPE>
        class asyncWrapper {
            atomic<bool> ready_{makeAtomic(false)};  ///< Atomic flag indicating result readiness
            strongPtr<TYPE> result_;                      ///< Storage of asynchronous computation result
            mutable condition cond_{};                   ///< Condition variable for synchronization
            mutable mutex mutex_{};                      ///< Mutex for thread safety
            std::exception_ptr e_{};                      ///< Exception pointer for error handling

        public:
            asyncWrapper();

            /**
             * @brief Sets the result value and marks as ready
             * @param v The result value to store
             */
            void setValue(TYPE&& v);

            /**
             * @brief Sets an exception and marks as ready
             * @param e Exception pointer to store
             */
            void setException(const std::exception_ptr& e);

            /**
             * @brief Checks if the result is ready
             * @return True if result is available (value or exception)
             */
            [[nodiscard]] bool ready() const;

            /**
             * @brief Waits until the result becomes ready
             */
            void wait() const;

            /**
             * @brief Waits for the result with a timeout
             * @param timeout Maximum time to wait
             * @return True if result is ready within timeout, false otherwise
             */
            [[nodiscard]] bool waitFor(time::duration timeout) const;

            /**
             * @brief Retrieves the result value (blocks until ready)
             * @return The computed result
             * @throws std::exception if an exception was set
             */
            TYPE get();

            /**
             * @brief Peeks at the result value without consuming it (blocks until ready)
             * @return Const reference to the computed result
             * @throws std::exception if an exception was set
             */
            const TYPE& peek() const;

            /**
             * @brief Gets a strong pointer to the result value
             * @return Strong pointer to the result
             */
            [[nodiscard]] strongPtr<TYPE> getPtr() const;

            /**
             * @brief Throws stored exception if present
             */
            void rethrowIfException() const;

            /**
             * @brief Gets the stored exception
             * @return Exception pointer (nullptr if no exception)
             */
            [[nodiscard]] std::exception_ptr exception() const;
        };

    public:
        /**
         * @class futureBase
         * @brief Abstract base interface for asynchronous consumers
         * @details Provides a common, type-erased interface for accessing asynchronous
         * results, regardless of their actual type or ownership semantics.
         *
         * Key methods include:
         * - `valid()`   : check if the consumer is valid
         * - `ready()`   : check if the result is ready
         * - `wait()`    : block until ready
         * - `waitFor()` : Block with a timeout
         * - `exception()`: retrieve any stored exception
         */
        class futureBase {
        public:
            /**
             * @brief Checks if the future is valid (has an associated async wrapper)
             * @return True if valid, false otherwise
             */
            [[nodiscard]] virtual bool valid() const noexcept = 0;

            /**
             * @brief Waits until the result becomes ready
             */
            virtual void wait() const = 0;

            /**
             * @brief Waits for the result with a timeout
             * @param timeout Maximum time to wait
             * @return True if result is ready within timeout, false otherwise
             */
            [[nodiscard]] virtual bool waitFor(time::duration timeout) const = 0;

            /**
             * @brief Gets the stored exception if computation failed
             * @return Exception pointer (nullptr if no exception)
             */
            [[nodiscard]] virtual std::exception_ptr exception() const noexcept = 0;

            /**
             * @brief Checks if the result is ready
             * @return True if result is available
             */
            [[nodiscard]] virtual bool ready() const = 0;

            virtual ~futureBase() = default;
        };

        // ==================== Future Class ====================

        template<typename TYPE>
        class sharedFuture;

        /**
         * @class future
         * @brief Represents a one-shot future result of an asynchronous computation
         * @tparam TYPE The result type of the computation
         * @details `future` allows retrieving the result exactly once.
         * After consumption, the value is moved out of storage.
         */
        template<typename TYPE>
        class future final : public futureBase {
            strongPtr<asyncWrapper<TYPE>> awr_{};  ///< Shared pointer to the async wrapper

            friend class async;
            explicit future(strongPtr<asyncWrapper<TYPE>> awr);

        public:
            future() = default;

            // Disable copying
            future(const future&) = delete;
            future& operator=(const future&) = delete;

            // Allow moving
            future(future&&) = default;
            future& operator=(future&&) = default;

            /**
             * @brief Checks if the future is valid (has an associated async wrapper)
             * @return True if valid, false otherwise
             */
            [[nodiscard]] bool valid() const noexcept override;

            /**
             * @brief Converts this future to a shared future
             * @return A shared future that shares the same async wrapper
             */
            sharedFuture<TYPE> share();

            /**
             * @brief Gets the result (blocks until ready)
             * @return The computed result
             * @throws std::exception if the computation threw an exception
             */
            TYPE result();

            /**
             * @brief Checks if the result is ready
             * @return True if result is available
             */
            [[nodiscard]] bool ready() const override;

            /**
             * @brief Gets the exception if computation failed
             * @return Exception pointer (nullptr if no exception)
             */
            [[nodiscard]] std::exception_ptr exception() const noexcept override;

            /**
             * @brief Waits until the result becomes ready
             */
            void wait() const override;

            /**
             * @brief Waits for the result with a timeout
             * @param timeout Maximum time to wait
             * @return True if result is ready within timeout, false otherwise
             */
            [[nodiscard]] bool waitFor(time::duration timeout) const override;
        };

        /**
         * @class sharedFuture
         * @brief Represents a sharable asynchronous result
         * @tparam TYPE The result type of the computation
         * @details Unlike `future`, `sharedFuture` can be copied and safely stored in
         * multiple places. All copies refer to the same asynchronous result, making it
         * suitable for scenarios where results are needed by multiple consumers.
         */
        template<typename TYPE>
        class sharedFuture final : public futureBase, public hashable<sharedFuture<TYPE>> {
            strongPtr<asyncWrapper<TYPE>> awr_{};

            friend class async;
            explicit sharedFuture(strongPtr<asyncWrapper<TYPE>> awr);

        public:
            sharedFuture() = default;

            // Allow copying and moving
            sharedFuture(const sharedFuture&) = default;
            sharedFuture& operator=(const sharedFuture&) = default;
            sharedFuture(sharedFuture&&) = default;
            sharedFuture& operator=(sharedFuture&&) = default;

            /**
             * @brief Checks if the shared future is valid (has an associated async wrapper)
             * @return True if valid, false otherwise
             */
            [[nodiscard]] bool valid() const noexcept override;

            /**
             * @brief Gets the result value (blocks until ready)
             * @return The computed result
             * @throws std::exception if the computation threw an exception
             */
            TYPE result() const;

            /**
             * @brief Gets a strong pointer to the result value
             * @details Returns a strong reference pointer to the asynchronous result,
             * extending the lifetime of the referenced object and avoiding dangling references.
             * The pointer approach avoids unnecessary copying
             * @warning the returned pointer directly references the result stored within the async module.
             * Modifying the stored result through constCastTo or other means may cause unexpected behavior
             * and is strongly discouraged.
             * @return Strong pointer to the const result value
             * @throws sysError if the shared future is invalid or not ready
             * @throws std::exception if the computation threw an exception
             */
            [[nodiscard]] strongPtr<const TYPE> strongPointer() const;

            /**
             * @brief Checks if the result is ready
             * @return True if result is available
             */
            [[nodiscard]] bool ready() const override;

            /**
             * @brief Gets the exception if computation failed
             * @return Exception pointer (nullptr if no exception)
             */
            [[nodiscard]] std::exception_ptr exception() const noexcept override;

            /**
             * @brief Equality comparison operator
             * @param other The other shared future to compare with
             * @return True if both shared futures refer to the same async wrapper
             */
            bool operator==(const sharedFuture& other) const noexcept;

            /**
             * @brief Inequality comparison operator
             * @param other The other shared future to compare with
             * @return True if shared futures refer to different async wrappers
             */
            bool operator!=(const sharedFuture& other) const noexcept;

            /**
             * @brief Waits until the result becomes ready
             */
            void wait() const override;

            /**
             * @brief Waits for the result with a timeout
             * @param timeout Maximum time to wait
             * @return True if result is ready within timeout, false otherwise
             */
            [[nodiscard]] bool waitFor(time::duration timeout) const override;

            /**
             * @brief Generates a hash value for the shared future
             * @return Hash value based on the async wrapper pointer
             */
            [[nodiscard]] u_integer toHash() const noexcept override;

            /**
             * @brief Checks if two shared futures are equal
             * @param other The other shared future to compare with
             * @return True if both shared futures refer to the same async wrapper
             */
            bool equals(const sharedFuture& other) const noexcept override;

            ~sharedFuture() override = default;
        };

        // ==================== Promise Class ====================

        /**
         * @class promise
         * @brief Represents a one-time asynchronous producer with result setting capability
         * @tparam TYPE The result type of the computation
         * @tparam Callback The type of the computation callback
         * @details Provides the means to set the result of an asynchronous computation.
         * A promise is a single-use object that can be either executed via run() or
         * have its computation function extracted via function(). Once used, the promise
         * becomes invalid and cannot be reused.
         *
         * Key characteristics:
         * - Single-use: can be executed or have its function extracted only once
         * - Thread-safe: internal state is properly synchronized
         * - Exception-safe: exceptions during computation are properly captured and stored
         */
        template<typename TYPE, typename Callback>
        class promise {
            std::function<TYPE()> c_{};                ///< The computation to execute (one-time use)
            strongPtr<asyncWrapper<TYPE>> awr_{};      ///< Shared pointer to the async wrapper
            bool valid_{false};                         ///< Whether the promise still holds a valid task

        public:
            // Disable copying to prevent multiple executions of the same computation
            promise(const promise&) = delete;
            promise& operator=(const promise&) = delete;

            // Allow moving to transfer ownership of the computation
            promise(promise&& other) noexcept;

            promise& operator=(promise&& other) noexcept;

            /**
             * @brief Default constructor creates an invalid promise
             * @details Creates a promise that is not associated with any computation.
             * The resulting promise is invalid and cannot be used until assigned a valid promise.
             */
            promise() = default;

            /**
             * @brief Constructs a promise with a computation callback
             * @param c Callback that will produce the result when executed
             * @details The promise takes ownership of the callback and becomes valid.
             * The callback will be invoked exactly once - either by run() or through function().
             */
            explicit promise(Callback&& c);

            /**
             * @brief Gets the future associated with this promise
             * @return A future that will receive the result
             * @throws Nothing - can be called multiple times while the promise is valid
             * @note The promise remains valid after calling getFuture()
             */
            future<TYPE> getFuture();

            /**
             * @brief Checks if the promise is still valid (can be executed or have function extracted)
             * @return True if the promise holds a valid computation that hasn't been used yet
             * @details A promise becomes invalid after:
             * - run() is called (executes the computation)
             * - function() is called (extracts the computation function)
             * - Being default-constructed (no computation associated)
             */
            [[nodiscard]] bool valid() const noexcept;

            /**
             * @brief Extracts the computation function from the promise
             * @return The computation function that was associated with this promise
             * @throws sysError if the promise is invalid (already used or default-constructed)
             * @details After calling this method:
             * - The promise becomes invalid
             * - The caller takes ownership of the computation function
             * - The promise can no longer be used for execution
             * @note This is an alternative to run() for manual execution control
             */
            std::function<TYPE()> function();

            /**
             * @brief Executes the computation and sets the result in the associated future
             * @throws sysError if the promise is invalid (already used or default-constructed)
             * @throws Any exception thrown by the computation function (captured in the future)
             * @details After calling this method:
             * - The promise becomes invalid
             * - The computation result (or exception) is stored in the associated future
             * - Any waiting futures will be notified of completion
             */
            void run();
        };

        // ==================== Static Factory Methods ====================

        /**
         * @brief Creates a promise from a callable and arguments
         * @tparam Callback The type of the callable
         * @tparam Args The types of the arguments
         * @param c Callable to execute asynchronously
         * @param args Arguments to forward to the callable
         * @return A promise object for the asynchronous computation
         * @details The created promise is a single-use object that can be executed
         * exactly once via run() or have its function extracted via function().
         */
        template <typename Callback, typename... Args>
        static auto makePromise(Callback&& c, Args&&... args);

        /**
         * @brief Executes a callable asynchronously and returns a future for the result
         * @tparam Callback The type of the callable
         * @tparam Args The types of the arguments
         * @param c Callable to execute
         * @param args Arguments to forward to the callable
         * @return A future that will hold the result of the computation
         */
        template <typename Callback, typename... Args>
        static auto get(Callback&& c, Args&&... args) -> future<std::invoke_result_t<std::decay_t<Callback>, std::decay_t<Args>...>>;
    };

    /**
     * @brief Pipe operator for chaining asynchronous computations
     * @details This operator dynamically attaches a new callback to an already running future.
     * The callback will be executed once the asynchronous result becomes available.
     * Unlike a lazy pipeline builder, this version modifies the execution flow immediately.
     *
     * @tparam T Result type of the future
     * @tparam Callback Callable type, must accept T or T&& (depending on constness and value category)
     * @param f Source future containing the asynchronous result
     * @param c Callback function to process the result
     * @return A new future holding the result of the callback
     */
    template<typename T, typename Callback>
    auto operator|(async::future<T> f, Callback&& c);

    /**
     * @brief Pipe operator specialization for future<void>
     * @details This operator dynamically attaches a new callback to an already running future<void>.
     * The callback will be executed once the asynchronous task completes. This is suitable for
     * chaining tasks that do not produce a value. Execution is immediate, not deferred.
     *
     * @tparam Callback Callable type, must accept no arguments
     * @param f Source future<void> representing completion of an asynchronous task
     * @param c Callback function to execute after completion
     * @return A new future holding the result of the callback
     */
    template <typename Callback>
    auto operator|(async::future<void> f, Callback&& c);

    /**
     * @brief Pipe operator for chaining computations on sharedFuture
     * @details This operator dynamically attaches a new callback to an already running sharedFuture.
     * Unlike lazy task builders, the continuation is immediately scheduled to run once the sharedFuture
     * produces a value. This makes it possible to share and extend asynchronous results across multiple consumers.
     *
     * @tparam T Result type of the sharedFuture
     * @tparam Callback Callable type, must accept T const& or T (depending on design)
     * @param sf Source sharedFuture containing the asynchronous result
     * @param c Callback function to process the result
     * @return A new future holding the result of the callback
     */
    template<typename T, typename Callback>
    auto operator|(const async::sharedFuture<T>& sf, Callback&& c);

    /**
     * @brief Pipe operator specialization for sharedFuture<void>
     * @details This operator dynamically attaches a new callback to an already running sharedFuture<void>.
     * The callback will be executed once the asynchronous task completes. Since sharedFuture can be copied,
     * multiple consumers can each dynamically attach their own continuations.
     *
     * @tparam Callback Callable type, must accept no arguments
     * @param sf Source sharedFuture<void> representing completion of an asynchronous task
     * @param c Callback function to execute after completion
     * @return A new future holding the result of the callback
     */
    template <typename Callback>
    auto operator|(const async::sharedFuture<void>& sf, Callback&& c);

    /**
     * @brief Lazy pipe operator for chaining promise computations
     * @details This operator creates a lazy composition of promises, where the computation
     * is only executed when the resulting promise is run. Unlike the dynamic pipe operator
     * for futures, this creates a new promise that combines both computations without
     * immediate execution.
     *
     * The pipeline is built lazily - no computation occurs until run() is called on the
     * resulting promise. This allows for building complex computation graphs without
     * immediate execution overhead.
     *
     * @tparam T Result type of the source promise
     * @tparam Callback1 Type of the source promise's computation
     * @tparam Callback2 Type of the continuation callback
     * @param p Source promise containing the initial computation
     * @param c Callback function to process the result of the source promise
     * @return A new promise that will execute both computations in sequence when run
     * @note The source promise is moved into the resulting promise, making it invalid
     * after this operation. The continuation callback must accept the result type T.
     */
    template<typename T, typename Callback1, typename Callback2>
    auto operator|(async::promise<T, Callback1> p, Callback2&& c);

    /**
     * @brief Lazy pipe operator specialization for promise<void>
     * @details This operator creates a lazy composition of void-returning promises,
     * where the first computation completes before the second begins. The pipeline
     * executes sequentially but only when the resulting promise is run.
     *
     * Useful for building sequences of side-effectful operations that need to
     * execute in order but don't produce values.
     *
     * @tparam Callback1 Type of the source promise's computation
     * @tparam Callback2 Type of the continuation callback
     * @param p Source promise containing the initial void computation
     * @param c Callback function to execute after the source promise completes
     * @return A new promise that will execute both computations in sequence when run
     * @note The source promise is moved into the resulting promise, making it invalid
     * after this operation. The continuation callback must accept no arguments.
     */
    template<typename Callback1, typename Callback2>
    auto operator|(async::promise<void, Callback1> p, Callback2&& c);


    // ==================== Void Specializations ====================

    /**
     * @brief Specialization of asyncWrapper for void results
     */
    template <>
    class async::asyncWrapper<void> {
        atomic<bool> ready_{makeAtomic(false)};  ///< Atomic flag indicating completion
        alternative<void> result_;                    ///< Mark of asynchronous computation status
        mutable condition cond_{};                   ///< Condition variable for synchronization
        mutable mutex mutex_{};                      ///< Mutex for thread safety
        std::exception_ptr e_{};                      ///< Exception pointer for error handling

    public:
        asyncWrapper() = default;

        /**
         * @brief Marks the computation as completed successfully
         */
        void setValue();

        /**
         * @brief Sets an exception and marks as completed
         * @param e Exception pointer to store
         */
        void setException(const std::exception_ptr& e);

        /**
         * @brief Checks if the computation is completed
         * @return True if completed (successfully or with exception)
         */
        [[nodiscard]] bool ready() const;

        /**
         * @brief Waits until the computation completes
         */
        void wait() const;

        /**
         * @brief Waits for the result with a timeout
         * @param timeout Maximum time to wait
         * @return True if result is ready within timeout, false otherwise
         */
        [[nodiscard]] bool waitFor(const time::duration& timeout) const;

        /**
         * @brief Waits for completion and checks for exceptions
         * @throws std::exception if the computation threw an exception
         */
        void get();

        /**
         * @brief Checks for completion and exceptions without consuming the result
         * @throws std::exception if the computation threw an exception
         */
        void peek() const;

        /**
         * @brief Throws stored exception if present
         * @throws std::exception if the computation threw an exception
         */
        void rethrowIfException() const;

        /**
         * @brief Gets the stored exception
         * @return Exception pointer (nullptr if no exception)
         */
        [[nodiscard]] std::exception_ptr exception() const noexcept;
    };

    /**
     * @brief Specialization of future for void results
     */
    template <>
    class async::future<void> final : public futureBase {
        strongPtr<asyncWrapper<void>> awr_{};  ///< Shared pointer to the async wrapper

        friend class async;
        explicit future(strongPtr<asyncWrapper<void>> awr);

    public:
        future() = default;

        // Disable copying
        future(const future&) = delete;
        future& operator=(const future&) = delete;

        // Allow moving
        future(future&&) = default;
        future& operator=(future&&) = default;

        /**
         * @brief Checks if the future is valid (has an associated async wrapper)
         * @return True if valid, false otherwise
         */
        [[nodiscard]] bool valid() const noexcept override;

        /**
         * @brief Converts this future to a shared future
         * @return A shared future that shares the same async wrapper
         */
        sharedFuture<void> share();

        /**
         * @brief Waits for completion and checks for exceptions
         * @throws std::exception if the computation threw an exception
         */
        void result();

        /**
         * @brief Checks if the computation is completed
         * @return True if completed
         */
        [[nodiscard]] bool ready() const override;

        /**
         * @brief Gets the exception if computation failed
         * @return Exception pointer (nullptr if no exception)
         */
        [[nodiscard]] std::exception_ptr exception() const noexcept override;

        /**
         * @brief Waits until the computation completes
         */
        void wait() const override;

        /**
         * @brief Waits for the result with a timeout
         * @param timeout Maximum time to wait
         * @return True if result is ready within timeout, false otherwise
         */
        [[nodiscard]] bool waitFor(time::duration timeout) const override;
    };

    template <>
    class async::sharedFuture<void> final : public futureBase, public hashable<sharedFuture<void>> {
        strongPtr<asyncWrapper<void>> awr_{};

        friend class async;
        explicit sharedFuture(strongPtr<asyncWrapper<void>> awr);

    public:
        sharedFuture() = default;

        // Allow copying and moving
        sharedFuture(const sharedFuture&) = default;
        sharedFuture& operator=(const sharedFuture&) = default;
        sharedFuture(sharedFuture&&) = default;
        sharedFuture& operator=(sharedFuture&&) = default;

        /**
         * @brief Checks if the shared future is valid (has an associated async wrapper)
         * @return True if valid, false otherwise
         */
        [[nodiscard]] bool valid() const noexcept override;

        /**
         * @brief Waits for completion and checks for exceptions
         * @throws std::exception if the computation threw an exception
         */
        void result() const;

        /**
         * @brief Checks if the computation is completed
         * @return True if completed
         */
        [[nodiscard]] bool ready() const override;

        /**
         * @brief Gets the exception if computation failed
         * @return Exception pointer (nullptr if no exception)
         */
        [[nodiscard]] std::exception_ptr exception() const noexcept override;

        /**
         * @brief Equality comparison operator
         * @param other The other shared future to compare with
         * @return True if both shared futures refer to the same async wrapper
         */
        bool operator==(const sharedFuture& other) const noexcept;

        /**
         * @brief Inequality comparison operator
         * @param other The other shared future to compare with
         * @return True if shared futures refer to different async wrappers
         */
        bool operator!=(const sharedFuture& other) const noexcept;

        /**
         * @brief Waits until the computation completes
         */
        void wait() const override;

        /**
         * @brief Waits for the result with a timeout
         * @param timeout Maximum time to wait
         * @return True if result is ready within timeout, false otherwise
         */
        [[nodiscard]] bool waitFor(time::duration timeout) const override;

        /**
         * @brief Generates a hash value for the shared future
         * @return Hash value based on the async wrapper pointer
         */
        [[nodiscard]] u_integer toHash() const noexcept override;

        /**
         * @brief Checks if two shared futures are equal
         * @param other The other shared future to compare with
         * @return True if both shared futures refer to the same async wrapper
         */
        [[nodiscard]] bool equals(const sharedFuture& other) const noexcept override;

        ~sharedFuture() override = default;
    };

    /**
     * @brief Specialization of promise for void results
     * @tparam Callback The type of the computation callback
     * @details Same single-use semantics as the general promise template, but for void-returning computations.
     */
    template <typename Callback>
    class async::promise<void, Callback> {
        std::function<void()> c_{};                  ///< The computation to execute (one-time use)
        strongPtr<asyncWrapper<void>> awr_{};        ///< Shared pointer to the async wrapper
        bool valid_{false};                           ///< Whether the promise still holds a valid task

    public:
        // Disable copying to prevent multiple executions of the same computation
        promise(const promise&) = delete;
        promise& operator=(const promise&) = delete;

        // Allow moving to transfer ownership of the computation
        promise(promise&& other) noexcept;

        promise& operator=(promise&& other) noexcept;

        /**
         * @brief Default constructor creates an invalid promise
         */
        promise() = default;

        /**
         * @brief Constructs a promise with a computation callback
         * @param c Callback that will execute the computation
         */
        explicit promise(Callback&& c);

        /**
         * @brief Gets the future associated with this promise
         * @return A future that will receive the completion signal
         */
        [[nodiscard]] future<void> getFuture() const;

        /**
         * @brief Checks if the promise is still valid
         * @return True if the promise holds a valid computation that hasn't been used yet
         */
        [[nodiscard]] bool valid() const noexcept;

        /**
         * @brief Extracts the computation function from the promise
         * @return The computation function that was associated with this promise
         * @throws sysError if the promise is invalid
         */
        std::function<void()> function();

        /**
         * @brief Executes the computation and marks completion in the associated future
         * @throws sysError if the promise is invalid
         * @throws Any exception thrown by the computation function
         */
        void run();
    };
} // namespace original

template <typename TYPE>
original::async::asyncWrapper<TYPE>::asyncWrapper() = default;

template <typename TYPE>
void original::async::asyncWrapper<TYPE>::setValue(TYPE&& v)
{
    {
        uniqueLock lock{this->mutex_};
        this->result_ = makeStrongPtr<TYPE>(std::move(v));
        this->ready_.store(true);
    }
    this->cond_.notifyAll();
}

template <typename TYPE>
void original::async::asyncWrapper<TYPE>::setException(const std::exception_ptr &e)
{
    {
        uniqueLock lock{this->mutex_};
        this->e_ = e;
        this->ready_.store(true);
    }
    this->cond_.notifyAll();
}

template <typename TYPE>
bool original::async::asyncWrapper<TYPE>::ready() const
{
    return *this->ready_;
}

template <typename TYPE>
void original::async::asyncWrapper<TYPE>::wait() const
{
    uniqueLock lock{this->mutex_};
    this->cond_.wait(this->mutex_, [this]
    {
        return this->ready();
    });
}

template <typename TYPE>
bool original::async::asyncWrapper<TYPE>::waitFor(time::duration timeout) const
{
    uniqueLock lock{this->mutex_};
    return this->cond_.waitFor(this->mutex_, timeout, [this]
    {
        return this->ready();
    });
}

template <typename TYPE>
TYPE original::async::asyncWrapper<TYPE>::get()
{
    uniqueLock lock{this->mutex_};
    this->cond_.wait(this->mutex_, [this]{
        return this->ready();
    });

    this->rethrowIfException();

    TYPE result = std::move(*this->result_);
    this->result_.reset();
    return result;
}

template <typename TYPE>
const TYPE& original::async::asyncWrapper<TYPE>::peek() const
{
    uniqueLock lock{this->mutex_};
    this->cond_.wait(this->mutex_, [this]{
        return this->ready();
    });

    this->rethrowIfException();
    return *this->result_;
}

template <typename TYPE>
original::strongPtr<TYPE> original::async::asyncWrapper<TYPE>::getPtr() const
{
    this->rethrowIfException();
    return this->result_;
}

template <typename TYPE>
void original::async::asyncWrapper<TYPE>::rethrowIfException() const
{
    if (this->e_)
        std::rethrow_exception(this->e_);
}

template <typename TYPE>
std::exception_ptr
original::async::asyncWrapper<TYPE>::exception() const
{
    uniqueLock lock{this->mutex_};
    return this->e_;
}

template <typename TYPE>
original::async::future<TYPE>::future(strongPtr<asyncWrapper<TYPE>> awr)
    : awr_(std::move(awr)) {}

template <typename TYPE>
bool original::async::future<TYPE>::valid() const noexcept
{
    return this->awr_ != nullptr;
}

template <typename TYPE>
original::async::sharedFuture<TYPE> original::async::future<TYPE>::share()
{
    return sharedFuture<TYPE>(std::move(this->awr_));
}


template <typename TYPE>
TYPE original::async::future<TYPE>::result()
{
    if (!this->valid()) {
        throw sysError("Access an invalid future");
    }
    return this->awr_->get();
}

template <typename TYPE>
bool original::async::future<TYPE>::ready() const
{
    if (!this->valid()) {
        throw sysError("Access an invalid future");
    }
    return this->awr_->ready();
}

template <typename TYPE>
std::exception_ptr original::async::future<TYPE>::exception() const noexcept
{
    if (!this->valid()) {
        return {};
    }
    return this->awr_->exception();
}

template <typename TYPE>
void original::async::future<TYPE>::wait() const
{
    if (!this->valid()) {
        throw sysError("Access an invalid future");
    }
    this->awr_->wait();
}

template <typename TYPE>
bool original::async::future<TYPE>::waitFor(time::duration timeout) const
{
    if (!this->valid()) {
        throw sysError("Access an invalid future");
    }
    return this->awr_->waitFor(timeout);
}

template <typename TYPE>
original::async::sharedFuture<TYPE>::sharedFuture(strongPtr<asyncWrapper<TYPE>> awr)
    : awr_(std::move(awr)) {}

template <typename TYPE>
bool original::async::sharedFuture<TYPE>::valid() const noexcept
{
    return this->awr_ != nullptr;
}

template <typename TYPE>
TYPE original::async::sharedFuture<TYPE>::result() const
{
    if (!this->valid()) {
        throw sysError("Access an invalid sharedFuture");
    }
    return this->awr_->peek();
}

template <typename TYPE>
original::strongPtr<const TYPE> original::async::sharedFuture<TYPE>::strongPointer() const
{
    if (!this->valid()) {
        throw sysError("Access an invalid sharedFuture");
    }
    if (!this->ready()) {
        throw sysError("Cannot get a strongPtr from a sharedFuture that is not ready");
    }
    return this->awr_->getPtr().template constCastTo<const TYPE>();
}

template <typename TYPE>
bool original::async::sharedFuture<TYPE>::ready() const
{
    if (!this->valid()) {
        throw sysError("Access an invalid sharedFuture");
    }
    return this->awr_->ready();
}

template <typename TYPE>
std::exception_ptr original::async::sharedFuture<TYPE>::exception() const noexcept
{
    if (!this->valid()) {
        return {};
    }
    return this->awr_->exception();
}

template <typename TYPE>
bool original::async::sharedFuture<TYPE>::operator==(const sharedFuture& other) const noexcept
{
    return this->awr_ == other.awr_;
}

template <typename TYPE>
bool original::async::sharedFuture<TYPE>::operator!=(const sharedFuture& other) const noexcept
{
    return this->awr_ != other.awr_;
}

template <typename TYPE>
void original::async::sharedFuture<TYPE>::wait() const
{
    if (!this->valid()) {
        throw sysError("Access an invalid sharedFuture");
    }
    this->awr_->wait();
}

template <typename TYPE>
bool original::async::sharedFuture<TYPE>::waitFor(time::duration timeout) const
{
    if (!this->valid()) {
        throw sysError("Access an invalid sharedFuture");
    }
    return this->awr_->waitFor(timeout);
}

template <typename TYPE>
original::u_integer original::async::sharedFuture<TYPE>::toHash() const noexcept
{
    return this->awr_.toHash();
}

template <typename TYPE>
bool original::async::sharedFuture<TYPE>::equals(const sharedFuture& other) const noexcept
{
    return *this == other;
}

template <typename TYPE, typename Callback>
original::async::promise<TYPE, Callback>::promise(promise&& other) noexcept
{
    if (!other.valid()){
        return;
    }
    this->valid_ = true;
    this->c_ = std::move(other.c_);
    this->awr_ = std::move(other.awr_);
    other.valid_ = false;
}

template <typename TYPE, typename Callback>
original::async::promise<TYPE, Callback>&
original::async::promise<TYPE, Callback>::operator=(promise&& other) noexcept
{
    if (this == &other || !other.valid()) {
        return *this;
    }
    this->valid_ = true;
    this->c_ = std::move(other.c_);
    this->awr_ = std::move(other.awr_);
    other.valid_ = false;
    return *this;
}

template <typename TYPE, typename Callback>
original::async::promise<TYPE, Callback>::promise(Callback&& c)
    : c_(std::forward<Callback>(c)), awr_(makeStrongPtr<asyncWrapper<TYPE>>()), valid_(true) {}

template <typename TYPE, typename Callback>
original::async::future<TYPE>
original::async::promise<TYPE, Callback>::getFuture()
{
    return future<TYPE>(this->awr_);
}

template <typename TYPE, typename Callback>
bool original::async::promise<TYPE, Callback>::valid() const noexcept
{
    return this->valid_;
}

template <typename TYPE, typename Callback>
std::function<TYPE()> original::async::promise<TYPE, Callback>::function()
{
    if (!this->valid_) {
        throw sysError("Try to get an invalid task");
    }
    this->valid_ = false;
    return std::move(this->c_);
}

template <typename TYPE, typename Callback>
void original::async::promise<TYPE, Callback>::run()
{
    if (!this->valid_) {
        throw sysError("Try to run an invalid task");
    }
    try {
        this->awr_->setValue(this->c_());
    } catch (...) {
        this->awr_->setException(std::current_exception());
    }
    this->valid_ = false;
}

template <typename Callback, typename... Args>
auto original::async::makePromise(Callback&& c, Args&&... args) {
    using Return = std::invoke_result_t<Callback, Args...>;
    auto bound = [c = std::forward<Callback>(c),
                  ... args = std::forward<Args>(args)]() mutable -> Return {
        return c(args...);
    };

    return promise<Return, decltype(bound)>{std::move(bound)};
}

template <typename Callback, typename ... Args>
auto original::async::get(Callback&& c, Args&&... args)
    -> future<std::invoke_result_t<std::decay_t<Callback>, std::decay_t<Args>...>>
{
    auto p = makePromise(std::forward<Callback>(c), std::forward<Args>(args)...);
    auto fut = p.getFuture();

    auto p_shared = makeStrongPtr<decltype(p)>(std::move(p));
    thread t{
        [p_shared]() mutable {
            p_shared->run();
        },
        thread::AUTO_DETACH
    };

    return fut;
}

template <typename T, typename Callback>
auto original::operator|(async::future<T> f, Callback&& c)
{
    using ResultType = std::invoke_result_t<Callback, T>;
    strongPtr<async::future<T>> shared_f = makeStrongPtr<async::future<T>>(std::move(f));
    return async::get([shared_f, c = std::forward<Callback>(c)] mutable -> ResultType {
        return c(shared_f->result());
    });
}

template <typename Callback>
auto original::operator|(async::future<void> f, Callback&& c)
{
    using ResultType = std::invoke_result_t<Callback>;
    strongPtr<async::future<void>> shared_f = makeStrongPtr<async::future<void>>(std::move(f));
    return async::get([shared_f, c = std::forward<Callback>(c)]() mutable -> ResultType {
        shared_f->result();
        return c();
    });
}

template <typename T, typename Callback>
auto original::operator|(const async::sharedFuture<T>& sf, Callback&& c)
{
    using ResultType = std::invoke_result_t<Callback, T>;
    return async::get([sf, c = std::forward<Callback>(c)] mutable -> ResultType {
        return c(sf.result());
    }).share();
}

template <typename Callback>
auto original::operator|(const async::sharedFuture<void>& sf, Callback&& c)
{
    using ResultType = std::invoke_result_t<Callback>;
    return async::get([sf, c = std::forward<Callback>(c)]() mutable -> ResultType {
        sf.result();
        return c();
    }).share();
}

template <typename T, typename Callback1, typename Callback2>
auto original::operator|(async::promise<T, Callback1> p, Callback2&& c)
{
    if (!p.valid()) {
        throw sysError("Try to build a lazy pipeline from an invalid promise");
    }
    using ResultType = decltype(c(std::declval<T>()));
    strongPtr<async::promise<T,Callback1>> shared_p = makeStrongPtr<async::promise<T, Callback1>>(std::move(p));
    return async::makePromise([shared_p, c = std::forward<Callback2>(c)]() mutable -> ResultType {
        return c(shared_p->function()());
    });
}

template <typename Callback1, typename Callback2>
auto original::operator|(async::promise<void, Callback1> p, Callback2&& c)
{
    if (!p.valid()) {
        throw sysError("Try to build a lazy pipeline from an invalid promise");
    }
    using ResultType = decltype(c());
    strongPtr<async::promise<void, Callback1>> shared_p = makeStrongPtr<async::promise<void, Callback1>>(std::move(p));
    return async::makePromise([shared_p, c = std::forward<Callback2>(c)]() mutable -> ResultType {
        shared_p->function()();
        return c();
    });
}

inline void original::async::asyncWrapper<void>::setValue()
{
    {
        uniqueLock lock{this->mutex_};
        this->result_.set();
        this->ready_.store(true);
    }
    this->cond_.notifyAll();
}

inline void original::async::asyncWrapper<void>::setException(const std::exception_ptr& e)
{
    {
        uniqueLock lock{this->mutex_};
        this->e_ = e;
        this->ready_.store(true);
    }
    this->cond_.notifyAll();
}

inline bool original::async::asyncWrapper<void>::ready() const
{
    return *this->ready_;
}

inline void original::async::asyncWrapper<void>::wait() const
{
    uniqueLock lock{this->mutex_};
    this->cond_.wait(this->mutex_, [this]
    {
        return this->ready();
    });
}

inline bool original::async::asyncWrapper<void>::waitFor(const time::duration& timeout) const
{
    uniqueLock lock{this->mutex_};
    return this->cond_.waitFor(this->mutex_, timeout, [this]
    {
       return this->ready();
    });
}

inline void original::async::asyncWrapper<void>::get()
{
    uniqueLock lock{this->mutex_};
    this->cond_.wait(this->mutex_, [this] {
        return this->ready();
    });

    this->rethrowIfException();
    this->result_.reset();
}

inline void original::async::asyncWrapper<void>::peek() const
{
    uniqueLock lock{this->mutex_};
    this->cond_.wait(this->mutex_, [this] {
        return this->ready();
    });

    this->rethrowIfException();
}

inline void original::async::asyncWrapper<void>::rethrowIfException() const
{
    if (this->e_)
        std::rethrow_exception(this->e_);
}

inline std::exception_ptr
original::async::asyncWrapper<void>::exception() const noexcept
{
    uniqueLock lock{this->mutex_};
    return this->e_;
}

inline original::async::future<void>::future(strongPtr<asyncWrapper<void>> awr)
    : awr_(std::move(awr)) {}

inline bool original::async::future<void>::valid() const noexcept
{
    return this->awr_ != nullptr;
}

inline original::async::sharedFuture<void> original::async::future<void>::share()
{
    return sharedFuture(std::move(this->awr_));
}

inline void original::async::future<void>::result()
{
    if (!this->valid()) {
        throw sysError("Access an invalid future");
    }
    this->awr_->get();
}

inline bool original::async::future<void>::ready() const
{
    if (!this->valid()) {
        throw sysError("Access an invalid future");
    }
    return this->awr_->ready();
}

inline std::exception_ptr original::async::future<void>::exception() const noexcept
{
    if (!this->valid()) {
        return {};
    }
    return this->awr_->exception();
}

inline void original::async::future<void>::wait() const
{
    if (!this->valid()) {
        throw sysError("Access an invalid future");
    }
    this->awr_->wait();
}

inline bool original::async::future<void>::waitFor(const time::duration timeout) const
{
    if (!this->valid()) {
        throw sysError("Access an invalid future");
    }
    return this->awr_->waitFor(timeout);
}

inline original::async::sharedFuture<void>::sharedFuture(strongPtr<asyncWrapper<void>> awr)
    : awr_(std::move(awr)) {}

inline bool original::async::sharedFuture<void>::valid() const noexcept
{
    return this->awr_ != nullptr;
}

inline void original::async::sharedFuture<void>::result() const
{
    if (!this->valid()) {
        throw sysError("Access an invalid sharedFuture");
    }
    this->awr_->peek();
}

inline bool original::async::sharedFuture<void>::ready() const
{
    if (!this->valid()) {
        throw sysError("Access an invalid sharedFuture");
    }
    return this->awr_->ready();
}

inline std::exception_ptr original::async::sharedFuture<void>::exception() const noexcept
{
    if (!this->valid()) {
        return {};
    }
    return this->awr_->exception();
}

inline bool original::async::sharedFuture<void>::operator==(const sharedFuture& other) const noexcept
{
    return this->awr_ == other.awr_;
}

inline bool original::async::sharedFuture<void>::operator!=(const sharedFuture& other) const noexcept
{
    return this->awr_ != other.awr_;
}


inline void original::async::sharedFuture<void>::wait() const
{
    if (!this->valid()) {
        throw sysError("Access an invalid sharedFuture");
    }
    this->awr_->wait();
}

inline bool original::async::sharedFuture<void>::waitFor(const time::duration timeout) const
{
    if (!this->valid()) {
        throw sysError("Access an invalid sharedFuture");
    }
    return this->awr_->waitFor(timeout);
}

inline original::u_integer original::async::sharedFuture<void>::toHash() const noexcept
{
    return this->awr_.toHash();
}

inline bool original::async::sharedFuture<void>::equals(const sharedFuture& other) const noexcept
{
    return *this == other;
}

template <typename Callback>
original::async::promise<void, Callback>::promise(promise&& other) noexcept
{
    if (!other.valid()){
        return;
    }
    this->valid_ = true;
    this->c_ = std::move(other.c_);
    this->awr_ = std::move(other.awr_);
    other.valid_ = false;
}

template <typename Callback>
original::async::promise<void, Callback>& original::async::promise<void, Callback>::operator=(promise&& other) noexcept
{
    if (this == &other || !other.valid()) {
        return *this;
    }
    this->valid_ = true;
    this->c_ = std::move(other.c_);
    this->awr_ = std::move(other.awr_);
    other.valid_ = false;
    return *this;
}

template <typename Callback>
original::async::promise<void, Callback>::promise(Callback&& c)
    : c_(std::forward<Callback>(c)), awr_(makeStrongPtr<asyncWrapper<void>>()), valid_(true) {}

template <typename Callback>
original::async::future<void> original::async::promise<void, Callback>::getFuture() const
{
    return future<void>(this->awr_);
}

template <typename Callback>
bool original::async::promise<void, Callback>::valid() const noexcept
{
    return this->valid_;
}

template <typename Callback>
std::function<void()> original::async::promise<void, Callback>::function()
{
    if (!this->valid_) {
        throw sysError("Try to get an invalid task");
    }
    this->valid_ = false;
    return std::move(this->c_);
}

template <typename Callback>
void original::async::promise<void, Callback>::run()
{
    if (!this->valid_) {
        throw sysError("Try to run an invalid task");
    }
    try {
        this->c_();
        this->awr_->setValue();
    } catch (...) {
        this->awr_->setException(std::current_exception());
    }
    this->valid_ = false;
}

#endif // ORIGINAL_ASYNC_H