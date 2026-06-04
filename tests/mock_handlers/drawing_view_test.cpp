// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/DrawingView.md
//
// Mock-handler tests for `mpapp::internal::basic_drawing_view` (ADR-0008).
// Covers every property Observable, every stroke-collection method, and
// both signals. Uses Catch2 with AAA structure.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/internal/basic_drawing_view.hpp>
#include <mpapp/handlers/mock/drawing_view_handler.hpp>

using namespace mpapp;
using drawing_view_mock = mpapp::internal::drawing_view_handler<mpapp::platform::mock>;

// ---------------------------------------------------------------------------
// point2d
// ---------------------------------------------------------------------------

TEST_CASE("point2d equality", "[mock][drawing_view]") {
    point2d a{1.0, 2.0};
    point2d b{1.0, 2.0};
    point2d c{3.0, 4.0};
    CHECK(a == b);
    CHECK(!(a == c));
}

// ---------------------------------------------------------------------------
// drawing_line
// ---------------------------------------------------------------------------

TEST_CASE("drawing_line equality", "[mock][drawing_view]") {
    drawing_line l1;
    l1.points     = {{0.0, 0.0}, {1.0, 1.0}};
    l1.line_color = brush_ref{"red"};
    l1.line_width = 2.0;

    drawing_line l2 = l1;
    CHECK(l1 == l2);

    l2.line_width = 3.0;
    CHECK(!(l1 == l2));
}

// ---------------------------------------------------------------------------
// Stroke collection: add_line / clear / line_count / line_at
// ---------------------------------------------------------------------------

TEST_CASE("drawing_view stroke collection: add and count",
          "[mock][drawing_view]") {
    internal::basic_drawing_view v;

    // Arrange
    drawing_line l;
    l.points     = {{0.0, 0.0}, {10.0, 10.0}};
    l.line_color = brush_ref{"blue"};
    l.line_width = 1.5;

    // Act
    v.add_line(l);

    // Assert
    REQUIRE(v.line_count() == 1);
    CHECK(v.line_at(0) == l);
}

TEST_CASE("drawing_view stroke collection: multiple lines",
          "[mock][drawing_view]") {
    internal::basic_drawing_view v;

    drawing_line a;
    a.points = {{0.0, 0.0}};
    drawing_line b;
    b.points = {{5.0, 5.0}};

    v.add_line(a);
    v.add_line(b);

    REQUIRE(v.line_count() == 2);
    CHECK(v.line_at(1).points[0].x == 5.0);
}

TEST_CASE("drawing_view stroke collection: clear removes all lines",
          "[mock][drawing_view]") {
    internal::basic_drawing_view v;

    v.add_line(drawing_line{});
    v.add_line(drawing_line{});
    REQUIRE(v.line_count() == 2);

    v.clear();
    CHECK(v.line_count() == 0);
}

TEST_CASE("drawing_view stroke collection: line_at throws on bad index",
          "[mock][drawing_view]") {
    internal::basic_drawing_view v;
    CHECK_THROWS_AS(v.line_at(0), std::out_of_range);
}

// ---------------------------------------------------------------------------
// Observable: default_line_color
// ---------------------------------------------------------------------------

TEST_CASE("drawing_view mock handler records initial default_line_color on bind",
          "[mock][drawing_view]") {
    internal::basic_drawing_view v;
    drawing_view_mock h;

    // Act
    h.map_default_line_color(v);

    // Assert: initial value is an empty brush_ref, formatted as ""
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "default_line_color");
    CHECK(h.calls()[0].value_repr    == "");
}

TEST_CASE("drawing_view mock handler records default_line_color change",
          "[mock][drawing_view]") {
    internal::basic_drawing_view v;
    drawing_view_mock h;

    h.map_default_line_color(v);
    h.clear_calls();

    // Act
    v.default_line_color = brush_ref{"#FF0000"};

    // Assert
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "default_line_color");
    CHECK(h.calls()[0].value_repr    == "#FF0000");
}

TEST_CASE("drawing_view mock handler ignores same-value default_line_color write",
          "[mock][drawing_view]") {
    internal::basic_drawing_view v;
    drawing_view_mock h;

    v.default_line_color = brush_ref{"red"};
    h.map_default_line_color(v);
    h.clear_calls();

    // Act: same value — Observable should not fire
    v.default_line_color = brush_ref{"red"};

    // Assert
    CHECK(h.calls().empty());
}

// ---------------------------------------------------------------------------
// Observable: default_line_width
// ---------------------------------------------------------------------------

TEST_CASE("drawing_view mock handler records initial default_line_width on bind",
          "[mock][drawing_view]") {
    internal::basic_drawing_view v;
    drawing_view_mock h;

    h.map_default_line_width(v);

    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "default_line_width");
    CHECK(h.calls()[0].value_repr    == "1");
}

TEST_CASE("drawing_view mock handler records default_line_width change",
          "[mock][drawing_view]") {
    internal::basic_drawing_view v;
    drawing_view_mock h;

    h.map_default_line_width(v);
    h.clear_calls();

    v.default_line_width = 4.0;

    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "4");
}

TEST_CASE("drawing_view mock handler ignores idempotent default_line_width write",
          "[mock][drawing_view]") {
    internal::basic_drawing_view v;
    drawing_view_mock h;

    h.map_default_line_width(v);
    h.clear_calls();

    v.default_line_width = 1.0;   // same as construction default
    CHECK(h.calls().empty());
}

// ---------------------------------------------------------------------------
// Observable: is_multi_line_mode
// ---------------------------------------------------------------------------

