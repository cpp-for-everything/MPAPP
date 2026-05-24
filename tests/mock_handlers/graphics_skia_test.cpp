// SPDX-License-Identifier: Apache-2.0
// Tests for the Skia backend of the graphics facade per ADR-0015.
// Only compiled when MPAPP_GRAPHICS_HAS_SKIA is defined (i.e. CMake
// detected the Skia prebuilt at configure time and the backend was
// selected). Drives the abstract `canvas*` API and reads pixels back
// via the public `pixel_data()` / `pixel_stride_bytes()` surface — no
// direct Skia headers needed, the test stays backend-agnostic at the
// source level.
//
// Mirrors the case set in `graphics_cairo_test.cpp`. The Skia
// implementation is `kN32_SkColorType` (= BGRA32 premultiplied on the
// little-endian platforms MPAPP supports), which matches Cairo's
// `CAIRO_FORMAT_ARGB32` byte ordering and the format documented on
// `canvas::pixel_data()`. The pixel-readback assertions below therefore
// look the same as the Cairo ones — same byte order, same alpha
// premultiplication rule.

#if defined(MPAPP_GRAPHICS_HAS_SKIA)

#include <cstdint>
#include <memory>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/detail/graphics/canvas.hpp>

using namespace mpapp::detail::graphics;

namespace {

// Premultiplied BGRA32 pixel access. The canvas API guarantees this
// byte order on every backend that returns non-null pixel_data().
struct bgra_pixel {
    std::uint8_t b{0};
    std::uint8_t g{0};
    std::uint8_t r{0};
    std::uint8_t a{0};
};

bgra_pixel sample(const canvas& c, int x, int y) {
    const auto* data   = c.pixel_data();
    const auto stride  = c.pixel_stride_bytes();
    REQUIRE(data    != nullptr);
    REQUIRE(stride  >  0);
    const auto* px = data + static_cast<std::ptrdiff_t>(y) * stride
                          + static_cast<std::ptrdiff_t>(x) * 4;
    bgra_pixel out;
    out.b = px[0];
    out.g = px[1];
    out.r = px[2];
    out.a = px[3];
    return out;
}

} // namespace

TEST_CASE("skia backend make_canvas returns a usable canvas",
          "[graphics][skia]") {
    auto c = make_canvas(64, 64);
    REQUIRE(c != nullptr);
    CHECK(c->width_px()  == 64);
    CHECK(c->height_px() == 64);
    // Backend must accept arbitrary ops without crashing. Pixel
    // assertions live in the readback test below.
    c->clear(color::rgba(1, 1, 1, 1));
    c->set_fill(color::rgb(1, 0, 0));
    c->fill_rect(rect{0, 0, 10, 10});
    // pixel_data() must be non-null for a CPU-backed Skia canvas.
    CHECK(c->pixel_data()         != nullptr);
    CHECK(c->pixel_stride_bytes() >  0);
}

TEST_CASE("skia backend: clear paints the full surface",
          "[graphics][skia]") {
    auto c = make_canvas(16, 16);
    REQUIRE(c != nullptr);
    c->clear(color::rgba(1, 0, 0, 1));

    // Every pixel should be fully-opaque red.
    const auto p = sample(*c, 5, 5);
    CHECK(p.r == 255);
    CHECK(p.g == 0);
    CHECK(p.b == 0);
    CHECK(p.a == 255);
}

TEST_CASE("skia backend: fill_rect draws to expected pixels",
          "[graphics][skia]") {
    auto c = make_canvas(32, 32);
    REQUIRE(c != nullptr);
    c->clear(color::rgba(1, 1, 1, 1));
    c->set_fill(color::rgb(0, 0, 0));
    c->fill_rect(rect{8, 8, 16, 16});

    // Inside the black rect — exactly black.
    auto p_inside = sample(*c, 12, 12);
    CHECK(p_inside.r == 0);
    CHECK(p_inside.g == 0);
    CHECK(p_inside.b == 0);
    CHECK(p_inside.a == 255);

    // Outside (still white).
    auto p_outside = sample(*c, 2, 2);
    CHECK(p_outside.r == 255);
    CHECK(p_outside.g == 255);
    CHECK(p_outside.b == 255);
    CHECK(p_outside.a == 255);
}

TEST_CASE("skia backend: BGRA byte order matches facade contract",
          "[graphics][skia]") {
    // Paint a known non-grey, non-axis color so a byte-order swap
    // (RGBA vs BGRA, accidentally surfacing Skia's raw kRGBA layout
    // through the facade) would be immediately visible. #E63946 — the
    // same red shape_view fill MPAPP's smoke tests use.
    auto c = make_canvas(8, 8);
    REQUIRE(c != nullptr);
    c->clear(color::rgba(
        static_cast<double>(0xE6) / 255.0,
        static_cast<double>(0x39) / 255.0,
        static_cast<double>(0x46) / 255.0,
        1.0));
    const auto p = sample(*c, 4, 4);
    CHECK(p.r == 0xE6);
    CHECK(p.g == 0x39);
    CHECK(p.b == 0x46);
    CHECK(p.a == 0xFF);
}

TEST_CASE("skia backend: save / restore actually balances",
          "[graphics][skia]") {
    auto c = make_canvas(16, 16);
    REQUIRE(c != nullptr);
    c->clear(color::rgba(1, 1, 1, 1));
    c->save();
    c->translate(5, 5);
    c->scale(2, 2);
    c->restore();
    // No assertion on pixel state — just confirming the calls don't
    // unbalance Skia's state stack (which would crash on a subsequent
    // restore when there's nothing to pop, same way Cairo's would).
    c->save();
    c->restore();
    SUCCEED("balanced state-stack survived");
}

TEST_CASE("skia backend: path emission accepts every op kind",
          "[graphics][skia]") {
    auto c = make_canvas(32, 32);
    REQUIRE(c != nullptr);
    c->clear(color::rgba(1, 1, 1, 1));
    c->set_stroke(color::rgb(0, 0, 0));
    c->set_stroke_width(2.0);

    // move / line / quad / cubic / close — the full set the facade's
    // SVG-path subset exposes. Skia's backend builds via SkPathBuilder
    // (per the immutability change in m140+); this case exercises
    // every branch of the to_skia_path() switch.
    path p;
    p.move_to(0, 0)
     .line_to(10, 0)
     .quad_to(15, 5, 10, 10)
     .cubic_to(5, 15, -5, 15, 0, 10)
     .close();
    c->stroke_path(p);
    SUCCEED("all path op kinds rendered without crash");
}

TEST_CASE("skia backend: ellipse + clip don't crash",
          "[graphics][skia]") {
    auto c = make_canvas(32, 32);
    REQUIRE(c != nullptr);
    c->clear(color::rgba(1, 1, 1, 1));
    c->set_fill(color::rgb(0, 0, 0));
    c->fill_ellipse(rect{4, 4, 24, 24});
    c->stroke_ellipse(rect{4, 4, 24, 24});

    path clip_p;
    clip_p.move_to(0, 0).line_to(32, 0).line_to(32, 32).line_to(0, 32).close();
    c->save();
    c->clip(clip_p);
    c->fill_rect(rect{0, 0, 32, 32});
    c->restore();
    SUCCEED("ellipse + clip path-ops survived");
}

#endif // MPAPP_GRAPHICS_HAS_SKIA
