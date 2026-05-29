// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for RFC-0014 commanding (relay_command).

#include <string>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/binding/relay_command.hpp>
#include <mpapp/signal.hpp>

using namespace mpapp;

TEST_CASE("relay_command executes its action", "[mock][command]") {
    int runs = 0;
    relay_command cmd{ [&runs]() { ++runs; } };

    CHECK(cmd.can_execute());      // no guard -> always executable
    cmd.execute();
    cmd.execute();
    CHECK(runs == 2);
}

TEST_CASE("relay_command can_execute gates execution", "[mock][command]") {
    int  runs    = 0;
    bool allowed = false;
    relay_command cmd{ [&runs]() { ++runs; }, [&allowed]() { return allowed; } };

    CHECK_FALSE(cmd.can_execute());
    cmd.execute();                 // gated -> no run
    CHECK(runs == 0);

    allowed = true;
    CHECK(cmd.can_execute());
    cmd.execute();
    CHECK(runs == 1);
}

TEST_CASE("relay_command raises can_execute_changed", "[mock][command]") {
    relay_command cmd{ []() {} };
    int hits = 0;
    signal_slot<> slot;
    auto cb = [&hits]() { ++hits; };
    cmd.can_execute_changed.subscribe(slot, cb);

    cmd.raise_can_execute_changed();
    cmd.raise_can_execute_changed();
    CHECK(hits == 2);
}

TEST_CASE("relay_command_of<T> passes the parameter through",
          "[mock][command]") {
    std::string last;
    relay_command_of<std::string> cmd{
        [&last](const std::string& s) { last = s; },
        [](const std::string& s) { return !s.empty(); }
    };

    CHECK_FALSE(cmd.can_execute(""));
    cmd.execute("");               // gated
    CHECK(last.empty());

    CHECK(cmd.can_execute("go"));
    cmd.execute("go");
    CHECK(last == "go");
}

TEST_CASE("command_base polymorphism: invoke via the ICommand interface",
          "[mock][command]") {
    int runs = 0;
    relay_command concrete{ [&runs]() { ++runs; } };
    command_base& cmd = concrete;   // upcast to the interface

    CHECK(cmd.can_execute());
    cmd.execute();
    CHECK(runs == 1);
}
