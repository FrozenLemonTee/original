/**
 * @file tasks.h
 * @brief Thread pool and task management utilities
 * @details
 * This header defines the `taskDelegator` class, which provides a managed thread pool
 * with support for prioritized tasks, deferred execution, and futures.
 *
 * The file also defines:
 * - `taskDelegator::taskBase`: abstract base class for tasks
 * - `taskDelegator::task<TYPE>`: concrete task wrapper with future/promise support
 *
 * Features:
 * - Task prioritization (IMMEDIATE, HIGH, NORMAL, LOW, DEFERRED)
 * - Automatic shutdown on destruction with configurable stop modes (DISCARD_DEFERRED, KEEP_DEFERRED, RUN_DEFERRED)
 *   to handle deferred tasks
 * - Deferred task handling (activate, discard, or keep on shutdown)
 * - Query interfaces for task counts and thread states
 * - Timeout-based immediate task submission
 * - Thread-safe execution and synchronization
 *
 * @note taskDelegator is **non-copyable** and **non-movable** to prevent accidental
 * duplication of threads or task state.
 */

#ifndef ORIGINAL_TASKS_H
#define ORIGINAL_TASKS_H

#include "async.h"
#include "atomic.h"
#include "lockedQueue.h"
#include "lockedDeque.h"
#include "refCntPtr.h"
#include "array.h"
#include "lockedPrique.h"

namespace original
{
    // ==================== Task Delegator (Thread Pool) ====================

    /**
     * @class taskDelegator
     * @brief Thread pool for managing and executing prioritized tasks
     * @details
     * Provides a managed thread pool that can execute tasks with different priority levels.
     * Supports immediate, high, normal, low, and deferred tasks. Deferred tasks can be
     * manually activated or discarded on shutdown.
     */
    class taskDelegator
    {
    public:
        // ==================== Task Base Interface ====================

        /**
         * @class taskBase
         * @brief Abstract base class for all tasks
         * @details
         * Provides the common interface for tasks to be executed in the thread pool.
         * Must implement the run() method.
         */
        class taskBase
        {
        public:
            /**
             * @brief Executes the task
             */
            virtual void run() = 0;

            /**
             * @brief Virtual destructor for proper polymorphic behavior
             */
            virtual ~taskBase() = default;
        };

        // ==================== Concrete Task Class ====================

        /**
         * @class task
         * @brief Concrete task implementation with future/promise support
         * @tparam TYPE Return type of the task
         * @details
         * Wraps a callable and its arguments, and provides integration with futures.
         */
        template <typename TYPE>
        class task final : public taskBase, public moveOnlyMeta
        {
            async::promise<TYPE, std::function<TYPE()>> p; ///< Promise for task result

        public:
            task(task&&) = default; ///< Allow move constructor
            task& operator=(task&&) = default; ///< Allow move assignment
            task() = default;

            /**
             * @brief Constructs a task from a callable and its arguments
             * @tparam Callback Callable type
             * @tparam Args Argument types
             * @param c Callable to execute
             * @param args Arguments for the callable
             */
            template <typename Callback, typename... Args>
            explicit task(Callback&& c, Args&&... args);

            /**
             * @brief Executes the task
             */
            void run() override;

            /**
             * @brief Gets the future associated with this task
             * @return Future object for the task result
             */
            async::future<TYPE> getFuture();
        };

        // ==================== Task Priorities ====================

        /**
         * @enum priority
         * @brief Task priority levels for execution scheduling
         */
        enum class priority : u_integer
        {
            IMMEDIATE = 0, ///< Execute immediately if threads available
            HIGH = 1, ///< High priority task
            NORMAL = 2, ///< Normal priority task (default)
            LOW = 3, ///< Low priority task
            DEFERRED = 4, ///< Deferred execution
        };

        /**
         * @enum stopMode
         * @brief Stop behavior for deferred tasks
         */
        enum class stopMode
        {
            DISCARD_DEFERRED, ///< Discard deferred tasks
            KEEP_DEFERRED, ///< Keep deferred tasks
            RUN_DEFERRED, ///< Execute all deferred tasks before stopping
        };

        // Convenience constants
        static constexpr auto IMMEDIATE = priority::IMMEDIATE;
        static constexpr auto HIGH = priority::HIGH;
        static constexpr auto NORMAL = priority::NORMAL;
        static constexpr auto LOW = priority::LOW;
        static constexpr auto DEFERRED = priority::DEFERRED;

        static constexpr auto DISCARD_DEFERRED = stopMode::DISCARD_DEFERRED;
        static constexpr auto KEEP_DEFERRED = stopMode::KEEP_DEFERRED;
        static constexpr auto RUN_DEFERRED = stopMode::RUN_DEFERRED;

    private:
        // Internal type definitions
        using priorityTask = couple<strongPtr<taskBase>, priority>; ///< Task with priority

        /**
         * @struct taskComparator
         * @brief Comparator for ordering tasks by priority
         */
        template <typename COUPLE>
        struct taskComparator
        {
            /**
             * @brief Compares two tasks based on priority
             * @param lhs Left-hand side task
             * @param rhs Right-hand side task
             * @return True if lhs has higher priority than rhs
             */
            bool operator()(const COUPLE& lhs, const COUPLE& rhs) const;
        };

        using priorityTaskQueue = lockedPrique<priorityTask, taskComparator>; ///< Priority queue with built-in locking

