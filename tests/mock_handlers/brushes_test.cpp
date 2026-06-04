// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Brushes.md
//
// Unit tests for `mpapp::brush` - solid-color, linear-gradient, and
// radial-gradient value types, the `brush` variant alias, and the
// `to_string` / `is_gradient` free helpers.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/brushes/brush.hpp>

using namespace mpapp;

// ── point2 ────────────────────────────────────────────────────────────────

TEST_CASE("point2 default-constructs to origin", "[brushes][point2]") {
    // Arrange / Act
    point2 p;
    // Assert
    CHECK(p.x == 0.0);
    CHECK(p.y == 0.0);
}

TEST_CASE("point2 value construction", "[brushes][point2]") {
    // Arrange / Act
    point2 p{3.0, 4.0};
    // Assert
    CHECK(p.x == 3.0);
    CHECK(p.y == 4.0);
}

TEST_CASE("point2 equality", "[brushes][point2]") {
    // Arrange
    point2 a{1.0, 2.0};
    point2 b{1.0, 2.0};
    point2 c{0.0, 0.0};
    // Assert
    CHECK(a == b);
    CHECK(!(a == c));
}

// ── gradient_stop ─────────────────────────────────────────────────────────

TEST_CASE("gradient_stop default-constructs to zero offset transparent black",
          "[brushes][gradient_stop]") {
    gradient_stop s;
    CHECK(s.offset == 0.0);
    CHECK(s.color == color{});
}

TEST_CASE("gradient_stop value construction and equality", "[brushes][gradient_stop]") {
    // Arrange
    gradient_stop a{0.5, color{1.0, 0.0, 0.0, 1.0}};
    gradient_stop b{0.5, color{1.0, 0.0, 0.0, 1.0}};
    gradient_stop c{1.0, color{0.0, 1.0, 0.0, 1.0}};
    // Assert
    CHECK(a == b);
    CHECK(!(a == c));
    CHECK(a.offset == 0.5);
    CHECK(a.color.r == 1.0);
}

// ── solid_color_brush ─────────────────────────────────────────────────────

TEST_CASE("solid_color_brush default-constructs to zero color", "[brushes][solid]") {
    solid_color_brush s;
    CHECK(s.color == color{});
}

TEST_CASE("solid_color_brush stores color correctly", "[brushes][solid]") {
    // Arrange
    color red{1.0, 0.0, 0.0, 1.0};
    // Act
    solid_color_brush s{red};
    // Assert
    CHECK(s.color == red);
}

TEST_CASE("solid_color_brush equality", "[brushes][solid]") {
    solid_color_brush a{color{0.5, 0.5, 0.5, 1.0}};
    solid_color_brush b{color{0.5, 0.5, 0.5, 1.0}};
    solid_color_brush c{color{0.0, 0.0, 0.0, 1.0}};
    CHECK(a == b);
    CHECK(!(a == c));
}

// ── linear_gradient_brush ─────────────────────────────────────────────────

TEST_CASE("linear_gradient_brush default construction", "[brushes][linear]") {
    linear_gradient_brush lg;
    CHECK(lg.start_point == point2{0.0, 0.0});
    CHECK(lg.end_point   == point2{1.0, 1.0});
    CHECK(lg.stops.empty());
}

TEST_CASE("linear_gradient_brush stores start/end points", "[brushes][linear]") {
    // Arrange
    point2 s{0.0, 0.5};
    point2 e{1.0, 0.5};
    // Act
    linear_gradient_brush lg{s, e, {}};
    // Assert
    CHECK(lg.start_point == s);
    CHECK(lg.end_point   == e);
}

TEST_CASE("linear_gradient_brush stores stops", "[brushes][linear]") {
    // Arrange
    gradient_stop s0{0.0, color{0.0, 0.0, 1.0, 1.0}};
    gradient_stop s1{1.0, color{1.0, 0.0, 0.0, 1.0}};
    // Act
    linear_gradient_brush lg{{0,0}, {1,1}, {s0, s1}};
    // Assert
    REQUIRE(lg.stops.size() == 2);
    CHECK(lg.stops[0] == s0);
    CHECK(lg.stops[1] == s1);
}

