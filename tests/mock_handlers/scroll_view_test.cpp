// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/ScrollView.md
//
// Mock-handler tests for `mpapp::scroll_view` (CLAUDE Rule 6 / ADR-0008).

#include <catch2/catch_test_macros.hpp>

#include <memory>

#include <mpapp/handlers/mock/scroll_view_handler.hpp>
#include <mpapp/scroll_view.hpp>
#include <mpapp/view.hpp>

using namespace mpapp;

namespace {

class plain_view : public view {};

} // namespace

TEST_CASE("scroll_view mock handler records initial values on bind",
          "[mock][scroll_view]") {
    scroll_view sv;
    scroll_view_handler<platform::mock> h;

    h.map_orientation(sv);
    h.map_horizontal_scroll_bar_visibility(sv);
    h.map_vertical_scroll_bar_visibility(sv);

    REQUIRE(h.calls().size() == 3);
    CHECK(h.calls()[0].property_name == "orientation");
    CHECK(h.calls()[0].value_repr    == "vertical");
    CHECK(h.calls()[1].value_repr    == "default");
    CHECK(h.calls()[2].value_repr    == "default");
}

TEST_CASE("scroll_view mock handler records single call per property change",
          "[mock][scroll_view]") {
    scroll_view sv;
    scroll_view_handler<platform::mock> h;

    h.map_orientation(sv);
    h.clear_calls();

    sv.orientation = scroll_orientation::horizontal;
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "horizontal");

    sv.orientation = scroll_orientation::horizontal;   // idempotent
    REQUIRE(h.calls().size() == 1);

    sv.orientation = scroll_orientation::both;
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "both");
}

TEST_CASE("scroll_view mock handler tracks content presence",
          "[mock][scroll_view][content]") {
    scroll_view sv;
    scroll_view_handler<platform::mock> h;

    h.map_content(sv);
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "content.present");
    CHECK(h.calls()[0].value_repr    == "false");

    sv.content = std::make_shared<plain_view>();
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "true");
}

TEST_CASE("scroll_view scroll_to command records request fields",
          "[mock][scroll_view][command]") {
    scroll_view sv;
    scroll_view_handler<platform::mock> h;

    h.map_scroll_to(sv, scroll_to_request{
        .x = 120.0, .y = 300.0,
        .element = nullptr,
        .position = scroll_to_position::start,
        .animated = false,
    });

    REQUIRE(h.calls().size() == 3);
    CHECK(h.calls()[0].property_name == "scroll_to.x");
    CHECK(h.calls()[0].value_repr    == "120");
    CHECK(h.calls()[1].property_name == "scroll_to.y");
    CHECK(h.calls()[1].value_repr    == "300");
    CHECK(h.calls()[2].property_name == "scroll_to.animated");
    CHECK(h.calls()[2].value_repr    == "false");
}
