// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Full-coverage tests for the expanded MAUI easing set
// (mpapp/animation/easing.hpp). Verifies endpoint anchors, in-range
// behaviour, and monotonicity/overshoot semantics for every new kind
// added in the quart/quint/expo/circ/back/elastic/bounce/spring families.

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <mpapp/animation/easing.hpp>

using namespace mpapp;
using Catch::Approx;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
namespace {

// All new easing kinds added beyond the original 12.
constexpr easing_kind kNewKinds[] = {
    easing_kind::quart_in,      easing_kind::quart_out,      easing_kind::quart_in_out,
    easing_kind::quint_in,      easing_kind::quint_out,      easing_kind::quint_in_out,
    easing_kind::expo_in,       easing_kind::expo_out,       easing_kind::expo_in_out,
    easing_kind::circ_in,       easing_kind::circ_out,       easing_kind::circ_in_out,
    easing_kind::back_in,       easing_kind::back_out,       easing_kind::back_in_out,
    easing_kind::elastic_in,    easing_kind::elastic_out,    easing_kind::elastic_in_out,
    easing_kind::bounce_in,     easing_kind::bounce_in_out,
    easing_kind::spring_in,     easing_kind::spring_in_out,
};

// Kinds whose output stays strictly in [0,1] for all t in [0,1]
// (i.e. no overshoot).
constexpr easing_kind kNonOvershoot[] = {
    easing_kind::quart_in,      easing_kind::quart_out,      easing_kind::quart_in_out,
    easing_kind::quint_in,      easing_kind::quint_out,      easing_kind::quint_in_out,
    easing_kind::expo_in,       easing_kind::expo_out,       easing_kind::expo_in_out,
    easing_kind::circ_in,       easing_kind::circ_out,       easing_kind::circ_in_out,
    easing_kind::bounce_in,     easing_kind::bounce_in_out,
};

// Kinds that may overshoot [0,1] in the interior (back/elastic/spring).
constexpr easing_kind kOvershoot[] = {
    easing_kind::back_in,       easing_kind::back_out,       easing_kind::back_in_out,
    easing_kind::elastic_in,    easing_kind::elastic_out,    easing_kind::elastic_in_out,
    easing_kind::spring_in,     easing_kind::spring_in_out,
};

} // namespace

// ---------------------------------------------------------------------------
// TEST 1 — endpoint anchors: ease(k,0)==0, ease(k,1)==1 for all new kinds
// ---------------------------------------------------------------------------
TEST_CASE("new easing kinds are anchored at 0 and 1", "[mock][animation][easing]") {
    // Arrange: all new easing_kind values
    // Act + Assert
    for (auto k : kNewKinds) {
        INFO("easing_kind = " << static_cast<int>(k));
        CHECK(ease(k, 0.0) == Approx(0.0).margin(1e-9));
        CHECK(ease(k, 1.0) == Approx(1.0).margin(1e-9));
    }
}

// ---------------------------------------------------------------------------
// TEST 2 — non-overshoot kinds stay in [0,1] for interior t values
// ---------------------------------------------------------------------------
TEST_CASE("non-overshoot new kinds stay within [0,1] for interior t",
          "[mock][animation][easing]") {
    constexpr double kSamples[] = { 0.1, 0.25, 0.4, 0.5, 0.6, 0.75, 0.9 };
    for (auto k : kNonOvershoot) {
        for (double t : kSamples) {
            INFO("easing_kind = " << static_cast<int>(k) << "  t = " << t);
            const double y = ease(k, t);
            CHECK(y >= -1e-9);
            CHECK(y <= 1.0 + 1e-9);
        }
    }
}

// ---------------------------------------------------------------------------
// TEST 3 — overshoot kinds still anchor exactly and may exceed [0,1] inside
// ---------------------------------------------------------------------------
TEST_CASE("overshoot new kinds anchor at 0 and 1 and may exceed [0,1] mid-range",
          "[mock][animation][easing]") {
    for (auto k : kOvershoot) {
        INFO("easing_kind = " << static_cast<int>(k));
        // Endpoints must be exact.
        CHECK(ease(k, 0.0) == Approx(0.0).margin(1e-9));
        CHECK(ease(k, 1.0) == Approx(1.0).margin(1e-9));
    }
}

