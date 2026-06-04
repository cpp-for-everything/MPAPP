// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Tests for the real cross-platform AppInfo backend
// (RFC-0013 Essentials).  Verifies that the ctor arguments are surfaced
// correctly through the abstract interface getters, that theme set/get
// round-trips (including the no-op same-value branch), that the
// `requested_theme_changed` signal fires on a real change, and that
// `show_settings_ui()` is recorded via `settings_ui_shown()`.

#include <string>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/real_app_info.hpp>
#include <mpapp/signal.hpp>

using namespace mpapp;

// ---- Identity getters -------------------------------------------------------

TEST_CASE("real_app_info getters return ctor values", "[mock][app_info]") {
    // Arrange
    real_app_info ai{
        "com.acme.widget",
        "Widget App",
        "2.3.4",
        "42"
    };

    // Act + Assert
    CHECK(ai.package_name()   == "com.acme.widget");
    CHECK(ai.name()           == "Widget App");
    CHECK(ai.version_string() == "2.3.4");
    CHECK(ai.build_string()   == "42");
}

TEST_CASE("real_app_info default-constructed metadata", "[mock][app_info]") {
    // Arrange + Act
    real_app_info ai{};

    // Assert - defaults must be non-empty sensible strings
    CHECK_FALSE(ai.package_name().empty());
    CHECK_FALSE(ai.name().empty());
    CHECK_FALSE(ai.version_string().empty());
    CHECK_FALSE(ai.build_string().empty());
}

TEST_CASE("real_app_info partial construction (name + version only)", "[mock][app_info]") {
    // Arrange
    real_app_info ai{ "org.test.partial", "Partial App" };

    // Act + Assert
    CHECK(ai.package_name()   == "org.test.partial");
    CHECK(ai.name()           == "Partial App");
    CHECK_FALSE(ai.version_string().empty());
    CHECK_FALSE(ai.build_string().empty());
}

// ---- Theme default + set/get -------------------------------------------------

TEST_CASE("real_app_info theme defaults to unspecified", "[mock][app_info]") {
    // Arrange + Act
    real_app_info ai{};

    // Assert
    CHECK(ai.requested_theme() == app_theme::unspecified);
}

TEST_CASE("real_app_info set_requested_theme round-trips", "[mock][app_info]") {
    // Arrange
    real_app_info ai{};

    // Act
    ai.set_requested_theme(app_theme::dark);

    // Assert
    CHECK(ai.requested_theme() == app_theme::dark);

    // Act - switch to light
    ai.set_requested_theme(app_theme::light);

    // Assert
    CHECK(ai.requested_theme() == app_theme::light);
}

TEST_CASE("real_app_info set_requested_theme same-value is no-op (no signal)", "[mock][app_info]") {
    // Arrange
    real_app_info ai{};
    int hits = 0;
    signal_slot<app_theme> slot;
    auto cb = [&](app_theme) { ++hits; };
    ai.requested_theme_changed.subscribe(slot, cb);

    // Act - set to same value (unspecified -> unspecified)
    ai.set_requested_theme(app_theme::unspecified);

    // Assert - signal must NOT fire
    CHECK(hits == 0);
    CHECK(ai.requested_theme() == app_theme::unspecified);
}

TEST_CASE("real_app_info set_requested_theme emits signal on change", "[mock][app_info]") {
    // Arrange
    real_app_info ai{};
    app_theme last = app_theme::unspecified;
    int hits = 0;
    signal_slot<app_theme> slot;
    auto cb = [&](app_theme t) { last = t; ++hits; };
    ai.requested_theme_changed.subscribe(slot, cb);

    // Act
    ai.set_requested_theme(app_theme::dark);

    // Assert
    CHECK(hits == 1);
    CHECK(last == app_theme::dark);

    // Act - change again
    ai.set_requested_theme(app_theme::light);

    // Assert
    CHECK(hits == 2);
    CHECK(last == app_theme::light);
}

TEST_CASE("real_app_info set_requested_theme all three theme values", "[mock][app_info]") {
    // Arrange
    real_app_info ai{};

    // Act + Assert - cycle through all enum values
    ai.set_requested_theme(app_theme::light);
    CHECK(ai.requested_theme() == app_theme::light);

    ai.set_requested_theme(app_theme::dark);
    CHECK(ai.requested_theme() == app_theme::dark);

    ai.set_requested_theme(app_theme::unspecified);
    CHECK(ai.requested_theme() == app_theme::unspecified);
}

