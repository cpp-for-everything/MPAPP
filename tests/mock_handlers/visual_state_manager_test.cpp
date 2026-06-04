// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for the RFC-0006 visual_state_manager.

#include <string>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/resources/visual_state_manager.hpp>
#include <mpapp/view.hpp>

using namespace mpapp;

namespace {

// Same concrete test_view helper used in resource_dictionary_test.cpp
// (view's dtor is virtual but otherwise abstract-free for default-
// construction; the test trivially derives so we can instantiate).
class test_view : public view {
public:
    test_view() = default;
};

// Tiny counter the test setters drive instead of touching real
// Observables - keeps each case's intent obvious + easy to assert.
struct counters {
    int normal   = 0;
    int pressed  = 0;
    int disabled = 0;
    int focused  = 0;
};

} // namespace

TEST_CASE("visual_state_manager.go_to_state hits the matching state + runs setters",
          "[mock][resources][vsm]") {
    counters cs{};

    visual_state_manager vsm;
    vsm.groups.push_back(visual_state_group{
        std::string{ "Common" }
    });
    auto& group = vsm.groups.back();
    group.states.push_back(visual_state{ std::string{ visual_states::normal } });
    group.states.back().setters["v"] = [&cs](view&) { ++cs.normal; };
    group.states.push_back(visual_state{ std::string{ visual_states::pressed } });
    group.states.back().setters["v"] = [&cs](view&) { ++cs.pressed; };
    group.states.push_back(visual_state{ std::string{ visual_states::disabled } });
    group.states.back().setters["v"] = [&cs](view&) { ++cs.disabled; };

    test_view v;

    CHECK(vsm.go_to_state(v, visual_states::pressed) == 1);
    CHECK(cs.normal   == 0);
    CHECK(cs.pressed  == 1);
    CHECK(cs.disabled == 0);

    CHECK(vsm.go_to_state(v, visual_states::disabled) == 1);
    CHECK(cs.pressed  == 1);
    CHECK(cs.disabled == 1);

    CHECK(vsm.go_to_state(v, visual_states::normal) == 1);
    CHECK(cs.normal   == 1);
}

TEST_CASE("visual_state_manager same-state transition is a no-op",
          "[mock][resources][vsm]") {
    int hits = 0;

    visual_state_manager vsm;
    vsm.groups.push_back(visual_state_group{ std::string{ "Common" } });
    auto& group = vsm.groups.back();
    group.states.push_back(visual_state{ std::string{ visual_states::pressed } });
    group.states.back().setters["v"] = [&hits](view&) { ++hits; };

    test_view v;
    CHECK(vsm.go_to_state(v, visual_states::pressed) == 1);
    CHECK(hits == 1);

    // Calling go_to_state with the same name is a no-op - setter
    // does not re-run + return value is 0 (zero groups transitioned).
    CHECK(vsm.go_to_state(v, visual_states::pressed) == 0);
    CHECK(hits == 1);
}

TEST_CASE("visual_state_manager unknown state returns 0 + runs nothing",
          "[mock][resources][vsm]") {
    int hits = 0;

    visual_state_manager vsm;
    vsm.groups.push_back(visual_state_group{ std::string{ "Common" } });
    vsm.groups.back().states.push_back(
        visual_state{ std::string{ visual_states::normal } });
    vsm.groups.back().states.back().setters["v"] = [&hits](view&) { ++hits; };

    test_view v;
    CHECK(vsm.go_to_state(v, "DefinitelyNotAState") == 0);
    CHECK(hits == 0);
}

TEST_CASE("visual_state_manager transitions in every group that has the state",
          "[mock][resources][vsm]") {
    int common_hits = 0;
    int focus_hits  = 0;

    visual_state_manager vsm;

    // Two independent groups. "Disabled" lives in both groups so a
    // single go_to_state call transitions both - matches MAUI's
    // behaviour where the state name is the key, not the group.
    vsm.groups.push_back(visual_state_group{ std::string{ "Common" } });
    vsm.groups.back().states.push_back(
        visual_state{ std::string{ visual_states::disabled } });
    vsm.groups.back().states.back().setters["v"] = [&common_hits](view&) { ++common_hits; };

    vsm.groups.push_back(visual_state_group{ std::string{ "FocusStates" } });
    vsm.groups.back().states.push_back(
        visual_state{ std::string{ visual_states::disabled } });
    vsm.groups.back().states.back().setters["v"] = [&focus_hits](view&) { ++focus_hits; };

    test_view v;
    CHECK(vsm.go_to_state(v, visual_states::disabled) == 2);
    CHECK(common_hits == 1);
    CHECK(focus_hits  == 1);
}

