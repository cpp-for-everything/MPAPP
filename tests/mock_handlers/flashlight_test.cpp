// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock / unit tests for mpapp::flashlight (RFC-0013).

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/flashlight.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// Default construction
// ---------------------------------------------------------------------------

TEST_CASE("mock_flashlight default state", "[mock][flashlight]") {
    // Arrange
    mock_flashlight fl;

    // Act / Assert — initial state
    CHECK(fl.is_supported() == true);
    CHECK(fl.is_on()        == false);
    CHECK(fl.not_supported_attempts() == 0);
}

// ---------------------------------------------------------------------------
// Supported device: turn_on / turn_off
// ---------------------------------------------------------------------------

TEST_CASE("mock_flashlight turn_on sets is_on to true", "[mock][flashlight]") {
    // Arrange
    mock_flashlight fl;
    REQUIRE_FALSE(fl.is_on());

    // Act
    fl.turn_on();

    // Assert
    CHECK(fl.is_on());
    CHECK(fl.not_supported_attempts() == 0);
}

TEST_CASE("mock_flashlight turn_off sets is_on to false", "[mock][flashlight]") {
    // Arrange
    mock_flashlight fl;
    fl.turn_on();
    REQUIRE(fl.is_on());

    // Act
    fl.turn_off();

    // Assert
    CHECK_FALSE(fl.is_on());
}

TEST_CASE("mock_flashlight turn_on is idempotent", "[mock][flashlight]") {
    // Arrange
    mock_flashlight fl;

    // Act — call turn_on() twice
    fl.turn_on();
    fl.turn_on();

    // Assert — still on, no side-effects
    CHECK(fl.is_on());
    CHECK(fl.not_supported_attempts() == 0);
}

TEST_CASE("mock_flashlight turn_off is idempotent", "[mock][flashlight]") {
    // Arrange
    mock_flashlight fl;

    // Act — turn off when already off
    fl.turn_off();
    fl.turn_off();

    // Assert
    CHECK_FALSE(fl.is_on());
}

TEST_CASE("mock_flashlight toggle sequence", "[mock][flashlight]") {
    // Arrange
    mock_flashlight fl;

    // Act / Assert — interleaved on/off
    fl.turn_on();
    CHECK(fl.is_on());

    fl.turn_off();
    CHECK_FALSE(fl.is_on());

    fl.turn_on();
    CHECK(fl.is_on());
}

// ---------------------------------------------------------------------------
// Unsupported device
// ---------------------------------------------------------------------------

TEST_CASE("mock_flashlight with supported=false turn_on is a no-op", "[mock][flashlight]") {
    // Arrange
    mock_flashlight fl{ false };
    REQUIRE_FALSE(fl.is_supported());

    // Act
    fl.turn_on();

    // Assert — light stays off, attempt is recorded
    CHECK_FALSE(fl.is_on());
    CHECK(fl.not_supported_attempts() == 1);
}

TEST_CASE("mock_flashlight not_supported_attempts accumulates", "[mock][flashlight]") {
    // Arrange
    mock_flashlight fl{ false };

    // Act
    fl.turn_on();
    fl.turn_on();
    fl.turn_on();

    // Assert
    CHECK(fl.not_supported_attempts() == 3);
    CHECK_FALSE(fl.is_on());
}

TEST_CASE("mock_flashlight turn_off on unsupported device is still a no-op safe call",
          "[mock][flashlight]") {
    // Arrange — unsupported device; turn_off should not crash or throw
    mock_flashlight fl{ false };
    REQUIRE_FALSE(fl.is_supported());

    // Act — turn_off is always safe
    fl.turn_off();

    // Assert
    CHECK_FALSE(fl.is_on());
    CHECK(fl.not_supported_attempts() == 0); // turn_off does NOT count
}

// ---------------------------------------------------------------------------
// set_supported transitions
// ---------------------------------------------------------------------------

TEST_CASE("mock_flashlight set_supported true restores normal operation",
          "[mock][flashlight]") {
    // Arrange — start unsupported, attempt to turn on
    mock_flashlight fl{ false };
    fl.turn_on();
    REQUIRE(fl.not_supported_attempts() == 1);
    REQUIRE_FALSE(fl.is_on());

    // Act — re-enable support
    fl.set_supported(true);
    fl.turn_on();

    // Assert
    CHECK(fl.is_on());
    CHECK(fl.is_supported());
    // Previously accumulated attempts are still visible
    CHECK(fl.not_supported_attempts() == 1);
}

TEST_CASE("mock_flashlight set_supported false disables a currently-on light",
          "[mock][flashlight]") {
    // Arrange — turn on first
    mock_flashlight fl;
    fl.turn_on();
    REQUIRE(fl.is_on());

    // Act — disable support (does not automatically turn off the light,
    // future turn_on() calls become no-ops)
    fl.set_supported(false);

    // Assert — light state unchanged by set_supported itself
    CHECK_FALSE(fl.is_supported());
    // turn_on() now blocked
    fl.turn_on();
    CHECK(fl.not_supported_attempts() == 1);
}

TEST_CASE("mock_flashlight explicit supported=false constructor", "[mock][flashlight]") {
    // Arrange / Act
    mock_flashlight fl{ false };

    // Assert
    CHECK_FALSE(fl.is_supported());
    CHECK_FALSE(fl.is_on());
    CHECK(fl.not_supported_attempts() == 0);
}

// ---------------------------------------------------------------------------
// Interface pointer usage (polymorphic)
// ---------------------------------------------------------------------------

TEST_CASE("flashlight abstract interface can be used via base pointer",
          "[mock][flashlight]") {
    // Arrange
    mock_flashlight impl;
    flashlight* fl = &impl;

    // Act
    fl->turn_on();

    // Assert
    CHECK(fl->is_on());
    CHECK(fl->is_supported());

    fl->turn_off();
    CHECK_FALSE(fl->is_on());
}
