// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Catch2 tests for mpapp::screenshot (RFC-0013 Essentials).
//
// Coverage targets: every public method, all to_string helpers, default /
// not-supported paths, capture-count tracking, and interface polymorphism.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/screenshot.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// to_string helpers
// ---------------------------------------------------------------------------

TEST_CASE("screenshot_format to_string covers every enumerator + fallback",
          "[mock][screenshot][enum]") {
    CHECK(to_string(screenshot_format::png)  == "png");
    CHECK(to_string(screenshot_format::jpeg) == "jpeg");
    CHECK(to_string(static_cast<screenshot_format>(99)) == "?");
}

// ---------------------------------------------------------------------------
// screenshot_result value type
// ---------------------------------------------------------------------------

TEST_CASE("screenshot_result default-constructs to zero dimensions and empty bytes",
          "[mock][screenshot][screenshot_result]") {
    screenshot_result r{};
    CHECK(r.width  == 0);
    CHECK(r.height == 0);
    CHECK(r.bytes.empty());
    CHECK(r.format == screenshot_format::png);
    CHECK(r == screenshot_result{});
}

TEST_CASE("screenshot_result compares by value",
          "[mock][screenshot][screenshot_result]") {
    screenshot_result a{ 640, 480, { 1, 2, 3 }, screenshot_format::jpeg };
    screenshot_result b = a;
    CHECK(a == b);

    b.width = 1280;
    CHECK_FALSE(a == b);

    screenshot_result c{ 640, 480, { 1, 2, 3 }, screenshot_format::jpeg };
    CHECK(a == c);

    c.format = screenshot_format::png;
    CHECK_FALSE(a == c);
}

// ---------------------------------------------------------------------------
// mock_screenshot - default (supported) construction
// ---------------------------------------------------------------------------

TEST_CASE("mock_screenshot default construction is supported with zero capture_count",
          "[mock][screenshot]") {
    // Arrange + Act
    mock_screenshot s;

    // Assert
    CHECK(s.is_captured_supported());
    CHECK(s.capture_count() == 0);
}

// ---------------------------------------------------------------------------
// mock_screenshot - capture() with canned result
// ---------------------------------------------------------------------------

TEST_CASE("capture returns canned result when supported and increments capture_count",
          "[mock][screenshot]") {
    // Arrange
    mock_screenshot s;
    screenshot_result expected{ 1920, 1080, { 0xDE, 0xAD, 0xBE, 0xEF }, screenshot_format::png };
    s.set_result(expected);

    // Act
    auto result = s.capture();

    // Assert
    REQUIRE(result.has_value());
    CHECK(*result == expected);
    CHECK(s.capture_count() == 1);
}

TEST_CASE("capture increments capture_count on each call",
          "[mock][screenshot]") {
    // Arrange
    mock_screenshot s;

    // Act
    (void)s.capture();
    (void)s.capture();
    (void)s.capture();

    // Assert
    CHECK(s.capture_count() == 3);
}

TEST_CASE("capture returns default-constructed result when no canned result is set",
          "[mock][screenshot]") {
    // Arrange
    mock_screenshot s;

    // Act
    auto result = s.capture();

    // Assert
    REQUIRE(result.has_value());
    CHECK(result->width  == 0);
    CHECK(result->height == 0);
    CHECK(result->bytes.empty());
    CHECK(result->format == screenshot_format::png);
}

// ---------------------------------------------------------------------------
// mock_screenshot - set_result round-trip
// ---------------------------------------------------------------------------

TEST_CASE("set_result + capture returns the exact seeded value",
          "[mock][screenshot]") {
    // Arrange
    mock_screenshot s;
    screenshot_result r{ 800, 600, { 1, 2, 3, 4 }, screenshot_format::jpeg };
    s.set_result(r);

    // Act
    auto got = s.capture();

    // Assert
    REQUIRE(got.has_value());
    CHECK(got->width  == 800);
    CHECK(got->height == 600);
    CHECK(got->bytes  == std::vector<std::uint8_t>{ 1, 2, 3, 4 });
    CHECK(got->format == screenshot_format::jpeg);
}

// ---------------------------------------------------------------------------
// mock_screenshot - not-supported path
// ---------------------------------------------------------------------------

TEST_CASE("mock_screenshot constructed with supported=false returns nullopt from capture",
          "[mock][screenshot][not_supported]") {
    // Arrange
    mock_screenshot s{ false };

    // Assert
    CHECK_FALSE(s.is_captured_supported());

    // Act
    auto result = s.capture();

    // Assert
    CHECK_FALSE(result.has_value());
    CHECK(s.capture_count() == 1);  // call is still counted
}

TEST_CASE("set_captured_supported(false) makes subsequent captures return nullopt",
          "[mock][screenshot][not_supported]") {
    // Arrange
    mock_screenshot s;
    s.set_result({ 320, 240, { 0xFF }, screenshot_format::png });
    auto first = s.capture();
    REQUIRE(first.has_value());

    // Act - disable support
    s.set_captured_supported(false);
    auto second = s.capture();

    // Assert
    CHECK_FALSE(s.is_captured_supported());
    CHECK_FALSE(second.has_value());
    CHECK(s.capture_count() == 2);
}

TEST_CASE("set_captured_supported(true) re-enables capture after being disabled",
          "[mock][screenshot][not_supported]") {
    // Arrange
    mock_screenshot s{ false };
    s.set_result({ 100, 100, { 0xAB }, screenshot_format::jpeg });

    // Act
    s.set_captured_supported(true);
    auto result = s.capture();

    // Assert
    CHECK(s.is_captured_supported());
    REQUIRE(result.has_value());
    CHECK(result->width  == 100);
    CHECK(result->height == 100);
    CHECK(result->format == screenshot_format::jpeg);
}

TEST_CASE("capture_count tracks calls regardless of supported flag",
          "[mock][screenshot][not_supported]") {
    // Arrange
    mock_screenshot s{ false };

    // Act
    (void)s.capture();
    (void)s.capture();
    s.set_captured_supported(true);
    (void)s.capture();

    // Assert - three calls total even though first two returned nullopt
    CHECK(s.capture_count() == 3);
}

// ---------------------------------------------------------------------------
// interface polymorphism
// ---------------------------------------------------------------------------

TEST_CASE("screenshot* base pointer dispatches correctly to mock_screenshot",
          "[mock][screenshot][polymorphism]") {
    // Arrange
    mock_screenshot concrete;
    concrete.set_result({ 1024, 768, { 0x01 }, screenshot_format::png });

    screenshot* base = &concrete;

    // Assert via interface
    CHECK(base->is_captured_supported());

    auto result = base->capture();
    REQUIRE(result.has_value());
    CHECK(result->width  == 1024);
    CHECK(result->height == 768);
}

TEST_CASE("screenshot* base pointer for unsupported mock returns nullopt",
          "[mock][screenshot][polymorphism]") {
    // Arrange
    mock_screenshot concrete{ false };
    screenshot* base = &concrete;

    // Assert
    CHECK_FALSE(base->is_captured_supported());
    CHECK_FALSE(base->capture().has_value());
}