TEST_CASE("visual_state_manager only the matching group's state transitions",
          "[mock][resources][vsm]") {
    int common_normal_hits  = 0;
    int focus_focused_hits  = 0;

    visual_state_manager vsm;

    vsm.groups.push_back(visual_state_group{ std::string{ "Common" } });
    vsm.groups.back().states.push_back(
        visual_state{ std::string{ visual_states::normal } });
    vsm.groups.back().states.back().setters["v"] = [&common_normal_hits](view&) { ++common_normal_hits; };

    vsm.groups.push_back(visual_state_group{ std::string{ "FocusStates" } });
    vsm.groups.back().states.push_back(
        visual_state{ std::string{ visual_states::focused } });
    vsm.groups.back().states.back().setters["v"] = [&focus_focused_hits](view&) { ++focus_focused_hits; };

    test_view v;
    // Normal hits Common only - FocusStates has no Normal entry, so
    // its current_state stays empty.
    CHECK(vsm.go_to_state(v, visual_states::normal) == 1);
    CHECK(common_normal_hits == 1);
    CHECK(focus_focused_hits == 0);

    auto snap = vsm.snapshot_current_states();
    REQUIRE(snap.size() == 2);
    CHECK(snap[0] == std::make_pair(std::string{ "Common" },      std::string{ "Normal" }));
    CHECK(snap[1] == std::make_pair(std::string{ "FocusStates" }, std::string{ "" }));
}

TEST_CASE("visual_state_manager setter exceptions are swallowed",
          "[mock][resources][vsm]") {
    bool ran_after_throw = false;

    visual_state_manager vsm;
    vsm.groups.push_back(visual_state_group{ std::string{ "Common" } });
    auto& state = vsm.groups.back().states.emplace_back(
        std::string{ visual_states::pressed });
    state.setters["bad"]  = [](view&) { throw std::runtime_error{ "oops" }; };
    state.setters["good"] = [&ran_after_throw](view&) { ran_after_throw = true; };

    test_view v;
    REQUIRE_NOTHROW(vsm.go_to_state(v, visual_states::pressed));
    CHECK(ran_after_throw);
    // The bad setter being skipped does NOT roll back the transition -
    // current_state still reflects the target name.
    CHECK(vsm.groups[0].current_state == "Pressed");
}

TEST_CASE("visual_state_manager null setters are tolerated",
          "[mock][resources][vsm]") {
    int hits = 0;

    visual_state_manager vsm;
    vsm.groups.push_back(visual_state_group{ std::string{ "Common" } });
    auto& state = vsm.groups.back().states.emplace_back(
        std::string{ visual_states::pressed });
    state.setters["null"] = nullptr;  // explicit null is skipped
    state.setters["live"] = [&hits](view&) { ++hits; };

    test_view v;
    REQUIRE_NOTHROW(vsm.go_to_state(v, visual_states::pressed));
    CHECK(hits == 1);
}

TEST_CASE("visual_state_manager works with a single-group lifecycle",
          "[mock][resources][vsm]") {
    // End-to-end shape an interactive control would use: declare a
    // Common group, declare Normal/Pressed/Disabled, transition
    // through all three, snapshot the final state. Mirrors the
    // example app's wire-up.
    visual_state_manager vsm;
    vsm.groups.push_back(visual_state_group{ std::string{ "Common" } });
    auto& g = vsm.groups.back();

    for (auto name : { visual_states::normal, visual_states::pressed, visual_states::disabled }) {
        g.states.push_back(visual_state{ std::string{ name } });
    }

    test_view v;

    REQUIRE(vsm.go_to_state(v, visual_states::normal)   == 1);
    REQUIRE(vsm.go_to_state(v, visual_states::pressed)  == 1);
    REQUIRE(vsm.go_to_state(v, visual_states::normal)   == 1);
    REQUIRE(vsm.go_to_state(v, visual_states::disabled) == 1);

    auto snap = vsm.snapshot_current_states();
    REQUIRE(snap.size() == 1);
    CHECK(snap[0].first  == "Common");
    CHECK(snap[0].second == "Disabled");
}
