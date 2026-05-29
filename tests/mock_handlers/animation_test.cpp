// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for the RFC-0010 animation engine.

#include <chrono>
#include <memory>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <mpapp/animation/animation.hpp>
#include <mpapp/animation/animation_manager.hpp>
#include <mpapp/animation/easing.hpp>
#include <mpapp/animation/view_animations.hpp>
#include <mpapp/view.hpp>

using namespace mpapp;
using namespace std::chrono_literals;
using Catch::Approx;

namespace {
class test_view : public view {
public:
    test_view() = default;
};
} // namespace

TEST_CASE("easing curves hit their known anchor points", "[mock][animation][easing]") {
    CHECK(ease(easing_kind::linear, 0.0)   == Approx(0.0));
    CHECK(ease(easing_kind::linear, 0.5)   == Approx(0.5));
    CHECK(ease(easing_kind::linear, 1.0)   == Approx(1.0));
    CHECK(ease(easing_kind::cubic_in, 0.5) == Approx(0.125));
    CHECK(ease(easing_kind::quad_out, 0.5) == Approx(0.75));
    CHECK(ease(easing_kind::sin_out, 1.0)  == Approx(1.0));
    CHECK(ease(easing_kind::sin_in, 0.0)   == Approx(0.0));
    CHECK(ease(easing_kind::bounce_out, 1.0) == Approx(1.0).margin(1e-9));
    // spring_out settles to ~1 at the end + ~0 at the start (the
    // overshoot polynomial leaves sub-epsilon FP residue at the anchors).
    CHECK(ease(easing_kind::spring_out, 0.0) == Approx(0.0).margin(1e-9));
    CHECK(ease(easing_kind::spring_out, 1.0) == Approx(1.0).margin(1e-9));
    // out-of-range t is clamped.
    CHECK(ease(easing_kind::linear, 2.0)   == Approx(1.0));
    CHECK(ease(easing_kind::linear, -1.0)  == Approx(0.0));
}

TEST_CASE("animation interpolates from->to and finishes once",
          "[mock][animation]") {
    double val  = -1.0;
    bool   done = false;

    animation a{ [&val](double v) { val = v; }, 0.0, 100.0, 100ms,
                 easing_kind::linear, [&done]() { done = true; } };

    CHECK(val == Approx(0.0));         // seeded at construction

    CHECK_FALSE(a.advance(50ms));      // halfway
    CHECK(val == Approx(50.0));
    CHECK_FALSE(done);

    CHECK(a.advance(50ms));            // reaches the end
    CHECK(val == Approx(100.0));
    CHECK(done);
    CHECK(a.finished());

    // Further advances are no-ops once finished.
    CHECK(a.advance(50ms));
    CHECK(val == Approx(100.0));
}

TEST_CASE("animation honours its easing curve mid-flight",
          "[mock][animation]") {
    double val = -1.0;
    animation a{ [&val](double v) { val = v; }, 0.0, 1.0, 100ms,
                 easing_kind::cubic_in };
    a.advance(50ms);                   // progress 0.5 -> cubic_in -> 0.125
    CHECK(val == Approx(0.125));
}

TEST_CASE("animation_manager ticks active animations + drops finished",
          "[mock][animation][manager]") {
    animation_manager m;
    auto a = std::make_shared<animation>(
        [](double) {}, 0.0, 1.0, 100ms, easing_kind::linear);
    auto b = std::make_shared<animation>(
        [](double) {}, 0.0, 1.0, 200ms, easing_kind::linear);

    m.start(a);
    m.start(b);
    CHECK(m.active_count() == 2);

    m.tick(100ms);                     // a finishes, b half-done
    CHECK(m.active_count() == 1);
    CHECK(a->finished());
    CHECK_FALSE(b->finished());

    m.tick(100ms);                     // b finishes
    CHECK(m.active_count() == 0);
    CHECK(b->finished());
}

TEST_CASE("fade_to animates the view's opacity Observable",
          "[mock][animation][view]") {
    test_view v;                       // opacity defaults to 1.0
    auto a = fade_to(v, 0.0, 100ms, easing_kind::linear);

    CHECK(v.opacity.get() == Approx(1.0));   // seeded, no change yet
    a->advance(50ms);
    CHECK(v.opacity.get() == Approx(0.5));
    a->advance(50ms);
    CHECK(v.opacity.get() == Approx(0.0));
}

TEST_CASE("translate_to animates both translation axes together",
          "[mock][animation][view]") {
    test_view v;
    auto a = translate_to(v, 10.0, 20.0, 100ms, easing_kind::linear);

    a->advance(50ms);                  // progress 0.5
    CHECK(v.translation_x.get() == Approx(5.0));
    CHECK(v.translation_y.get() == Approx(10.0));
    a->advance(50ms);
    CHECK(v.translation_x.get() == Approx(10.0));
    CHECK(v.translation_y.get() == Approx(20.0));
}

TEST_CASE("scale_to + rotate_to drive their Observables",
          "[mock][animation][view]") {
    test_view v;                       // scale 1.0, rotation 0.0
    auto s = scale_to(v, 2.0, 100ms, easing_kind::linear);
    auto r = rotate_to(v, 90.0, 100ms, easing_kind::linear);
    s->advance(100ms);
    r->advance(100ms);
    CHECK(v.scale.get()    == Approx(2.0));
    CHECK(v.rotation.get() == Approx(90.0));
}
