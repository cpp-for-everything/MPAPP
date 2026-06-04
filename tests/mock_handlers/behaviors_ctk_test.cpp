// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for CTK-style neutral behaviors
// (event_to_command_behavior, text_validation_behavior).

#include <string>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/behaviors/behaviors_ctk.hpp>
#include <mpapp/binding/relay_command.hpp>
#include <mpapp/signal.hpp>

// Use fully-qualified mpapp:: names throughout to avoid ambiguity between
// mpapp::signal<> and the POSIX signal() function declared in <signal.h>.

// ==========================================================================
// event_to_command_behavior
// ==========================================================================

TEST_CASE("event_to_command_behavior: signal fires -> command executes",
          "[mock][behaviors_ctk]") {
    // Arrange
    int runs = 0;
    mpapp::relay_command cmd{ [&runs]() { ++runs; } };
    mpapp::signal<> sig;
    mpapp::event_to_command_behavior<> b{ sig, cmd };

    // Act
    sig.emit();
    sig.emit();

    // Assert
    CHECK(runs == 2);
}

TEST_CASE("event_to_command_behavior: command blocked by can_execute",
          "[mock][behaviors_ctk]") {
    // Arrange
    int  runs    = 0;
    bool allowed = false;
    mpapp::relay_command cmd{ [&runs]() { ++runs; },
                              [&allowed]() { return allowed; } };
    mpapp::signal<> sig;
    mpapp::event_to_command_behavior<> b{ sig, cmd };

    // Act – gated: no run
    sig.emit();
    CHECK(runs == 0);

    // Act – allow and fire
    allowed = true;
    sig.emit();

    // Assert
    CHECK(runs == 1);
}

TEST_CASE("event_to_command_behavior: mapper returning false suppresses execute",
          "[mock][behaviors_ctk]") {
    // Arrange
    int runs = 0;
    mpapp::relay_command cmd{ [&runs]() { ++runs; } };
    mpapp::signal<int> sig;
    // Only execute when the value is positive.
    mpapp::event_to_command_behavior<int> b{ sig, cmd,
        [](int n) { return n > 0; } };

    // Act – negative: suppressed
    sig.emit(-1);
    CHECK(runs == 0);

    // Act – zero: suppressed
    sig.emit(0);
    CHECK(runs == 0);

    // Act – positive: allowed
    sig.emit(1);
    CHECK(runs == 1);
}

TEST_CASE("event_to_command_behavior: mapper returning true allows execute",
          "[mock][behaviors_ctk]") {
    // Arrange
    int runs = 0;
    mpapp::relay_command cmd{ [&runs]() { ++runs; } };
    mpapp::signal<std::string> sig;
    // Execute only when the string is non-empty.
    mpapp::event_to_command_behavior<std::string> b{ sig, cmd,
        [](const std::string& s) { return !s.empty(); } };

    // Act
    sig.emit(std::string{""});       // empty: suppressed
    CHECK(runs == 0);

    sig.emit(std::string{"hello"});  // non-empty: allowed
    CHECK(runs == 1);
}

TEST_CASE("event_to_command_behavior: slot auto-disconnects on destruction",
          "[mock][behaviors_ctk]") {
    // Arrange
    int runs = 0;
    mpapp::relay_command cmd{ [&runs]() { ++runs; } };
    mpapp::signal<> sig;

    {
        mpapp::event_to_command_behavior<> b{ sig, cmd };
        sig.emit();
        CHECK(runs == 1);
    } // b destroyed here; slot must disconnect

    // Act – signal has no subscribers now
    sig.emit();

    // Assert – run count unchanged
    CHECK(runs == 1);
}

TEST_CASE("event_to_command_behavior: multi-arg signal forwarded through mapper",
          "[mock][behaviors_ctk]") {
    // Arrange
    int runs = 0;
    mpapp::relay_command cmd{ [&runs]() { ++runs; } };
    mpapp::signal<int, int> sig;
    // Execute only when both values are positive.
    mpapp::event_to_command_behavior<int, int> b{ sig, cmd,
        [](int a, int bb) { return a > 0 && bb > 0; } };

    // Act
    sig.emit(1, -1);    // second negative: suppressed
    CHECK(runs == 0);

    sig.emit(2, 3);     // both positive: allowed
    CHECK(runs == 1);
}

TEST_CASE("event_to_command_behavior: can_execute blocked overrides mapper",
          "[mock][behaviors_ctk]") {
    // Arrange: mapper always says true, but can_execute is false.
    int  runs    = 0;
    bool allowed = false;
    mpapp::relay_command cmd{ [&runs]() { ++runs; },
                              [&allowed]() { return allowed; } };
    mpapp::signal<int> sig;
    mpapp::event_to_command_behavior<int> b{ sig, cmd,
        [](int) { return true; } };

    // can_execute gates first
    sig.emit(42);
    CHECK(runs == 0);

    allowed = true;
    sig.emit(42);
    CHECK(runs == 1);
}

// ==========================================================================
// text_validation_behavior
// ==========================================================================

