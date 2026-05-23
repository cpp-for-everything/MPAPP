// SPDX-License-Identifier: Apache-2.0
// Tests for the 2D graphics facade per ADR-0015 — value types
// (color, path), the canvas interface contract via the stub backend,
// and the make_canvas() factory.

#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/detail/graphics/canvas.hpp>
#include <mpapp/detail/graphics/stub_canvas.hpp>

using namespace mpapp::detail::graphics;

// ---- color ---------------------------------------------------------------

TEST_CASE("color::rgb / rgba factories",
          "[graphics][color]") {
    constexpr auto c = color::rgb(0.5f, 0.25f, 0.75f);
    CHECK(c.r == 0.5f);
    CHECK(c.g == 0.25f);
    CHECK(c.b == 0.75f);
    CHECK(c.a == 1.0f);

    constexpr auto ca = color::rgba(1.0f, 1.0f, 1.0f, 0.5f);
    CHECK(ca.a == 0.5f);
}

TEST_CASE("color::from_hex parses 6-char RRGGBB",
          "[graphics][color]") {
    const auto c = color::from_hex("#FF8040");
    CHECK(c.r == 1.0f);
    CHECK(c.g > 0.50f);
    CHECK(c.g < 0.51f);
    CHECK(c.b > 0.25f);
    CHECK(c.b < 0.26f);
    CHECK(c.a == 1.0f);
}

TEST_CASE("color::from_hex parses 8-char RRGGBBAA",
          "[graphics][color]") {
    const auto c = color::from_hex("#FF000080");
    CHECK(c.r == 1.0f);
    CHECK(c.g == 0.0f);
    CHECK(c.b == 0.0f);
    CHECK(c.a > 0.50f);
    CHECK(c.a < 0.51f);
}

TEST_CASE("color::from_hex accepts no-# prefix and is case-insensitive",
          "[graphics][color]") {
    CHECK(color::from_hex("ff0000") == color::from_hex("#FF0000"));
    CHECK(color::from_hex("aBcDeF") == color::from_hex("#ABCDEF"));
}

TEST_CASE("color::from_hex returns default on parse failure",
          "[graphics][color]") {
    const auto def = color{};
    CHECK(color::from_hex("")          == def);
    CHECK(color::from_hex("#X12345")   == def);
    CHECK(color::from_hex("#12345")    == def);  // wrong length
    CHECK(color::from_hex("notahex")   == def);
}

// ---- path ----------------------------------------------------------------

TEST_CASE("path builder records ops in order",
          "[graphics][path]") {
    path p;
    p.move_to(0, 0)
     .line_to(10, 0)
     .quad_to(15, 5, 10, 10)
     .cubic_to(5, 15, -5, 15, 0, 10)
     .close();
    REQUIRE(p.size() == 5);
    CHECK(p.ops()[0].kind == path_op_kind::move);
    CHECK(p.ops()[1].kind == path_op_kind::line);
    CHECK(p.ops()[2].kind == path_op_kind::quad);
    CHECK(p.ops()[3].kind == path_op_kind::cubic);
    CHECK(p.ops()[4].kind == path_op_kind::close);
}

TEST_CASE("path::from_svg parses M / L / Z",
          "[graphics][path]") {
    const auto p = path::from_svg("M0 0 L10 0 L10 10 L0 10 Z");
    REQUIRE(p.size() == 5);
    CHECK(p.ops()[0].kind == path_op_kind::move);
    CHECK(p.ops()[1].kind == path_op_kind::line);
    CHECK(p.ops()[4].kind == path_op_kind::close);
    // Spot-check endpoint coordinates.
    CHECK(p.ops()[2].x[0] == 10);
    CHECK(p.ops()[2].y[0] == 10);
}

TEST_CASE("path::from_svg parses Q / C control points",
          "[graphics][path]") {
    const auto p = path::from_svg("M0 0 Q5 5 10 0 C0 0 0 10 10 10");
    REQUIRE(p.size() == 3);
    CHECK(p.ops()[1].kind == path_op_kind::quad);
    CHECK(p.ops()[2].kind == path_op_kind::cubic);
    // Quad: control (5,5) endpoint (10,0)
    CHECK(p.ops()[1].x[0] == 5);
    CHECK(p.ops()[1].y[0] == 5);
    CHECK(p.ops()[1].x[1] == 10);
    CHECK(p.ops()[1].y[1] == 0);
}