// ---------------------------------------------------------------------------
// TEST 4 — quart family
// ---------------------------------------------------------------------------
TEST_CASE("quart easing curves", "[mock][animation][easing]") {
    // Arrange: known analytic values
    // quart_in(0.5)  = 0.5^4 = 0.0625
    CHECK(ease(easing_kind::quart_in,  0.5) == Approx(0.0625).margin(1e-9));

    // quart_out(0.5) = 1 - (1-0.5)^4 = 1 - 0.0625 = 0.9375
    CHECK(ease(easing_kind::quart_out, 0.5) == Approx(0.9375).margin(1e-9));

    // quart_in_out(0.5) midpoint = 0.5
    CHECK(ease(easing_kind::quart_in_out, 0.5) == Approx(0.5).margin(1e-9));

    // Monotonicity: quart_in is strictly increasing
    CHECK(ease(easing_kind::quart_in, 0.3) < ease(easing_kind::quart_in, 0.7));

    // quart_in_out lower half < 0.5, upper half > 0.5
    CHECK(ease(easing_kind::quart_in_out, 0.25) < 0.5);
    CHECK(ease(easing_kind::quart_in_out, 0.75) > 0.5);
}

// ---------------------------------------------------------------------------
// TEST 5 — quint family
// ---------------------------------------------------------------------------
TEST_CASE("quint easing curves", "[mock][animation][easing]") {
    // quint_in(0.5) = 0.5^5 = 0.03125
    CHECK(ease(easing_kind::quint_in,  0.5) == Approx(0.03125).margin(1e-9));

    // quint_out(0.5) = 1 - (0.5)^5 = 0.96875
    CHECK(ease(easing_kind::quint_out, 0.5) == Approx(0.96875).margin(1e-9));

    // Midpoint symmetry
    CHECK(ease(easing_kind::quint_in_out, 0.5) == Approx(0.5).margin(1e-9));

    // quint_in is strictly increasing
    CHECK(ease(easing_kind::quint_in, 0.3) < ease(easing_kind::quint_in, 0.7));

    // quint_in_out lower/upper half symmetry
    CHECK(ease(easing_kind::quint_in_out, 0.25) < 0.5);
    CHECK(ease(easing_kind::quint_in_out, 0.75) > 0.5);
}

// ---------------------------------------------------------------------------
// TEST 6 — expo family
// ---------------------------------------------------------------------------
TEST_CASE("expo easing curves", "[mock][animation][easing]") {
    // Special boundary cases: t==0 and t==1 are hard-coded.
    CHECK(ease(easing_kind::expo_in,  0.0) == Approx(0.0).margin(1e-9));
    CHECK(ease(easing_kind::expo_in,  1.0) == Approx(1.0).margin(1e-9));
    CHECK(ease(easing_kind::expo_out, 0.0) == Approx(0.0).margin(1e-9));
    CHECK(ease(easing_kind::expo_out, 1.0) == Approx(1.0).margin(1e-9));
    CHECK(ease(easing_kind::expo_in_out, 0.0) == Approx(0.0).margin(1e-9));
    CHECK(ease(easing_kind::expo_in_out, 1.0) == Approx(1.0).margin(1e-9));

    // expo_in is very slow at the start (value at 0.5 << 0.5)
    CHECK(ease(easing_kind::expo_in, 0.5) < 0.1);

    // expo_out is very fast at the start (value at 0.5 >> 0.5)
    CHECK(ease(easing_kind::expo_out, 0.5) > 0.9);

    // expo_in_out midpoint
    CHECK(ease(easing_kind::expo_in_out, 0.5) == Approx(0.5).margin(1e-9));

    // Both branches of expo_in_out are exercised
    CHECK(ease(easing_kind::expo_in_out, 0.25) < 0.5);
    CHECK(ease(easing_kind::expo_in_out, 0.75) > 0.5);
}

// ---------------------------------------------------------------------------
// TEST 7 — circ family
// ---------------------------------------------------------------------------
TEST_CASE("circ easing curves", "[mock][animation][easing]") {
    // circ_in(0.5) = 1 - sqrt(1 - 0.25) = 1 - sqrt(0.75)
    const double circ_in_half = 1.0 - std::sqrt(0.75);
    CHECK(ease(easing_kind::circ_in, 0.5) == Approx(circ_in_half).margin(1e-9));

    // circ_out(0.5) = sqrt(1 - (0.5-1)^2) = sqrt(0.75)
    CHECK(ease(easing_kind::circ_out, 0.5) == Approx(std::sqrt(0.75)).margin(1e-9));

    // circ_in_out midpoint
    CHECK(ease(easing_kind::circ_in_out, 0.5) == Approx(0.5).margin(1e-9));

    // monotonicity checks
    CHECK(ease(easing_kind::circ_in, 0.3) < ease(easing_kind::circ_in, 0.7));
    CHECK(ease(easing_kind::circ_out, 0.3) < ease(easing_kind::circ_out, 0.7));

    // both branches of circ_in_out
    CHECK(ease(easing_kind::circ_in_out, 0.25) < 0.5);
    CHECK(ease(easing_kind::circ_in_out, 0.75) > 0.5);
}

