// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for the RFC-0013 HapticFeedback Essentials API.

#include <optional>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/haptic_feedback.hpp>

using namespace mpapp;

// ---- to_string --------------------------------------------------------------

TEST_CASE("to_string returns correct label for every haptic_feedback_type",
          "[mock][essentials][haptic_feedback][to_string]") {
    CHECK(to_string(haptic_feedback_type::click)      == "click");
    CHECK(to_string(haptic_feedback_type::long_press) == "long_press");
}

TEST_CASE("to_string returns 'unknown' for out-of-range value",
          "[mock][essentials][haptic_feedback][to_string]") {
    // Cast an out-of-range value to test the default branch.
    const auto unknown_type = static_cast<haptic_feedback_type>(255u);
    CHECK(to_string(unknown_type) == "unknown");
}

// ---- Default (supported) mock -----------------------------------------------

TEST_CASE("mock_haptic_feedback defaults to supported with no recorded calls",
          "[mock][essentials][haptic_feedback]") {
    mock_haptic_feedback hf;

    // Arrange/Assert — initial state before any perform().
    CHECK(hf.is_supported() == true);
    CHECK(hf.perform_count() == 0);
    CHECK_FALSE(hf.last_type().has_value());
}

TEST_CASE("perform(click) records last_type and increments perform_count",
          "[mock][essentials][haptic_feedback]") {
    // Arrange
    mock_haptic_feedback hf;

    // Act
    hf.perform(haptic_feedback_type::click);

    // Assert
    REQUIRE(hf.last_type().has_value());
    CHECK(*hf.last_type() == haptic_feedback_type::click);
    CHECK(hf.perform_count() == 1);
}

TEST_CASE("perform(long_press) records last_type as long_press",
          "[mock][essentials][haptic_feedback]") {
    // Arrange
    mock_haptic_feedback hf;

    // Act
    hf.perform(haptic_feedback_type::long_press);

    // Assert
    REQUIRE(hf.last_type().has_value());
    CHECK(*hf.last_type() == haptic_feedback_type::long_press);
    CHECK(hf.perform_count() == 1);
}

TEST_CASE("multiple perform() calls accumulate perform_count and track last_type",
          "[mock][essentials][haptic_feedback]") {
    // Arrange
    mock_haptic_feedback hf;

    // Act
    hf.perform(haptic_feedback_type::click);
    hf.perform(haptic_feedback_type::click);
    hf.perform(haptic_feedback_type::long_press);

    // Assert
    CHECK(hf.perform_count() == 3);
    REQUIRE(hf.last_type().has_value());
    CHECK(*hf.last_type() == haptic_feedback_type::long_press);
}

// ---- Not-supported path -----------------------------------------------------

TEST_CASE("mock_haptic_feedback constructed not-supported has correct initial state",
          "[mock][essentials][haptic_feedback][not_supported]") {
    // Arrange + Assert
    mock_haptic_feedback hf{ false };

    CHECK(hf.is_supported() == false);
    CHECK(hf.perform_count() == 0);
    CHECK_FALSE(hf.last_type().has_value());
}

TEST_CASE("perform() on not-supported mock is a no-op",
          "[mock][essentials][haptic_feedback][not_supported]") {
    // Arrange
    mock_haptic_feedback hf{ false };

    // Act
    hf.perform(haptic_feedback_type::click);
    hf.perform(haptic_feedback_type::long_press);

    // Assert — state must not have changed.
    CHECK(hf.perform_count() == 0);
    CHECK_FALSE(hf.last_type().has_value());
}

TEST_CASE("set_supported(false) after calls stops further recording",
          "[mock][essentials][haptic_feedback][not_supported]") {
    // Arrange
    mock_haptic_feedback hf;
    hf.perform(haptic_feedback_type::click);     // count == 1, last == click
    REQUIRE(hf.perform_count() == 1);

    // Act — disable support then try another call.
    hf.set_supported(false);
    hf.perform(haptic_feedback_type::long_press);

    // Assert — count and last_type must remain from before the disable.
    CHECK(hf.perform_count() == 1);
    REQUIRE(hf.last_type().has_value());
    CHECK(*hf.last_type() == haptic_feedback_type::click);
}

TEST_CASE("set_supported(true) re-enables recording after disabled period",
          "[mock][essentials][haptic_feedback][not_supported]") {
    // Arrange
    mock_haptic_feedback hf{ false };
    hf.perform(haptic_feedback_type::click);   // ignored
    CHECK(hf.perform_count() == 0);

    // Act — re-enable then perform.
    hf.set_supported(true);
    hf.perform(haptic_feedback_type::long_press);

    // Assert
    CHECK(hf.is_supported() == true);
    CHECK(hf.perform_count() == 1);
    REQUIRE(hf.last_type().has_value());
    CHECK(*hf.last_type() == haptic_feedback_type::long_press);
}

// ---- reset() ----------------------------------------------------------------

TEST_CASE("reset() clears last_type and perform_count without changing supported flag",
          "[mock][essentials][haptic_feedback][reset]") {
    // Arrange
    mock_haptic_feedback hf;
    hf.perform(haptic_feedback_type::click);
    hf.perform(haptic_feedback_type::long_press);
    REQUIRE(hf.perform_count() == 2);

    // Act
    hf.reset();

    // Assert
    CHECK(hf.perform_count() == 0);
    CHECK_FALSE(hf.last_type().has_value());
    CHECK(hf.is_supported() == true);  // supported flag untouched
}

TEST_CASE("reset() on not-supported mock preserves not-supported flag",
          "[mock][essentials][haptic_feedback][reset]") {
    // Arrange
    mock_haptic_feedback hf{ false };

    // Act
    hf.reset();

    // Assert
    CHECK(hf.is_supported() == false);
    CHECK(hf.perform_count() == 0);
    CHECK_FALSE(hf.last_type().has_value());
}

// ---- Interface polymorphism -------------------------------------------------

TEST_CASE("haptic_feedback abstract interface is callable via base pointer",
          "[mock][essentials][haptic_feedback][interface]") {
    // Arrange
    mock_haptic_feedback concrete;
    haptic_feedback* hf = &concrete;

    // Act
    hf->perform(haptic_feedback_type::click);

    // Assert via the concrete object.
    CHECK(concrete.perform_count() == 1);
    REQUIRE(concrete.last_type().has_value());
    CHECK(*concrete.last_type() == haptic_feedback_type::click);
    CHECK(hf->is_supported() == true);
}