TEST_CASE("text_validation_behavior: default-constructed is invalid (false)",
          "[mock][behaviors_ctk]") {
    mpapp::text_validation_behavior b;
    CHECK_FALSE(b.is_valid());
}

TEST_CASE("text_validation_behavior: empty string is valid when not required",
          "[mock][behaviors_ctk]") {
    // Arrange
    mpapp::text_validation_behavior b{ /*required=*/false };

    // Act
    b.validate("");

    // Assert
    CHECK(b.is_valid());
}

TEST_CASE("text_validation_behavior: required flag rejects empty string",
          "[mock][behaviors_ctk]") {
    // Arrange
    mpapp::text_validation_behavior b{ /*required=*/true };

    // Act – empty is invalid
    b.validate("");
    CHECK_FALSE(b.is_valid());

    // Act – non-empty is valid
    b.validate("x");
    CHECK(b.is_valid());
}

TEST_CASE("text_validation_behavior: min_length enforced",
          "[mock][behaviors_ctk]") {
    // Arrange
    mpapp::text_validation_behavior b{ false, /*min_length=*/3 };

    // too short
    b.validate("ab");
    CHECK_FALSE(b.is_valid());

    // exactly at minimum
    b.validate("abc");
    CHECK(b.is_valid());

    // above minimum
    b.validate("abcd");
    CHECK(b.is_valid());
}

TEST_CASE("text_validation_behavior: max_length enforced",
          "[mock][behaviors_ctk]") {
    // Arrange
    mpapp::text_validation_behavior b{ false, 0, /*max_length=*/5 };

    // within limit
    b.validate("hello");
    CHECK(b.is_valid());

    // over limit
    b.validate("toolong");
    CHECK_FALSE(b.is_valid());
}

TEST_CASE("text_validation_behavior: min and max together",
          "[mock][behaviors_ctk]") {
    // Arrange: 3 <= length <= 5
    mpapp::text_validation_behavior b{ false, 3, 5 };

    b.validate("ab");      // too short
    CHECK_FALSE(b.is_valid());

    b.validate("abc");     // at min
    CHECK(b.is_valid());

    b.validate("abcde");   // at max
    CHECK(b.is_valid());

    b.validate("abcdef");  // over max
    CHECK_FALSE(b.is_valid());
}

TEST_CASE("text_validation_behavior: validity_changed fires on flip",
          "[mock][behaviors_ctk]") {
    // Arrange
    mpapp::text_validation_behavior b{ /*required=*/true };
    int hits = 0;
    bool last_value = false;
    mpapp::signal_slot<bool> slot;
    auto cb = [&hits, &last_value](bool v) { ++hits; last_value = v; };
    b.validity_changed.subscribe(slot, cb);

    // Act – first call: invalid (false) is still the default, no flip yet
    b.validate("");         // false -> false: no flip
    CHECK(hits == 0);

    // Act – non-empty: flips to valid
    b.validate("x");        // false -> true: flip
    CHECK(hits == 1);
    CHECK(last_value == true);

    // Act – same valid string: no flip
    b.validate("y");        // true -> true: no flip
    CHECK(hits == 1);

    // Act – back to empty: flips to invalid
    b.validate("");         // true -> false: flip
    CHECK(hits == 2);
    CHECK(last_value == false);
}

TEST_CASE("text_validation_behavior: validity_changed does not fire on non-flips",
          "[mock][behaviors_ctk]") {
    // Arrange: no constraints, so any string is valid
    mpapp::text_validation_behavior b;
    int hits = 0;
    mpapp::signal_slot<bool> slot;
    auto cb = [&hits](bool) { ++hits; };
    b.validity_changed.subscribe(slot, cb);

    // Start invalid (default false). Force to valid once.
    b.validate("anything");
    CHECK(hits == 1);

    // Repeated calls with valid strings: no more flips.
    b.validate("more");
    b.validate("even more");
    CHECK(hits == 1);
}

TEST_CASE("text_validation_behavior: required + min_length combined",
          "[mock][behaviors_ctk]") {
    // Arrange: required AND at least 2 chars
    mpapp::text_validation_behavior b{ true, 2, 0 };

    b.validate("");     // fails required
    CHECK_FALSE(b.is_valid());

    b.validate("a");    // fails min_length
    CHECK_FALSE(b.is_valid());

    b.validate("ab");   // passes both
    CHECK(b.is_valid());
}

TEST_CASE("text_validation_behavior: getter accessors return configured values",
          "[mock][behaviors_ctk]") {
    mpapp::text_validation_behavior b{ true, 2, 10 };
    CHECK(b.required()    == true);
    CHECK(b.min_length()  == 2);
    CHECK(b.max_length()  == 10);
}

TEST_CASE("text_validation_behavior: max_length=0 means no upper cap",
          "[mock][behaviors_ctk]") {
    // Arrange: no upper cap
    mpapp::text_validation_behavior b{ false, 0, 0 };

    std::string very_long(1000, 'x');
    b.validate(very_long);
    CHECK(b.is_valid());
}
