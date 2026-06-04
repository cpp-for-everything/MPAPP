// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Catch2 unit tests for mpapp::launcher / mock_launcher.

#include <string>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/launcher.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// can_open — default (allow-all) mode
// ---------------------------------------------------------------------------

TEST_CASE("mock_launcher default allow-all: can_open returns true for any URI",
          "[mock][essentials][launcher]") {
    mock_launcher l;

    CHECK(l.can_open("https://example.com"));
    CHECK(l.can_open("mailto:user@example.com"));
    CHECK(l.can_open("custom-scheme://some/path"));
    CHECK(l.can_open(""));  // edge: empty string
}

// ---------------------------------------------------------------------------
// can_open — whitelist mode via constructor
// ---------------------------------------------------------------------------

TEST_CASE("mock_launcher whitelist constructor: can_open respects the set",
          "[mock][essentials][launcher]") {
    mock_launcher l{ std::unordered_set<std::string>{ "https://allowed.com", "tel:123" } };

    CHECK(l.can_open("https://allowed.com"));
    CHECK(l.can_open("tel:123"));
    CHECK_FALSE(l.can_open("https://blocked.com"));
    CHECK_FALSE(l.can_open(""));
}

// ---------------------------------------------------------------------------
// try_open — allow-all path
// ---------------------------------------------------------------------------

TEST_CASE("try_open succeeds and records URI in allow-all mode",
          "[mock][essentials][launcher]") {
    // Arrange
    mock_launcher l;

    // Act
    bool result = l.try_open("https://example.com");

    // Assert
    CHECK(result == true);
    REQUIRE(l.last_opened().has_value());
    CHECK(*l.last_opened() == "https://example.com");
    REQUIRE(l.last_try_open_uri().has_value());
    CHECK(*l.last_try_open_uri() == "https://example.com");
    CHECK(l.try_open_call_count() == 1);
}

// ---------------------------------------------------------------------------
// try_open — blocked URI
// ---------------------------------------------------------------------------

TEST_CASE("try_open returns false and does NOT update last_opened when blocked",
          "[mock][essentials][launcher]") {
    // Arrange
    mock_launcher l{ std::unordered_set<std::string>{ "https://ok.com" } };

    // Act
    bool result = l.try_open("https://blocked.com");

    // Assert
    CHECK(result == false);
    CHECK_FALSE(l.last_opened().has_value());   // nothing was opened
    REQUIRE(l.last_try_open_uri().has_value()); // but the attempt IS recorded
    CHECK(*l.last_try_open_uri() == "https://blocked.com");
    CHECK(l.try_open_call_count() == 1);
}

// ---------------------------------------------------------------------------
// try_open — multiple calls; last URI wins
// ---------------------------------------------------------------------------

TEST_CASE("try_open successive calls accumulate call count and update last URI",
          "[mock][essentials][launcher]") {
    mock_launcher l;

    l.try_open("https://first.com");
    l.try_open("https://second.com");
    l.try_open("https://third.com");

    CHECK(l.try_open_call_count() == 3);
    REQUIRE(l.last_try_open_uri().has_value());
    CHECK(*l.last_try_open_uri() == "https://third.com");
    CHECK(*l.last_opened() == "https://third.com");
}

// ---------------------------------------------------------------------------
// open — always records regardless of can_open
// ---------------------------------------------------------------------------

TEST_CASE("open records URI and increments counter",
          "[mock][essentials][launcher]") {
    // Arrange
    mock_launcher l;

    // Act
    l.open("https://example.com");

    // Assert
    REQUIRE(l.last_opened().has_value());
    CHECK(*l.last_opened() == "https://example.com");
    REQUIRE(l.last_open_uri().has_value());
    CHECK(*l.last_open_uri() == "https://example.com");
    CHECK(l.open_call_count() == 1);
    CHECK(l.try_open_call_count() == 0); // open != try_open
}

TEST_CASE("open works even when URI is not in whitelist",
          "[mock][essentials][launcher]") {
    // Arrange — whitelist only allows 'https://ok.com'
    mock_launcher l{ std::unordered_set<std::string>{ "https://ok.com" } };

    // Act — open a URI that would fail can_open
    l.open("https://blocked.com");

    // Assert — open records unconditionally
    REQUIRE(l.last_open_uri().has_value());
    CHECK(*l.last_open_uri() == "https://blocked.com");
    CHECK(l.open_call_count() == 1);
}