        array<thread> threads_; ///< Worker threads
        priorityTaskQueue tasks_waiting_; ///< Waiting tasks (thread-safe)
        lockedQueue<strongPtr<taskBase>> task_immediate_; ///< Immediate tasks (thread-safe)
        lockedQueue<strongPtr<taskBase>> tasks_deferred_; ///< Deferred tasks (thread-safe)
        mutable condition condition_; ///< Synchronization condition
        mutable mutex mutex_wait_; ///< Mutex for thread waiting synchronization
        atomic<bool> stopped_; ///< Stop flag (lock-free)
        atomic<u_integer> active_threads_; ///< Count of active threads (lock-free)
        atomic<u_integer> idle_threads_; ///< Count of idle threads (lock-free)

        /**
         * @brief Submits a pre-created task with specified priority
         * @tparam TYPE Task result type
         * @param priority Task priority level
         * @param t Shared pointer to the task
         * @return Future for the task result
         */
        template <typename TYPE>
        async::future<TYPE> submit(priority priority, strongPtr<task<TYPE>> t);

        /**
         * @brief Worker thread main loop
         * @details
         * Each worker thread continuously:
         * 1. Checks for immediate tasks first (lock-free pop)
         * 2. Then checks for prioritized waiting tasks (lock-free pop)
         * 3. If no tasks available, waits on condition variable with mutex_wait_
         * 4. Updates idle_threads_ count atomically when waiting/awakening
         * 5. Updates active_threads_ count atomically when executing tasks
         * 6. Exits when stopped and all queues are empty
         */
        void workerThread();

        /**
        * @brief Move all delayed tasks to the waiting queue
        * @return The number of delayed tasks that were activated
        * @details Internal method used to move all tasks from the tasks_deferred_ queue
        *          to the tasks_waiting_ queue, with priority set to DEFERRED
        */
        u_integer moveAllDeferred();

    public:
        taskDelegator(const taskDelegator&) = delete; ///< Disable copy constructor
        taskDelegator& operator=(const taskDelegator&) = delete; ///< Disable copy assignment
        taskDelegator(taskDelegator&&) = delete; ///< Disable move constructor
        taskDelegator& operator=(taskDelegator&&) = delete; ///< Disable move assignment

        /**
         * @brief Constructs a task delegator with a given number of threads
         * @param thread_cnt Number of threads (default: 8)
         */
        explicit taskDelegator(u_integer thread_cnt = 8);

        /**
         * @brief Submits a task with normal priority
         * @tparam Callback Type of the callable
         * @tparam Args Types of the arguments
         * @param c Callable to execute
         * @param args Arguments to forward to the callable
         * @return Future for the task result
         */
        template <typename Callback, typename... Args>
        auto submit(Callback&& c, Args&&... args);

        /**
         * @brief Submits a task with specified priority
         * @tparam Callback Type of the callable
         * @tparam Args Types of the arguments
         * @param priority Task priority level
         * @param c Callable to execute
         * @param args Arguments to forward to the callable
         * @return Future for the task result
         *
         * @throw sysError if delegator is stopped or no idle thread is available
         *        for IMMEDIATE submission
         */
        template <typename Callback, typename... Args>
        auto submit(priority priority, Callback&& c, Args&&... args);

        /**
         * @brief Submits a task with IMMEDIATE priority and a timeout
         * @tparam Callback Type of the callable
         * @tparam Args Types of the arguments
         * @param timeout Maximum duration to wait for an idle thread
         * @param c Callable to execute
         * @param args Arguments to forward to the callable
         * @return Future for the task result
         *
         * @throw sysError if delegator is stopped
         * @throw sysError if no idle thread becomes available within the timeout
         *
         * @note Unlike normal submission, this will not throw immediately if
         *       no idle threads are available, but waits up to the given timeout.
         */
        template <typename Callback, typename... Args>
        auto submit(time::duration timeout, Callback&& c, Args&&... args);

        /**
         * @brief Returns the number of waiting (non-immediate, non-deferred) tasks
         */
        u_integer waitingCnt() const noexcept;

        /**
         * @brief Returns the number of immediate tasks pending execution
         */
        u_integer immediateCnt() const noexcept;

        /**
         * @brief Activates one deferred task
         */
        void runDeferred();

        /**
         * @brief Activates all deferred tasks
         */
        void runAllDeferred();

        /**
         * @brief Discards one deferred task
         * @return true if a deferred task was successfully discarded,
         *         false if no deferred tasks were available
         */
        bool discardDeferred();

        /**
         * @brief Discards all deferred tasks
         */
        void discardAllDeferred();

        /**
         * @brief Returns number of deferred tasks
         */
        u_integer deferredCnt() const noexcept;

        /**
         * @brief Stops the task delegator
         * @param mode Stop mode (default: KEEP_DEFERRED)
         * @details
         * - DISCARD_DEFERRED: Discard all deferred tasks
         * - KEEP_DEFERRED: Keep deferred tasks without executing them
         * - RUN_DEFERRED: Activate all deferred tasks before stopping
         */
        void stop(stopMode mode = stopMode::KEEP_DEFERRED);

        bool isStopped() const noexcept;

        /**
         * @brief Gets the number of active threads
         * @return Count of currently active threads
         */
        u_integer activeThreads() const noexcept;

        /**
         * @brief Gets the number of idle threads
         * @return Count of currently idle threads
         */
        u_integer idleThreads() const noexcept;

        /**
         * @brief Destructor
         * @details Calls stop(RUN_DEFERRED) and joins all threads
         */
        ~taskDelegator();
    };

    // ==================== Task Delegator V2 (Work-Stealing Thread Pool) ====================

