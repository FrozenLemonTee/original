#ifndef ORIGINAL_EXECUTORS_H
#define ORIGINAL_EXECUTORS_H
#include "async.h"
#include "lockedQueue.h"
#include "tasks.h"
#include "executor.h"

namespace original {
    class syncExecutor final : public executor {
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

        void timerLoop();
    public:
        syncExecutor();

        void schedule(Func fn) override;

        void schedule(time::duration delay, Func fn) override;

        void runOnce();

        void run();

        void runUntilIdle();

        [[nodiscard]] bool hasStopped() const noexcept;

        void stop() noexcept;

        ~syncExecutor() override;
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
bool original::syncExecutor::timerComparator<COUPLE>::operator()(const COUPLE& lhs, const COUPLE& rhs) const
{
    return lhs.first() < rhs.first();
}

inline void original::syncExecutor::timerLoop()
{
    while (!this->hasStopped()) {
        const auto opt = this->timer_queue_.top();
        if (!opt) {
            thread::yield();
            continue;
        }
        auto& task = *opt;
        if (auto now = time::point::now(); task.first() > now) {
            auto wait_time = task.first() - now;
            if (auto pop = this->timer_queue_.popFor(wait_time); !pop) {
                thread::yield();
                continue;
            }
            this->schedule(task.second());
        } else {
            if (auto pop = this->timer_queue_.tryPop())
                this->schedule(std::move(pop->second()));
        }
    }
}

inline original::syncExecutor::syncExecutor()
    : stopping_(makeAtomic(false)), worker_(std::move([this]{ this->timerLoop(); })) {}

inline void original::syncExecutor::schedule(Func fn)
{
    if (!this->hasStopped() && fn)
        this->events_queue_.push(std::move(fn));
}

inline void original::syncExecutor::schedule(const time::duration delay, Func fn)
{
    if (!this->hasStopped() && fn) {
        const auto when = time::point::now() + delay;
        this->timer_queue_.push(std::move(timerTask{when, fn}));
    }
}

inline void original::syncExecutor::runOnce()
{
    if (const auto fn = this->events_queue_.pop()) {
        fn();
    } else {
        thread::yield();
    }
}

inline void original::syncExecutor::run()
{
    while (!this->hasStopped()) {
        this->runOnce();
    }
}

inline void original::syncExecutor::runUntilIdle()
{
    while (auto opt = this->events_queue_.tryPop()) {
        if (auto& fn = *opt) {
            fn();
        } else {
            thread::yield();
        }
    }
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
