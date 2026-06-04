// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Catch2 tests for mpapp::phone_dialer (RFC-0013 Essentials).

#include <optional>
#include <stdexcept>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/phone_dialer.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// Default-constructed state
// ---------------------------------------------------------------------------

TEST_CASE("mock_phone_dialer default state: supported=true, no recorded calls",
          "[mock][essentials][phone_dialer]") {
    // Arrange / Act
    mock_phone_dialer d;

    // Assert
    CHECK(d.is_supported());
    CHECK(d.open_count() == 0);
    CHECK_FALSE(d.last_number().has_value());
}

// ---------------------------------------------------------------------------
// open() - happy path
// ---------------------------------------------------------------------------

TEST_CASE("mock_phone_dialer records number passed to open()",
          "[mock][essentials][phone_dialer]") {
    // Arrange
    mock_phone_dialer d;

    // Act
    d.open("+1-800-555-0100");

    // Assert
    REQUIRE(d.last_number().has_value());
    CHECK(*d.last_number() == "+1-800-555-0100");
    CHECK(d.open_count() == 1);
}

TEST_CASE("mock_phone_dialer accepts empty string as number",
          "[mock][essentials][phone_dialer]") {
    // Arrange
    mock_phone_dialer d;

    // Act
    d.open("");

    // Assert
    REQUIRE(d.last_number().has_value());
    CHECK(d.last_number()->empty());
    CHECK(d.open_count() == 1);
}

TEST_CASE("mock_phone_dialer overwrites last_number on successive open() calls",
          "[mock][essentials][phone_dialer]") {
    // Arrange
    mock_phone_dialer d;

    // Act
    d.open("111");
    d.open("222");
    d.open("333");

    // Assert - only the most recent number is retained
    REQUIRE(d.last_number().has_value());
    CHECK(*d.last_number() == "333");
    CHECK(d.open_count() == 3);
}

TEST_CASE("mock_phone_dialer open_count increments on every call",
          "[mock][essentials][phone_dialer]") {
    // Arrange
    mock_phone_dialer d;

    // Act + Assert incrementally
    d.open("1");
    CHECK(d.open_count() == 1);

    d.open("2");
    CHECK(d.open_count() == 2);

    d.open("3");
    CHECK(d.open_count() == 3);
}

// ---------------------------------------------------------------------------
// open() - not-supported path
// ---------------------------------------------------------------------------

TEST_CASE("mock_phone_dialer is_supported() returns false after set_supported(false)",
          "[mock][essentials][phone_dialer]") {
    // Arrange
    mock_phone_dialer d;
    d.set_supported(false);

    // Act / Assert
    CHECK_FALSE(d.is_supported());
}

TEST_CASE("mock_phone_dialer open() throws when not supported",
          "[mock][essentials][phone_dialer]") {
    // Arrange
    mock_phone_dialer d;
    d.set_supported(false);

    // Act + Assert
    CHECK_THROWS_AS(d.open("+44-20-1234-5678"), std::runtime_error);
}

TEST_CASE("mock_phone_dialer open() does not record number when not supported",
          "[mock][essentials][phone_dialer]") {
    // Arrange
    mock_phone_dialer d;
    d.set_supported(false);

    // Act - ignore the thrown exception
    try { d.open("999"); } catch (...) {}

    // Assert - state must remain unmodified
    CHECK_FALSE(d.last_number().has_value());
    CHECK(d.open_count() == 0);
}

TEST_CASE("mock_phone_dialer can be re-enabled after set_supported(false)",
          "[mock][essentials][phone_dialer]") {
    // Arrange
    mock_phone_dialer d;
    d.set_supported(false);
    try { d.open("bad"); } catch (...) {}

    // Act - re-enable
    d.set_supported(true);
    d.open("+1-555-0100");

    // Assert
    CHECK(d.is_supported());
    REQUIRE(d.last_number().has_value());
    CHECK(*d.last_number() == "+1-555-0100");
    CHECK(d.open_count() == 1);   // only the successful call counted
}

// ---------------------------------------------------------------------------
// reset()
// ---------------------------------------------------------------------------

TEST_CASE("mock_phone_dialer reset() clears last_number and open_count",
          "[mock][essentials][phone_dialer]") {
    // Arrange
    mock_phone_dialer d;
    d.open("+1-800-555-9999");
    REQUIRE(d.open_count() == 1);
    REQUIRE(d.last_number().has_value());

    // Act
    d.reset();

    // Assert
    CHECK(d.open_count() == 0);
    CHECK_FALSE(d.last_number().has_value());
}

TEST_CASE("mock_phone_dialer reset() does not alter the supported flag",
          "[mock][essentials][phone_dialer]") {
    // Arrange - supported stays true
    mock_phone_dialer d_true;
    d_true.open("+1");
    d_true.reset();
    CHECK(d_true.is_supported());

    // Arrange - supported stays false
    mock_phone_dialer d_false;
    d_false.set_supported(false);
    d_false.reset();
    CHECK_FALSE(d_false.is_supported());
}

TEST_CASE("mock_phone_dialer reset() on a fresh instance is idempotent",
          "[mock][essentials][phone_dialer]") {
    // Arrange
    mock_phone_dialer d;

    // Act
    d.reset();

    // Assert
    CHECK(d.open_count() == 0);
    CHECK_FALSE(d.last_number().has_value());
    CHECK(d.is_supported());
}

// ---------------------------------------------------------------------------
// Abstract interface polymorphism
// ---------------------------------------------------------------------------

TEST_CASE("mock_phone_dialer is usable through the abstract phone_dialer interface",
          "[mock][essentials][phone_dialer]") {
    // Arrange
    mock_phone_dialer impl;
    phone_dialer& iface = impl;

    // Act
    CHECK(iface.is_supported());
    iface.open("+1-202-555-0173");

    // Assert via concrete type
    REQUIRE(impl.last_number().has_value());
    CHECK(*impl.last_number() == "+1-202-555-0173");
    CHECK(impl.open_count() == 1);
}

TEST_CASE("phone_dialer abstract interface: is_supported() returns false when mock set unsupported",
          "[mock][essentials][phone_dialer]") {
    // Arrange
    mock_phone_dialer impl;
    impl.set_supported(false);
    phone_dialer& iface = impl;

    // Act + Assert via interface
    CHECK_FALSE(iface.is_supported());
    CHECK_THROWS_AS(iface.open("999"), std::runtime_error);
}

// ---------------------------------------------------------------------------
// Edge case: interleave supported/unsupported and reset
// ---------------------------------------------------------------------------

TEST_CASE("mock_phone_dialer: mixed supported/unsupported calls + reset sequence",
          "[mock][essentials][phone_dialer]") {
    // Arrange
    mock_phone_dialer d;

    // Act: two good calls, then disable, then two failed calls, then reset, then one good
    d.open("001");
    d.open("002");
    d.set_supported(false);
    try { d.open("bad1"); } catch (...) {}
    try { d.open("bad2"); } catch (...) {}
    d.reset();
    d.set_supported(true);
    d.open("003");

    // Assert
    CHECK(d.open_count() == 1);
    REQUIRE(d.last_number().has_value());
    CHECK(*d.last_number() == "003");
    CHECK(d.is_supported());
}