TEST_CASE("drawing_view mock handler records initial is_multi_line_mode on bind",
          "[mock][drawing_view]") {
    internal::basic_drawing_view v;
    drawing_view_mock h;

    h.map_is_multi_line_mode(v);

    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "is_multi_line_mode");
    CHECK(h.calls()[0].value_repr    == "false");
}

TEST_CASE("drawing_view mock handler records is_multi_line_mode toggle",
          "[mock][drawing_view]") {
    internal::basic_drawing_view v;
    drawing_view_mock h;

    h.map_is_multi_line_mode(v);
    h.clear_calls();

    v.is_multi_line_mode = true;

    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "true");

    v.is_multi_line_mode = false;
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "false");
}

TEST_CASE("drawing_view mock handler ignores same is_multi_line_mode value",
          "[mock][drawing_view]") {
    internal::basic_drawing_view v;
    drawing_view_mock h;

    h.map_is_multi_line_mode(v);
    h.clear_calls();

    v.is_multi_line_mode = false;   // same as default
    CHECK(h.calls().empty());
}

// ---------------------------------------------------------------------------
// Command: map_clear
// ---------------------------------------------------------------------------

TEST_CASE("drawing_view mock handler records clear command",
          "[mock][drawing_view]") {
    internal::basic_drawing_view v;
    drawing_view_mock h;

    h.map_clear(v);

    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "clear");
    CHECK(h.calls()[0].has_value     == false);
}

// ---------------------------------------------------------------------------
// Command: map_add_line
// ---------------------------------------------------------------------------

TEST_CASE("drawing_view mock handler records add_line command",
          "[mock][drawing_view]") {
    internal::basic_drawing_view v;
    drawing_view_mock h;

    drawing_line l;
    l.points     = {{1.0, 2.0}, {3.0, 4.0}};
    l.line_color = brush_ref{"green"};
    l.line_width = 3.0;

    h.map_add_line(v, l);

    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "add_line");
    CHECK(h.calls()[0].has_value     == true);
}

// ---------------------------------------------------------------------------
// Signal: drawing_started
// ---------------------------------------------------------------------------

TEST_CASE("drawing_view mock handler records drawing_started signal",
          "[mock][drawing_view]") {
    internal::basic_drawing_view v;
    drawing_view_mock h;

    h.map_drawing_started(v);

    // Simulate platform raising the event twice
    h.simulate_drawing_started(v);
    h.simulate_drawing_started(v);

    REQUIRE(h.calls_as_strings() ==
            std::vector<std::string>{"drawing_started", "drawing_started"});
}

TEST_CASE("drawing_view drawing_started not recorded before map",
          "[mock][drawing_view]") {
    internal::basic_drawing_view v;
    drawing_view_mock h;

    // Wire-up NOT called — signal should not reach handler
    v.drawing_started.emit();
    CHECK(h.calls().empty());
}

// ---------------------------------------------------------------------------
// Signal: drawing_line_completed
// ---------------------------------------------------------------------------

TEST_CASE("drawing_view mock handler records drawing_line_completed signal",
          "[mock][drawing_view]") {
    internal::basic_drawing_view v;
    drawing_view_mock h;

    h.map_drawing_line_completed(v);

    drawing_line l;
    l.points     = {{0.0, 0.0}, {5.0, 5.0}};
    l.line_color = brush_ref{"black"};
    l.line_width = 2.0;

    h.simulate_drawing_line_completed(v, l);

    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "drawing_line_completed");
    CHECK(h.calls()[0].has_value     == true);
}

TEST_CASE("drawing_view mock handler records multiple drawing_line_completed",
          "[mock][drawing_view]") {
    internal::basic_drawing_view v;
    drawing_view_mock h;

    h.map_drawing_line_completed(v);

    drawing_line l1;
    l1.line_width = 1.0;
    drawing_line l2;
    l2.line_width = 2.0;

    h.simulate_drawing_line_completed(v, l1);
    h.simulate_drawing_line_completed(v, l2);

    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].property_name == "drawing_line_completed");
    CHECK(h.calls()[1].property_name == "drawing_line_completed");
}

// ---------------------------------------------------------------------------
// Combined: full property + event sequence
// ---------------------------------------------------------------------------

TEST_CASE("drawing_view full sequence: properties then signals then clear",
          "[mock][drawing_view][sequence]") {
    internal::basic_drawing_view v;
    drawing_view_mock h;

    // Arrange: wire up all mappers
    h.map_default_line_color(v);
    h.map_default_line_width(v);
    h.map_is_multi_line_mode(v);
    h.map_drawing_started(v);
    h.map_drawing_line_completed(v);
    h.clear_calls();

    // Act: property changes
    v.default_line_color = brush_ref{"purple"};
    v.default_line_width = 5.0;
    v.is_multi_line_mode = true;

    // Act: simulate draw cycle
    h.simulate_drawing_started(v);
    drawing_line finished;
    finished.points     = {{0.0, 0.0}, {100.0, 100.0}};
    finished.line_color = brush_ref{"purple"};
    finished.line_width = 5.0;
    h.simulate_drawing_line_completed(v, finished);

    // Assert sequence
    REQUIRE(h.calls().size() == 5);
    CHECK(h.calls()[0].property_name == "default_line_color");
    CHECK(h.calls()[1].property_name == "default_line_width");
    CHECK(h.calls()[2].property_name == "is_multi_line_mode");
    CHECK(h.calls()[3].property_name == "drawing_started");
    CHECK(h.calls()[4].property_name == "drawing_line_completed");
}

// ---------------------------------------------------------------------------
// Handler attachment: has_handler
// ---------------------------------------------------------------------------

TEST_CASE("drawing_view has_handler is false by default",
          "[mock][drawing_view]") {
    internal::basic_drawing_view v;
    CHECK(!v.has_handler());
}