    /**
     * @class taskDelegatorV2
     * @brief Work-stealing thread pool for managing and executing prioritized tasks
     * @details
     * Provides the same public task submission and lifecycle interface as taskDelegator,
     * but uses per-worker task queues internally. Each worker owns a workerSlot containing
     * its thread and multiple thread-safe deques split by priority. Deferred tasks are
     * kept in a pool-level FIFO queue until activated, then moved into worker-local
     * deferred queues where they can be executed or stolen.
     *
     * @note This class is introduced alongside taskDelegator so the new implementation can
     * be completed and tested before replacing the original implementation.
     */
    class taskDelegatorV2 : public noMeta
    {
    public:
        // ==================== Task Base Interface ====================

        /**
         * @class taskBase
         * @brief Abstract base class for all tasks
         */
        class taskBase
        {
        public:
            /**
             * @brief Executes the task
             */
            virtual void run() = 0;

            /**
             * @brief Virtual destructor for proper polymorphic behavior
             */
            virtual ~taskBase() = default;
        };

        // ==================== Concrete Task Class ====================

        /**
         * @class task
         * @brief Concrete task implementation with future/promise support
         * @tparam TYPE Return type of the task
         */
        template <typename TYPE>
        class task final : public taskBase, public moveOnlyMeta
        {
            async::promise<TYPE, std::function<TYPE()>> p; ///< Promise for task result

        public:
            task(task&&) = default;
            task& operator=(task&&) = default;
            task() = default;

            /**
             * @brief Constructs a task from a callable and its arguments
             * @tparam Callback Callable type
             * @tparam Args Argument types
             * @param c Callable to execute
             * @param args Arguments for the callable
             */
            template <typename Callback, typename... Args>
            explicit task(Callback&& c, Args&&... args);

            /**
             * @brief Executes the task
             */
            void run() override;

            /**
             * @brief Gets the future associated with this task
             * @return Future object for the task result
             */
            async::future<TYPE> getFuture();
        };

        // ==================== Task Priorities ====================

        /**
         * @enum priority
         * @brief Task priority levels for execution scheduling
         */
        enum class priority : u_integer
        {
            IMMEDIATE = 0,
            HIGH = 1,
            NORMAL = 2,
            LOW = 3,
            DEFERRED = 4,
        };

        /**
         * @enum stopMode
         * @brief Stop behavior for deferred tasks
         */
        enum class stopMode
        {
            DISCARD_DEFERRED,
            KEEP_DEFERRED,
            RUN_DEFERRED,
        };

        static constexpr auto IMMEDIATE = priority::IMMEDIATE;
        static constexpr auto HIGH = priority::HIGH;
        static constexpr auto NORMAL = priority::NORMAL;
        static constexpr auto LOW = priority::LOW;
        static constexpr auto DEFERRED = priority::DEFERRED;

        static constexpr auto DISCARD_DEFERRED = stopMode::DISCARD_DEFERRED;
        static constexpr auto KEEP_DEFERRED = stopMode::KEEP_DEFERRED;
        static constexpr auto RUN_DEFERRED = stopMode::RUN_DEFERRED;

    private:
        using taskPtr = strongPtr<taskBase>;

        /**
         * @struct workerSlot
         * @brief Worker thread and its paired task queues
         * @details
         * Immediate, high, normal, low, and activated deferred tasks are stored in
         * separate locked deques. The owning worker pops regular tasks from the end,
         * while other workers steal from the beginning. Priority is handled by probing
         * queues in priority order instead of using a priority queue.
         *
         * @note Unactivated deferred tasks are not stored here. They remain in the
         *       taskDelegatorV2-level FIFO deferred queue until runDeferred() or
         *       runAllDeferred() activates them.
         */
        struct workerSlot
        {
            thread worker_;
            lockedDeque<taskPtr> immediate_; ///< Immediate tasks assigned to this worker
            lockedDeque<taskPtr> high_; ///< High-priority runnable tasks
            lockedDeque<taskPtr> normal_; ///< Normal-priority runnable tasks
            lockedDeque<taskPtr> low_; ///< Low-priority runnable tasks
            lockedDeque<taskPtr> deferred_; ///< Activated deferred runnable tasks
            atomic<bool> idle_; ///< Whether this worker is currently idle

            /**
             * @brief Constructs an idle worker slot without starting its thread
             */
            workerSlot();

            /**
             * @brief Pushes a runnable task into the queue matching its priority
             * @param p Task priority, expected to be HIGH, NORMAL, LOW, or DEFERRED
             * @param task Task to enqueue
             * @details DEFERRED here means an already activated deferred task. New
             *          unactivated deferred submissions are stored in the pool-level
             *          deferred FIFO queue instead.
             */
            void push(priority p, taskPtr task);

            /**
             * @brief Pushes an immediate task into this worker slot
             * @param task Task to enqueue
             */
            void pushImmediate(taskPtr task);

            /**
             * @brief Pops an immediate task for the owning worker
             * @return A task if available, otherwise empty alternative
             */
            alternative<taskPtr> popImmediate();

            /**
             * @brief Pops a regular task for the owning worker
             * @return A task if available, otherwise empty alternative
             * @details Probes HIGH, NORMAL, LOW, then DEFERRED, popping from the end.
             */
            alternative<taskPtr> popLocal();

            /**
             * @brief Steals a regular task from this worker slot
             * @return A task if available, otherwise empty alternative
             * @details Probes HIGH, NORMAL, LOW, then DEFERRED, popping from the beginning.
             *          Immediate tasks are not stolen.
             */
            alternative<taskPtr> steal();

            /**
             * @brief Returns whether this slot has no runnable tasks
             */
            [[nodiscard]] bool empty() const noexcept;

            /**
             * @brief Returns number of runnable tasks owned by this slot
             */
            [[nodiscard]] u_integer size() const noexcept;

            /**
             * @brief Returns number of immediate tasks owned by this slot
             */
            [[nodiscard]] u_integer immediateSize() const noexcept;

