// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for RFC-0014 commanding (async_relay_command).

#include <functional>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/binding/async_relay_command.hpp>
#include <mpapp/signal.hpp>

using namespace mpapp;

TEST_CASE("async_relay_command sets is_running and blocks re-entry until done",
          "[mock][command]") {
    // Arrange: a manual-completion action that captures `done`.
    std::function<void()> stored_done;
    int                   starts = 0;
    async_relay_command   cmd{ [&](std::function<void()> done) {
        ++starts;
        stored_done = std::move(done);
    } };

    // Act + Assert: starts idle and executable.
    CHECK_FALSE(cmd.is_running());
    CHECK(cmd.can_execute());

    // Execute starts the action and flips is_running true.
    cmd.execute();
    CHECK(cmd.is_running());
    CHECK_FALSE(cmd.can_execute());   // gated while running
    CHECK(starts == 1);

    // Re-entry while running is blocked.
    cmd.execute();
    CHECK(starts == 1);

    // Invoking the stored done() flips is_running false and re-enables.
    stored_done();
    CHECK_FALSE(cmd.is_running());
    CHECK(cmd.can_execute());

    // It can run again afterwards.
    cmd.execute();
    CHECK(cmd.is_running());
    CHECK(starts == 2);
    stored_done();
    CHECK_FALSE(cmd.is_running());
}

TEST_CASE("async_relay_command emits can_execute_changed on start and completion",
          "[mock][command]") {
    // Arrange.
    std::function<void()> stored_done;
    async_relay_command   cmd{ [&](std::function<void()> done) {
        stored_done = std::move(done);
    } };

    int          hits = 0;
    signal_slot<> slot;
    auto         cb = [&hits]() { ++hits; };
    cmd.can_execute_changed.subscribe(slot, cb);

    // Act + Assert: one emit on start, one on completion.
    cmd.execute();
    CHECK(hits == 1);
    stored_done();
    CHECK(hits == 2);
}

TEST_CASE("async_relay_command honours the can_execute guard", "[mock][command]") {
    bool                  allowed = false;
    int                   starts  = 0;
    std::function<void()> stored_done;
    async_relay_command   cmd{
        [&](std::function<void()> done) { ++starts; stored_done = std::move(done); },
        [&allowed]() { return allowed; }
    };

    CHECK_FALSE(cmd.can_execute());
    cmd.execute();                 // gated -> no start
    CHECK(starts == 0);
    CHECK_FALSE(cmd.is_running());

    allowed = true;
    CHECK(cmd.can_execute());
    cmd.execute();
    CHECK(starts == 1);
    CHECK(cmd.is_running());
    stored_done();
    CHECK_FALSE(cmd.is_running());
}

TEST_CASE("async_relay_command allow_concurrent permits re-entry", "[mock][command]") {
    int                   starts = 0;
    std::function<void()> last_done;
    async_relay_command   cmd{
        [&](std::function<void()> done) { ++starts; last_done = std::move(done); },
        {},
        /*allow_concurrent=*/true
    };

    CHECK(cmd.allow_concurrent());
    cmd.execute();
    CHECK(cmd.is_running());
    CHECK(cmd.can_execute());      // still executable while running
    cmd.execute();                 // concurrent start allowed
    CHECK(starts == 2);
    last_done();                   // completion clears is_running
    CHECK_FALSE(cmd.is_running());
}

TEST_CASE("async_relay_command synchronous completion ends not-running",
          "[mock][command]") {
    // An action that completes synchronously (calls done immediately).
    int                 runs = 0;
    async_relay_command cmd{ [&runs](std::function<void()> done) {
        ++runs;
        done();
    } };

    cmd.execute();
    CHECK(runs == 1);
    CHECK_FALSE(cmd.is_running());  // already completed
    CHECK(cmd.can_execute());
    cmd.execute();
    CHECK(runs == 2);
}

