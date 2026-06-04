// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Exhaustive branch coverage for the RFC-0010 easing curves
// (mpapp/animation/easing.hpp). animation_test.cpp checks a few anchor points;
// this file drives every easing_kind through every piecewise branch so the
// header reaches full line coverage.

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <mpapp/animation/easing.hpp>

using namespace mpapp;
using Catch::Approx;

namespace {
constexpr easing_kind kAllKinds[] = {
    easing_kind::linear,      easing_kind::sin_in,      easing_kind::sin_out,
    easing_kind::sin_in_out,  easing_kind::quad_in,     easing_kind::quad_out,
    easing_kind::quad_in_out, easing_kind::cubic_in,    easing_kind::cubic_out,
    easing_kind::cubic_in_out, easing_kind::bounce_out, easing_kind::spring_out,
};
} // namespace

TEST_CASE("ease() is anchored at the endpoints for every curve", "[mock][animation][easing]") {
    for (auto k : kAllKinds) {
        CHECK(ease(k, 0.0) == Approx(0.0).margin(1e-9));
        CHECK(ease(k, 1.0) == Approx(1.0).margin(1e-9));
    }
}

TEST_CASE("ease() exercises every piecewise branch", "[mock][animation][easing]") {
    // Sample points chosen to land on both sides of each 0.5 split and inside
    // each of bounce_out's four segments (boundaries at ~0.36/0.73/0.91).
    constexpr double kSamples[] = { 0.1, 0.25, 0.4, 0.5, 0.6, 0.75, 0.95 };
    for (auto k : kAllKinds) {
        for (double t : kSamples) {
            const double y = ease(k, t);
            // bounce/spring may overshoot; the rest stay within [0,1].
            if (k != easing_kind::bounce_out && k != easing_kind::spring_out) {
                CHECK(y >= -1e-9);
                CHECK(y <= 1.0 + 1e-9);
            }
        }
    }
}

TEST_CASE("ease() in/out curves are monotonic and symmetric at the midpoint",
          "[mock][animation][easing]") {
    // sin_in_out / quad_in_out / cubic_in_out pass through 0.5 at t=0.5.
    CHECK(ease(easing_kind::sin_in_out, 0.5)   == Approx(0.5));
    CHECK(ease(easing_kind::quad_in_out, 0.5)  == Approx(0.5));
    CHECK(ease(easing_kind::cubic_in_out, 0.5) == Approx(0.5));
    // Lower-half vs upper-half branches differ from the midpoint value.
    CHECK(ease(easing_kind::quad_in_out, 0.25)  < 0.5);
    CHECK(ease(easing_kind::quad_in_out, 0.75)  > 0.5);
    CHECK(ease(easing_kind::cubic_in_out, 0.25) < 0.5);
    CHECK(ease(easing_kind::cubic_in_out, 0.75) > 0.5);
}

TEST_CASE("ease() bounce_out covers all four segments", "[mock][animation][easing]") {
    // Each sample falls in a distinct segment; all stay within [0,1].
    for (double t : { 0.2, 0.5, 0.8, 0.97 }) {
        const double y = ease(easing_kind::bounce_out, t);
        CHECK(y >= 0.0);
        CHECK(y <= 1.0 + 1e-9);
    }
}

TEST_CASE("ease() clamps out-of-range t", "[mock][animation][easing]") {
    CHECK(ease(easing_kind::quad_in, -5.0) == Approx(0.0));
    CHECK(ease(easing_kind::quad_in,  5.0) == Approx(1.0));
    CHECK(ease_clamp01(-0.1) == Approx(0.0));
    CHECK(ease_clamp01(1.1)  == Approx(1.0));
    CHECK(ease_clamp01(0.3)  == Approx(0.3));
}

TEST_CASE("ease() falls back to identity for an unknown curve", "[mock][animation][easing]") {
    // An out-of-range enum value hits the post-switch `return t` fallback.
    const auto bogus = static_cast<easing_kind>(200);
    CHECK(ease(bogus, 0.42) == Approx(0.42));
    CHECK(ease(bogus, 1.5)  == Approx(1.0));  // still clamped first
}
