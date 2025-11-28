#ifndef ORIGINAL_EXECUTORS_H
#define ORIGINAL_EXECUTORS_H
#include "async.h"
#include "executor.h"

namespace original {
    class syncExecutor : public executor {
        lockedQueue<std::coroutine_handle<>> queue_;
        atomic<bool> stopping_;
    public:
        syncExecutor();

        void schedule(std::coroutine_handle<> handle) override;

        template<typename TYPE>
        TYPE wait(coroutine::task<TYPE>&& t);

        bool hasStopped() const noexcept;

        void stop() noexcept;

        ~syncExecutor() override;
    };

    class threadPoolExecutor : public executor {
        taskDelegator& delegator_;
    public:
        explicit threadPoolExecutor(taskDelegator& delegator);

        void schedule(std::coroutine_handle<> handle) override;
    };
}

original::syncExecutor::syncExecutor() : stopping_(makeAtomic(false)) {}

void original::syncExecutor::schedule(std::coroutine_handle<> handle) {
    if (!this->stopping_)
        this->queue_.push(handle);
}

template<typename TYPE>
TYPE original::syncExecutor::wait(coroutine::task<TYPE>&& t) {
    t.via(*this);
    if (t.ready()) {
        return t.result();
    }
    while (!t.ready() && !this->hasStopped()) {
        std::coroutine_handle<> handle = this->queue_.pop();
        if (handle) {
            handle.resume();
        }
    }
    if (!t.ready()) {
        throw sysError("syncExecutor stopped before task finished");
    }
    return t.result();
}

bool original::syncExecutor::hasStopped() const noexcept {
    return *this->stopping_;
}

void original::syncExecutor::stop() noexcept {
    if (!this->hasStopped())
        this->stopping_ = true;
}

original::syncExecutor::~syncExecutor() {
    this->stop();
}

original::threadPoolExecutor::threadPoolExecutor(taskDelegator& delegator)
        : delegator_(delegator) {}

void original::threadPoolExecutor::schedule(std::coroutine_handle<> handle) {
    this->delegator_.submit([handle]{
        handle.resume();
    });
}

#endif //ORIGINAL_EXECUTORS_H
