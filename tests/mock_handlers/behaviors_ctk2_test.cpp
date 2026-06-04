// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for additional CTK-style neutral behaviors
// (max_length_behavior, regex_validation_behavior, numeric_validation_behavior).

#include <regex>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/behaviors/behaviors_ctk2.hpp>
#include <mpapp/signal.hpp>

// ==========================================================================
// max_length_behavior
// ==========================================================================

TEST_CASE("max_length_behavior: default-constructed has no limit",
          "[mock][behaviors_ctk2]") {
    // Arrange + Act
    mpapp::max_length_behavior b;

    // Assert
    CHECK(b.max_length()      == 0);
    CHECK(b.is_within_limit() == true);
}

TEST_CASE("max_length_behavior: max_length getter returns configured value",
          "[mock][behaviors_ctk2]") {
    mpapp::max_length_behavior b{ 10 };
    CHECK(b.max_length() == 10);
}

TEST_CASE("max_length_behavior: string within limit is accepted",
          "[mock][behaviors_ctk2]") {
    // Arrange
    mpapp::max_length_behavior b{ 5 };

    // Act
    b.validate("hi");

    // Assert
    CHECK(b.is_within_limit() == true);
}

TEST_CASE("max_length_behavior: string exactly at limit is accepted",
          "[mock][behaviors_ctk2]") {
    // Arrange
    mpapp::max_length_behavior b{ 5 };

    // Act
    b.validate("hello");   // exactly 5 bytes

    // Assert
    CHECK(b.is_within_limit() == true);
}

TEST_CASE("max_length_behavior: string over limit is rejected",
          "[mock][behaviors_ctk2]") {
    // Arrange
    mpapp::max_length_behavior b{ 3 };

    // Act
    b.validate("abcd");    // 4 bytes > 3

    // Assert
    CHECK(b.is_within_limit() == false);
}

TEST_CASE("max_length_behavior: value_truncated fires with truncated string",
          "[mock][behaviors_ctk2]") {
    // Arrange
    mpapp::max_length_behavior b{ 4 };
    std::string fired_value;
    int         hit_count = 0;
    mpapp::signal_slot<std::string> slot;
    auto cb = [&fired_value, &hit_count](const std::string& v) {
        fired_value = v;
        ++hit_count;
    };
    b.value_truncated.subscribe(slot, cb);

    // Act
    b.validate("abcdefgh");  // 8 bytes, limit 4

    // Assert
    CHECK(hit_count       == 1);
    CHECK(fired_value     == "abcd");
    CHECK(b.is_within_limit() == false);
}

TEST_CASE("max_length_behavior: value_truncated does not fire when within limit",
          "[mock][behaviors_ctk2]") {
    // Arrange
    mpapp::max_length_behavior b{ 10 };
    int hit_count = 0;
    mpapp::signal_slot<std::string> slot;
    auto cb = [&hit_count](const std::string&) { ++hit_count; };
    b.value_truncated.subscribe(slot, cb);

    // Act
    b.validate("short");

    // Assert
    CHECK(hit_count == 0);
}

TEST_CASE("max_length_behavior: value_truncated fires every over-limit call",
          "[mock][behaviors_ctk2]") {
    // Arrange
    mpapp::max_length_behavior b{ 2 };
    int hit_count = 0;
    mpapp::signal_slot<std::string> slot;
    auto cb = [&hit_count](const std::string&) { ++hit_count; };
    b.value_truncated.subscribe(slot, cb);

    // Act
    b.validate("abcde");  // over limit
    b.validate("xyz");    // over limit again

    // Assert: fires on every call that exceeds the limit
    CHECK(hit_count == 2);
}

TEST_CASE("max_length_behavior: max_length=0 means no cap",
          "[mock][behaviors_ctk2]") {
    // Arrange
    mpapp::max_length_behavior b{ 0 };
    int hit_count = 0;
    mpapp::signal_slot<std::string> slot;
    auto cb = [&hit_count](const std::string&) { ++hit_count; };
    b.value_truncated.subscribe(slot, cb);

    // Act
    std::string very_long(1000, 'z');
    b.validate(very_long);

    // Assert
    CHECK(b.is_within_limit() == true);
    CHECK(hit_count            == 0);
}