            /**
             * @brief Returns number of regular non-immediate tasks owned by this slot
             * @details Includes high, normal, low, and activated deferred queues.
             */
            [[nodiscard]] u_integer waitingSize() const noexcept;

            /**
             * @brief Returns number of activated deferred tasks owned by this slot
             */
            [[nodiscard]] u_integer deferredSize() const noexcept;

            /**
             * @brief Clears all queues owned by this slot
             */
            void clear() noexcept;
        };

        array<strongPtr<workerSlot>> workers_; ///< Worker slots with threads and local queues
        lockedQueue<taskPtr> tasks_deferred_; ///< Pool-level FIFO queue for unactivated deferred tasks
        mutable condition condition_; ///< Synchronization condition
        mutable mutex mutex_wait_; ///< Mutex for sleeping and lifecycle transitions
        atomic<bool> stopped_; ///< Stop flag
        atomic<u_integer> active_threads_; ///< Count of active threads
        atomic<u_integer> idle_threads_; ///< Count of idle threads
        atomic<u_integer> next_worker_; ///< Round-robin external submission target

        /**
         * @brief Submits a pre-created task with specified priority
         * @tparam TYPE Task result type
         * @param priority Task priority level
         * @param t Shared pointer to the task
         * @return Future for the task result
         */
        template <typename TYPE>
        async::future<TYPE> submit(priority priority, strongPtr<task<TYPE>> t);

        /**
         * @brief Worker thread main loop
         * @param worker_index Index of the worker slot owned by this thread
         */
        void workerThread(u_integer worker_index);

        /**
         * @brief Attempts to get runnable work for a worker
         * @param worker_index Index of the worker requesting work
         * @return A task if local or stealable work exists, otherwise empty alternative
         * @details Acquisition order is immediate, local HIGH/NORMAL/LOW/DEFERRED,
         *          then stolen HIGH/NORMAL/LOW/DEFERRED from other workers.
         */
        alternative<taskPtr> tryAcquireTask(u_integer worker_index);

        /**
         * @brief Attempts to steal work from other worker slots
         * @param worker_index Index of the worker requesting stolen work
         * @return A task if stealing succeeds, otherwise empty alternative
         * @details Immediate tasks are not stolen. Victims are scanned in ring order
         *          starting after worker_index.
         */
        alternative<taskPtr> tryStealTask(u_integer worker_index);

        /**
         * @brief Chooses a worker slot for a newly submitted runnable task
         * @return Worker index selected for submission
         * @details Uses round-robin selection for external submissions and activated
         *          deferred tasks.
         */
        [[nodiscard]] u_integer chooseWorker() noexcept;

        /**
         * @brief Finds an idle worker slot for immediate submission
         * @return Worker index if one is idle, otherwise empty alternative
         */
        [[nodiscard]] alternative<u_integer> findIdleWorker() const noexcept;

        /**
         * @brief Returns whether any runnable work exists in worker slots
         * @details Includes immediate, high, normal, low, and activated deferred queues.
         */
        [[nodiscard]] bool hasRunnableTasks() const noexcept;

        /**
         * @brief Moves all deferred tasks to runnable worker queues
         * @return The number of deferred tasks that were activated
         * @details Moves from the pool-level FIFO deferred queue into worker-local
         *          activated deferred queues using chooseWorker().
         */
        u_integer moveAllDeferred();

    public:
        /**
         * @brief Constructs a work-stealing task delegator with a given number of threads
         * @param thread_cnt Number of threads (default: 8)
         */
        explicit taskDelegatorV2(u_integer thread_cnt = 8);

        /**
         * @brief Submits a task with normal priority
         * @tparam Callback Type of the callable
         * @tparam Args Types of the arguments
         * @param c Callable to execute
         * @param args Arguments to forward to the callable
         * @return Future for the task result
         */
        template <typename Callback, typename... Args>
        auto submit(Callback&& c, Args&&... args);

        /**
         * @brief Submits a task with specified priority
         * @tparam Callback Type of the callable
         * @tparam Args Types of the arguments
         * @param priority Task priority level
         * @param c Callable to execute
         * @param args Arguments to forward to the callable
         * @return Future for the task result
         */
        template <typename Callback, typename... Args>
        auto submit(priority priority, Callback&& c, Args&&... args);

        /**
         * @brief Submits an immediate task with timeout
         * @tparam Callback Type of the callable
         * @tparam Args Types of the arguments
         * @param timeout Maximum duration to wait for an idle worker
         * @param c Callable to execute
         * @param args Arguments to forward to the callable
         * @return Future for the task result
         */
        template <typename Callback, typename... Args>
        auto submit(time::duration timeout, Callback&& c, Args&&... args);

        /**
         * @brief Returns number of waiting runnable tasks across worker slots
         * @details Counts high, normal, low, and activated deferred tasks. Unactivated
         *          deferred tasks are reported by deferredCnt().
         */
        u_integer waitingCnt() const noexcept;

        /**
         * @brief Returns number of immediate tasks pending execution
         */
        u_integer immediateCnt() const noexcept;

        /**
         * @brief Activates one deferred task
         */
        void runDeferred();

        /**
         * @brief Activates all deferred tasks
         */
        void runAllDeferred();

        /**
         * @brief Discards one deferred task
         * @return true if a deferred task was successfully discarded
         */
        bool discardDeferred();

        /**
         * @brief Discards all deferred tasks
         */
        void discardAllDeferred();

        /**
         * @brief Returns number of unactivated deferred tasks
         */
        u_integer deferredCnt() const noexcept;

