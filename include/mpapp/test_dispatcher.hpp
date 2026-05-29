// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Async Executor and Event Loops.md
//
// Deterministic test dispatcher.
//
// Backs `mpapp::main_dispatcher()` for the skeleton build (no real platform
// event-loop is hooked up yet). The dispatcher exposes:
//
//   * post(callable)              — enqueue ready-to-run work
//   * post_after(duration, fn)    — enqueue a timed callback
//   * advance(duration)           — virtually advance the clock by `d` and
//                                   fire every callback whose deadline is at
//                                   or before the new "now". Ready work is
//                                   drained after each tick so callbacks that
//                                   post follow-on work are observed.
//   * run_until_idle()            — drain the ready queue without advancing
//                                   the virtual clock.
//
// The clock is virtual: it does not advance on its own and never sleeps the
// real thread. This is what `test_dispatcher::advance(d)` exists for and what
// makes time-dependent tests deterministic per ADR-0008.

#ifndef MPAPP_TEST_DISPATCHER_HPP
#define MPAPP_TEST_DISPATCHER_HPP

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>
#include <utility>

#include <mpapp/executor.hpp>

namespace mpapp {

class test_dispatcher : public dispatcher {
public:
    using clock      = std::chrono::steady_clock;
    using duration   = clock::duration;
    using time_point = clock::time_point;

    test_dispatcher() noexcept
        : owner_thread_(std::this_thread::get_id()) {}

    // dispatcher contract: enqueue immediately-runnable work.
    void post(std::function<void()> work) override {
        std::lock_guard lock{mu_};
        ready_.push(std::move(work));
    }

    // Enqueue work to fire after `d` of virtual time. Overrides the
    // dispatcher contract; the mock uses virtual time (fires on advance())
    // rather than a wall-clock timer.
    void post_after(duration d, std::function<void()> work) override {
        std::lock_guard lock{mu_};
        timed_.push(timed_entry{now_ + d, ++sequence_, std::move(work)});
    }

    // Drain the ready queue. Returns the number of callbacks fired.
    std::size_t run_until_idle() {
        std::size_t fired = 0;
        while (auto fn = pop_ready_()) {
            (*fn)();
            ++fired;
        }
        return fired;
    }

    // Advance virtual time by `d`. Every timed callback whose deadline is at
    // or before the new now fires; ready work is drained after each timer tick
    // so cascading work (a timer posts follow-on work) settles deterministically.
    std::size_t advance(duration d) {
        std::size_t fired = 0;
        {
            std::lock_guard lock{mu_};
            now_ += d;
        }
        for (;;) {
            fired += run_until_idle();
            auto fn = pop_due_timer_();
            if (!fn) {
                break;
            }
            (*fn)();
            ++fired;
        }
        return fired;
    }

    // Virtual "now". Stable until advance() is called.
    time_point now() const noexcept {
        std::lock_guard lock{mu_};
        return now_;
    }

    // The thread on which the dispatcher was constructed. ui_task<> uses this
    // to check that resumption landed on the right thread in tests.
    std::thread::id owner_thread() const noexcept { return owner_thread_; }

private:
    struct timed_entry {
        time_point                deadline;
        std::uint64_t             seq;
        std::function<void()>     fn;

        // Min-heap by deadline; ties broken by insertion order for determinism.
        bool operator<(const timed_entry& other) const noexcept {
            if (deadline != other.deadline) {
                return deadline > other.deadline;
            }
            return seq > other.seq;
        }
    };

    std::optional<std::function<void()>> pop_ready_() {
        std::lock_guard lock{mu_};
        if (ready_.empty()) {
            return std::nullopt;
        }
        auto fn = std::move(ready_.front());
        ready_.pop();
        return fn;
    }

    std::optional<std::function<void()>> pop_due_timer_() {
        std::lock_guard lock{mu_};
        if (timed_.empty() || timed_.top().deadline > now_) {
            return std::nullopt;
        }
        auto fn = std::move(const_cast<timed_entry&>(timed_.top()).fn);
        timed_.pop();
        return fn;
    }

    mutable std::mutex                  mu_;
    std::queue<std::function<void()>>   ready_;
    std::priority_queue<timed_entry>    timed_;
    time_point                          now_{};
    std::uint64_t                       sequence_  = 0;
    std::thread::id                     owner_thread_;
};

} // namespace mpapp

#endif // MPAPP_TEST_DISPATCHER_HPP
