// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Async Executor and Event Loops.md
//
// Async executor + coroutine task types — skeleton implementation.
//
// This header defines the public API surface for MPAPP's coroutine-based
// async model:
//
//   * mpapp::dispatcher          — abstract dispatcher. Posting work and being
//                                  awaited resume on the dispatcher's thread.
//   * mpapp::main_dispatcher()   — process-wide UI-thread dispatcher (backed
//                                  by a `test_dispatcher` for this skeleton;
//                                  real per-platform impls land in P3+).
//   * mpapp::executor            — background thread-pool stub. `schedule()`
//                                  enqueues work; `co_await` resumes on the
//                                  pool.
//   * mpapp::task<T>             — eager-start coroutine return type.
//                                  Cancellable via `::mpapp::stop_token`.
//   * mpapp::ui_task<T>          — same, but its final_suspend reschedules
//                                  resumption of the continuation on
//                                  `main_dispatcher()`.
//   * mpapp::async_sleep(d, ...) — awaitable that yields control for `d` of
//                                  virtual time on the main dispatcher.
//
// Real platform-native event-loop integrations (IOCP, io_uring, kqueue,
// ALooper) stay stubbed for this batch. The mock implementation that backs
// `main_dispatcher()` lives in `src/executor/mock.cpp`.

#ifndef MPAPP_EXECUTOR_HPP
#define MPAPP_EXECUTOR_HPP

#include <atomic>
#include <chrono>
#include <coroutine>
#include <exception>
#include <functional>
#include "detail/stop_token_compat.hpp"
#include <type_traits>
#include <utility>
#include <variant>

namespace mpapp {

// Forward declarations for the awaitable returned by `dispatcher::operator
// co_await()`. Defined inline at the bottom of this header.
class dispatcher;
class executor;

// ---------------------------------------------------------------------------
// dispatcher
// ---------------------------------------------------------------------------
//
// Abstract base for a per-thread message dispatcher. Awaiting a dispatcher
// suspends the current coroutine and reschedules its resumption via post().

class dispatcher {
public:
    virtual ~dispatcher() = default;

    // Enqueue `work` to run on the dispatcher's thread. Implementations must
    // be safe to call from any thread.
    virtual void post(std::function<void()> work) = 0;

    // Awaitable: resumes the coroutine on this dispatcher's thread.
    auto operator co_await() const noexcept;
};

// The process-wide UI-thread dispatcher. Defined in src/executor/mock.cpp.
// Returns a reference to a deterministic test_dispatcher in the skeleton
// build; real per-platform implementations replace this in P3+.
dispatcher& main_dispatcher() noexcept;

// ---------------------------------------------------------------------------
// executor
// ---------------------------------------------------------------------------
//
// Background-thread executor. The skeleton implementation forwards work to
// `main_dispatcher()` so tests can drive it deterministically. A real
// work-stealing pool replaces this in P3+ (see the architecture note).

class executor {
public:
    executor() noexcept = default;

    executor(const executor&)            = delete;
    executor& operator=(const executor&) = delete;
    executor(executor&&)                 = delete;
    executor& operator=(executor&&)      = delete;

    void schedule(std::function<void()> work) {
        main_dispatcher().post(std::move(work));
    }

    auto operator co_await() const noexcept;

    static executor& current() noexcept {
        static executor inst;
        return inst;
    }
};

// ---------------------------------------------------------------------------
// task<T> — eager-start coroutine
// ---------------------------------------------------------------------------
//
// Awaiting a task<T> suspends the awaiting coroutine until the task
// completes, then resumes it with the result. If the task threw, the
// exception is rethrown on await_resume.
//
// Cancellation is observed via `::mpapp::stop_token`. Calling `request_stop()`
// signals the token; coroutines inside the task must check
// `stop_token.stop_requested()` explicitly — no exception unwind (matches
// `std::jthread`, per the architecture note).

namespace detail {

// Continuation resumer used by `final_suspend`. For plain `task<T>` it just
// resumes the continuation directly; `ui_task<T>` overrides this to post the
// resumption to `main_dispatcher()`.
struct continuation_resumer_direct {
    void operator()(std::coroutine_handle<> h) const noexcept { h.resume(); }
};

struct continuation_resumer_main {
    void operator()(std::coroutine_handle<> h) const {
        main_dispatcher().post([h] { h.resume(); });
    }
};

// Shared state stored inside the promise. Holds either the result (T or
// std::monostate-for-void), an exception_ptr, or nothing yet.
template <class T>
struct result_slot {
    using stored_t = std::conditional_t<std::is_void_v<T>, std::monostate, T>;
    std::variant<std::monostate, stored_t, std::exception_ptr> v;

    template <class U>
    void set_value(U&& u) {
        v.template emplace<1>(std::forward<U>(u));
    }
    void set_void() {
        v.template emplace<1>(std::monostate{});
    }
    void set_exception(std::exception_ptr e) noexcept {
        v.template emplace<2>(std::move(e));
    }

    T extract() {
        if (v.index() == 2) {
            std::rethrow_exception(std::get<2>(v));
        }
        if constexpr (std::is_void_v<T>) {
            return;
        } else {
            return std::move(std::get<1>(v));
        }
    }
};

template <class T, class FinalResumer>
struct task_promise_base {
    result_slot<T>         slot;
    std::coroutine_handle<> continuation = nullptr;
    ::mpapp::stop_source       stop_src;
    std::atomic<bool>      done{false};

    struct final_awaiter {
        bool await_ready() const noexcept { return false; }