        /**
         * @brief Stops the task delegator
         * @param mode Stop mode (default: KEEP_DEFERRED)
         */
        void stop(stopMode mode = stopMode::KEEP_DEFERRED);

        /**
         * @brief Returns whether this delegator has been stopped
         */
        bool isStopped() const noexcept;

        /**
         * @brief Gets the number of active threads
         */
        u_integer activeThreads() const noexcept;

        /**
         * @brief Gets the number of idle threads
         */
        u_integer idleThreads() const noexcept;

        /**
         * @brief Destructor
         */
        ~taskDelegatorV2();
    };
} // namespace original

// ==================== Task Delegator V2 Internal Implementation ====================

inline original::taskDelegatorV2::workerSlot::workerSlot()
        : idle_(makeAtomic(false))
{
}

inline void original::taskDelegatorV2::workerSlot::push(const priority p, taskPtr task)
{
    switch (p)
    {
    case priority::IMMEDIATE:
        this->pushImmediate(std::move(task));
        break;
    case priority::HIGH:
        this->high_.pushEnd(std::move(task));
        break;
    case priority::NORMAL:
        this->normal_.pushEnd(std::move(task));
        break;
    case priority::LOW:
        this->low_.pushEnd(std::move(task));
        break;
    case priority::DEFERRED:
        this->deferred_.pushEnd(std::move(task));
        break;
    default:
        throw sysError("Unknown priority");
    }
}

inline void original::taskDelegatorV2::workerSlot::pushImmediate(taskPtr task)
{
    this->immediate_.pushEnd(std::move(task));
}

inline auto original::taskDelegatorV2::workerSlot::popImmediate() -> alternative<taskPtr>
{
    return this->immediate_.tryPopEnd();
}

inline auto original::taskDelegatorV2::workerSlot::popLocal() -> alternative<taskPtr>
{
    if (auto task = this->high_.tryPopEnd())
    {
        return task;
    }
    if (auto task = this->normal_.tryPopEnd())
    {
        return task;
    }
    if (auto task = this->low_.tryPopEnd())
    {
        return task;
    }
    if (auto task = this->deferred_.tryPopEnd())
    {
        return task;
    }
    return alternative<taskPtr>{};
}

inline auto original::taskDelegatorV2::workerSlot::steal() -> alternative<taskPtr>
{
    if (auto task = this->high_.tryPopBegin())
    {
        return task;
    }
    if (auto task = this->normal_.tryPopBegin())
    {
        return task;
    }
    if (auto task = this->low_.tryPopBegin())
    {
        return task;
    }
    if (auto task = this->deferred_.tryPopBegin())
    {
        return task;
    }
    return alternative<taskPtr>{};
}

inline bool original::taskDelegatorV2::workerSlot::empty() const noexcept
{
    return this->size() == 0;
}

inline original::u_integer original::taskDelegatorV2::workerSlot::size() const noexcept
{
    return this->immediateSize() + this->waitingSize();
}

inline original::u_integer original::taskDelegatorV2::workerSlot::immediateSize() const noexcept
{
    return this->immediate_.size();
}

inline original::u_integer original::taskDelegatorV2::workerSlot::waitingSize() const noexcept
{
    return this->high_.size() + this->normal_.size()
        + this->low_.size() + this->deferred_.size();
}

inline original::u_integer original::taskDelegatorV2::workerSlot::deferredSize() const noexcept
{
    return this->deferred_.size();
}

inline void original::taskDelegatorV2::workerSlot::clear() noexcept
{
    this->immediate_.clear();
    this->high_.clear();
    this->normal_.clear();
    this->low_.clear();
    this->deferred_.clear();
}

inline auto original::taskDelegatorV2::tryAcquireTask(const u_integer worker_index) -> alternative<taskPtr>
{
    const u_integer worker_count = this->workers_.size();
    if (worker_count == 0)
    {
        return alternative<taskPtr>{};
    }

    workerSlot& self = *this->workers_[worker_index % worker_count];
    if (auto task = self.popImmediate())
    {
        return task;
    }
    if (auto task = self.popLocal())
    {
        return task;
    }
    return this->tryStealTask(worker_index);
}

inline auto original::taskDelegatorV2::tryStealTask(const u_integer worker_index) -> alternative<taskPtr>
{
    const u_integer worker_count = this->workers_.size();
    if (worker_count <= 1)
    {
        return alternative<taskPtr>{};
    }

    const u_integer start = worker_index % worker_count;
    for (u_integer offset = 1; offset < worker_count; offset += 1)
    {
        const u_integer victim_index = (start + offset) % worker_count;
        if (auto task = this->workers_[victim_index]->steal())
        {
            return task;
        }
    }
    return alternative<taskPtr>{};
}

inline original::u_integer original::taskDelegatorV2::chooseWorker() noexcept
{
    const u_integer worker_count = this->workers_.size();
    if (worker_count == 0)
    {
        return 0;
    }

    const u_integer selected = *this->next_worker_ % worker_count;
    this->next_worker_ += 1;
    return selected;
}

inline original::alternative<original::u_integer>
original::taskDelegatorV2::findIdleWorker() const noexcept
{
    const u_integer worker_count = this->workers_.size();
    if (worker_count == 0)
    {
        return alternative<u_integer>{};
    }

    for (u_integer i = 0; i < worker_count; i += 1)
    {
        if (*this->workers_[i]->idle_ && this->workers_[i]->empty())
        {
            return alternative<u_integer>{i};
        }
    }
    return alternative<u_integer>{};
}