TEST_CASE("async_relay_command double done() is idempotent", "[mock][command]") {
    std::function<void()> stored_done;
    async_relay_command   cmd{ [&](std::function<void()> done) {
        stored_done = std::move(done);
    } };

    int          hits = 0;
    signal_slot<> slot;
    auto         cb = [&hits]() { ++hits; };
    cmd.can_execute_changed.subscribe(slot, cb);

    cmd.execute();          // hits == 1
    stored_done();          // hits == 2
    stored_done();          // idempotent: no extra emit, stays not-running
    CHECK(hits == 2);
    CHECK_FALSE(cmd.is_running());
}

TEST_CASE("async_relay_command empty action is a no-op", "[mock][command]") {
    async_relay_command cmd{ {} };
    CHECK(cmd.can_execute());
    cmd.execute();                 // no action -> nothing happens
    CHECK_FALSE(cmd.is_running());
}

TEST_CASE("async_relay_command raises can_execute_changed manually",
          "[mock][command]") {
    async_relay_command cmd{ [](std::function<void()>) {} };
    int          hits = 0;
    signal_slot<> slot;
    auto         cb = [&hits]() { ++hits; };
    cmd.can_execute_changed.subscribe(slot, cb);

    cmd.raise_can_execute_changed();
    cmd.raise_can_execute_changed();
    CHECK(hits == 2);
}

TEST_CASE("async_relay_command via command_base interface", "[mock][command]") {
    std::function<void()> stored_done;
    async_relay_command   concrete{ [&](std::function<void()> done) {
        stored_done = std::move(done);
    } };
    command_base& cmd = concrete;

    CHECK(cmd.can_execute());
    cmd.execute();
    CHECK_FALSE(cmd.can_execute());
    stored_done();
    CHECK(cmd.can_execute());
}

TEST_CASE("async_relay_command_of<T> passes parameter and tracks running",
          "[mock][command]") {
    std::string           last;
    std::function<void()> stored_done;
    async_relay_command_of<std::string> cmd{
        [&](const std::string& s, std::function<void()> done) {
            last        = s;
            stored_done = std::move(done);
        },
        [](const std::string& s) { return !s.empty(); }
    };

    CHECK_FALSE(cmd.can_execute(""));
    cmd.execute("");               // gated
    CHECK(last.empty());
    CHECK_FALSE(cmd.is_running());

    CHECK(cmd.can_execute("go"));
    cmd.execute("go");
    CHECK(last == "go");
    CHECK(cmd.is_running());
    CHECK_FALSE(cmd.can_execute("again"));   // gated while running

    stored_done();
    CHECK_FALSE(cmd.is_running());
    CHECK(cmd.can_execute("again"));
}

TEST_CASE("async_relay_command_of<T> emits can_execute_changed", "[mock][command]") {
    std::function<void()> stored_done;
    async_relay_command_of<int> cmd{
        [&](const int&, std::function<void()> done) { stored_done = std::move(done); }
    };

    int          hits = 0;
    signal_slot<> slot;
    auto         cb = [&hits]() { ++hits; };
    cmd.can_execute_changed.subscribe(slot, cb);

    cmd.execute(1);
    CHECK(hits == 1);
    stored_done();
    CHECK(hits == 2);

    cmd.raise_can_execute_changed();
    CHECK(hits == 3);
}

TEST_CASE("async_relay_command_of<T> allow_concurrent and empty action",
          "[mock][command]") {
    int                   starts = 0;
    std::function<void()> last_done;
    async_relay_command_of<int> cmd{
        [&](const int&, std::function<void()> done) { ++starts; last_done = std::move(done); },
        {},
        /*allow_concurrent=*/true
    };

    CHECK(cmd.allow_concurrent());
    cmd.execute(1);
    CHECK(cmd.is_running());
    CHECK(cmd.can_execute(2));     // concurrent ok
    cmd.execute(2);
    CHECK(starts == 2);
    last_done();
    CHECK_FALSE(cmd.is_running());

    // Empty action is a no-op.
    async_relay_command_of<int> empty{ {} };
    CHECK(empty.can_execute(7));
    empty.execute(7);
    CHECK_FALSE(empty.is_running());
}
