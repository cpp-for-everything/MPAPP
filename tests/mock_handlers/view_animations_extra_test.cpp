// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for view_animations_extra.hpp - generic
// Observable-driving animation helpers.

#include <chrono>
#include <memory>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <mpapp/animation/animation.hpp>
#include <mpapp/animation/easing.hpp>
#include <mpapp/animation/view_animations_extra.hpp>
#include <mpapp/color.hpp>
#include <mpapp/observable.hpp>

using namespace mpapp;
using namespace std::chrono_literals;
using Catch::Approx;

// ---------------------------------------------------------------------------
// animate_value - Observable<double>
// ---------------------------------------------------------------------------

TEST_CASE("animate_value seeds the Observable at construction",
          "[mock][view_animations_extra]") {
    // Arrange
    Observable<double> obs{ 10.0 };

    // Act
    auto anim = animate_value(obs, 20.0, 100ms, easing_kind::linear);

    // Assert - construction seeds the value back to 'from'
    CHECK(obs.get() == Approx(10.0));
    CHECK_FALSE(anim->finished());
}

TEST_CASE("animate_value drives Observable<double> from start to end",
          "[mock][view_animations_extra]") {
    // Arrange
    Observable<double> obs{ 0.0 };
    auto anim = animate_value(obs, 100.0, 100ms, easing_kind::linear);

    // Act + Assert - start
    CHECK(obs.get() == Approx(0.0));

    // midpoint
    anim->advance(50ms);
    CHECK(obs.get() == Approx(50.0));

    // end
    anim->advance(50ms);
    CHECK(obs.get() == Approx(100.0));
    CHECK(anim->finished());
}

TEST_CASE("animate_value respects easing at midpoint (cubic_in)",
          "[mock][view_animations_extra]") {
    // Arrange
    Observable<double> obs{ 0.0 };
    auto anim = animate_value(obs, 1.0, 100ms, easing_kind::cubic_in);

    // Act - advance to 50 % (cubic_in(0.5) == 0.125)
    anim->advance(50ms);

    // Assert
    CHECK(obs.get() == Approx(0.125));
}

TEST_CASE("animate_value: further advances after finish are no-ops",
          "[mock][view_animations_extra]") {
    // Arrange
    Observable<double> obs{ 5.0 };
    auto anim = animate_value(obs, 15.0, 100ms, easing_kind::linear);

    // Act
    anim->advance(100ms);   // finishes
    CHECK(anim->finished());
    const double final_val = obs.get();
    anim->advance(100ms);   // should be a no-op

    // Assert
    CHECK(obs.get() == Approx(final_val));
}

TEST_CASE("animate_value: seek() scrubs to arbitrary progress without finishing",
          "[mock][view_animations_extra]") {
    // Arrange
    Observable<double> obs{ 0.0 };
    auto anim = animate_value(obs, 200.0, 500ms, easing_kind::linear);

    // Act
    anim->seek(0.25);

    // Assert - 25 % of 0→200
    CHECK(obs.get() == Approx(50.0));
    CHECK_FALSE(anim->finished());

    // Act
    anim->seek(1.0);
    CHECK(obs.get() == Approx(200.0));
    CHECK_FALSE(anim->finished());  // seek does NOT flip finished_
}

TEST_CASE("animate_value: zero-duration animation jumps to end immediately",
          "[mock][view_animations_extra]") {
    // Arrange
    Observable<double> obs{ 3.0 };
    auto anim = animate_value(obs, 7.0, 0ms, easing_kind::linear);

    // Act - any advance finishes it
    anim->advance(0ms);

    // Assert
    CHECK(obs.get() == Approx(7.0));
    CHECK(anim->finished());
}

// ---------------------------------------------------------------------------
// color_lerp helper
// ---------------------------------------------------------------------------

TEST_CASE("color_lerp interpolates each channel independently",
          "[mock][view_animations_extra][color_lerp]") {
    // Arrange
    const color from{ 0.0, 0.0, 0.0, 0.0 };
    const color to  { 1.0, 0.5, 0.25, 0.8 };

    // Act - midpoint
    const color mid = color_lerp(from, to, 0.5);

    // Assert
    CHECK(mid.r == Approx(0.5));
    CHECK(mid.g == Approx(0.25));
    CHECK(mid.b == Approx(0.125));
    CHECK(mid.a == Approx(0.4));
}

TEST_CASE("color_lerp at t=0 returns 'from', at t=1 returns 'to'",
          "[mock][view_animations_extra][color_lerp]") {
    const color from{ 0.1, 0.2, 0.3, 0.4 };
    const color to  { 0.9, 0.8, 0.7, 0.6 };

    const color at0 = color_lerp(from, to, 0.0);
    CHECK(at0.r == Approx(from.r));
    CHECK(at0.g == Approx(from.g));
    CHECK(at0.b == Approx(from.b));
    CHECK(at0.a == Approx(from.a));

    const color at1 = color_lerp(from, to, 1.0);
    CHECK(at1.r == Approx(to.r));
    CHECK(at1.g == Approx(to.g));
    CHECK(at1.b == Approx(to.b));
    CHECK(at1.a == Approx(to.a));
}

// ---------------------------------------------------------------------------
// color_to - Observable<color>
// ---------------------------------------------------------------------------

