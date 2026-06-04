// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Catch2 tests for the RFC-0013 Vibration Essentials mock.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/vibration.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// Default-constructed state
// ---------------------------------------------------------------------------

TEST_CASE("mock_vibration starts with zero counts and no cancellation",
          "[mock][essentials][vibration]") {
    // Arrange
    mock_vibration v;

    // Act / Assert (nothing called yet)
    CHECK(v.vibrate_count() == 0);
    CHECK(v.last_duration_ms() == 0.0);
    CHECK_FALSE(v.was_canceled());
}

// ---------------------------------------------------------------------------
// vibrate() - default duration overload
// ---------------------------------------------------------------------------

TEST_CASE("vibrate() with no argument uses vibration_default_ms",
          "[mock][essentials][vibration]") {
    // Arrange
    mock_vibration v;

    // Act
    v.vibrate();

    // Assert
    CHECK(v.vibrate_count() == 1);
    CHECK(v.last_duration_ms() == vibration_default_ms);
    CHECK_FALSE(v.was_canceled());
}

// ---------------------------------------------------------------------------
// vibrate(double) - explicit duration overload
// ---------------------------------------------------------------------------

TEST_CASE("vibrate(ms) records the supplied duration",
          "[mock][essentials][vibration]") {
    // Arrange
    mock_vibration v;

    // Act
    v.vibrate(250.0);

    // Assert
    CHECK(v.vibrate_count() == 1);
    CHECK(v.last_duration_ms() == 250.0);
    CHECK_FALSE(v.was_canceled());
}

TEST_CASE("vibrate(ms) called multiple times increments the counter",
          "[mock][essentials][vibration]") {
    // Arrange
    mock_vibration v;

    // Act
    v.vibrate(100.0);
    v.vibrate(200.0);
    v.vibrate(300.0);

    // Assert
    CHECK(v.vibrate_count() == 3);
    CHECK(v.last_duration_ms() == 300.0);  // most recent wins
}

TEST_CASE("vibrate() and vibrate(ms) both increment the same counter",
          "[mock][essentials][vibration]") {
    // Arrange
    mock_vibration v;

    // Act
    v.vibrate();        // default overload
    v.vibrate(100.0);   // explicit overload

    // Assert
    CHECK(v.vibrate_count() == 2);
    CHECK(v.last_duration_ms() == 100.0);
}

TEST_CASE("vibrate(ms) accepts zero duration",
          "[mock][essentials][vibration]") {
    // Arrange
    mock_vibration v;

    // Act
    v.vibrate(0.0);

    // Assert
    CHECK(v.vibrate_count() == 1);
    CHECK(v.last_duration_ms() == 0.0);
}

TEST_CASE("vibrate(ms) accepts a very large duration",
          "[mock][essentials][vibration]") {
    // Arrange
    mock_vibration v;

    // Act
    v.vibrate(60000.0);  // 60 seconds

    // Assert
    CHECK(v.vibrate_count() == 1);
    CHECK(v.last_duration_ms() == 60000.0);
}

// ---------------------------------------------------------------------------
// cancel()
// ---------------------------------------------------------------------------

TEST_CASE("cancel() sets was_canceled to true",
          "[mock][essentials][vibration]") {
    // Arrange
    mock_vibration v;

    // Act
    v.cancel();

    // Assert
    CHECK(v.was_canceled());
}

TEST_CASE("vibrate() after cancel() clears the canceled flag",
          "[mock][essentials][vibration]") {
    // Arrange
    mock_vibration v;
    v.cancel();
    REQUIRE(v.was_canceled());

    // Act
    v.vibrate();

    // Assert
    CHECK_FALSE(v.was_canceled());
    CHECK(v.vibrate_count() == 1);
}

TEST_CASE("vibrate(ms) after cancel() clears the canceled flag",
          "[mock][essentials][vibration]") {
    // Arrange
    mock_vibration v;
    v.cancel();
    REQUIRE(v.was_canceled());

    // Act
    v.vibrate(150.0);

    // Assert
    CHECK_FALSE(v.was_canceled());
    CHECK(v.vibrate_count() == 1);
}

TEST_CASE("cancel() does not change vibrate_count or last_duration_ms",
          "[mock][essentials][vibration]") {
    // Arrange
    mock_vibration v;
    v.vibrate(400.0);
    REQUIRE(v.vibrate_count() == 1);

    // Act
    v.cancel();

    // Assert
    CHECK(v.vibrate_count() == 1);           // unchanged
    CHECK(v.last_duration_ms() == 400.0);    // unchanged
    CHECK(v.was_canceled());
}

TEST_CASE("cancel() called multiple times remains canceled",
          "[mock][essentials][vibration]") {
    // Arrange
    mock_vibration v;

    // Act
    v.cancel();
    v.cancel();

    // Assert
    CHECK(v.was_canceled());
}

// ---------------------------------------------------------------------------
// reset()
// ---------------------------------------------------------------------------

TEST_CASE("reset() clears all recorded state",
          "[mock][essentials][vibration]") {
    // Arrange
    mock_vibration v;
    v.vibrate(300.0);
    v.cancel();
    REQUIRE(v.vibrate_count() == 1);

    // Act
    v.reset();

    // Assert
    CHECK(v.vibrate_count() == 0);
    CHECK(v.last_duration_ms() == 0.0);
    CHECK_FALSE(v.was_canceled());
}

// ---------------------------------------------------------------------------
// Interface polymorphism - using base-class pointer
// ---------------------------------------------------------------------------

TEST_CASE("mock_vibration is usable through the base vibration pointer",
          "[mock][essentials][vibration]") {
    // Arrange
    mock_vibration impl;
    vibration* iface = &impl;

    // Act
    iface->vibrate();
    iface->vibrate(750.0);
    iface->cancel();

    // Assert via concrete type
    CHECK(impl.vibrate_count() == 2);
    CHECK(impl.last_duration_ms() == 750.0);
    CHECK(impl.was_canceled());
}

// ---------------------------------------------------------------------------
// vibration_default_ms constant
// ---------------------------------------------------------------------------

TEST_CASE("vibration_default_ms is a positive value",
          "[mock][essentials][vibration]") {
    CHECK(vibration_default_ms > 0.0);
}