inline bool original::taskDelegatorV2::hasRunnableTasks() const noexcept
{
    const u_integer worker_count = this->workers_.size();
    if (worker_count == 0)
    {
        return false;
    }

    for (u_integer i = 0; i < worker_count; i += 1)
    {
        if (!this->workers_[i]->empty())
        {
            return true;
        }
    }
    return false;
}

inline original::u_integer original::taskDelegatorV2::moveAllDeferred()
{
    if (const u_integer worker_count = this->workers_.size(); worker_count == 0)
    {
        return 0;
    }

    u_integer activated = 0;
    while (auto task = this->tasks_deferred_.tryPop())
    {
        this->workers_[this->chooseWorker()]->push(priority::DEFERRED, std::move(*task));
        activated += 1;
    }
    return activated;
}

inline original::taskDelegatorV2::taskDelegatorV2(const u_integer thread_cnt)
    : workers_(thread_cnt),
      stopped_(makeAtomic(false)),
      active_threads_(makeAtomic<u_integer>(0)),
      idle_threads_(makeAtomic<u_integer>(0)),
      next_worker_(makeAtomic<u_integer>(0))
{
    for (u_integer i = 0; i < this->workers_.size(); i += 1)
    {
        this->workers_[i] = makeStrongPtr<workerSlot>();
    }

    for (u_integer i = 0; i < this->workers_.size(); i += 1)
    {
        this->workers_[i]->worker_ = std::move(thread{
            [this, i] { this->workerThread(i); }
        });
    }
}

inline void original::taskDelegatorV2::workerThread(const u_integer worker_index)
{
    const u_integer worker_count = this->workers_.size();
    if (worker_count == 0)
    {
        return;
    }

    workerSlot& slot = *this->workers_[worker_index % worker_count];
    while (true)
    {
        if (auto task = this->tryAcquireTask(worker_index))
        {
            slot.idle_ = false;
            this->active_threads_ += 1;
            (*task)->run();
            this->active_threads_ -= 1;
            continue;
        }

        {
            uniqueLock lock{this->mutex_wait_};
            if (this->stopped_ && !this->hasRunnableTasks())
            {
                slot.idle_ = false;
                return;
            }

            slot.idle_ = true;
            this->idle_threads_ += 1;
            this->condition_.wait(this->mutex_wait_, [this]
            {
                return this->stopped_ || this->hasRunnableTasks();
            });
            this->idle_threads_ -= 1;
            slot.idle_ = false;

            if (this->stopped_ && !this->hasRunnableTasks())
            {
                return;
            }
        }
    }
}

inline original::u_integer original::taskDelegatorV2::waitingCnt() const noexcept
{
    const u_integer worker_count = this->workers_.size();
    if (worker_count == 0)
    {
        return 0;
    }

    u_integer count = 0;
    for (u_integer i = 0; i < worker_count; i += 1)
    {
        count += this->workers_[i]->waitingSize();
    }
    return count;
}

inline original::u_integer original::taskDelegatorV2::immediateCnt() const noexcept
{
    const u_integer worker_count = this->workers_.size();
    if (worker_count == 0)
    {
        return 0;
    }

    u_integer count = 0;
    for (u_integer i = 0; i < worker_count; i += 1)
    {
        count += this->workers_[i]->immediateSize();
    }
    return count;
}

inline original::u_integer original::taskDelegatorV2::deferredCnt() const noexcept
{
    return this->tasks_deferred_.size();
}

inline bool original::taskDelegatorV2::isStopped() const noexcept
{
    return *this->stopped_;
}

inline original::u_integer original::taskDelegatorV2::activeThreads() const noexcept
{
    return *this->active_threads_;
}

inline original::u_integer original::taskDelegatorV2::idleThreads() const noexcept
{
    return *this->idle_threads_;
}

template <typename TYPE>
template <typename Callback, typename... Args>
original::taskDelegatorV2::task<TYPE>::task(Callback&& c, Args&&... args)
    : p([c = std::forward<Callback>(c), ...args = std::forward<Args>(args)]() mutable
    {
        return c(args...);
    })
{
}

template <typename TYPE>
void original::taskDelegatorV2::task<TYPE>::run()
{
    this->p.run();
}

template <typename TYPE>
original::async::future<TYPE> original::taskDelegatorV2::task<TYPE>::getFuture()
{
    return this->p.getFuture();
}

template <typename Callback, typename... Args>
auto original::taskDelegatorV2::submit(Callback&& c, Args&&... args)
{
    return this->submit(priority::NORMAL, std::forward<Callback>(c), std::forward<Args>(args)...);
}

template <typename Callback, typename... Args>
auto original::taskDelegatorV2::submit(const priority priority, Callback&& c, Args&&... args)
{
    using ReturnType = decltype(c(args...));
    strongPtr<task<ReturnType>> new_task = makeStrongPtr<task<ReturnType>>(
        std::forward<Callback>(c),
        std::forward<Args>(args)...
    );
    return this->submit<ReturnType>(priority, new_task);
}

template <typename Callback, typename... Args>
auto original::taskDelegatorV2::submit(time::duration timeout, Callback&& c, Args&&... args)
{
    using ReturnType = decltype(c(args...));
    strongPtr<task<ReturnType>> new_task = makeStrongPtr<task<ReturnType>>(
        std::forward<Callback>(c),
        std::forward<Args>(args)...
    );
    auto f = new_task->getFuture();
    {
        uniqueLock lock{this->mutex_wait_};
        if (*this->stopped_)
        {
            throw sysError("taskDelegator already stopped");
        }
        if (this->workers_.size() == 0)
        {
            throw sysError("No worker threads available");
        }

        const bool success = this->condition_.waitFor(this->mutex_wait_, timeout, [this]
        {
            return *this->stopped_ || *this->idle_threads_ > 0;
        });
        if (*this->stopped_)
        {
            throw sysError("taskDelegator already stopped");
        }
        if (!success)
        {
            throw sysError("No idle threads available within timeout");
        }

        auto worker_index = this->findIdleWorker();
        if (!worker_index)
        {
            throw sysError("No idle threads now");
        }
        this->workers_[*worker_index]->pushImmediate(new_task.template dynamicCastTo<taskBase>());
    }
    this->condition_.notify();
    return f;
}

