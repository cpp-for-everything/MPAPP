// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for the RFC-0008 trigger family.

#include <string>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/observable.hpp>
#include <mpapp/signal.hpp>
#include <mpapp/triggers/event_trigger.hpp>
#include <mpapp/triggers/state_trigger.hpp>
#include <mpapp/triggers/trigger.hpp>
#include <mpapp/view.hpp>

using namespace mpapp;

namespace {
class test_view : public view {
public:
    test_view() = default;
};
} // namespace

TEST_CASE("property/data trigger runs enter on match, exit on un-match",
          "[mock][trigger]") {
    Observable<bool> is_pressed{ false };
    test_view        v;

    int entered = 0;
    int exited  = 0;

    trigger<bool> t{ is_pressed, true, v };
    t.enter_setters["x"] = [&entered](view&) { ++entered; };
    t.exit_setters["x"]  = [&exited](view&) { ++exited; };

    // Constructed inactive (false != true); no setters run yet.
    CHECK_FALSE(t.is_active());
    CHECK(entered == 0);

    is_pressed = true;          // match -> enter
    CHECK(t.is_active());
    CHECK(entered == 1);
    CHECK(exited == 0);

    is_pressed = false;         // un-match -> exit
    CHECK_FALSE(t.is_active());
    CHECK(exited == 1);
    CHECK(entered == 1);        // not re-run
}

TEST_CASE("trigger ignores same-value writes and re-fires on re-match edge",
          "[mock][trigger]") {
    Observable<int> mode{ 2 };
    test_view       v;

    int entered = 0;
    trigger<int> t{ mode, 2, v };
    t.enter_setters["x"] = [&entered](view&) { ++entered; };

    // Setters were attached AFTER construction, so the initial evaluate
    // marked active but had no setter to run. A subsequent re-match edge
    // is what exercises them - verify same-value writes don't re-fire.
    CHECK(t.is_active());
    mode = 2;                   // same value -> Observable no-op -> no edge
    CHECK(entered == 0);
    mode = 5;                   // leave
    CHECK_FALSE(t.is_active());
    mode = 2;                   // re-enter -> setter runs
    CHECK(entered == 1);
}

TEST_CASE("multi_trigger activates only when ALL conditions match",
          "[mock][trigger][multi]") {
    Observable<bool> enabled{ false };
    Observable<int>  count{ 0 };
    test_view        v;

    int entered = 0;
    int exited  = 0;

    multi_trigger<bool, int> mt{ v, when{ enabled, true }, when{ count, 3 } };
    mt.enter_setters["x"] = [&entered](view&) { ++entered; };
    mt.exit_setters["x"]  = [&exited](view&) { ++exited; };

    CHECK_FALSE(mt.is_active());

    enabled = true;             // only one condition true
    CHECK_FALSE(mt.is_active());
    CHECK(entered == 0);

    count = 3;                  // now both true -> enter
    CHECK(mt.is_active());
    CHECK(entered == 1);

    count = 4;                  // one breaks -> exit
    CHECK_FALSE(mt.is_active());
    CHECK(exited == 1);
}

TEST_CASE("event_trigger runs its action when the signal fires",
          "[mock][trigger][event]") {
    signal<const int&> clicked;
    int last = -1;
    int hits = 0;

    event_trigger<const int&> et{ clicked, [&](const int& v) { last = v; ++hits; } };

    clicked.emit(7);
    CHECK(hits == 1);
    CHECK(last == 7);

    clicked.emit(42);
    CHECK(hits == 2);
    CHECK(last == 42);
}

TEST_CASE("state_trigger drives the visual_state_manager from a bool",
          "[mock][trigger][state]") {
    Observable<bool>     is_error{ false };
    test_view            v;
    visual_state_manager vsm;
    vsm.groups.push_back(visual_state_group{ std::string{ "Validation" } });
    vsm.groups.back().states.push_back(visual_state{ std::string{ "Valid" } });
    vsm.groups.back().states.push_back(visual_state{ std::string{ "Invalid" } });

    state_trigger st{ is_error, v, vsm, "Invalid", "Valid" };

    // Initial: condition false -> inactive_state "Valid".
    CHECK(vsm.groups[0].current_state == "Valid");

    is_error = true;
    CHECK(vsm.groups[0].current_state == "Invalid");

    is_error = false;
    CHECK(vsm.groups[0].current_state == "Valid");
}
