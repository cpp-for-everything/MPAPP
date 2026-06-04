// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/AvatarView.md
//
// Mock-handler tests for `mpapp::internal::basic_avatar_view` (CLAUDE Rule 6 / ADR-0008).

#include <catch2/catch_test_macros.hpp>

#include <mpapp/internal/basic_avatar_view.hpp>
#include <mpapp/handlers/mock/avatar_view_handler.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// Default state
// ---------------------------------------------------------------------------

TEST_CASE("avatar_view default property values",
          "[mock][avatar_view]") {
    internal::basic_avatar_view a;

    CHECK(a.initials.get()      == "");
    CHECK(a.image_source.get()  == "");
    CHECK(a.corner_radius.get() == 0.0);
    CHECK(a.background.get()    == brush_ref{});
    CHECK(a.text_color.get()    == brush_ref{});
    CHECK(a.shape.get()         == avatar_shape::circle);
}

// ---------------------------------------------------------------------------
// map_* records initial values
// ---------------------------------------------------------------------------

TEST_CASE("avatar_view mock handler records initial values on bind",
          "[mock][avatar_view]") {
    internal::basic_avatar_view a;
    internal::avatar_view_handler<platform::mock> h;

    h.map_initials(a);
    h.map_image_source(a);
    h.map_corner_radius(a);
    h.map_background(a);
    h.map_text_color(a);
    h.map_shape(a);

    REQUIRE(h.calls().size() == 6);
    CHECK(h.calls()[0].property_name == "initials");
    CHECK(h.calls()[0].value_repr    == "");
    CHECK(h.calls()[1].property_name == "image_source");
    CHECK(h.calls()[1].value_repr    == "");
    CHECK(h.calls()[2].property_name == "corner_radius");
    CHECK(h.calls()[2].value_repr    == "0");
    CHECK(h.calls()[3].property_name == "background");
    CHECK(h.calls()[3].value_repr    == "");
    CHECK(h.calls()[4].property_name == "text_color");
    CHECK(h.calls()[4].value_repr    == "");
    CHECK(h.calls()[5].property_name == "shape");
    CHECK(h.calls()[5].value_repr    == "circle");
}

// ---------------------------------------------------------------------------
// initials
// ---------------------------------------------------------------------------

TEST_CASE("avatar_view mock handler records initials changes",
          "[mock][avatar_view]") {
    internal::basic_avatar_view a;
    internal::avatar_view_handler<platform::mock> h;

    h.map_initials(a);
    h.clear_calls();

    a.initials = "AB";
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "initials");
    CHECK(h.calls()[0].value_repr    == "AB");

    a.initials = "AB";  // idempotent
    REQUIRE(h.calls().size() == 1);

    a.initials = "XY";
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "XY");
}

// ---------------------------------------------------------------------------
// image_source
// ---------------------------------------------------------------------------

TEST_CASE("avatar_view mock handler records image_source changes",
          "[mock][avatar_view]") {
    internal::basic_avatar_view a;
    internal::avatar_view_handler<platform::mock> h;

    h.map_image_source(a);
    h.clear_calls();

    a.image_source = "avatar.png";
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "avatar.png");

    a.image_source = "avatar.png";  // idempotent
    REQUIRE(h.calls().size() == 1);

    a.image_source = "profile.jpg";
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "profile.jpg");
}

// ---------------------------------------------------------------------------
// corner_radius
// ---------------------------------------------------------------------------

TEST_CASE("avatar_view mock handler records corner_radius changes",
          "[mock][avatar_view]") {
    internal::basic_avatar_view a;
    internal::avatar_view_handler<platform::mock> h;

    h.map_corner_radius(a);
    h.clear_calls();

    a.corner_radius = 8.0;
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "corner_radius");
    CHECK(h.calls()[0].value_repr    == "8");

    a.corner_radius = 8.0;  // idempotent
    REQUIRE(h.calls().size() == 1);

    a.corner_radius = 16.0;
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "16");
}

// ---------------------------------------------------------------------------
// background
// ---------------------------------------------------------------------------

TEST_CASE("avatar_view mock handler records background changes",
          "[mock][avatar_view]") {
    internal::basic_avatar_view a;
    internal::avatar_view_handler<platform::mock> h;

    h.map_background(a);
    h.clear_calls();

    a.background = brush_ref{"blue"};
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "background");
    CHECK(h.calls()[0].value_repr    == "blue");

    a.background = brush_ref{"blue"};  // idempotent
    REQUIRE(h.calls().size() == 1);

    a.background = brush_ref{"red"};
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "red");
}

// ---------------------------------------------------------------------------
// text_color
// ---------------------------------------------------------------------------

TEST_CASE("avatar_view mock handler records text_color changes",
          "[mock][avatar_view]") {
    internal::basic_avatar_view a;
    internal::avatar_view_handler<platform::mock> h;

    h.map_text_color(a);
    h.clear_calls();

    a.text_color = brush_ref{"white"};
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "text_color");
    CHECK(h.calls()[0].value_repr    == "white");

    a.text_color = brush_ref{"white"};  // idempotent
    REQUIRE(h.calls().size() == 1);

    a.text_color = brush_ref{"black"};
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "black");
}

// ---------------------------------------------------------------------------
// shape (enum)
// ---------------------------------------------------------------------------

TEST_CASE("avatar_view mock handler records shape changes",
          "[mock][avatar_view]") {
    internal::basic_avatar_view a;
    internal::avatar_view_handler<platform::mock> h;

    h.map_shape(a);
    h.clear_calls();

    a.shape = avatar_shape::square;
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "shape");
    CHECK(h.calls()[0].value_repr    == "square");

    a.shape = avatar_shape::square;  // idempotent
    REQUIRE(h.calls().size() == 1);

    a.shape = avatar_shape::circle;
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "circle");
}

// ---------------------------------------------------------------------------
// Multi-property sequence
// ---------------------------------------------------------------------------

TEST_CASE("avatar_view multi-property sequence is ordered",
          "[mock][avatar_view][sequence]") {
    internal::basic_avatar_view a;
    internal::avatar_view_handler<platform::mock> h;

    h.map_initials(a);
    h.map_corner_radius(a);
    h.map_shape(a);
    h.clear_calls();

    a.initials      = "JD";
    a.corner_radius = 4.0;
    a.shape         = avatar_shape::square;
    a.initials      = "JD";  // idempotent
    a.corner_radius = 12.0;

    REQUIRE(h.calls().size() == 4);
    CHECK(h.calls()[0].property_name == "initials");
    CHECK(h.calls()[0].value_repr    == "JD");
    CHECK(h.calls()[1].property_name == "corner_radius");
    CHECK(h.calls()[1].value_repr    == "4");
    CHECK(h.calls()[2].property_name == "shape");
    CHECK(h.calls()[2].value_repr    == "square");
    CHECK(h.calls()[3].property_name == "corner_radius");
    CHECK(h.calls()[3].value_repr    == "12");
}

// ---------------------------------------------------------------------------
// clear_calls
// ---------------------------------------------------------------------------

TEST_CASE("avatar_view clear_calls resets the log",
          "[mock][avatar_view]") {
    internal::basic_avatar_view a;
    internal::avatar_view_handler<platform::mock> h;

    h.map_initials(a);
    h.map_shape(a);
    REQUIRE(h.calls().size() == 2);

    h.clear_calls();
    CHECK(h.calls().empty());

    a.initials = "ZZ";
    REQUIRE(h.calls().size() == 1);
}