TEST_CASE("linear_gradient_brush equality - same stops", "[brushes][linear]") {
    gradient_stop s{0.5, color{1.0, 1.0, 0.0, 1.0}};
    linear_gradient_brush a{{0,0},{1,1},{s}};
    linear_gradient_brush b{{0,0},{1,1},{s}};
    CHECK(a == b);
}

TEST_CASE("linear_gradient_brush inequality - different stops", "[brushes][linear]") {
    gradient_stop s0{0.0, color{0,0,0,1}};
    gradient_stop s1{1.0, color{1,1,1,1}};
    linear_gradient_brush a{{0,0},{1,1},{s0}};
    linear_gradient_brush b{{0,0},{1,1},{s1}};
    CHECK(!(a == b));
}

TEST_CASE("linear_gradient_brush inequality - different endpoints", "[brushes][linear]") {
    linear_gradient_brush a{{0,0},{1,0},{}};
    linear_gradient_brush b{{0,0},{0,1},{}};
    CHECK(!(a == b));
}

// ── radial_gradient_origin ────────────────────────────────────────────────

TEST_CASE("radial_gradient_origin to_string", "[brushes][radial_origin]") {
    CHECK(to_string(radial_gradient_origin::relative_to_bounding_box) == "relative_to_bounding_box");
    CHECK(to_string(radial_gradient_origin::absolute)                  == "absolute");
}

// ── radial_gradient_brush ─────────────────────────────────────────────────

TEST_CASE("radial_gradient_brush default construction", "[brushes][radial]") {
    radial_gradient_brush rb;
    CHECK(rb.center == point2{0.5, 0.5});
    CHECK(rb.radius == 0.5);
    CHECK(rb.stops.empty());
}

TEST_CASE("radial_gradient_brush stores center and radius", "[brushes][radial]") {
    // Arrange / Act
    radial_gradient_brush rb{point2{0.3, 0.7}, 0.25, {}};
    // Assert
    CHECK(rb.center == point2{0.3, 0.7});
    CHECK(rb.radius == 0.25);
}

TEST_CASE("radial_gradient_brush stores stops", "[brushes][radial]") {
    // Arrange
    gradient_stop inner{0.0, color{1.0, 1.0, 1.0, 1.0}};
    gradient_stop outer{1.0, color{0.0, 0.0, 0.0, 0.0}};
    // Act
    radial_gradient_brush rb{{0.5,0.5}, 0.5, {inner, outer}};
    // Assert
    REQUIRE(rb.stops.size() == 2);
    CHECK(rb.stops[0] == inner);
    CHECK(rb.stops[1] == outer);
}

TEST_CASE("radial_gradient_brush equality", "[brushes][radial]") {
    gradient_stop s{0.5, color{0.5, 0.5, 0.5, 1.0}};
    radial_gradient_brush a{{0.5,0.5}, 0.5, {s}};
    radial_gradient_brush b{{0.5,0.5}, 0.5, {s}};
    CHECK(a == b);
}

TEST_CASE("radial_gradient_brush inequality - different radius", "[brushes][radial]") {
    radial_gradient_brush a{{0.5,0.5}, 0.5, {}};
    radial_gradient_brush b{{0.5,0.5}, 0.3, {}};
    CHECK(!(a == b));
}

TEST_CASE("radial_gradient_brush inequality - different center", "[brushes][radial]") {
    radial_gradient_brush a{{0.5,0.5}, 0.5, {}};
    radial_gradient_brush b{{0.0,0.0}, 0.5, {}};
    CHECK(!(a == b));
}

// ── brush variant ─────────────────────────────────────────────────────────

TEST_CASE("brush variant holds solid_color_brush", "[brushes][variant]") {
    brush b = solid_color_brush{color{1.0, 0.0, 0.0, 1.0}};
    CHECK(std::holds_alternative<solid_color_brush>(b));
    CHECK(std::get<solid_color_brush>(b).color == color{1.0, 0.0, 0.0, 1.0});
}

