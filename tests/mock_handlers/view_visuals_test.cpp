// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/View.md
//
// Tests for the rich-visual additions to `mpapp::view`: the
// gradient-capable `background_brush`, the `shadow`, and the `clip`
// primitive (RFC gradient-brush family). Exercises get/set semantics and
// the `changed` signal emission for each, plus the `clip_geometry`
// value-type equality used by the Observable change-detection path.

#include <catch2/catch_test_macros.hpp>

#include <optional>

#include <mpapp/view.hpp>
#include <mpapp/brushes/brush.hpp>
#include <mpapp/color.hpp>

using namespace mpapp;

namespace {

// Minimal concrete surface: `view` has no pure-virtual members, so a
// trivial subclass is a test-drivable instance of the cross-platform
// surface without dragging in a platform handler.
struct test_surface : view {};

}  // namespace

TEST_CASE("view background_brush defaults to nullopt", "[mock][view][brush]") {
    // Arrange / Act
    test_surface s;

    // Assert
    CHECK_FALSE(s.background_brush.get().has_value());
}

TEST_CASE("view background_brush get/set holds a solid brush",
          "[mock][view][brush]") {
    // Arrange
    test_surface s;
    const brush solid = solid_color_brush{color::from_rgb8(10, 20, 30)};

    // Act
    s.background_brush.set(solid);

    // Assert
    REQUIRE(s.background_brush.get().has_value());
    CHECK(s.background_brush.get().value() == solid);
}

TEST_CASE("view background_brush fires changed on a real change",
          "[mock][view][brush]") {
    // Arrange
    test_surface s;
    int fires = 0;
    mpapp::signal<const std::optional<brush>&>::slot_type slot;
    auto cb = [&fires](const std::optional<brush>&) { ++fires; };
    s.background_brush.changed.subscribe(slot, cb);

    // Act: set a gradient brush
    brush grad = linear_gradient_brush{
        point2{0.0, 0.0},
        point2{1.0, 1.0},
        {gradient_stop{0.0, color::from_rgb8(0, 0, 0)},
         gradient_stop{1.0, color::from_rgb8(255, 255, 255)}}};
    s.background_brush.set(grad);

    // Assert
    CHECK(fires == 1);
    REQUIRE(s.background_brush.get().has_value());
    CHECK(is_gradient(s.background_brush.get().value()));

    // Act: identical set is idempotent (no extra fire)
    s.background_brush.set(grad);
    CHECK(fires == 1);

    // Act: clearing back to nullopt is a real change
    s.background_brush.set(std::nullopt);
    CHECK(fires == 2);
    CHECK_FALSE(s.background_brush.get().has_value());
}

TEST_CASE("view shadow defaults to a zeroed descriptor", "[mock][view][shadow]") {
    // Arrange / Act
    test_surface s;

    // Assert
    CHECK(s.shadow.get() == shadow_desc{});
}

TEST_CASE("view shadow get/set and changed signal", "[mock][view][shadow]") {
    // Arrange
    test_surface s;
    int fires = 0;
    mpapp::signal<const shadow_desc&>::slot_type slot;
    auto cb = [&fires](const shadow_desc&) { ++fires; };
    s.shadow.changed.subscribe(slot, cb);

    // Act
    const shadow_desc sh{2.0, 4.0, 8.0, 0.5};
    s.shadow.set(sh);

    // Assert
    CHECK(fires == 1);
    CHECK(s.shadow.get() == sh);

    // Idempotent set
    s.shadow.set(sh);
    CHECK(fires == 1);
}

TEST_CASE("view clip defaults to nullopt", "[mock][view][clip]") {
    // Arrange / Act
    test_surface s;

    // Assert
    CHECK_FALSE(s.clip.get().has_value());
}

TEST_CASE("clip_geometry default members and equality", "[mock][view][clip]") {
    // Arrange
    const clip_geometry a{};
    const clip_geometry b{};
    const clip_geometry c{1.0, 2.0, 3.0, 4.0, 5.0, 6.0};

    // Assert defaults
    CHECK(a.x == 0.0);
    CHECK(a.y == 0.0);
    CHECK(a.width == 0.0);
    CHECK(a.height == 0.0);
    CHECK(a.radius_x == 0.0);
    CHECK(a.radius_y == 0.0);

    // Assert equality semantics
    CHECK(a == b);
    CHECK_FALSE(a == c);
    CHECK(c.width == 3.0);
    CHECK(c.radius_y == 6.0);
}

TEST_CASE("view clip get/set and changed signal", "[mock][view][clip]") {
    // Arrange
    test_surface s;
    int fires = 0;
    mpapp::signal<const std::optional<clip_geometry>&>::slot_type slot;
    auto cb = [&fires](const std::optional<clip_geometry>&) { ++fires; };
    s.clip.changed.subscribe(slot, cb);

    // Act: set a rounded-rect clip
    const clip_geometry g{0.0, 0.0, 100.0, 50.0, 8.0, 8.0};
    s.clip.set(g);

    // Assert
    CHECK(fires == 1);
    REQUIRE(s.clip.get().has_value());
    CHECK(s.clip.get().value() == g);

    // Idempotent set
    s.clip.set(g);
    CHECK(fires == 1);

    // Clear back to nullopt
    s.clip.set(std::nullopt);
    CHECK(fires == 2);
    CHECK_FALSE(s.clip.get().has_value());
}
