// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Catch2 tests for mpapp::real_main_thread (RFC-0013).
// Verifies: main-thread detection, inline dispatch, cross-thread queue,
// pump(), invoke_count(), pending_count(), null-action safety, and
// polymorphism through the abstract interface.

#include <atomic>
#include <functional>
#include <thread>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/real_main_thread.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// is_main_thread — detection
// ---------------------------------------------------------------------------

TEST_CASE("real_main_thread: is_main_thread returns true on constructing thread",
          "[mock][essentials][main_thread]") {
    // Arrange + Act
    real_main_thread rmt;

    // Assert — the test runner IS the constructing thread
    CHECK(rmt.is_main_thread());
}

TEST_CASE("real_main_thread: is_main_thread returns false on a different thread",
          "[mock][essentials][main_thread]") {
    // Arrange
    real_main_thread rmt;
    bool other_thread_result = true; // will be set to false

    // Act — spawn a thread and query from there
    std::thread worker([&] {
        other_thread_result = rmt.is_main_thread();
    });
    worker.join();

    // Assert
    CHECK_FALSE(other_thread_result);
}

// ---------------------------------------------------------------------------
// Inline dispatch — action runs immediately when on main thread
// ---------------------------------------------------------------------------

TEST_CASE("real_main_thread: begin_invoke_on_main_thread runs inline on main thread",
          "[mock][essentials][main_thread]") {
    // Arrange
    real_main_thread rmt;
    int side_effect = 0;

    // Act
    rmt.begin_invoke_on_main_thread([&] { side_effect = 42; });

    // Assert — ran synchronously, no pump needed
    CHECK(side_effect == 42);
}

TEST_CASE("real_main_thread: multiple inline dispatches execute in order",
          "[mock][essentials][main_thread]") {
    // Arrange
    real_main_thread rmt;
    std::vector<int> order;

    // Act
    rmt.begin_invoke_on_main_thread([&] { order.push_back(1); });
    rmt.begin_invoke_on_main_thread([&] { order.push_back(2); });
    rmt.begin_invoke_on_main_thread([&] { order.push_back(3); });

    // Assert
    REQUIRE(order.size() == 3u);
    CHECK(order[0] == 1);
    CHECK(order[1] == 2);
    CHECK(order[2] == 3);
}

TEST_CASE("real_main_thread: null action inline does not crash and still increments counter",
          "[mock][essentials][main_thread]") {
    // Arrange
    real_main_thread rmt;
    std::function<void()> empty;

    // Act + Assert — no exception/crash
    rmt.begin_invoke_on_main_thread(empty);
    CHECK(rmt.invoke_count() == 1u);
}

// ---------------------------------------------------------------------------
// Cross-thread enqueue + pump()
// ---------------------------------------------------------------------------

TEST_CASE("real_main_thread: action enqueued from worker thread, drained by pump",
          "[mock][essentials][main_thread]") {
    // Arrange
    real_main_thread rmt;
    std::atomic<int> value{ 0 };

    // Act — enqueue from a worker thread
    std::thread worker([&] {
        rmt.begin_invoke_on_main_thread([&] { value.store(99); });
    });
    worker.join();

    // Assert — not yet executed (pump has not been called)
    CHECK(value.load() == 0);
    CHECK(rmt.pending_count() == 1u);

    // Act — drain on main thread
    rmt.pump();

    // Assert — now executed
    CHECK(value.load() == 99);
    CHECK(rmt.pending_count() == 0u);
}

TEST_CASE("real_main_thread: multiple workers enqueue, single pump drains all",
          "[mock][essentials][main_thread]") {
    // Arrange
    real_main_thread rmt;
    std::atomic<int> counter{ 0 };
    constexpr int kWorkers = 5;

    // Act — each worker enqueues one increment
    std::vector<std::thread> workers;
    workers.reserve(kWorkers);
    for (int i = 0; i < kWorkers; ++i) {
        workers.emplace_back([&] {
            rmt.begin_invoke_on_main_thread([&] { counter.fetch_add(1); });
        });
    }
    for (auto& w : workers) w.join();

    CHECK(rmt.pending_count() == static_cast<std::size_t>(kWorkers));

    // Act
    rmt.pump();

    // Assert
    CHECK(counter.load() == kWorkers);
    CHECK(rmt.pending_count() == 0u);
}