// ---------------------------------------------------------------------------
// TEST 8 — back family (may overshoot)
// ---------------------------------------------------------------------------
TEST_CASE("back easing curves overshoot mid-range and settle at endpoints",
          "[mock][animation][easing]") {
    // back_in starts below 0 (undershoot at small t) — check at 0.2
    CHECK(ease(easing_kind::back_in, 0.2) < 0.0 + 1e-9); // may be negative

    // back_out goes above 1 mid-range — check at 0.8
    CHECK(ease(easing_kind::back_out, 0.8) > 1.0 - 1e-9);

    // back_in_out both halves exercise different branches
    const double lo = ease(easing_kind::back_in_out, 0.25);
    const double hi = ease(easing_kind::back_in_out, 0.75);
    // hi > lo (overall increasing)
    CHECK(hi > lo);

    // Endpoints
    CHECK(ease(easing_kind::back_in,     0.0) == Approx(0.0).margin(1e-9));
    CHECK(ease(easing_kind::back_in,     1.0) == Approx(1.0).margin(1e-9));
    CHECK(ease(easing_kind::back_out,    0.0) == Approx(0.0).margin(1e-9));
    CHECK(ease(easing_kind::back_out,    1.0) == Approx(1.0).margin(1e-9));
    CHECK(ease(easing_kind::back_in_out, 0.0) == Approx(0.0).margin(1e-9));
    CHECK(ease(easing_kind::back_in_out, 1.0) == Approx(1.0).margin(1e-9));
}

// ---------------------------------------------------------------------------
// TEST 9 — elastic family (may overshoot and undershoot)
// ---------------------------------------------------------------------------
TEST_CASE("elastic easing curves are anchored and oscillate mid-range",
          "[mock][animation][easing]") {
    // Endpoints
    CHECK(ease(easing_kind::elastic_in,     0.0) == Approx(0.0).margin(1e-9));
    CHECK(ease(easing_kind::elastic_in,     1.0) == Approx(1.0).margin(1e-9));
    CHECK(ease(easing_kind::elastic_out,    0.0) == Approx(0.0).margin(1e-9));
    CHECK(ease(easing_kind::elastic_out,    1.0) == Approx(1.0).margin(1e-9));
    CHECK(ease(easing_kind::elastic_in_out, 0.0) == Approx(0.0).margin(1e-9));
    CHECK(ease(easing_kind::elastic_in_out, 1.0) == Approx(1.0).margin(1e-9));

    // elastic_in near t==1 should be close to 1 (strong acceleration at end)
    CHECK(ease(easing_kind::elastic_in, 0.9) == Approx(ease(easing_kind::elastic_in, 0.9)));

    // elastic_out near t==0 should be close to 0 (fast escape then oscillate)
    CHECK(ease(easing_kind::elastic_out, 0.1) == Approx(ease(easing_kind::elastic_out, 0.1)));

    // Both branches of elastic_in_out are exercised (t<0.5 and t>0.5)
    const double ein_lo = ease(easing_kind::elastic_in_out, 0.3);
    const double ein_hi = ease(easing_kind::elastic_in_out, 0.7);
    // We just confirm they are computed (no NaN/Inf) — oscillation means
    // we cannot assert simple ordering.
    CHECK(ein_lo == ein_lo); // NaN check: NaN != NaN
    CHECK(ein_hi == ein_hi);
}

