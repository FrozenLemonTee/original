#ifndef ORIGINAL_EXECUTORS_H
#define ORIGINAL_EXECUTORS_H
#include "async.h"
#include "lockedQueue.h"
#include "tasks.h"
#include "executor.h"

namespace original {
    class syncExecutor final : public executor {
        lockedQueue<std::coroutine_handle<>> queue_{};
        atomic<bool> stopping_;
    public:
        syncExecutor();

        void schedule(std::coroutine_handle<> handle) override;

        template<typename TYPE>
        TYPE wait(coroutine::task<TYPE> t);

        template<typename TYPE>
        TYPE spinWait(coroutine::task<TYPE> t);

        [[nodiscard]] bool hasStopped() const noexcept;

        void stop() noexcept;

        ~syncExecutor() override;
    };

    class threadPoolExecutor final : public executor {
        taskDelegator& delegator_;
    public:
        explicit threadPoolExecutor(taskDelegator& delegator);

        void schedule(std::coroutine_handle<> handle) override;
    };
}

inline original::syncExecutor::syncExecutor() : stopping_(makeAtomic(false)) {}

inline void original::syncExecutor::schedule(const std::coroutine_handle<> handle) {
    if (!this->stopping_)
        this->queue_.push(handle);
}

template<typename TYPE>
TYPE original::syncExecutor::wait(coroutine::task<TYPE> t) {
    t.viaNext(*this);
    if (!t.started()) {
        t.start();
    }

    if (t.finished()) {
        return t.result();
    }

    while (!t.finished() && !this->hasStopped()) {
        if (auto h = this->queue_.pop()) {
            h.resume();
        }
    }

    if (!t.finished()) {
        throw sysError("syncExecutor stopped before task finished");
    }
    return t.result();
}

template <typename TYPE>
TYPE original::syncExecutor::spinWait(coroutine::task<TYPE> t)
{
    t.viaNext(*this);
    if (!t.started()) {
        t.start();
    }

    if (t.finished()) {
        return t.result();
    }

    while (!t.finished() && !this->hasStopped()) {
        if (auto alt = this->queue_.tryPop()) {
            if (auto h = *alt)
                h.resume();
        } else {
            thread::yield();
        }
    }

    if (!t.finished()) {
        throw sysError("syncExecutor stopped before task finished");
    }

    return t.result();
}

inline bool original::syncExecutor::hasStopped() const noexcept {
    return *this->stopping_;
}

inline void original::syncExecutor::stop() noexcept {
    if (!this->hasStopped())
        this->stopping_ = true;
}

inline original::syncExecutor::~syncExecutor() {
    this->stop();
}

inline original::threadPoolExecutor::threadPoolExecutor(taskDelegator& delegator)
        : delegator_(delegator) {}

inline void original::threadPoolExecutor::schedule(std::coroutine_handle<> handle) {
    if (!handle)
        return;

    this->delegator_.submit([handle]{
        handle.resume();
    });
}

#endif //ORIGINAL_EXECUTORS_H