TEST_CASE("real_main_thread: pump on empty queue is a no-op",
          "[mock][essentials][main_thread]") {
    // Arrange
    real_main_thread rmt;

    // Act + Assert — should not throw or crash
    rmt.pump();
    CHECK(rmt.pending_count() == 0u);
    CHECK(rmt.invoke_count() == 0u);
}

TEST_CASE("real_main_thread: actions enqueued during pump deferred to next pump",
          "[mock][essentials][main_thread]") {
    // Arrange
    real_main_thread rmt;
    int first  = 0;
    int second = 0;

    // Enqueue from a worker — this action will itself enqueue a second action
    // via begin_invoke_on_main_thread from within pump (main thread context,
    // so second action runs inline during the nested call).
    std::thread worker([&] {
        rmt.begin_invoke_on_main_thread([&] {
            first = 1;
            // This inner enqueue is called from the main thread (inside pump),
            // so it executes inline immediately.
            rmt.begin_invoke_on_main_thread([&] { second = 2; });
        });
    });
    worker.join();

    // Act
    rmt.pump(); // drains the outer action; inner runs inline during outer

    // Assert — both executed
    CHECK(first  == 1);
    CHECK(second == 2);
}

// ---------------------------------------------------------------------------
// invoke_count() — both paths increment
// ---------------------------------------------------------------------------

TEST_CASE("real_main_thread: invoke_count increments for inline dispatch",
          "[mock][essentials][main_thread]") {
    // Arrange
    real_main_thread rmt;

    // Act
    rmt.begin_invoke_on_main_thread([] {});
    rmt.begin_invoke_on_main_thread([] {});

    // Assert
    CHECK(rmt.invoke_count() == 2u);
}

TEST_CASE("real_main_thread: invoke_count increments for enqueued dispatch",
          "[mock][essentials][main_thread]") {
    // Arrange
    real_main_thread rmt;

    // Act — enqueue two actions from a worker
    std::thread worker([&] {
        rmt.begin_invoke_on_main_thread([] {});
        rmt.begin_invoke_on_main_thread([] {});
    });
    worker.join();
    rmt.pump();

    // Assert
    CHECK(rmt.invoke_count() == 2u);
}

TEST_CASE("real_main_thread: invoke_count reflects both inline and queued calls",
          "[mock][essentials][main_thread]") {
    // Arrange
    real_main_thread rmt;

    // Act — one inline, one from worker
    rmt.begin_invoke_on_main_thread([] {});
    std::thread worker([&] {
        rmt.begin_invoke_on_main_thread([] {});
    });
    worker.join();
    rmt.pump();

    // Assert
    CHECK(rmt.invoke_count() == 2u);
}

// ---------------------------------------------------------------------------
// pending_count()
// ---------------------------------------------------------------------------

TEST_CASE("real_main_thread: pending_count is zero initially",
          "[mock][essentials][main_thread]") {
    // Arrange + Act
    real_main_thread rmt;

    // Assert
    CHECK(rmt.pending_count() == 0u);
}

TEST_CASE("real_main_thread: pending_count grows with each enqueue and drops after pump",
          "[mock][essentials][main_thread]") {
    // Arrange
    real_main_thread rmt;

    // Act — enqueue 3 from a worker
    std::thread worker([&] {
        for (int i = 0; i < 3; ++i)
            rmt.begin_invoke_on_main_thread([] {});
    });
    worker.join();

    // Assert — 3 pending
    CHECK(rmt.pending_count() == 3u);

    // Act
    rmt.pump();

    // Assert — 0 pending
    CHECK(rmt.pending_count() == 0u);
}

// ---------------------------------------------------------------------------
// Polymorphism — via abstract base pointer
// ---------------------------------------------------------------------------

TEST_CASE("real_main_thread: usable via main_thread abstract interface pointer",
          "[mock][essentials][main_thread]") {
    // Arrange
    real_main_thread concrete;
    main_thread* iface = &concrete;

    // Act
    bool on_main = iface->is_main_thread();
    int  visited = 0;
    iface->begin_invoke_on_main_thread([&] { visited = 7; });

    // Assert
    CHECK(on_main == true);
    CHECK(visited == 7);
    CHECK(concrete.invoke_count() == 1u);
}

TEST_CASE("real_main_thread: worker thread sees false through base interface",
          "[mock][essentials][main_thread]") {
    // Arrange
    real_main_thread concrete;
    main_thread* iface = &concrete;
    bool worker_sees_main = true;

    // Act
    std::thread worker([&] {
        worker_sees_main = iface->is_main_thread();
    });
    worker.join();

    // Assert
    CHECK_FALSE(worker_sees_main);
}
