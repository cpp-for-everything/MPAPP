// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Catch2 tests for mpapp::browser (RFC-0013 Essentials).

#include <optional>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/browser.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// to_string helpers
// ---------------------------------------------------------------------------

TEST_CASE("to_string(browser_launch_mode) covers every case + out-of-range",
          "[mock][browser][enum]") {
    CHECK(to_string(browser_launch_mode::system_preferred) == "system_preferred");
    CHECK(to_string(browser_launch_mode::external)         == "external");
    CHECK(to_string(static_cast<browser_launch_mode>(99))  == "?");
}

TEST_CASE("to_string(browser_title_mode) covers every case + out-of-range",
          "[mock][browser][enum]") {
    CHECK(to_string(browser_title_mode::default_)          == "default");
    CHECK(to_string(browser_title_mode::show)              == "show");
    CHECK(to_string(browser_title_mode::hide)              == "hide");
    CHECK(to_string(static_cast<browser_title_mode>(99))   == "?");
}

// ---------------------------------------------------------------------------
// browser_launch_options value type
// ---------------------------------------------------------------------------

TEST_CASE("browser_launch_options default-constructs with expected values",
          "[mock][browser][options]") {
    // Arrange / Act
    browser_launch_options opts;

    // Assert
    CHECK(opts.mode       == browser_launch_mode::system_preferred);
    CHECK(opts.title_mode == browser_title_mode::default_);
}

TEST_CASE("browser_launch_options equality comparison works",
          "[mock][browser][options]") {
    browser_launch_options a;
    browser_launch_options b;
    CHECK(a == b);

    b.mode = browser_launch_mode::external;
    CHECK_FALSE(a == b);

    b.mode = browser_launch_mode::system_preferred;
    b.title_mode = browser_title_mode::show;
    CHECK_FALSE(a == b);

    b.title_mode = browser_title_mode::default_;
    CHECK(a == b);
}

// ---------------------------------------------------------------------------
// Initial / default state
// ---------------------------------------------------------------------------

TEST_CASE("mock_browser starts with no recorded calls",
          "[mock][browser]") {
    // Arrange
    mock_browser b;

    // Assert
    CHECK_FALSE(b.last_uri().has_value());
    CHECK_FALSE(b.last_options().has_value());
    CHECK(b.open_count() == 0);
}

// ---------------------------------------------------------------------------
// open(uri) - default options overload
// ---------------------------------------------------------------------------

TEST_CASE("mock_browser open(uri) returns true by default",
          "[mock][browser]") {
    // Arrange
    mock_browser b;

    // Act
    bool result = b.open("https://example.com");

    // Assert
    CHECK(result == true);
}

TEST_CASE("mock_browser open(uri) records last_uri",
          "[mock][browser]") {
    // Arrange
    mock_browser b;

    // Act
    (void)b.open("https://example.com");

    // Assert
    REQUIRE(b.last_uri().has_value());
    CHECK(*b.last_uri() == "https://example.com");
}

TEST_CASE("mock_browser open(uri) records default options",
          "[mock][browser]") {
    // Arrange
    mock_browser b;

    // Act
    (void)b.open("https://example.com");

    // Assert
    REQUIRE(b.last_options().has_value());
    CHECK(b.last_options()->mode       == browser_launch_mode::system_preferred);
    CHECK(b.last_options()->title_mode == browser_title_mode::default_);
}

TEST_CASE("mock_browser open(uri) increments open_count",
          "[mock][browser]") {
    // Arrange
    mock_browser b;

    // Act
    (void)b.open("https://a.com");
    (void)b.open("https://b.com");

    // Assert
    CHECK(b.open_count() == 2);
}

// ---------------------------------------------------------------------------
// open(uri, browser_launch_mode) - mode overload
// ---------------------------------------------------------------------------

TEST_CASE("mock_browser open(uri, mode) records the specified mode",
          "[mock][browser]") {
    // Arrange
    mock_browser b;

    // Act
    (void)b.open("https://example.com", browser_launch_mode::external);

    // Assert
    REQUIRE(b.last_options().has_value());
    CHECK(b.last_options()->mode       == browser_launch_mode::external);
    CHECK(b.last_options()->title_mode == browser_title_mode::default_);
}

TEST_CASE("mock_browser open(uri, mode) returns configured result",
          "[mock][browser]") {
    // Arrange
    mock_browser b;
    b.set_result(false);

    // Act
    bool result = b.open("https://example.com", browser_launch_mode::external);

    // Assert
    CHECK(result == false);
}

TEST_CASE("mock_browser open(uri, system_preferred) records system_preferred",
          "[mock][browser]") {
    // Arrange
    mock_browser b;

    // Act
    (void)b.open("https://example.com", browser_launch_mode::system_preferred);

    // Assert
    REQUIRE(b.last_options().has_value());
    CHECK(b.last_options()->mode == browser_launch_mode::system_preferred);
}

// ---------------------------------------------------------------------------
// open(uri, browser_launch_options) - full options overload
// ---------------------------------------------------------------------------

TEST_CASE("mock_browser open(uri, options) records full options",
          "[mock][browser]") {
    // Arrange
    mock_browser b;
    browser_launch_options opts;
    opts.mode       = browser_launch_mode::external;
    opts.title_mode = browser_title_mode::hide;

    // Act
    (void)b.open("https://example.com", opts);

    // Assert
    REQUIRE(b.last_options().has_value());
    CHECK(*b.last_options() == opts);
    CHECK(b.last_options()->mode       == browser_launch_mode::external);
    CHECK(b.last_options()->title_mode == browser_title_mode::hide);
}

