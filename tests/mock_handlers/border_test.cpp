// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Border.md
//
// Mock-handler tests for `mpapp::border` (CLAUDE Rule 6 / ADR-0008).

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <vector>

#include <mpapp/border.hpp>
#include <mpapp/handlers/mock/border_handler.hpp>
#include <mpapp/view.hpp>

using namespace mpapp;

namespace {

class plain_view : public view {};

} // namespace

TEST_CASE("border mock handler records initial values on bind",
          "[mock][border]") {
    border b;
    border_handler<platform::mock> h;

    h.map_stroke_thickness(b);
    h.map_stroke_line_cap(b);
    h.map_stroke_line_join(b);
    h.map_stroke_miter_limit(b);

    REQUIRE(h.calls().size() == 4);
    CHECK(h.calls()[0].property_name == "stroke_thickness");
    CHECK(h.calls()[0].value_repr    == "1");
    CHECK(h.calls()[1].property_name == "stroke_line_cap");
    CHECK(h.calls()[1].value_repr    == "flat");
    CHECK(h.calls()[2].property_name == "stroke_line_join");
    CHECK(h.calls()[2].value_repr    == "miter");
    CHECK(h.calls()[3].property_name == "stroke_miter_limit");
    CHECK(h.calls()[3].value_repr    == "10");
}

TEST_CASE("border mock handler records single call per property change",
          "[mock][border]") {
    border b;
    border_handler<platform::mock> h;

    h.map_stroke_thickness(b);
    h.clear_calls();

    b.stroke_thickness = 3.0;
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "3");

    b.stroke_thickness = 3.0;            // idempotent
    REQUIRE(h.calls().size() == 1);

    b.stroke_thickness = 4.5;
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "4.5");
}

TEST_CASE("border mock handler tracks content presence and dash array size",
          "[mock][border]") {
    border b;
    border_handler<platform::mock> h;

    h.map_content(b);
    h.map_stroke_dash_array(b);

    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].property_name == "content.present");
    CHECK(h.calls()[0].value_repr    == "false");
    CHECK(h.calls()[1].property_name == "stroke_dash_array.size");
    CHECK(h.calls()[1].value_repr    == "0");

    b.content           = std::make_shared<plain_view>();
    b.stroke_dash_array = std::vector<double>{1.0, 2.0, 4.0};

    REQUIRE(h.calls().size() == 4);
    CHECK(h.calls()[2].value_repr == "true");
    CHECK(h.calls()[3].value_repr == "3");
}

TEST_CASE("border mock handler records stroke and shape mappers",
          "[mock][border][sequence]") {
    border b;
    border_handler<platform::mock> h;

    h.map_stroke_shape(b);
    h.map_stroke(b);
    h.clear_calls();

    b.stroke_shape = stroke_shape_desc{"RoundRectangle(12)"};
    b.stroke       = brush_ref{"Gray"};

    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].property_name == "stroke_shape");
    CHECK(h.calls()[0].value_repr    == "RoundRectangle(12)");
    CHECK(h.calls()[1].property_name == "stroke");
    CHECK(h.calls()[1].value_repr    == "Gray");
}