TEST_CASE("color_to seeds Observable<color> at construction",
          "[mock][view_animations_extra][color_to]") {
    // Arrange
    Observable<color> obs{ color{ 1.0, 0.0, 0.0, 1.0 } };

    // Act
    auto anim = color_to(obs, color{ 0.0, 1.0, 0.0, 1.0 }, 100ms,
                         easing_kind::linear);

    // Assert - seeded at from
    CHECK(obs.get().r == Approx(1.0));
    CHECK(obs.get().g == Approx(0.0));
    CHECK(obs.get().b == Approx(0.0));
    CHECK(obs.get().a == Approx(1.0));
}

TEST_CASE("color_to drives Observable<color> from start to end",
          "[mock][view_animations_extra][color_to]") {
    // Arrange
    const color from_c{ 0.0, 0.0, 0.0, 0.0 };
    const color to_c  { 1.0, 1.0, 1.0, 1.0 };
    Observable<color> obs{ from_c };
    auto anim = color_to(obs, to_c, 100ms, easing_kind::linear);

    // Act + Assert - start
    CHECK(obs.get().r == Approx(0.0));

    // midpoint
    anim->advance(50ms);
    CHECK(obs.get().r == Approx(0.5));
    CHECK(obs.get().g == Approx(0.5));
    CHECK(obs.get().b == Approx(0.5));
    CHECK(obs.get().a == Approx(0.5));

    // end
    anim->advance(50ms);
    CHECK(obs.get().r == Approx(1.0));
    CHECK(obs.get().g == Approx(1.0));
    CHECK(obs.get().b == Approx(1.0));
    CHECK(obs.get().a == Approx(1.0));
    CHECK(anim->finished());
}

TEST_CASE("color_to respects easing curve (quad_out) at midpoint",
          "[mock][view_animations_extra][color_to]") {
    // Arrange - quad_out(0.5) = 0.75
    const color from_c{ 0.0, 0.0, 0.0, 1.0 };
    const color to_c  { 1.0, 1.0, 1.0, 1.0 };
    Observable<color> obs{ from_c };
    auto anim = color_to(obs, to_c, 100ms, easing_kind::quad_out);

    // Act
    anim->advance(50ms);  // progress = 0.5 → quad_out(0.5) = 0.75

    // Assert
    CHECK(obs.get().r == Approx(0.75));
    CHECK(obs.get().g == Approx(0.75));
    CHECK(obs.get().b == Approx(0.75));
}

TEST_CASE("color_to: zero-duration jumps to target immediately",
          "[mock][view_animations_extra][color_to]") {
    // Arrange
    Observable<color> obs{ color{ 0.0, 0.0, 0.0, 0.0 } };
    const color target{ 0.5, 0.25, 0.1, 1.0 };
    auto anim = color_to(obs, target, 0ms, easing_kind::linear);

    // Act
    anim->advance(0ms);

    // Assert
    CHECK(obs.get().r == Approx(target.r));
    CHECK(obs.get().g == Approx(target.g));
    CHECK(obs.get().b == Approx(target.b));
    CHECK(obs.get().a == Approx(target.a));
    CHECK(anim->finished());
}

// ---------------------------------------------------------------------------
// animate_to<T> - generic overload with caller-supplied lerp
// ---------------------------------------------------------------------------

TEST_CASE("animate_to<double> with lambda lerp drives value correctly",
          "[mock][view_animations_extra][animate_to]") {
    // Arrange
    Observable<double> obs{ 10.0 };
    auto lerp = [](const double& f, const double& t, double prog) {
        return f + (t - f) * prog;
    };
    auto anim = animate_to(obs, 30.0, 100ms, lerp, easing_kind::linear);

    // Act + Assert - start (seeded from 'from')
    CHECK(obs.get() == Approx(10.0));

    anim->advance(50ms);   // progress=0.5, lerp(10,30,0.5)=20
    CHECK(obs.get() == Approx(20.0));

    anim->advance(50ms);   // progress=1.0
    CHECK(obs.get() == Approx(30.0));
    CHECK(anim->finished());
}

TEST_CASE("animate_to<color> with color_lerp matches color_to behaviour",
          "[mock][view_animations_extra][animate_to]") {
    // Arrange - use mpapp::color_lerp as the callable
    const color from_c{ 0.0, 0.0, 0.0, 0.0 };
    const color to_c  { 1.0, 1.0, 1.0, 1.0 };
    Observable<color> obs{ from_c };
    auto anim = animate_to<color>(obs, to_c, 100ms,
        [](const color& f, const color& t, double prog) {
            return color_lerp(f, t, prog);
        },
        easing_kind::linear);

    // Act
    anim->advance(50ms);

    // Assert
    CHECK(obs.get().r == Approx(0.5));
    CHECK(obs.get().g == Approx(0.5));
    CHECK(obs.get().b == Approx(0.5));
    CHECK(obs.get().a == Approx(0.5));
}

TEST_CASE("animate_to: seek() scrubs generic animation without finishing",
          "[mock][view_animations_extra][animate_to]") {
    // Arrange
    Observable<double> obs{ 0.0 };
    auto anim = animate_to(obs, 100.0, 200ms,
        [](const double& f, const double& t, double prog) {
            return f + (t - f) * prog;
        },
        easing_kind::linear);

    // Act
    anim->seek(0.75);

    // Assert
    CHECK(obs.get() == Approx(75.0));
    CHECK_FALSE(anim->finished());
}
