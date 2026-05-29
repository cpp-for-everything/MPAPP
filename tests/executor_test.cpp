// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Async Executor and Event Loops.md
//
// Skeleton tests for the executor + task<T> + ui_task<T> + test_dispatcher
// public surface. Real platform dispatchers (IOCP / io_uring / kqueue /
// ALooper) get their own dedicated tests in P3+.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/executor.hpp>
#include <mpapp/test_dispatcher.hpp>

#include <atomic>
#include <chrono>
#include <stop_token>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

namespace {

mpapp::test_dispatcher& main_test_dispatcher() {
    return static_cast<mpapp::test_dispatcher&>(mpapp::main_dispatcher());
}

mpapp::task<int> make_int_task() {
    co_return 42;
}

mpapp::ui_task<void> make_void_ui_task(std::thread::id& observed_thread) {
    co_await mpapp::main_dispatcher();
    observed_thread = std::this_thread::get_id();
    co_return;
}

mpapp::task<int> sleep_then_return(int value) {
    co_await mpapp::async_sleep(100ms);
    co_return value;
}

mpapp::task<int> cancellable_loop(std::stop_token stop,
                                  std::atomic<int>* tick_counter = nullptr) {
    int ticks = 0;
    while (!stop.stop_requested()) {
        co_await mpapp::async_sleep(50ms, stop);
        if (stop.stop_requested()) {
            break;
        }
        ++ticks;
        if (tick_counter != nullptr) {
            tick_counter->store(ticks, std::memory_order_release);
        }
        if (ticks > 1'000'000) {
            break; // safety: should never run away in test
        }
    }
    co_return ticks;
}

} // namespace

TEST_CASE("task<int> resolves on the test dispatcher", "[executor]") {
    auto& d = main_test_dispatcher();
    d.run_until_idle();

    auto t = make_int_task();

    // Eager start: the coroutine ran synchronously through to co_return.
    REQUIRE(t.is_ready());
    REQUIRE(t.await_resume() == 42);
}

TEST_CASE("ui_task<void> resumes on the main dispatcher's thread",
          "[executor]") {
    auto& d = main_test_dispatcher();
    d.run_until_idle();

    std::thread::id observed_thread{};
    auto ui = make_void_ui_task(observed_thread);

    // The ui_task suspends at `co_await main_dispatcher()`. Pumping the
    // dispatcher resumes it on the dispatcher's owner thread.
    REQUIRE_FALSE(ui.is_ready());
    d.run_until_idle();
    REQUIRE(ui.is_ready());
    REQUIRE(observed_thread == d.owner_thread());
}

TEST_CASE("async_sleep fires when test_dispatcher::advance crosses its deadline",
          "[executor]") {
    auto& d = main_test_dispatcher();
    d.run_until_idle();

    auto t = sleep_then_return(7);
    REQUIRE_FALSE(t.is_ready());

    // Not enough virtual time elapsed yet.
    d.advance(50ms);
    REQUIRE_FALSE(t.is_ready());

    // Cross the 100ms deadline; the task completes.
    d.advance(60ms);
    REQUIRE(t.is_ready());
    REQUIRE(t.await_resume() == 7);
}

TEST_CASE("request_stop cancels a long-running task", "[executor]") {
    auto& d = main_test_dispatcher();
    d.run_until_idle();

    // Drive the loop with an external stop_source so we can both observe
    // task::is_cancelled() flip (when we mirror the request on the task) and
    // verify the loop actually exits.
    std::stop_source     src;
    std::atomic<int>     ticks{0};
    auto t = cancellable_loop(src.get_token(), &ticks);

    // Let the loop tick a few times.
    d.advance(50ms);
    d.advance(50ms);
    d.advance(50ms);
    REQUIRE(ticks.load() >= 1);
    REQUIRE_FALSE(t.is_ready());
    REQUIRE_FALSE(t.is_cancelled());

    // Request stop both on the task's own source (so is_cancelled() is true)
    // and on the source the loop observes (so the loop body exits).
    t.request_stop();
    src.request_stop();
    REQUIRE(t.is_cancelled());
    REQUIRE(t.get_stop_token().stop_requested());

    // The next dispatcher tick lets the in-flight async_sleep callback fire,
    // observe stop_requested(), and the coroutine returns.
    d.advance(50ms);
    REQUIRE(t.is_ready());
}

namespace {

// A minimal dispatcher that records how it was driven — stands in for a real
// per-platform dispatcher (glib / DispatcherQueue / Handler) so we can verify
// install_main_dispatcher() routes main_dispatcher() and post_after_on_main()
// through the installed instance, then reverts cleanly to the default.
struct recording_dispatcher : mpapp::dispatcher {
    int posts = 0;
    int timed_posts = 0;
    std::chrono::steady_clock::duration last_delay{};

    void post(std::function<void()> work) override {
        ++posts;
        if (work) work();
    }
    void post_after(std::chrono::steady_clock::duration d,
                    std::function<void()> work) override {
        ++timed_posts;
        last_delay = d;
        if (work) work();
    }
};

} // namespace

TEST_CASE("install_main_dispatcher routes main_dispatcher + reverts to default",
          "[executor]") {
    // Default: main_dispatcher() is the deterministic test_dispatcher.
    REQUIRE(dynamic_cast<mpapp::test_dispatcher*>(&mpapp::main_dispatcher()) != nullptr);

    recording_dispatcher rec;
    mpapp::install_main_dispatcher(&rec);
    REQUIRE(&mpapp::main_dispatcher() == &rec);

    bool ran = false;
    mpapp::main_dispatcher().post([&] { ran = true; });
    REQUIRE(ran);
    REQUIRE(rec.posts == 1);

    // The async_sleep timer path (post_after_on_main) must reach the
    // installed dispatcher's post_after with the right delay.
    auto t = sleep_then_return(99);
    REQUIRE(rec.timed_posts == 1);
    REQUIRE(rec.last_delay == std::chrono::duration_cast<std::chrono::steady_clock::duration>(100ms));
    // recording_dispatcher fires the timer inline, so the task completed.
    REQUIRE(t.is_ready());
    REQUIRE(t.await_resume() == 99);

    // Revert so the remaining suite sees the default test_dispatcher again.
    mpapp::install_main_dispatcher(nullptr);
    REQUIRE(dynamic_cast<mpapp::test_dispatcher*>(&mpapp::main_dispatcher()) != nullptr);
}

TEST_CASE("multiple concurrent tasks all run to completion", "[executor]") {
    auto& d = main_test_dispatcher();
    d.run_until_idle();

    constexpr int N = 8;
    std::vector<mpapp::task<int>> tasks;
    tasks.reserve(N);
    for (int i = 0; i < N; ++i) {
        tasks.emplace_back(sleep_then_return(i * 2));
    }

    for (const auto& t : tasks) {
        REQUIRE_FALSE(t.is_ready());
    }

    d.advance(100ms);

    for (int i = 0; i < N; ++i) {
        REQUIRE(tasks[i].is_ready());
        REQUIRE(tasks[i].await_resume() == i * 2);
    }
}
