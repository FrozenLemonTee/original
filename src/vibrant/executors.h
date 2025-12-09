#ifndef ORIGINAL_EXECUTORS_H
#define ORIGINAL_EXECUTORS_H
#include "async.h"
#include "lockedQueue.h"
#include "tasks.h"
#include "executor.h"

namespace original {
    class eventsLoopExecutor final : public executor {
        using executor::Func;

        lockedQueue<Func> events_queue_{};
        atomic<bool> stopping_;

        using timerTask = couple<time::point, Func>;
        template<typename COUPLE>
        struct timerComparator {
            bool operator()(const COUPLE& lhs, const COUPLE& rhs) const;
        };

        lockedPrique<timerTask, timerComparator> timer_queue_{};
        thread worker_;

        void eventsLoop();

        void timerLoop();
    public:
        eventsLoopExecutor();

        void schedule(Func fn) override;

        void schedule(time::duration delay, Func fn) override;

        void runOnce();

        void run();

        [[nodiscard]] bool hasStopped() const noexcept;

        void stop() noexcept;

        ~eventsLoopExecutor() override;
    };

    class threadPoolExecutor final : public executor {
        using executor::Func;

        taskDelegator& delegator_;
    public:
        explicit threadPoolExecutor(taskDelegator& delegator);

        void schedule(Func fn) override;

        void schedule(time::duration delay, Func fn) override;
    };
}

template <typename COUPLE>
bool original::eventsLoopExecutor::timerComparator<COUPLE>::operator()(const COUPLE& lhs, const COUPLE& rhs) const
{
    return lhs.first() < rhs.first();
}

inline void original::eventsLoopExecutor::eventsLoop()
{
    while (!this->hasStopped()) {
        if (auto opt_fn = this->events_queue_.tryPop()) {
            if (auto& fn = *opt_fn) {
                fn();
            }
        } else {
            thread::yield();
        }
    }
}

inline void original::eventsLoopExecutor::timerLoop()
{
    while (!this->hasStopped()) {
        auto opt = this->timer_queue_.top();
        if (!opt) {
            thread::yield();
            continue;
        }

        if (auto now = time::point::now(); opt->first() <= now) {
            if (auto popped = timer_queue_.tryPop())
            {
                this->schedule(std::move(popped->second()));
            }
        } else {
            thread::yield();
        }
    }
}

inline original::eventsLoopExecutor::eventsLoopExecutor()
    : stopping_(makeAtomic(false)),
      worker_(std::move([this]{ this->timerLoop(); })) {}

inline void original::eventsLoopExecutor::schedule(Func fn)
{
    if (!this->hasStopped() && fn)
        this->events_queue_.push(std::move(fn));
}

inline void original::eventsLoopExecutor::schedule(const time::duration delay, Func fn)
{
    if (!this->hasStopped() && fn) {
        const auto when = time::point::now() + delay;
        this->timer_queue_.push(std::move(timerTask{when, std::move(fn)}));
    }
}

inline void original::eventsLoopExecutor::runOnce()
{
    if (const auto opt = this->events_queue_.tryPop()) {
        if (auto& fn = *opt) {
            fn();
        }
    }
}

inline void original::eventsLoopExecutor::run()
{
    this->eventsLoop();
}

inline bool original::eventsLoopExecutor::hasStopped() const noexcept {
    return *this->stopping_;
}

inline void original::eventsLoopExecutor::stop() noexcept {
    if (!this->hasStopped())
        this->stopping_ = true;
}

inline original::eventsLoopExecutor::~eventsLoopExecutor() {
    this->stop();
    if (this->worker_.joinable())
        this->worker_.join();
}

inline original::threadPoolExecutor::threadPoolExecutor(taskDelegator& delegator)
        : delegator_(delegator) {}

inline void original::threadPoolExecutor::schedule(Func fn)
{
    if (!fn)
        return;

    this->delegator_.submit(std::move(fn));
}

inline void original::threadPoolExecutor::schedule(time::duration delay, Func fn)
{
    if (!fn)
        return;

    this->delegator_.submit([fn = std::move(fn), delay = std::move(delay)] {
        thread::sleep(delay);
        fn();
    });
}

#endif //ORIGINAL_EXECUTORS_H