TEST_CASE("max_length_behavior: is_within_limit transitions from false back to true",
          "[mock][behaviors_ctk2]") {
    // Arrange
    mpapp::max_length_behavior b{ 3 };

    b.validate("toolong");   // over limit
    CHECK(b.is_within_limit() == false);

    b.validate("ok");        // within limit
    CHECK(b.is_within_limit() == true);
}

// ==========================================================================
// regex_validation_behavior
// ==========================================================================

TEST_CASE("regex_validation_behavior: default-constructed is invalid",
          "[mock][behaviors_ctk2]") {
    mpapp::regex_validation_behavior b{ std::regex{ ".*" } };
    CHECK_FALSE(b.is_valid());
}

TEST_CASE("regex_validation_behavior: matching string becomes valid",
          "[mock][behaviors_ctk2]") {
    // Arrange
    mpapp::regex_validation_behavior b{ std::regex{ "[0-9]+" } };

    // Act
    b.validate("123");

    // Assert
    CHECK(b.is_valid() == true);
}

TEST_CASE("regex_validation_behavior: non-matching string stays invalid",
          "[mock][behaviors_ctk2]") {
    // Arrange
    mpapp::regex_validation_behavior b{ std::regex{ "[0-9]+" } };

    // Act
    b.validate("abc");

    // Assert
    CHECK(b.is_valid() == false);
}

TEST_CASE("regex_validation_behavior: validity_changed fires on flip to valid",
          "[mock][behaviors_ctk2]") {
    // Arrange
    mpapp::regex_validation_behavior b{ std::regex{ "[a-z]+" } };
    int  hits      = 0;
    bool last_val  = false;
    mpapp::signal_slot<bool> slot;
    auto cb = [&hits, &last_val](bool v) { ++hits; last_val = v; };
    b.validity_changed.subscribe(slot, cb);

    // Act
    b.validate("hello");   // false -> true: flip

    // Assert
    CHECK(hits     == 1);
    CHECK(last_val == true);
}

TEST_CASE("regex_validation_behavior: validity_changed fires on flip to invalid",
          "[mock][behaviors_ctk2]") {
    // Arrange
    mpapp::regex_validation_behavior b{ std::regex{ "[a-z]+" } };
    int  hits = 0;
    mpapp::signal_slot<bool> slot;
    auto cb = [&hits](bool) { ++hits; };
    b.validity_changed.subscribe(slot, cb);

    b.validate("hello");   // -> valid (flip 1)
    b.validate("123");     // -> invalid (flip 2)

    // Assert
    CHECK(hits == 2);
    CHECK(b.is_valid() == false);
}

TEST_CASE("regex_validation_behavior: validity_changed does not fire on non-flips",
          "[mock][behaviors_ctk2]") {
    // Arrange
    mpapp::regex_validation_behavior b{ std::regex{ "[a-z]+" } };
    int hits = 0;
    mpapp::signal_slot<bool> slot;
    auto cb = [&hits](bool) { ++hits; };
    b.validity_changed.subscribe(slot, cb);

    // Act
    b.validate("abc");   // false -> true (flip)
    b.validate("xyz");   // true  -> true (no flip)
    b.validate("def");   // true  -> true (no flip)

    // Assert
    CHECK(hits == 1);
}

TEST_CASE("regex_validation_behavior: full mode rejects partial matches",
          "[mock][behaviors_ctk2]") {
    // Arrange: pattern matches only a digit sequence — full match
    mpapp::regex_validation_behavior b{ std::regex{ "[0-9]+" },
                                        mpapp::regex_match_mode::full };

    // Act
    b.validate("abc123");   // not a full match

    // Assert
    CHECK(b.is_valid() == false);
}

TEST_CASE("regex_validation_behavior: partial mode accepts sub-sequence matches",
          "[mock][behaviors_ctk2]") {
    // Arrange
    mpapp::regex_validation_behavior b{ std::regex{ "[0-9]+" },
                                        mpapp::regex_match_mode::partial };

    // Act: "abc123" contains digits, so search succeeds
    b.validate("abc123");

    // Assert
    CHECK(b.is_valid() == true);
}