        template <class P>
        void await_suspend(std::coroutine_handle<P> h) const noexcept {
            auto& p = h.promise();
            p.done.store(true, std::memory_order_release);
            if (p.continuation) {
                FinalResumer{}(p.continuation);
            }
        }

        void await_resume() const noexcept {}
    };

    std::suspend_never  initial_suspend() noexcept { return {}; }
    final_awaiter       final_suspend() noexcept { return {}; }

    void unhandled_exception() noexcept {
        slot.set_exception(std::current_exception());
    }
};

// Specialization-friendly promise. `return_value` / `return_void` chosen by
// `T`. We use an inheritance helper to avoid duplicating boilerplate.
template <class T, class FinalResumer>
struct task_promise_value : task_promise_base<T, FinalResumer> {
    template <class U>
        requires std::is_convertible_v<U&&, T>
    void return_value(U&& u) {
        this->slot.template set_value<U>(std::forward<U>(u));
    }
};

template <class FinalResumer>
struct task_promise_value<void, FinalResumer> : task_promise_base<void, FinalResumer> {
    void return_void() noexcept { this->slot.set_void(); }
};

// basic_task is the shared coroutine return type. `task<T>` and `ui_task<T>`
// alias it with different FinalResumer policies — only difference is whether
// final_suspend resumes the continuation directly (background) or reposts it
// through `main_dispatcher()` (UI thread).
template <class T, class FinalResumer>
struct basic_task {
    struct promise_type
        : task_promise_value<T, FinalResumer> {
        basic_task get_return_object() noexcept {
            return basic_task{
                std::coroutine_handle<promise_type>::from_promise(*this)};
        }
    };

    using handle_type = std::coroutine_handle<promise_type>;

    basic_task() noexcept = default;
    explicit basic_task(handle_type h) noexcept : h_(h) {}

    basic_task(const basic_task&)            = delete;
    basic_task& operator=(const basic_task&) = delete;

    basic_task(basic_task&& other) noexcept
        : h_(std::exchange(other.h_, {})) {}
    basic_task& operator=(basic_task&& other) noexcept {
        if (this != &other) {
            if (h_) {
                h_.destroy();
            }
            h_ = std::exchange(other.h_, {});
        }
        return *this;
    }

    ~basic_task() {
        if (h_) {
            h_.destroy();
        }
    }

    // Cancellation. Future stop-aware awaitables (e.g. async_sleep) short-
    // circuit once the token is signalled; in-task code polls via
    // stop_requested(). Matches std::jthread semantics — no exception unwind.
    void request_stop() noexcept {
        if (h_) {
            h_.promise().stop_src.request_stop();
        }
    }

    bool is_cancelled() const noexcept {
        return h_ && h_.promise().stop_src.stop_requested();
    }

    ::mpapp::stop_token get_stop_token() const noexcept {
        return h_ ? h_.promise().stop_src.get_token() : ::mpapp::stop_token{};
    }

    bool is_ready() const noexcept {
        return h_ && h_.promise().done.load(std::memory_order_acquire);
    }

    bool await_ready() const noexcept { return is_ready(); }

    void await_suspend(std::coroutine_handle<> awaiter) noexcept {
        h_.promise().continuation = awaiter;
    }

    T await_resume() {
        return h_.promise().slot.extract();
    }

private:
    handle_type h_{};
};

} // namespace detail

template <class T = void>
using task = detail::basic_task<T, detail::continuation_resumer_direct>;

template <class T = void>
using ui_task = detail::basic_task<T, detail::continuation_resumer_main>;

// ---------------------------------------------------------------------------
// dispatcher / executor co_await — defined after task<> so they're complete.
// ---------------------------------------------------------------------------

namespace detail {

struct dispatcher_awaitable {
    dispatcher* d;

    bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<> h) const {
        d->post([h] { h.resume(); });
    }
    void await_resume() const noexcept {}
};

} // namespace detail

inline auto dispatcher::operator co_await() const noexcept {
    return detail::dispatcher_awaitable{const_cast<dispatcher*>(this)};
}

inline auto executor::operator co_await() const noexcept {
    // The skeleton routes pool work back to main_dispatcher() so tests can
    // drive it deterministically. A real pool replaces this in P3+.
    return detail::dispatcher_awaitable{&main_dispatcher()};
}

// ---------------------------------------------------------------------------
// async_sleep
// ---------------------------------------------------------------------------
//
// Suspends the current coroutine for `d` of virtual time on the main
// dispatcher. The skeleton uses `test_dispatcher::post_after`; real
// implementations route through the platform's timer queue.
//
// If a stop_token is supplied and is already requested, the awaitable
// resumes immediately without scheduling a timer.

namespace detail {

void post_after_on_main(std::chrono::steady_clock::duration d,
                        std::function<void()>               work);

template <class Rep, class Period>
struct sleep_awaitable {
    std::chrono::duration<Rep, Period> dur;
    ::mpapp::stop_token                    stop;

    bool await_ready() const noexcept {
        return stop.stop_requested() || dur.count() <= 0;
    }

    void await_suspend(std::coroutine_handle<> h) const {
        if (stop.stop_requested()) {
            h.resume();
            return;
        }
        post_after_on_main(
            std::chrono::duration_cast<std::chrono::steady_clock::duration>(dur),
            [h] { h.resume(); });
    }

    void await_resume() const noexcept {}
};

} // namespace detail

template <class Rep, class Period>
auto async_sleep(std::chrono::duration<Rep, Period> d,
                 ::mpapp::stop_token                    stop = {}) {
    return detail::sleep_awaitable<Rep, Period>{d, std::move(stop)};
}

} // namespace mpapp

#endif // MPAPP_EXECUTOR_HPP
