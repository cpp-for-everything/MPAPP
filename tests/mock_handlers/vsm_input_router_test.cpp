// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Tests for the VSM input-routing layer
// (mpapp::visual_state_input_router) — maps enabled/pressed/pointer-over/
// focused input state onto the canonical CommonStates and drives the VSM.

#include <string>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/internal/basic_label.hpp>
#include <mpapp/resources/visual_state_input_router.hpp>
#include <mpapp/resources/visual_state_manager.hpp>

using namespace mpapp;

namespace {

// Build a CommonStates group with the five canonical states. Setters
// bump a counter so we can also confirm transitions actually ran.
void add_common_states(visual_state_manager& vsm, int* applied) {
    visual_state_group g{"CommonStates"};
    for (auto n : {visual_states::normal, visual_states::pressed,
                   visual_states::pointer_over, visual_states::disabled,
                   visual_states::focused}) {
        visual_state s{std::string{n}};
        s.setters["mark"] = [applied](view&) { ++*applied; };
        g.states.push_back(std::move(s));
    }
    vsm.groups.push_back(std::move(g));
}

} // namespace

TEST_CASE("vsm input router resolves the canonical priority order",
          "[mock][vsm][router]") {
    internal::basic_label    v;
    visual_state_manager     vsm;
    int                      applied = 0;
    add_common_states(vsm, &applied);

    visual_state_input_router r{v, vsm};
    // ctor routes the initial enabled/idle state.
    CHECK(r.current() == "Normal");
    CHECK(vsm.groups[0].current_state == "Normal");
    CHECK(applied == 1);

    r.set_pointer_over(true);
    CHECK(r.current() == "PointerOver");
    CHECK(vsm.groups[0].current_state == "PointerOver");

    r.set_pressed(true);              // Pressed outranks PointerOver
    CHECK(r.current() == "Pressed");
    CHECK(vsm.groups[0].current_state == "Pressed");

    v.is_enabled = false;             // Disabled outranks everything
    CHECK(r.current() == "Disabled");
    CHECK(vsm.groups[0].current_state == "Disabled");

    v.is_enabled = true;              // back to Pressed (still pressed+over)
    CHECK(r.current() == "Pressed");

    r.set_pressed(false);
    CHECK(r.current() == "PointerOver");

    r.set_pointer_over(false);
    CHECK(r.current() == "Normal");
}

TEST_CASE("vsm input router focused state below pointer/pressed",
          "[mock][vsm][router]") {
    internal::basic_label    v;
    visual_state_manager     vsm;
    int                      applied = 0;
    add_common_states(vsm, &applied);

    visual_state_input_router r{v, vsm};
    r.set_focused(true);
    CHECK(r.current() == "Focused");

    r.set_pointer_over(true);          // PointerOver outranks Focused
    CHECK(r.current() == "PointerOver");

    r.set_pointer_over(false);
    CHECK(r.current() == "Focused");   // falls back to Focused
}
