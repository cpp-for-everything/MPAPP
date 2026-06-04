// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for the RFC-0013 Essentials app_info API.
//
// Covers every public method, every signal emission path, the same-value
// no-op guard on set_requested_theme, and all to_string helpers including
// out-of-range fallbacks.  Follows the AAA pattern used throughout the
// mock_handlers suite.

#include <string>
#include <string_view>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/app_info.hpp>
#include <mpapp/signal.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// to_string helpers
// ---------------------------------------------------------------------------

TEST_CASE("app_theme to_string covers every case + out-of-range fallback",
          "[mock][app_info][enum]") {
    CHECK(to_string(app_theme::unspecified) == "unspecified");
    CHECK(to_string(app_theme::light)       == "light");
    CHECK(to_string(app_theme::dark)        == "dark");
    CHECK(to_string(static_cast<app_theme>(99)) == "?");
}

TEST_CASE("layout_direction to_string covers every case + out-of-range fallback",
          "[mock][app_info][enum]") {
    CHECK(to_string(layout_direction::unknown)       == "unknown");
    CHECK(to_string(layout_direction::left_to_right) == "left_to_right");
    CHECK(to_string(layout_direction::right_to_left) == "right_to_left");
    CHECK(to_string(static_cast<layout_direction>(99)) == "?");
}

// ---------------------------------------------------------------------------
// Default-constructed mock state
// ---------------------------------------------------------------------------

TEST_CASE("mock_app_info default state is sensible and non-empty",
          "[mock][app_info][defaults]") {
    // Arrange
    mock_app_info ai;

    // Assert — identity strings are non-empty by default
    CHECK_FALSE(ai.package_name().empty());
    CHECK_FALSE(ai.name().empty());
    CHECK_FALSE(ai.version_string().empty());
    CHECK_FALSE(ai.build_string().empty());

    // Assert — defaults for enum fields
    CHECK(ai.requested_theme()            == app_theme::unspecified);
    CHECK(ai.requested_layout_direction() == layout_direction::left_to_right);

    // Assert — settings page not yet shown
    CHECK_FALSE(ai.settings_ui_shown());
}

// ---------------------------------------------------------------------------
// String setters / getters round-trip
// ---------------------------------------------------------------------------

TEST_CASE("mock_app_info string fields round-trip through setters",
          "[mock][app_info][setters]") {
    // Arrange
    mock_app_info ai;

    // Act
    ai.set_package_name("com.myorg.myapp");
    ai.set_name("My App");
    ai.set_version_string("2.3.4");
    ai.set_build_string("42");

    // Assert
    CHECK(ai.package_name()   == "com.myorg.myapp");
    CHECK(ai.name()           == "My App");
    CHECK(ai.version_string() == "2.3.4");
    CHECK(ai.build_string()   == "42");
}

// ---------------------------------------------------------------------------
// show_settings_ui records the call
// ---------------------------------------------------------------------------

TEST_CASE("show_settings_ui sets the shown flag; reset clears it",
          "[mock][app_info][settings_ui]") {
    // Arrange
    mock_app_info ai;
    REQUIRE_FALSE(ai.settings_ui_shown());

    // Act
    ai.show_settings_ui();

    // Assert
    CHECK(ai.settings_ui_shown());

    // Act — reset
    ai.reset_settings_ui_shown();

    // Assert
    CHECK_FALSE(ai.settings_ui_shown());
}

TEST_CASE("show_settings_ui can be called multiple times without error",
          "[mock][app_info][settings_ui]") {
    mock_app_info ai;
    ai.show_settings_ui();
    ai.show_settings_ui();
    CHECK(ai.settings_ui_shown());
}

// ---------------------------------------------------------------------------
// layout_direction setter
// ---------------------------------------------------------------------------

TEST_CASE("set_requested_layout_direction updates the value",
          "[mock][app_info][layout_direction]") {
    // Arrange
    mock_app_info ai;
    REQUIRE(ai.requested_layout_direction() == layout_direction::left_to_right);

    // Act
    ai.set_requested_layout_direction(layout_direction::right_to_left);

    // Assert
    CHECK(ai.requested_layout_direction() == layout_direction::right_to_left);

    // Act — set to unknown
    ai.set_requested_layout_direction(layout_direction::unknown);

    // Assert
    CHECK(ai.requested_layout_direction() == layout_direction::unknown);
}

// ---------------------------------------------------------------------------
// requested_theme setter + signal
// ---------------------------------------------------------------------------

