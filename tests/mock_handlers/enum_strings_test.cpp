// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Exhaustive coverage for the enum->string helpers and the
// corner_radius defaults across the layout/border/scroll/box-view headers.
// Each switch is driven through every case plus an out-of-range value to hit
// the "?" fallback arm.

#include <string_view>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/internal/basic_border.hpp>
#include <mpapp/internal/basic_box_view.hpp>
#include <mpapp/internal/basic_scroll_view.hpp>
#include <mpapp/layout_types.hpp>

using namespace mpapp;

TEST_CASE("corner_radius defaults to all-zero and compares by value",
          "[mock][box_view][corner_radius]") {
    corner_radius cr{};   // exercises every default member initializer
    CHECK(cr.top_left     == 0.0);
    CHECK(cr.top_right    == 0.0);
    CHECK(cr.bottom_left  == 0.0);
    CHECK(cr.bottom_right == 0.0);
    CHECK(cr == corner_radius{});
    CHECK(corner_radius{ 1.0, 2.0, 3.0, 4.0 } != cr);
}

TEST_CASE("pen_line_cap/pen_line_join stringify every case + fallback",
          "[mock][border][enum]") {
    CHECK(to_string(pen_line_cap::flat)   == "flat");
    CHECK(to_string(pen_line_cap::round)  == "round");
    CHECK(to_string(pen_line_cap::square) == "square");
    CHECK(to_string(static_cast<pen_line_cap>(99)) == "?");

    CHECK(to_string(pen_line_join::miter) == "miter");
    CHECK(to_string(pen_line_join::round) == "round");
    CHECK(to_string(pen_line_join::bevel) == "bevel");
    CHECK(to_string(static_cast<pen_line_join>(99)) == "?");
}

TEST_CASE("layout enum names cover every case + fallback",
          "[mock][layout][enum]") {
    CHECK(detail::orientation_name(orientation::vertical)   == "vertical");
    CHECK(detail::orientation_name(orientation::horizontal) == "horizontal");
    CHECK(detail::orientation_name(static_cast<orientation>(99)) == "?");

    CHECK(detail::h_align_name(h_align::start)   == "start");
    CHECK(detail::h_align_name(h_align::center)  == "center");
    CHECK(detail::h_align_name(h_align::end)     == "end");
    CHECK(detail::h_align_name(h_align::stretch) == "stretch");
    CHECK(detail::h_align_name(static_cast<h_align>(99)) == "?");

    CHECK(detail::v_align_name(v_align::start)   == "start");
    CHECK(detail::v_align_name(v_align::center)  == "center");
    CHECK(detail::v_align_name(v_align::end)     == "end");
    CHECK(detail::v_align_name(v_align::stretch) == "stretch");
    CHECK(detail::v_align_name(static_cast<v_align>(99)) == "?");
}

TEST_CASE("scroll enums stringify every case + fallback",
          "[mock][scroll_view][enum]") {
    CHECK(to_string(scroll_orientation::vertical)   == "vertical");
    CHECK(to_string(scroll_orientation::horizontal) == "horizontal");
    CHECK(to_string(scroll_orientation::both)       == "both");
    CHECK(to_string(scroll_orientation::neither)    == "neither");
    CHECK(to_string(static_cast<scroll_orientation>(99)) == "?");

    CHECK(to_string(scroll_bar_visibility::default_visibility) == "default");
    CHECK(to_string(scroll_bar_visibility::always)             == "always");
    CHECK(to_string(scroll_bar_visibility::never)              == "never");
    CHECK(to_string(static_cast<scroll_bar_visibility>(99))    == "?");
}