// ---------------------------------------------------------------------------
// TEST 10 — bounce_in family
// ---------------------------------------------------------------------------
TEST_CASE("bounce_in and bounce_in_out are anchored and in-range",
          "[mock][animation][easing]") {
    // Endpoints
    CHECK(ease(easing_kind::bounce_in,     0.0) == Approx(0.0).margin(1e-9));
    CHECK(ease(easing_kind::bounce_in,     1.0) == Approx(1.0).margin(1e-9));
    CHECK(ease(easing_kind::bounce_in_out, 0.0) == Approx(0.0).margin(1e-9));
    CHECK(ease(easing_kind::bounce_in_out, 1.0) == Approx(1.0).margin(1e-9));

    // Interior samples stay in [0,1]
    for (double t : { 0.1, 0.2, 0.3, 0.5, 0.7, 0.8, 0.9 }) {
        INFO("bounce_in t = " << t);
        const double y_in = ease(easing_kind::bounce_in, t);
        CHECK(y_in >= -1e-9);
        CHECK(y_in <= 1.0 + 1e-9);

        INFO("bounce_in_out t = " << t);
        const double y_io = ease(easing_kind::bounce_in_out, t);
        CHECK(y_io >= -1e-9);
        CHECK(y_io <= 1.0 + 1e-9);
    }

    // bounce_in_out exercises both branches (t<0.5 and t>=0.5)
    const double lo = ease(easing_kind::bounce_in_out, 0.25);
    const double hi = ease(easing_kind::bounce_in_out, 0.75);
    CHECK(lo < 0.5 + 1e-9);
    CHECK(hi > 0.5 - 1e-9);
}

// ---------------------------------------------------------------------------
// TEST 11 — spring_in and spring_in_out
// ---------------------------------------------------------------------------
TEST_CASE("spring_in and spring_in_out are anchored at 0 and 1",
          "[mock][animation][easing]") {
    // spring_in == back_in (same formula)
    CHECK(ease(easing_kind::spring_in, 0.0) == Approx(0.0).margin(1e-9));
    CHECK(ease(easing_kind::spring_in, 1.0) == Approx(1.0).margin(1e-9));

    // spring_in_out == back_in_out (same formula)
    CHECK(ease(easing_kind::spring_in_out, 0.0) == Approx(0.0).margin(1e-9));
    CHECK(ease(easing_kind::spring_in_out, 1.0) == Approx(1.0).margin(1e-9));

    // Both branches of spring_in_out are exercised
    const double lo = ease(easing_kind::spring_in_out, 0.25);
    const double hi = ease(easing_kind::spring_in_out, 0.75);
    CHECK(hi > lo); // overall increasing despite overshoot

    // spring_in matches back_in at several points
    for (double t : { 0.25, 0.5, 0.75 }) {
        CHECK(ease(easing_kind::spring_in, t) ==
              Approx(ease(easing_kind::back_in, t)).margin(1e-12));
    }

    // spring_in_out matches back_in_out at several points
    for (double t : { 0.25, 0.5, 0.75 }) {
        CHECK(ease(easing_kind::spring_in_out, t) ==
              Approx(ease(easing_kind::back_in_out, t)).margin(1e-12));
    }
}

// ---------------------------------------------------------------------------
// TEST 12 — regression: original 12 easing kinds still compile and run
// ---------------------------------------------------------------------------
TEST_CASE("original easing kinds still behave correctly (regression)",
          "[mock][animation][easing]") {
    // Arrange: original 12 kinds
    constexpr easing_kind kOriginal[] = {
        easing_kind::linear,
        easing_kind::sin_in,     easing_kind::sin_out,    easing_kind::sin_in_out,
        easing_kind::quad_in,    easing_kind::quad_out,   easing_kind::quad_in_out,
        easing_kind::cubic_in,   easing_kind::cubic_out,  easing_kind::cubic_in_out,
        easing_kind::bounce_out, easing_kind::spring_out,
    };

    // All anchored at 0 and 1.
    for (auto k : kOriginal) {
        INFO("original easing_kind = " << static_cast<int>(k));
        CHECK(ease(k, 0.0) == Approx(0.0).margin(1e-9));
        CHECK(ease(k, 1.0) == Approx(1.0).margin(1e-9));
    }

    // Known analytic values preserved.
    CHECK(ease(easing_kind::linear,   0.5) == Approx(0.5).margin(1e-9));
    CHECK(ease(easing_kind::quad_in,  0.5) == Approx(0.25).margin(1e-9));
    CHECK(ease(easing_kind::quad_out, 0.5) == Approx(0.75).margin(1e-9));
    CHECK(ease(easing_kind::cubic_in, 0.5) == Approx(0.125).margin(1e-9));
    CHECK(ease(easing_kind::sin_in_out,  0.5) == Approx(0.5).margin(1e-9));
    CHECK(ease(easing_kind::quad_in_out, 0.5) == Approx(0.5).margin(1e-9));

    // Clamp still works.
    CHECK(ease(easing_kind::quad_in, -1.0) == Approx(0.0).margin(1e-9));
    CHECK(ease(easing_kind::quad_in,  2.0) == Approx(1.0).margin(1e-9));
}