TEST_CASE("regex_validation_behavior: mode getter returns configured mode",
          "[mock][behaviors_ctk2]") {
    mpapp::regex_validation_behavior full_b{ std::regex{ "." },
                                             mpapp::regex_match_mode::full };
    mpapp::regex_validation_behavior part_b{ std::regex{ "." },
                                             mpapp::regex_match_mode::partial };
    CHECK(full_b.mode() == mpapp::regex_match_mode::full);
    CHECK(part_b.mode() == mpapp::regex_match_mode::partial);
}

TEST_CASE("regex_match_mode to_string coverage",
          "[mock][behaviors_ctk2]") {
    CHECK(mpapp::to_string(mpapp::regex_match_mode::full)    == "full");
    CHECK(mpapp::to_string(mpapp::regex_match_mode::partial) == "partial");
}

TEST_CASE("regex_validation_behavior: empty string against non-empty pattern",
          "[mock][behaviors_ctk2]") {
    // Arrange: pattern requires at least one char
    mpapp::regex_validation_behavior b{ std::regex{ ".+" } };

    // Act
    b.validate("");

    // Assert
    CHECK(b.is_valid() == false);
}

TEST_CASE("regex_validation_behavior: wildcard pattern matches any string",
          "[mock][behaviors_ctk2]") {
    // Arrange: .* matches everything including empty
    mpapp::regex_validation_behavior b{ std::regex{ ".*" } };

    // Act
    b.validate("");

    // Assert
    CHECK(b.is_valid() == true);
}

// ==========================================================================
// numeric_validation_behavior
// ==========================================================================

TEST_CASE("numeric_validation_behavior: default-constructed is invalid",
          "[mock][behaviors_ctk2]") {
    mpapp::numeric_validation_behavior b;
    CHECK_FALSE(b.is_valid());
}

TEST_CASE("numeric_validation_behavior: getter accessors return configured values",
          "[mock][behaviors_ctk2]") {
    mpapp::numeric_validation_behavior b{ -10.0, 10.0 };
    CHECK(b.min_value() == -10.0);
    CHECK(b.max_value() ==  10.0);
}

TEST_CASE("numeric_validation_behavior: valid integer within range",
          "[mock][behaviors_ctk2]") {
    // Arrange
    mpapp::numeric_validation_behavior b{ 0.0, 100.0 };

    // Act
    b.validate("42");

    // Assert
    CHECK(b.is_valid() == true);
}

TEST_CASE("numeric_validation_behavior: valid float within range",
          "[mock][behaviors_ctk2]") {
    // Arrange
    mpapp::numeric_validation_behavior b{ 0.0, 1.0 };

    // Act
    b.validate("0.5");

    // Assert
    CHECK(b.is_valid() == true);
}

TEST_CASE("numeric_validation_behavior: value at minimum boundary is valid",
          "[mock][behaviors_ctk2]") {
    // Arrange
    mpapp::numeric_validation_behavior b{ 5.0, 10.0 };

    // Act
    b.validate("5.0");

    // Assert
    CHECK(b.is_valid() == true);
}

TEST_CASE("numeric_validation_behavior: value at maximum boundary is valid",
          "[mock][behaviors_ctk2]") {
    // Arrange
    mpapp::numeric_validation_behavior b{ 5.0, 10.0 };

    // Act
    b.validate("10.0");

    // Assert
    CHECK(b.is_valid() == true);
}

TEST_CASE("numeric_validation_behavior: value below minimum is invalid",
          "[mock][behaviors_ctk2]") {
    // Arrange
    mpapp::numeric_validation_behavior b{ 5.0, 10.0 };

    // Act
    b.validate("4.9");

    // Assert
    CHECK(b.is_valid() == false);
}

TEST_CASE("numeric_validation_behavior: value above maximum is invalid",
          "[mock][behaviors_ctk2]") {
    // Arrange
    mpapp::numeric_validation_behavior b{ 5.0, 10.0 };

    // Act
    b.validate("10.1");

    // Assert
    CHECK(b.is_valid() == false);
}