template <typename TYPE>
original::async::future<TYPE>
original::taskDelegatorV2::submit(const priority priority, strongPtr<task<TYPE>> t)
{
    auto f = t->getFuture();
    {
        uniqueLock lock{this->mutex_wait_};
        if (*this->stopped_)
        {
            throw sysError("taskDelegator already stopped");
        }

        switch (priority)
        {
        case priority::IMMEDIATE:
            {
                if (this->workers_.size() == 0)
                {
                    throw sysError("No worker threads available");
                }
                if (*this->idle_threads_ == 0)
                {
                    throw sysError("No idle threads now");
                }
                auto worker_index = this->findIdleWorker();
                if (!worker_index)
                {
                    throw sysError("No idle threads now");
                }
                this->workers_[*worker_index]->pushImmediate(t.template dynamicCastTo<taskBase>());
                break;
            }
        case priority::HIGH:
        case priority::NORMAL:
        case priority::LOW:
            if (this->workers_.size() == 0)
            {
                throw sysError("No worker threads available");
            }
            this->workers_[this->chooseWorker()]->push(priority, t.template dynamicCastTo<taskBase>());
            break;
        case priority::DEFERRED:
            this->tasks_deferred_.push(t.template dynamicCastTo<taskBase>());
            return f;
        default:
            throw sysError("Unknown priority");
        }
    }
    this->condition_.notify();
    return f;
}

inline void original::taskDelegatorV2::runDeferred()
{
    bool activated = false;
    {
        uniqueLock lock{this->mutex_wait_};
        if (this->workers_.size() == 0)
        {
            return;
        }

        if (auto task = this->tasks_deferred_.tryPop())
        {
            this->workers_[this->chooseWorker()]->push(priority::DEFERRED, std::move(*task));
            activated = true;
        }
    }

    if (activated)
    {
        this->condition_.notify();
    }
}

inline void original::taskDelegatorV2::runAllDeferred()
{
    u_integer activated = 0;
    {
        uniqueLock lock{this->mutex_wait_};
        activated = this->moveAllDeferred();
    }

    if (activated != 0)
    {
        this->condition_.notifySome(activated);
    }
}

inline bool original::taskDelegatorV2::discardDeferred()
{
    uniqueLock lock{this->mutex_wait_};
    return this->tasks_deferred_.tryPop().hasValue();
}

inline void original::taskDelegatorV2::discardAllDeferred()
{
    uniqueLock lock{this->mutex_wait_};
    this->tasks_deferred_.clear();
}

inline void original::taskDelegatorV2::stop(const stopMode mode)
{
    if (*this->stopped_)
    {
        return;
    }

    {
        uniqueLock lock{this->mutex_wait_};
        if (bool expected = false;
            !this->stopped_.exchangeCmp(expected, true))
        {
            return;
        }

        switch (mode)
        {
        case RUN_DEFERRED:
            this->moveAllDeferred();
            break;
        case DISCARD_DEFERRED:
            this->tasks_deferred_.clear();
            break;
        case KEEP_DEFERRED:
            break;
        default:
            throw sysError("Unknown stop mode");
        }
    }
    this->condition_.notifyAll();
}

inline original::taskDelegatorV2::~taskDelegatorV2()
{
    this->stop(stopMode::RUN_DEFERRED);
    for (u_integer i = 0; i < this->workers_.size(); i += 1)
    {
        if (this->workers_[i] && this->workers_[i]->worker_.joinable())
        {
            this->workers_[i]->worker_.join();
        }
    }
}

// ==================== Task Implementation ====================

template <typename TYPE>
template <typename Callback, typename... Args>
original::taskDelegator::task<TYPE>::task(Callback&& c, Args&&... args)
    : p([c = std::forward<Callback>(c), ...args = std::forward<Args>(args)]() mutable
    {
        return c(args...);
    })
{
}

template <typename TYPE>
void original::taskDelegator::task<TYPE>::run()
{
    this->p.run();
}

template <typename TYPE>
original::async::future<TYPE> original::taskDelegator::task<TYPE>::getFuture()
{
    return this->p.getFuture();
}

// ==================== Task Delegator Implementation ====================

template <typename COUPLE>
bool original::taskDelegator::taskComparator<COUPLE>::operator()(const COUPLE& lhs, const COUPLE& rhs) const
{
    return static_cast<u_integer>(lhs.second()) < static_cast<u_integer>(rhs.second());
}

inline void original::taskDelegator::workerThread()
{
    while (true)
    {
        strongPtr<taskBase> task;

        if (auto opt = this->task_immediate_.tryPop())
        {
            task = std::move(*opt);
        }
        else if (auto opt2 = this->tasks_waiting_.tryPop())
        {
            task = std::move(opt2->first());
        }
        else
        {
            uniqueLock lock{this->mutex_wait_};
            this->idle_threads_ += 1;
            this->condition_.wait(this->mutex_wait_, [this]
            {
                return this->stopped_ || !this->task_immediate_.empty()
                    || !this->tasks_waiting_.empty();
            });
            this->idle_threads_ -= 1;

            if (this->stopped_ && this->task_immediate_.empty()
                && this->tasks_waiting_.empty())
            {
                return;
            }
            continue;
        }

        if (task)
        {
            this->active_threads_ += 1;
            task->run();
            this->active_threads_ -= 1;
        }
    }
}