TEST_CASE("mock_browser open(uri, options) with show title_mode records correctly",
          "[mock][browser]") {
    // Arrange
    mock_browser b;
    browser_launch_options opts;
    opts.mode       = browser_launch_mode::system_preferred;
    opts.title_mode = browser_title_mode::show;

    // Act
    (void)b.open("https://example.com", opts);

    // Assert
    REQUIRE(b.last_options().has_value());
    CHECK(b.last_options()->title_mode == browser_title_mode::show);
}

TEST_CASE("mock_browser open(uri, options) returns true by default",
          "[mock][browser]") {
    // Arrange
    mock_browser b;
    browser_launch_options opts;

    // Act / Assert
    CHECK(b.open("https://example.com", opts) == true);
}

// ---------------------------------------------------------------------------
// set_result - failure simulation
// ---------------------------------------------------------------------------

TEST_CASE("mock_browser set_result(false) makes all open overloads return false",
          "[mock][browser]") {
    // Arrange
    mock_browser b;
    b.set_result(false);

    // Act / Assert
    CHECK(b.open("https://example.com")                                   == false);
    CHECK(b.open("https://example.com", browser_launch_mode::external)    == false);
    CHECK(b.open("https://example.com", browser_launch_options{})         == false);
}

TEST_CASE("mock_browser open_count increments even when result is false",
          "[mock][browser]") {
    // Arrange
    mock_browser b;
    b.set_result(false);

    // Act
    (void)b.open("https://example.com");
    (void)b.open("https://example.com", browser_launch_mode::external);
    (void)b.open("https://example.com", browser_launch_options{});

    // Assert - all three calls recorded despite returning false.
    CHECK(b.open_count() == 3);
}

TEST_CASE("mock_browser set_result toggles between true and false",
          "[mock][browser]") {
    // Arrange
    mock_browser b;

    // Act / Assert
    CHECK(b.open("https://example.com") == true);

    b.set_result(false);
    CHECK(b.open("https://example.com") == false);

    b.set_result(true);
    CHECK(b.open("https://example.com") == true);
}

// ---------------------------------------------------------------------------
// Overwrite: last call always wins
// ---------------------------------------------------------------------------

TEST_CASE("mock_browser last_uri reflects the most recent open call",
          "[mock][browser]") {
    // Arrange
    mock_browser b;

    // Act
    (void)b.open("https://first.com");
    (void)b.open("https://second.com");

    // Assert
    REQUIRE(b.last_uri().has_value());
    CHECK(*b.last_uri() == "https://second.com");
}

TEST_CASE("mock_browser last_options reflects the most recent open call",
          "[mock][browser]") {
    // Arrange
    mock_browser b;
    browser_launch_options opts_a;
    browser_launch_options opts_b;
    opts_b.title_mode = browser_title_mode::hide;

    // Act
    (void)b.open("https://first.com",  opts_a);
    (void)b.open("https://second.com", opts_b);

    // Assert
    REQUIRE(b.last_options().has_value());
    CHECK(b.last_options()->title_mode == browser_title_mode::hide);
}

// ---------------------------------------------------------------------------
// reset()
// ---------------------------------------------------------------------------

TEST_CASE("mock_browser reset() clears last_uri, last_options, and open_count",
          "[mock][browser]") {
    // Arrange
    mock_browser b;
    (void)b.open("https://example.com");
    REQUIRE(b.open_count() == 1);

    // Act
    b.reset();

    // Assert
    CHECK_FALSE(b.last_uri().has_value());
    CHECK_FALSE(b.last_options().has_value());
    CHECK(b.open_count() == 0);
}

TEST_CASE("mock_browser reset() does not change the result flag",
          "[mock][browser]") {
    // Arrange
    mock_browser b;
    b.set_result(false);
    (void)b.open("https://example.com");

    // Act
    b.reset();

    // Assert - result flag is still false.
    CHECK(b.open("https://example.com") == false);  // NOLINT: result intentionally checked
}

// ---------------------------------------------------------------------------
// Constructor: explicit initial result flag
// ---------------------------------------------------------------------------

TEST_CASE("mock_browser constructed with result=false returns false immediately",
          "[mock][browser]") {
    // Arrange
    mock_browser b{ false };

    // Act / Assert
    CHECK(b.open("https://example.com") == false);
}

// ---------------------------------------------------------------------------
// Abstract interface polymorphism
// ---------------------------------------------------------------------------

TEST_CASE("mock_browser is usable through the abstract browser interface",
          "[mock][browser]") {
    // Arrange
    mock_browser impl;
    browser& iface = impl;

    // Act
    bool result = iface.open("https://example.com");

    // Assert
    CHECK(result == true);
    REQUIRE(impl.last_uri().has_value());
    CHECK(*impl.last_uri() == "https://example.com");
}

TEST_CASE("browser interface open(uri, mode) works polymorphically",
          "[mock][browser]") {
    // Arrange
    mock_browser impl;
    browser& iface = impl;

    // Act
    bool result = iface.open("https://example.com", browser_launch_mode::external);

    // Assert
    CHECK(result == true);
    REQUIRE(impl.last_options().has_value());
    CHECK(impl.last_options()->mode == browser_launch_mode::external);
}

TEST_CASE("browser interface open(uri, options) works polymorphically",
          "[mock][browser]") {
    // Arrange
    mock_browser impl;
    browser& iface = impl;
    browser_launch_options opts;
    opts.title_mode = browser_title_mode::show;

    // Act
    bool result = iface.open("https://example.com", opts);

    // Assert
    CHECK(result == true);
    REQUIRE(impl.last_options().has_value());
    CHECK(impl.last_options()->title_mode == browser_title_mode::show);
}