TEST_CASE("numeric_validation_behavior: non-numeric text is invalid",
          "[mock][behaviors_ctk2]") {
    // Arrange
    mpapp::numeric_validation_behavior b{ 0.0, 100.0 };

    // Act
    b.validate("abc");

    // Assert
    CHECK(b.is_valid() == false);
}

TEST_CASE("numeric_validation_behavior: empty string is invalid",
          "[mock][behaviors_ctk2]") {
    // Arrange
    mpapp::numeric_validation_behavior b{ 0.0, 100.0 };

    // Act
    b.validate("");

    // Assert
    CHECK(b.is_valid() == false);
}

TEST_CASE("numeric_validation_behavior: trailing non-numeric chars make it invalid",
          "[mock][behaviors_ctk2]") {
    // Arrange
    mpapp::numeric_validation_behavior b{ 0.0, 100.0 };

    // Act
    b.validate("42abc");

    // Assert
    CHECK(b.is_valid() == false);
}

TEST_CASE("numeric_validation_behavior: no range constraint when both bounds are zero",
          "[mock][behaviors_ctk2]") {
    // Arrange: both min and max are 0.0 -> no range check
    mpapp::numeric_validation_behavior b{ 0.0, 0.0 };

    // Act
    b.validate("999999");

    // Assert
    CHECK(b.is_valid() == true);
}

TEST_CASE("numeric_validation_behavior: no range constraint accepts negative values",
          "[mock][behaviors_ctk2]") {
    // Arrange
    mpapp::numeric_validation_behavior b{ 0.0, 0.0 };

    // Act
    b.validate("-1234.5");

    // Assert
    CHECK(b.is_valid() == true);
}

TEST_CASE("numeric_validation_behavior: validity_changed fires on flip to valid",
          "[mock][behaviors_ctk2]") {
    // Arrange
    mpapp::numeric_validation_behavior b{ 0.0, 100.0 };
    int  hits     = 0;
    bool last_val = false;
    mpapp::signal_slot<bool> slot;
    auto cb = [&hits, &last_val](bool v) { ++hits; last_val = v; };
    b.validity_changed.subscribe(slot, cb);

    // Act
    b.validate("50");   // false -> true: flip

    // Assert
    CHECK(hits     == 1);
    CHECK(last_val == true);
}

TEST_CASE("numeric_validation_behavior: validity_changed fires on flip to invalid",
          "[mock][behaviors_ctk2]") {
    // Arrange
    mpapp::numeric_validation_behavior b{ 0.0, 100.0 };
    int  hits = 0;
    mpapp::signal_slot<bool> slot;
    auto cb = [&hits](bool) { ++hits; };
    b.validity_changed.subscribe(slot, cb);

    b.validate("50");    // -> valid (flip 1)
    b.validate("200");   // -> invalid (flip 2)

    // Assert
    CHECK(hits         == 2);
    CHECK(b.is_valid() == false);
}

TEST_CASE("numeric_validation_behavior: validity_changed does not fire on non-flips",
          "[mock][behaviors_ctk2]") {
    // Arrange
    mpapp::numeric_validation_behavior b{ 0.0, 100.0 };
    int hits = 0;
    mpapp::signal_slot<bool> slot;
    auto cb = [&hits](bool) { ++hits; };
    b.validity_changed.subscribe(slot, cb);

    // Act
    b.validate("10");   // false -> true (flip)
    b.validate("20");   // true  -> true (no flip)
    b.validate("30");   // true  -> true (no flip)

    // Assert
    CHECK(hits == 1);
}

TEST_CASE("numeric_validation_behavior: negative range accepted",
          "[mock][behaviors_ctk2]") {
    // Arrange
    mpapp::numeric_validation_behavior b{ -50.0, -1.0 };

    // Act
    b.validate("-25");

    // Assert
    CHECK(b.is_valid() == true);
}

TEST_CASE("numeric_validation_behavior: value outside negative range is invalid",
          "[mock][behaviors_ctk2]") {
    // Arrange
    mpapp::numeric_validation_behavior b{ -50.0, -1.0 };

    // Act
    b.validate("0");   // 0 > -1.0 -> out of range

    // Assert
    CHECK(b.is_valid() == false);
}
