// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/BoxView.md
//
// Mock-handler tests for `mpapp::box_view` (CLAUDE Rule 6 / ADR-0008).

#include <catch2/catch_test_macros.hpp>

#include <mpapp/box_view.hpp>
#include <mpapp/handlers/mock/box_view_handler.hpp>

using namespace mpapp;

TEST_CASE("box_view mock handler records initial values on bind",
          "[mock][box_view]") {
    box_view b;
    box_view_handler<platform::mock> h;

    h.map_fill(b);
    h.map_corners(b);

    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].property_name == "fill");
    CHECK(h.calls()[0].value_repr    == "rgba(0,0,0,1)");
    CHECK(h.calls()[1].property_name == "corners");
    CHECK(h.calls()[1].value_repr    == "corners(0,0,0,0)");
}

TEST_CASE("box_view mock handler records single call per fill change",
          "[mock][box_view]") {
    box_view b;
    box_view_handler<platform::mock> h;

    h.map_fill(b);
    h.clear_calls();

    b.fill = color{1.0, 0.0, 0.0, 1.0};
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "rgba(1,0,0,1)");

    b.fill = color{1.0, 0.0, 0.0, 1.0};      // idempotent
    REQUIRE(h.calls().size() == 1);

    b.fill = color{0.0, 1.0, 0.0, 0.5};
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "rgba(0,1,0,0.5)");
}

TEST_CASE("box_view mock handler records asymmetric corners",
          "[mock][box_view][corners]") {
    box_view b;
    box_view_handler<platform::mock> h;

    h.map_corners(b);
    h.clear_calls();

    b.corners = corner_radius{8.0, 0.0, 8.0, 0.0};
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "corners(8,0,8,0)");
}

TEST_CASE("box_view sequence: fill then corners then fill back",
          "[mock][box_view][sequence]") {
    box_view b;
    box_view_handler<platform::mock> h;

    h.map_fill(b);
    h.map_corners(b);
    h.clear_calls();

    b.fill    = color{0.5, 0.5, 0.5, 1.0};
    b.corners = corner_radius{4.0, 4.0, 4.0, 4.0};
    b.fill    = color{0.5, 0.5, 0.5, 1.0};    // idempotent — same as previous
    b.fill    = color{1.0, 1.0, 1.0, 1.0};

    REQUIRE(h.calls().size() == 3);
    CHECK(h.calls()[0].property_name == "fill");
    CHECK(h.calls()[1].property_name == "corners");
    CHECK(h.calls()[2].property_name == "fill");
    CHECK(h.calls()[2].value_repr    == "rgba(1,1,1,1)");
}