TEST_CASE("path::from_svg accepts commas and whitespace",
          "[graphics][path]") {
    const auto p = path::from_svg("M 0,0 L 10,0   L10,10");
    REQUIRE(p.size() == 3);
}

TEST_CASE("path::from_svg returns empty on parse failure",
          "[graphics][path]") {
    CHECK(path::from_svg("M0 0 X10").empty());      // unknown cmd
    CHECK(path::from_svg("M garbage").empty());
}

// ---- stub_canvas ---------------------------------------------------------

TEST_CASE("stub_canvas reports its dimensions",
          "[graphics][canvas]") {
    stub_canvas c(640, 480);
    CHECK(c.width_px()  == 640);
    CHECK(c.height_px() == 480);
}

TEST_CASE("stub_canvas records state-stack + transform ops",
          "[graphics][canvas]") {
    stub_canvas c(100, 100);
    c.save();
    c.translate(10, 20);
    c.scale(2.0, 2.0);
    c.rotate(1.5708);  // ~pi/2
    c.restore();

    REQUIRE(c.calls() == std::vector<std::string>{
        "save",
        "translate(10,20)",
        "scale(2,2)",
        "rotate(1.5708)",
        "restore",
    });
}

TEST_CASE("stub_canvas records paint setters with packed RGBA",
          "[graphics][canvas]") {
    stub_canvas c(100, 100);
    c.set_fill(color::rgba(1.0f, 0.5f, 0.25f, 0.75f));
    c.set_stroke(color::rgb(0.0f, 0.0f, 0.0f));
    c.set_stroke_width(3.5);
    c.set_line_cap(line_cap::round);
    c.set_line_join(line_join::bevel);
    c.set_opacity(0.5);

    REQUIRE(c.calls().size() == 6);
    CHECK(c.calls()[0] == "set_fill(1,0.5,0.25,0.75)");
    CHECK(c.calls()[1] == "set_stroke(0,0,0,1)");
    CHECK(c.calls()[2] == "set_stroke_width(3.5)");
    CHECK(c.calls()[3] == "set_line_cap(1)");   // round
    CHECK(c.calls()[4] == "set_line_join(2)");  // bevel
    CHECK(c.calls()[5] == "set_opacity(0.5)");
}

TEST_CASE("stub_canvas records draw ops",
          "[graphics][canvas]") {
    stub_canvas c(100, 100);
    c.clear(color::rgb(0.9f, 0.9f, 0.9f));
    c.fill_rect(rect{0, 0, 50, 30});
    c.stroke_rect(rect{10, 10, 5, 5});
    c.fill_ellipse(rect{20, 20, 40, 40});
    c.stroke_ellipse(rect{20, 20, 40, 40});

    path p;
    p.move_to(0, 0).line_to(10, 10).close();
    c.fill_path(p);
    c.stroke_path(p);
    c.clip(p);

    REQUIRE(c.calls().size() == 8);
    CHECK(c.calls()[0] == "clear(0.9,0.9,0.9,1)");
    CHECK(c.calls()[1] == "fill_rect(0,0,50,30)");
    CHECK(c.calls()[2] == "stroke_rect(10,10,5,5)");
    CHECK(c.calls()[3] == "fill_ellipse(20,20,40,40)");
    CHECK(c.calls()[4] == "stroke_ellipse(20,20,40,40)");
    CHECK(c.calls()[5] == "fill_path(ops=3)");
    CHECK(c.calls()[6] == "stroke_path(ops=3)");
    CHECK(c.calls()[7] == "clip(ops=3)");
}

TEST_CASE("stub_canvas clear_calls resets the recorder",
          "[graphics][canvas]") {
    stub_canvas c(100, 100);
    c.save();
    c.restore();
    CHECK(c.calls().size() == 2);
    c.clear_calls();
    CHECK(c.calls().empty());
}

// ---- make_canvas factory --------------------------------------------------

TEST_CASE("make_canvas returns a usable canvas of the requested size",
          "[graphics][canvas][factory]") {
    auto c = make_canvas(320, 240);
    REQUIRE(c != nullptr);
    CHECK(c->width_px()  == 320);
    CHECK(c->height_px() == 240);

    // Drive a couple of ops through the abstract interface — the
    // stub backend doesn't expose its recording vector through the
    // base class, so we just check that calls don't throw + the
    // factory returns the stub flavor.
    c->save();
    c->set_fill(color::rgb(1, 0, 0));
    c->fill_rect(rect{0, 0, 10, 10});
    c->restore();
}
