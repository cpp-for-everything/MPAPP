// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/View.md
//
// Mock-handler tests for `mpapp::view` (CLAUDE Rule 6 / ADR-0008).
// Asserts the recorded mapper sequence on the cross-cutting view
// surface - every layout-group widget inherits this surface, so the
// invariants here apply transitively to layout/border/box_view/etc.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/view_handler.hpp>
#include <mpapp/view.hpp>

using namespace mpapp;

TEST_CASE("view mock handler records initial values on bind", "[mock][view]") {
    view v;
    view_handler<platform::mock> h;

    h.map_opacity(v);
    h.map_is_enabled(v);
    h.map_width(v);

    REQUIRE(h.calls().size() == 3);
    CHECK(h.calls()[0].property_name == "opacity");
    CHECK(h.calls()[0].value_repr    == "1");
    CHECK(h.calls()[1].property_name == "is_enabled");
    CHECK(h.calls()[1].value_repr    == "true");
    CHECK(h.calls()[2].property_name == "width");
    CHECK(h.calls()[2].value_repr    == "-1");
}

TEST_CASE("view mock handler records a single call per real property change",
          "[mock][view]") {
    view v;
    view_handler<platform::mock> h;

    h.map_opacity(v);              // initial: opacity=1
    h.clear_calls();

    v.opacity = 0.5;
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "opacity");
    CHECK(h.calls()[0].value_repr    == "0.5");
}

TEST_CASE("view mock handler ignores idempotent sets", "[mock][view][idempotent]") {
    view v;
    view_handler<platform::mock> h;

    h.map_is_enabled(v);
    h.clear_calls();

    v.is_enabled = true;            // same value - Observable short-circuits
    v.is_enabled = true;
    CHECK(h.calls().empty());

    v.is_enabled = false;           // real change fires once
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "is_enabled");
    CHECK(h.calls()[0].value_repr    == "false");

    v.is_enabled = false;           // idempotent again
    REQUIRE(h.calls().size() == 1);
}

TEST_CASE("view mock handler records multiple property changes in order",
          "[mock][view][sequence]") {
    view v;
    view_handler<platform::mock> h;

    h.map_opacity(v);
    h.map_visibility(v);
    h.clear_calls();

    v.opacity          = 0.25;
    v.visibility_state = visibility::hidden;
    v.opacity          = 0.75;
    v.visibility_state = visibility::collapsed;

    REQUIRE(h.calls().size() == 4);
    CHECK(h.calls()[0].property_name == "opacity");
    CHECK(h.calls()[0].value_repr    == "0.25");
    CHECK(h.calls()[1].property_name == "visibility");
    CHECK(h.calls()[1].value_repr    == "hidden");
    CHECK(h.calls()[2].property_name == "opacity");
    CHECK(h.calls()[2].value_repr    == "0.75");
    CHECK(h.calls()[3].property_name == "visibility");
    CHECK(h.calls()[3].value_repr    == "collapsed");
}

TEST_CASE("view mock handler records flow_direction with symbolic repr",
          "[mock][view][enum]") {
    view v;
    view_handler<platform::mock> h;

    h.map_flow_direction(v);
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "match_parent");

    h.clear_calls();
    v.flow = flow_direction::right_to_left;
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "rtl");
}