TEST_CASE("set_requested_theme updates the value",
          "[mock][app_info][theme]") {
    // Arrange
    mock_app_info ai;
    REQUIRE(ai.requested_theme() == app_theme::unspecified);

    // Act
    ai.set_requested_theme(app_theme::dark);

    // Assert
    CHECK(ai.requested_theme() == app_theme::dark);

    // Act
    ai.set_requested_theme(app_theme::light);

    // Assert
    CHECK(ai.requested_theme() == app_theme::light);
}

TEST_CASE("set_requested_theme fires requested_theme_changed on change",
          "[mock][app_info][theme][signal]") {
    // Arrange
    mock_app_info ai;
    app_theme last_theme = app_theme::unspecified;
    int call_count = 0;

    signal_slot<app_theme> slot;
    auto cb = [&](app_theme t) {
        last_theme = t;
        ++call_count;
    };
    ai.requested_theme_changed.subscribe(slot, cb);

    // Act — change from unspecified to dark
    ai.set_requested_theme(app_theme::dark);

    // Assert
    CHECK(call_count == 1);
    CHECK(last_theme == app_theme::dark);

    // Act — change again to light
    ai.set_requested_theme(app_theme::light);

    // Assert
    CHECK(call_count == 2);
    CHECK(last_theme == app_theme::light);
}

TEST_CASE("set_requested_theme does NOT fire signal when value is unchanged",
          "[mock][app_info][theme][signal]") {
    // Arrange
    mock_app_info ai;
    ai.set_requested_theme(app_theme::dark);

    int call_count = 0;
    signal_slot<app_theme> slot;
    auto cb = [&](app_theme) { ++call_count; };
    ai.requested_theme_changed.subscribe(slot, cb);

    // Act — same value — must be a no-op
    ai.set_requested_theme(app_theme::dark);

    // Assert
    CHECK(call_count == 0);

    // Act — different value — must fire
    ai.set_requested_theme(app_theme::light);

    // Assert
    CHECK(call_count == 1);
}

TEST_CASE("requested_theme_changed: multiple subscribers all receive the event",
          "[mock][app_info][theme][signal]") {
    // Arrange
    mock_app_info ai;
    int hits_a = 0, hits_b = 0;

    signal_slot<app_theme> slot_a, slot_b;
    auto cb_a = [&](app_theme) { ++hits_a; };
    auto cb_b = [&](app_theme) { ++hits_b; };
    ai.requested_theme_changed.subscribe(slot_a, cb_a);
    ai.requested_theme_changed.subscribe(slot_b, cb_b);

    // Act
    ai.set_requested_theme(app_theme::dark);

    // Assert
    CHECK(hits_a == 1);
    CHECK(hits_b == 1);
}

TEST_CASE("requested_theme_changed: disconnecting a slot stops delivery",
          "[mock][app_info][theme][signal]") {
    // Arrange
    mock_app_info ai;
    int call_count = 0;

    signal_slot<app_theme> slot;
    auto cb = [&](app_theme) { ++call_count; };
    ai.requested_theme_changed.subscribe(slot, cb);

    // Act — fire once while connected
    ai.set_requested_theme(app_theme::dark);
    CHECK(call_count == 1);

    // Act — disconnect, then fire again
    slot.disconnect();
    ai.set_requested_theme(app_theme::light);

    // Assert — count must not have increased
    CHECK(call_count == 1);
}

// ---------------------------------------------------------------------------
// Interface pointer usage (polymorphic access)
// ---------------------------------------------------------------------------

TEST_CASE("mock_app_info is usable through the app_info abstract interface",
          "[mock][app_info][interface]") {
    // Arrange
    mock_app_info concrete;
    concrete.set_name("Polymorphic App");
    concrete.set_requested_theme(app_theme::dark);

    app_info& ref = concrete;

    // Assert — all pure virtual methods are callable via the interface
    CHECK(ref.name()             == "Polymorphic App");
    CHECK(ref.requested_theme()  == app_theme::dark);
    CHECK(ref.requested_layout_direction() == layout_direction::left_to_right);
    CHECK_FALSE(ref.package_name().empty());
    CHECK_FALSE(ref.version_string().empty());
    CHECK_FALSE(ref.build_string().empty());

    // Act — show_settings_ui through base reference
    ref.show_settings_ui();

    // Assert — recorded in the concrete mock
    CHECK(concrete.settings_ui_shown());
}
