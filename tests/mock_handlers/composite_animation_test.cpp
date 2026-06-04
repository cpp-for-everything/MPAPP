// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for the RFC-0010 composite (child-timeline)
// animation — MAUI's Animation.Add(beginAt, finishAt, childAnimation).

#include <vector>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <mpapp/animation/composite_animation.hpp>
#include <mpapp/animation/easing.hpp>

using namespace mpapp;
using Catch::Approx;

TEST_CASE("two staggered children get correct local progress",
          "[mock][animation][composite]") {
    // Arrange: child A owns [0, 0.5], child B owns [0.5, 1].
    double a = -1.0;
    double b = -1.0;
    composite_animation timeline;
    timeline.add(0.0, 0.5, [&a](double v) { a = v; }, easing_kind::linear);
    timeline.add(0.5, 1.0, [&b](double v) { b = v; }, easing_kind::linear);
    CHECK(timeline.child_count() == 2);

    // Act + Assert: global 0.25 -> A halfway, B not started.
    timeline.seek(0.25);
    CHECK(a == Approx(0.5));
    CHECK(b == Approx(0.0));

    // global 0.5 -> A finished (at finish_at), B just starting.
    timeline.seek(0.5);
    CHECK(a == Approx(1.0));
    CHECK(b == Approx(0.0));

    // global 0.75 -> A finished, B halfway.
    timeline.seek(0.75);
    CHECK(a == Approx(1.0));
    CHECK(b == Approx(0.5));
}

TEST_CASE("seek clamps global progress before begin / after finish",
          "[mock][animation][composite]") {
    // Arrange: a single child over the middle of the timeline.
    double v = -1.0;
    composite_animation timeline;
    timeline.add(0.25, 0.75, [&v](double x) { v = x; });

    // Act + Assert: anything at/below begin_at -> local 0.
    timeline.seek(0.0);
    CHECK(v == Approx(0.0));
    timeline.seek(0.25);
    CHECK(v == Approx(0.0));

    // Out-of-range global is clamped to [0,1]; below 0 stays 0.
    timeline.seek(-5.0);
    CHECK(v == Approx(0.0));

    // Midpoint of the range -> local 0.5.
    timeline.seek(0.5);
    CHECK(v == Approx(0.5));

    // At/above finish_at -> local 1.
    timeline.seek(0.75);
    CHECK(v == Approx(1.0));
    timeline.seek(1.0);
    CHECK(v == Approx(1.0));

    // Out-of-range global above 1 is clamped to 1 -> still local 1.
    timeline.seek(5.0);
    CHECK(v == Approx(1.0));
}

TEST_CASE("composite applies a child's easing curve to local progress",
          "[mock][animation][composite]") {
    // Arrange: a child over the whole timeline with cubic_in easing.
    double v = -1.0;
    composite_animation timeline;
    timeline.add(0.0, 1.0, [&v](double x) { v = x; }, easing_kind::cubic_in);

    // Act: global 0.5 -> local 0.5 -> cubic_in(0.5) == 0.125.
    timeline.seek(0.5);

    // Assert.
    CHECK(v == Approx(0.125));
}

TEST_CASE("degenerate range snaps to 1 from begin_at onward",
          "[mock][animation][composite]") {
    // Arrange: finish_at <= begin_at makes the child a step.
    double v = -1.0;
    composite_animation timeline;
    timeline.add(0.5, 0.5, [&v](double x) { v = x; });

    // Act + Assert: strictly before begin_at -> 0.
    timeline.seek(0.25);
    CHECK(v == Approx(0.0));

    // Exactly at the collapsed point, the begin_at guard (g <= begin_at)
    // wins, so it still reads 0.
    timeline.seek(0.5);
    CHECK(v == Approx(0.0));

    // Strictly after begin_at -> snaps to 1 (g >= finish_at branch).
    timeline.seek(0.9);
    CHECK(v == Approx(1.0));
}

TEST_CASE("begin_at / finish_at are clamped to [0,1]",
          "[mock][animation][composite]") {
    // Arrange: out-of-range bounds get clamped on add().
    double v = -1.0;
    composite_animation timeline;
    timeline.add(-2.0, 3.0, [&v](double x) { v = x; });

    // Act + Assert: clamped to [0,1], so global 0.5 -> local 0.5.
    timeline.seek(0.5);
    CHECK(v == Approx(0.5));
    timeline.seek(0.0);
    CHECK(v == Approx(0.0));
    timeline.seek(1.0);
    CHECK(v == Approx(1.0));
}

TEST_CASE("seek tolerates a child with no tick callback",
          "[mock][animation][composite]") {
    // Arrange: a null callback must be skipped without crashing.
    composite_animation timeline;
    timeline.add(0.0, 1.0, nullptr);

    // Act: should be a no-op, not a null deref.
    timeline.seek(0.5);

    // Assert.
    CHECK(timeline.child_count() == 1);
}

TEST_CASE("clear removes all scheduled children",
          "[mock][animation][composite]") {
    // Arrange.
    int hits = 0;
    composite_animation timeline;
    timeline.add(0.0, 1.0, [&hits](double) { ++hits; });
    CHECK(timeline.child_count() == 1);

    // Act.
    timeline.clear();
    timeline.seek(0.5);

    // Assert: nothing left to tick.
    CHECK(timeline.child_count() == 0);
    CHECK(hits == 0);
}