// ---------------------------------------------------------------------------
// open vs try_open — last_opened is shared across both
// ---------------------------------------------------------------------------

TEST_CASE("last_opened reflects the most recent open or successful try_open",
          "[mock][essentials][launcher]") {
    mock_launcher l;

    l.open("https://first.com");
    CHECK(*l.last_opened() == "https://first.com");

    l.try_open("https://second.com");
    CHECK(*l.last_opened() == "https://second.com");

    l.open("https://third.com");
    CHECK(*l.last_opened() == "https://third.com");
}

// ---------------------------------------------------------------------------
// allow / disallow / allow_all
// ---------------------------------------------------------------------------

TEST_CASE("allow adds a URI to the whitelist and activates whitelist mode",
          "[mock][essentials][launcher]") {
    mock_launcher l;  // starts in allow-all mode
    l.allow("https://specific.com");

    CHECK(l.can_open("https://specific.com"));
    CHECK_FALSE(l.can_open("https://anything-else.com")); // whitelist now active
}

TEST_CASE("disallow removes a URI from the whitelist",
          "[mock][essentials][launcher]") {
    mock_launcher l{ std::unordered_set<std::string>{ "https://a.com", "https://b.com" } };

    l.disallow("https://a.com");

    CHECK_FALSE(l.can_open("https://a.com"));
    CHECK(l.can_open("https://b.com")); // unaffected
}

TEST_CASE("allow_all reverts to allow-all mode even after whitelist was set",
          "[mock][essentials][launcher]") {
    mock_launcher l{ std::unordered_set<std::string>{ "https://only.com" } };

    CHECK_FALSE(l.can_open("https://other.com")); // blocked initially

    l.allow_all();

    CHECK(l.can_open("https://other.com")); // now allowed
    CHECK(l.can_open("https://only.com"));  // still allowed
}

// ---------------------------------------------------------------------------
// reset_history
// ---------------------------------------------------------------------------

TEST_CASE("reset_history clears recorded URIs and counters but keeps config",
          "[mock][essentials][launcher]") {
    mock_launcher l{ std::unordered_set<std::string>{ "https://ok.com" } };

    l.try_open("https://ok.com");
    l.open("https://ok.com");
    REQUIRE(l.try_open_call_count() == 1);
    REQUIRE(l.open_call_count() == 1);
    REQUIRE(l.last_opened().has_value());

    l.reset_history();

    CHECK(l.try_open_call_count() == 0);
    CHECK(l.open_call_count() == 0);
    CHECK_FALSE(l.last_opened().has_value());
    CHECK_FALSE(l.last_try_open_uri().has_value());
    CHECK_FALSE(l.last_open_uri().has_value());

    // Whitelist config is preserved
    CHECK(l.can_open("https://ok.com"));
    CHECK_FALSE(l.can_open("https://other.com"));
}

// ---------------------------------------------------------------------------
// Initial state — no calls yet
// ---------------------------------------------------------------------------

TEST_CASE("fresh mock_launcher has empty history and zero counters",
          "[mock][essentials][launcher]") {
    mock_launcher l;

    CHECK_FALSE(l.last_opened().has_value());
    CHECK_FALSE(l.last_try_open_uri().has_value());
    CHECK_FALSE(l.last_open_uri().has_value());
    CHECK(l.try_open_call_count() == 0);
    CHECK(l.open_call_count() == 0);
}

// ---------------------------------------------------------------------------
// Polymorphic use through the abstract interface
// ---------------------------------------------------------------------------

TEST_CASE("mock_launcher is usable through the launcher base interface",
          "[mock][essentials][launcher]") {
    mock_launcher impl;
    launcher& l = impl;

    CHECK(l.can_open("https://example.com"));

    bool opened = l.try_open("https://example.com");
    CHECK(opened);

    l.open("https://example.com");

    CHECK(impl.open_call_count() == 1);
    CHECK(impl.try_open_call_count() == 1);
}

// ---------------------------------------------------------------------------
// try_open returns false for blocked URI — last_open_uri NOT polluted
// ---------------------------------------------------------------------------

TEST_CASE("try_open failure does not update last_open_uri",
          "[mock][essentials][launcher]") {
    mock_launcher l{ std::unordered_set<std::string>{} }; // empty whitelist

    l.try_open("https://blocked.com");

    CHECK_FALSE(l.last_open_uri().has_value()); // open() never called
    CHECK(l.last_try_open_uri().has_value());   // attempt was recorded
}
