// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for the RFC-0010 repeat / loop / auto-reverse
// animation wrapper (`mpapp::repeating_animation`).

#include <chrono>
#include <vector>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <mpapp/animation/animation_repeat.hpp>

using namespace mpapp;
using namespace std::chrono_literals;
using Catch::Approx;

TEST_CASE("repeating_animation seeds the first frame at construction",
          "[mock][animation][repeat]") {
    // Arrange / Act
    double last = -1.0;
    repeating_animation r{ [&last](double v) { last = v; }, 100ms };

    // Assert
    CHECK(last == Approx(0.0));
    CHECK(r.current() == Approx(0.0));
    CHECK(r.completed_cycles() == 0);
    CHECK_FALSE(r.is_finished());
}

TEST_CASE("two repeats with no reverse tick 0..1 twice then finish",
          "[mock][animation][repeat]") {
    // Arrange
    std::vector<double> ticks;
    bool                done = false;
    repeating_animation r{ [&ticks](double v) { ticks.push_back(v); }, 100ms,
                           /*repeat_count=*/2, /*auto_reverse=*/false,
                           [&done]() { done = true; } };
    ticks.clear();  // drop the construction seed

    // Act - cycle 1
    CHECK_FALSE(r.advance(50ms));  // progress 0.5
    CHECK(r.current() == Approx(0.5));
    CHECK(r.completed_cycles() == 0);

    CHECK_FALSE(r.advance(50ms));  // cycle 1 ends, rolls into cycle 2 at 0.0
    CHECK(r.completed_cycles() == 1);
    CHECK(r.current() == Approx(0.0));
    CHECK_FALSE(done);

    // Act - cycle 2
    CHECK_FALSE(r.advance(50ms));  // progress 0.5 of cycle 2
    CHECK(r.current() == Approx(0.5));

    CHECK(r.advance(50ms));        // cycle 2 ends -> finished
    CHECK(r.completed_cycles() == 2);
    CHECK(r.current() == Approx(1.0));

    // Assert - saw forward progress only, ending at 1.0, fired once.
    CHECK(done);
    CHECK(r.is_finished());
    CHECK(ticks.front() == Approx(0.5));
    CHECK(ticks.back() == Approx(1.0));
    for (double v : ticks) {
        CHECK(v >= 0.0);
        CHECK(v <= 1.0);
    }

    // Further advances are no-ops once finished.
    CHECK(r.advance(50ms));
    CHECK(r.completed_cycles() == 2);
}

TEST_CASE("auto_reverse plays the second cycle backwards 1..0",
          "[mock][animation][repeat]") {
    // Arrange
    double              last = -1.0;
    repeating_animation r{ [&last](double v) { last = v; }, 100ms,
                           /*repeat_count=*/2, /*auto_reverse=*/true };

    // Act - cycle 0 is forward 0..1.
    r.advance(50ms);
    CHECK(last == Approx(0.5));
    r.advance(50ms);              // ends cycle 0 (emits 1.0), seeds cycle 1 at 1.0
    CHECK(r.completed_cycles() == 1);
    CHECK(last == Approx(1.0));   // reversed cycle 1 starts at 1.0

    // Act - cycle 1 is reversed 1..0.
    r.advance(50ms);
    CHECK(last == Approx(0.5));   // 1.0 - 0.5
    bool finished = r.advance(50ms);

    // Assert - reversed cycle finishes at 0.0.
    CHECK(finished);
    CHECK(last == Approx(0.0));
    CHECK(r.is_finished());
    CHECK(r.completed_cycles() == 2);
}

TEST_CASE("infinite repeat never finishes and keeps counting cycles",
          "[mock][animation][repeat]") {
    // Arrange
    int                 cycles_seen = 0;
    repeating_animation r{ [](double) {}, 100ms, /*repeat_count=*/-1 };

    // Act - drive well past any finite count.
    for (int i = 0; i < 10; ++i) {
        const bool finished = r.advance(100ms);
        CHECK_FALSE(finished);
        cycles_seen = r.completed_cycles();
    }

    // Assert
    CHECK_FALSE(r.is_finished());
    CHECK(cycles_seen == 10);
}

TEST_CASE("multiple cycles complete inside one large advance",
          "[mock][animation][repeat]") {
    // Arrange - 3 cycles of 100ms each.
    repeating_animation r{ [](double) {}, 100ms, /*repeat_count=*/3 };

    // Act - a single 350ms step crosses all three cycle boundaries.
    const bool finished = r.advance(350ms);

    // Assert
    CHECK(finished);
    CHECK(r.is_finished());
    CHECK(r.completed_cycles() == 3);
    CHECK(r.current() == Approx(1.0));
}

TEST_CASE("zero duration collapses each cycle to a single end-frame",
          "[mock][animation][repeat]") {
    // Arrange
    double              last = -1.0;
    repeating_animation r{ [&last](double v) { last = v; }, 0ms,
                           /*repeat_count=*/2 };

    // Act - any non-zero advance completes all collapsed cycles at once.
    const bool finished = r.advance(1ms);

    // Assert
    CHECK(finished);
    CHECK(r.completed_cycles() == 2);
    CHECK(last == Approx(1.0));
}

TEST_CASE("cancel stops the animation without firing on_finished",
          "[mock][animation][repeat]") {
    // Arrange
    bool                done = false;
    repeating_animation r{ [](double) {}, 100ms, /*repeat_count=*/3,
                           /*auto_reverse=*/false, [&done]() { done = true; } };

    // Act
    r.advance(50ms);
    CHECK_FALSE(r.is_finished());
    r.cancel();

    // Assert - finished but on_finished never ran; advances are no-ops.
    CHECK(r.is_finished());
    CHECK_FALSE(done);
    CHECK(r.advance(100ms));
    CHECK(r.completed_cycles() == 0);
}
