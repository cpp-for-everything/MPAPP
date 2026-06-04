// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::real_main_thread` — a REAL, cross-platform backend for the
// `mpapp::main_thread` interface, written once and compiled into every
// target (no ifdefs): the constructing thread's id is captured via
// std::this_thread::get_id() and used as the canonical "main thread".
// begin_invoke_on_main_thread() executes inline when the caller is already
// on the main thread; otherwise it enqueues the action into a thread-safe
// queue that is drained by an explicit pump() call (so unit tests can drive
// dispatch without a real event loop).  Integration with real per-OS
// dispatchers (DispatcherQueue, Android Looper, GTK GMainContext) is a
// per-platform follow-up that replaces pump() with a native wake mechanism.
// Counterpart to MAUI Essentials' MainThread.  No macros; header-only.

#ifndef MPAPP_ESSENTIALS_REAL_MAIN_THREAD_HPP
#define MPAPP_ESSENTIALS_REAL_MAIN_THREAD_HPP

#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

#include "main_thread.hpp"

namespace mpapp {

// Real, portable main-thread dispatcher.
//
// Construction policy:
//   Construct this object on the thread you want to be treated as "main".
//   The constructor captures std::this_thread::get_id() at that moment.
//
// Dispatch policy:
//   - Calling begin_invoke_on_main_thread() from the main thread executes
//     the action immediately (inline) and still increments invoke_count_.
//   - Calling it from any other thread enqueues the action; enqueued actions
//     are run in FIFO order by the next pump() call on the main thread.
//
// pump() policy:
//   Drains every action that was in the queue at the moment pump() was called.
//   Actions enqueued *during* a pump iteration (i.e. by a running action) are
//   left for the next pump() call to avoid unbounded loops.
//
// Rule of Zero: all members are value/RAII types; no explicit destructor,
// copy, or move is needed.  Copy is deleted because copying a mutex and a
// queue with shared thread semantics would be meaningless.
class real_main_thread final : public main_thread {
public:
    // Captures the calling thread's id as the main thread.
    real_main_thread()
        : main_thread_id_{ std::this_thread::get_id() }
    {}

    real_main_thread(const real_main_thread&)            = delete;
    real_main_thread& operator=(const real_main_thread&) = delete;
    real_main_thread(real_main_thread&&)                 = delete;
    real_main_thread& operator=(real_main_thread&&)      = delete;

    // ---- main_thread interface ---------------------------------------------

    // Returns true when the calling thread is the main (UI) thread.
    [[nodiscard]] bool is_main_thread() const override {
        return std::this_thread::get_id() == main_thread_id_;
    }

    // Executes `action` on the main thread.
    // - If called from the main thread: runs `action` inline immediately.
    // - If called from another thread: enqueues `action` for the next pump().
    // Empty/null actions are silently ignored (counter is still incremented).
    void begin_invoke_on_main_thread(std::function<void()> action) override {
        ++invoke_count_;
        if (is_main_thread()) {
            if (action) action();
        } else {
            std::lock_guard<std::mutex> lock{ queue_mutex_ };
            queue_.push(std::move(action));
        }
    }

    // ---- Extended API ------------------------------------------------------

    // Drain all actions currently in the queue.
    // MUST be called from the main thread.  Actions added to the queue by
    // callbacks running inside pump() are deferred to the next call.
    void pump() {
        // Snapshot the queue under the lock; release the lock before
        // executing callbacks so that begin_invoke_on_main_thread() calls
        // made by those callbacks don't dead-lock.
        std::queue<std::function<void()>> batch;
        {
            std::lock_guard<std::mutex> lock{ queue_mutex_ };
            std::swap(batch, queue_);
        }
        while (!batch.empty()) {
            auto action = std::move(batch.front());
            batch.pop();
            if (action) action();
        }
    }

    // Number of times begin_invoke_on_main_thread() has been called (both
    // inline-executed and enqueued paths).
    [[nodiscard]] std::size_t invoke_count() const noexcept {
        return invoke_count_;
    }

    // Number of actions currently waiting in the queue (not yet pumped).
    [[nodiscard]] std::size_t pending_count() const noexcept {
        std::lock_guard<std::mutex> lock{ queue_mutex_ };
        return queue_.size();
    }

private:
    std::thread::id                      main_thread_id_;
    std::size_t                          invoke_count_{ 0 };
    mutable std::mutex                   queue_mutex_{};
    std::queue<std::function<void()>>    queue_{};
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_REAL_MAIN_THREAD_HPP