// ---- Layout direction -------------------------------------------------------

TEST_CASE("real_app_info layout direction defaults to left_to_right", "[mock][app_info]") {
    // Arrange + Act
    real_app_info ai{};

    // Assert
    CHECK(ai.requested_layout_direction() == layout_direction::left_to_right);
}

TEST_CASE("real_app_info set_requested_layout_direction round-trips", "[mock][app_info]") {
    // Arrange
    real_app_info ai{};

    // Act
    ai.set_requested_layout_direction(layout_direction::right_to_left);

    // Assert
    CHECK(ai.requested_layout_direction() == layout_direction::right_to_left);

    // Act - reset to LTR
    ai.set_requested_layout_direction(layout_direction::left_to_right);

    // Assert
    CHECK(ai.requested_layout_direction() == layout_direction::left_to_right);
}

TEST_CASE("real_app_info set_requested_layout_direction unknown variant", "[mock][app_info]") {
    // Arrange
    real_app_info ai{};

    // Act
    ai.set_requested_layout_direction(layout_direction::unknown);

    // Assert
    CHECK(ai.requested_layout_direction() == layout_direction::unknown);
}

// ---- show_settings_ui -------------------------------------------------------

TEST_CASE("real_app_info settings_ui_shown starts false", "[mock][app_info]") {
    // Arrange + Act
    real_app_info ai{};

    // Assert
    CHECK_FALSE(ai.settings_ui_shown());
}

TEST_CASE("real_app_info show_settings_ui records intent", "[mock][app_info]") {
    // Arrange
    real_app_info ai{};
    CHECK_FALSE(ai.settings_ui_shown());

    // Act
    ai.show_settings_ui();

    // Assert
    CHECK(ai.settings_ui_shown());
}

TEST_CASE("real_app_info show_settings_ui idempotent", "[mock][app_info]") {
    // Arrange
    real_app_info ai{};

    // Act - call twice
    ai.show_settings_ui();
    ai.show_settings_ui();

    // Assert - still true, no side-effects
    CHECK(ai.settings_ui_shown());
}

TEST_CASE("real_app_info reset_settings_ui_shown clears the flag", "[mock][app_info]") {
    // Arrange
    real_app_info ai{};
    ai.show_settings_ui();
    REQUIRE(ai.settings_ui_shown());

    // Act
    ai.reset_settings_ui_shown();

    // Assert
    CHECK_FALSE(ai.settings_ui_shown());

    // Act - can be set again after reset
    ai.show_settings_ui();
    CHECK(ai.settings_ui_shown());
}

// ---- Polymorphic interface ---------------------------------------------------

TEST_CASE("real_app_info is usable through the abstract interface", "[mock][app_info]") {
    // Arrange - hold via base pointer
    real_app_info concrete{ "com.poly.test", "Poly App", "0.1.0", "9" };
    app_info& iface = concrete;

    // Act + Assert - all interface getters work through the vtable
    CHECK(iface.package_name()   == "com.poly.test");
    CHECK(iface.name()           == "Poly App");
    CHECK(iface.version_string() == "0.1.0");
    CHECK(iface.build_string()   == "9");
    CHECK(iface.requested_theme() == app_theme::unspecified);
    CHECK(iface.requested_layout_direction() == layout_direction::left_to_right);

    // Act - call show_settings_ui through the interface
    iface.show_settings_ui();

    // Assert - the concrete flag is set
    CHECK(concrete.settings_ui_shown());
}

// ---- Signal unsubscribe via slot lifetime -----------------------------------

TEST_CASE("real_app_info signal does not fire after slot is disconnected", "[mock][app_info]") {
    // Arrange
    real_app_info ai{};
    int hits = 0;
    {
        signal_slot<app_theme> slot;
        auto cb = [&](app_theme) { ++hits; };
        ai.requested_theme_changed.subscribe(slot, cb);

        // Act - fire while slot is alive
        ai.set_requested_theme(app_theme::dark);
        CHECK(hits == 1);
    } // slot destroyed here -> auto-disconnects

    // Act - fire after slot is gone
    ai.set_requested_theme(app_theme::light);

    // Assert - no additional fires
    CHECK(hits == 1);
}