TEST_CASE("brush variant holds linear_gradient_brush", "[brushes][variant]") {
    brush b = linear_gradient_brush{};
    CHECK(std::holds_alternative<linear_gradient_brush>(b));
}

TEST_CASE("brush variant holds radial_gradient_brush", "[brushes][variant]") {
    brush b = radial_gradient_brush{};
    CHECK(std::holds_alternative<radial_gradient_brush>(b));
}

TEST_CASE("brush variant can be reassigned to a different type", "[brushes][variant]") {
    // Arrange
    brush b = solid_color_brush{};
    CHECK(std::holds_alternative<solid_color_brush>(b));
    // Act
    b = radial_gradient_brush{};
    // Assert
    CHECK(std::holds_alternative<radial_gradient_brush>(b));
}

// ── is_gradient ───────────────────────────────────────────────────────────

TEST_CASE("is_gradient returns false for solid_color_brush", "[brushes][is_gradient]") {
    brush b = solid_color_brush{};
    CHECK(!is_gradient(b));
}

TEST_CASE("is_gradient returns true for linear_gradient_brush", "[brushes][is_gradient]") {
    brush b = linear_gradient_brush{};
    CHECK(is_gradient(b));
}

TEST_CASE("is_gradient returns true for radial_gradient_brush", "[brushes][is_gradient]") {
    brush b = radial_gradient_brush{};
    CHECK(is_gradient(b));
}

// ── to_string(brush) ──────────────────────────────────────────────────────

TEST_CASE("to_string solid_color_brush contains 'solid'", "[brushes][to_string]") {
    brush b = solid_color_brush{color{1.0, 0.0, 0.0, 1.0}};
    std::string s = to_string(b);
    CHECK(s.find("solid") != std::string::npos);
}

TEST_CASE("to_string solid_color_brush encodes RGBA components", "[brushes][to_string]") {
    brush b = solid_color_brush{color{0.5, 0.25, 0.0, 1.0}};
    std::string s = to_string(b);
    // Must contain 'rgba(' prefix
    CHECK(s.find("rgba(") != std::string::npos);
}

TEST_CASE("to_string linear_gradient_brush contains 'linear_gradient'",
          "[brushes][to_string]") {
    gradient_stop a{0.0, color{0,0,0,1}};
    gradient_stop b_stop{1.0, color{1,1,1,1}};
    brush b = linear_gradient_brush{{0,0},{1,1},{a, b_stop}};
    std::string s = to_string(b);
    CHECK(s.find("linear_gradient") != std::string::npos);
    CHECK(s.find("2") != std::string::npos);   // stops=2
}

TEST_CASE("to_string radial_gradient_brush contains 'radial_gradient'",
          "[brushes][to_string]") {
    brush b = radial_gradient_brush{{0.5,0.5}, 0.5, {}};
    std::string s = to_string(b);
    CHECK(s.find("radial_gradient") != std::string::npos);
    CHECK(s.find("0") != std::string::npos);   // stops=0
}

TEST_CASE("to_string solid_color_brush with default color", "[brushes][to_string]") {
    brush b = solid_color_brush{};
    std::string s = to_string(b);
    CHECK(!s.empty());
    CHECK(s.find("solid") != std::string::npos);
}

TEST_CASE("to_string linear_gradient_brush with no stops shows stops=0",
          "[brushes][to_string]") {
    brush b = linear_gradient_brush{};
    std::string s = to_string(b);
    CHECK(s.find("stops=0") != std::string::npos);
}

TEST_CASE("to_string radial_gradient_brush with stops shows correct count",
          "[brushes][to_string]") {
    gradient_stop s1{0.0, color{1,0,0,1}};
    gradient_stop s2{0.5, color{0,1,0,1}};
    gradient_stop s3{1.0, color{0,0,1,1}};
    brush b = radial_gradient_brush{{0.5,0.5}, 0.5, {s1, s2, s3}};
    std::string s = to_string(b);
    CHECK(s.find("stops=3") != std::string::npos);
}