inline original::u_integer original::taskDelegator::moveAllDeferred()
{
    u_integer activated = 0;
    while (auto task = this->tasks_deferred_.tryPop())
    {
        this->tasks_waiting_.push(priorityTask{std::move(*task), priority::DEFERRED});
        activated += 1;
    }
    return activated;
}

inline original::taskDelegator::taskDelegator(const u_integer thread_cnt)
    : threads_(thread_cnt),
      stopped_(makeAtomic(false)),
      active_threads_(makeAtomic<u_integer>(0)),
      idle_threads_(makeAtomic<u_integer>(0))
{
    for (auto& thread_ : this->threads_)
    {
        thread_ = std::move(thread{
            [this] { this->workerThread(); }
        });
    }
}

template <typename Callback, typename... Args>
auto original::taskDelegator::submit(Callback&& c, Args&&... args)
{
    return this->submit(priority::NORMAL, std::forward<Callback>(c), std::forward<Args>(args)...);
}

template <typename Callback, typename... Args>
auto original::taskDelegator::submit(const priority priority, Callback&& c, Args&&... args)
{
    using ReturnType = decltype(c(args...));
    strongPtr<task<ReturnType>> new_task = makeStrongPtr<task<ReturnType>>(
        std::forward<Callback>(c),
        std::forward<Args>(args)...
    );
    return this->submit<ReturnType>(priority, new_task);
}

template <typename Callback, typename... Args>
auto original::taskDelegator::submit(time::duration timeout, Callback&& c, Args&&... args)
{
    using ReturnType = decltype(c(args...));
    strongPtr<task<ReturnType>> new_task = makeStrongPtr<task<ReturnType>>(
        std::forward<Callback>(c),
        std::forward<Args>(args)...
    );
    auto f = new_task->getFuture();
    {
        uniqueLock lock{this->mutex_wait_};
        if (this->stopped_)
        {
            throw sysError("taskDelegator already stopped");
        }
        const bool success = this->condition_.waitFor(this->mutex_wait_, timeout, [this]
        {
            return *this->idle_threads_ > 0;
        });
        if (!success)
        {
            throw sysError("No idle threads available within timeout");
        }
        this->task_immediate_.push(std::move(new_task.template dynamicCastTo<taskBase>()));
    }
    this->condition_.notify();
    return f;
}

inline original::u_integer original::taskDelegator::waitingCnt() const noexcept
{
    return this->tasks_waiting_.size();
}

inline original::u_integer original::taskDelegator::immediateCnt() const noexcept
{
    return this->task_immediate_.size();
}

template <typename TYPE>
original::async::future<TYPE>
original::taskDelegator::submit(const priority priority, strongPtr<task<TYPE>> t)
{
    auto f = t->getFuture();
    {
        uniqueLock lock{this->mutex_wait_};
        if (this->stopped_)
        {
            throw sysError("taskDelegator already stopped");
        }
        switch (priority)
        {
        case priority::IMMEDIATE:
            if (*this->idle_threads_ == 0)
            {
                throw sysError("No idle threads now");
            }
            this->task_immediate_.push(std::move(t.template dynamicCastTo<taskBase>()));
            break;
        case priority::HIGH:
        case priority::NORMAL:
        case priority::LOW:
            this->tasks_waiting_.push(priorityTask{t.template dynamicCastTo<taskBase>(), priority});
            break;
        case priority::DEFERRED:
            this->tasks_deferred_.push(t.template dynamicCastTo<taskBase>());
            return f;
        default:
            throw sysError("Unknown priority");
        }
    }
    this->condition_.notify();
    return f;
}

inline void original::taskDelegator::runDeferred()
{
    if (auto task = this->tasks_deferred_.tryPop())
    {
        this->tasks_waiting_.push(priorityTask{std::move(*task), priority::DEFERRED});
        this->condition_.notify();
    }
}

inline void original::taskDelegator::runAllDeferred()
{
    this->condition_.notifySome(this->moveAllDeferred());
}

inline bool original::taskDelegator::discardDeferred()
{
    return this->tasks_deferred_.tryPop().hasValue();
}

inline void original::taskDelegator::discardAllDeferred()
{
    this->tasks_deferred_.clear();
}

inline original::u_integer original::taskDelegator::deferredCnt() const noexcept
{
    return this->tasks_deferred_.size();
}

inline void original::taskDelegator::stop(const stopMode mode)
{
    if (this->stopped_)
        return;

    {
        uniqueLock lock{this->mutex_wait_};
        if (bool expected = false;
            !this->stopped_.exchangeCmp(expected, true))
        {
            return;
        }
        switch (mode)
        {
        case RUN_DEFERRED:
            this->moveAllDeferred();
            break;
        case DISCARD_DEFERRED:
            this->discardAllDeferred();
            break;
        case KEEP_DEFERRED:
            break;
        default:
            throw sysError("Unknown stop mode");
        }
    }
    this->condition_.notifyAll();
}

inline bool original::taskDelegator::isStopped() const noexcept
{
    return *this->stopped_;
}

inline original::u_integer original::taskDelegator::activeThreads() const noexcept
{
    return *this->active_threads_;
}

inline original::u_integer original::taskDelegator::idleThreads() const noexcept
{
    return *this->idle_threads_;
}

inline original::taskDelegator::~taskDelegator()
{
    this->stop(stopMode::RUN_DEFERRED);
    for (auto& thread : this->threads_)
    {
        if (thread.joinable())
            thread.join();
    }
}

#endif //ORIGINAL_TASKS_H
