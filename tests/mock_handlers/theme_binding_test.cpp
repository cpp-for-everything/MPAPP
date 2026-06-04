// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Tests for the RFC-0013 theme app_theme_binding<T> value type.
//
// Covers resolve() for light/dark/unspecified with and without a
// default_value, the free-function app_theme_value wrapper, and out-of-range
// theme values - exercising every branch of the switch.  Instantiated with
// both T=double and T=std::string.  Follows the AAA pattern used throughout
// the mock_handlers suite.

#include <optional>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/app_info.hpp>
#include <mpapp/theme/app_theme_binding.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// resolve() - T = double, no default_value
// ---------------------------------------------------------------------------

TEST_CASE("app_theme_binding<double> resolves light and dark directly",
          "[mock][theme][binding]") {
    // Arrange
    app_theme_binding<double> b{.light = 1.0, .dark = 2.0};

    // Act / Assert
    CHECK(b.resolve(app_theme::light) == 1.0);
    CHECK(b.resolve(app_theme::dark)  == 2.0);
}

TEST_CASE("app_theme_binding<double> unspecified falls back to light without default",
          "[mock][theme][binding][fallback]") {
    // Arrange - no default_value provided
    app_theme_binding<double> b{.light = 1.0, .dark = 2.0};

    // Act / Assert - unspecified must use the light value
    CHECK(b.resolve(app_theme::unspecified) == 1.0);
}

TEST_CASE("app_theme_binding<double> unspecified uses default_value when present",
          "[mock][theme][binding][fallback]") {
    // Arrange - explicit default_value differs from light/dark
    app_theme_binding<double> b{.light = 1.0, .dark = 2.0, .default_value = 9.5};

    // Act / Assert
    CHECK(b.resolve(app_theme::unspecified) == 9.5);
    // light/dark still map directly even when a default is present
    CHECK(b.resolve(app_theme::light) == 1.0);
    CHECK(b.resolve(app_theme::dark)  == 2.0);
}

TEST_CASE("app_theme_binding<double> out-of-range theme uses fallback branch",
          "[mock][theme][binding][fallback]") {
    // Arrange - without default, an unknown theme value hits the default: arm
    app_theme_binding<double> no_default{.light = 3.0, .dark = 4.0};
    app_theme_binding<double> with_default{.light = 3.0, .dark = 4.0,
                                           .default_value = 7.0};

    // Act / Assert
    CHECK(no_default.resolve(static_cast<app_theme>(99))   == 3.0);
    CHECK(with_default.resolve(static_cast<app_theme>(99)) == 7.0);
}

// ---------------------------------------------------------------------------
// resolve() - T = std::string
// ---------------------------------------------------------------------------

TEST_CASE("app_theme_binding<std::string> resolves light and dark directly",
          "[mock][theme][binding]") {
    // Arrange
    app_theme_binding<std::string> b{.light = "white", .dark = "black"};

    // Act / Assert
    CHECK(b.resolve(app_theme::light) == "white");
    CHECK(b.resolve(app_theme::dark)  == "black");
}

TEST_CASE("app_theme_binding<std::string> unspecified falls back to light without default",
          "[mock][theme][binding][fallback]") {
    // Arrange
    app_theme_binding<std::string> b{.light = "white", .dark = "black"};

    // Act / Assert
    CHECK(b.resolve(app_theme::unspecified) == "white");
}

TEST_CASE("app_theme_binding<std::string> unspecified uses default_value when present",
          "[mock][theme][binding][fallback]") {
    // Arrange
    app_theme_binding<std::string> b{
        .light = "white", .dark = "black", .default_value = std::string{"system"}};

    // Act / Assert
    CHECK(b.resolve(app_theme::unspecified) == "system");
    CHECK(b.resolve(app_theme::light) == "white");
    CHECK(b.resolve(app_theme::dark)  == "black");
}

// ---------------------------------------------------------------------------
// Default-constructed binding
// ---------------------------------------------------------------------------

TEST_CASE("default-constructed app_theme_binding yields value-initialized members",
          "[mock][theme][binding][defaults]") {
    // Arrange
    app_theme_binding<double>      bd;
    app_theme_binding<std::string> bs;

    // Assert - value-initialized members, absent default
    CHECK(bd.resolve(app_theme::light)       == 0.0);
    CHECK(bd.resolve(app_theme::dark)        == 0.0);
    CHECK(bd.resolve(app_theme::unspecified) == 0.0);
    CHECK_FALSE(bd.default_value.has_value());

    CHECK(bs.resolve(app_theme::light).empty());
    CHECK(bs.resolve(app_theme::unspecified).empty());
    CHECK_FALSE(bs.default_value.has_value());
}

// ---------------------------------------------------------------------------
// Free-function wrapper app_theme_value
// ---------------------------------------------------------------------------

TEST_CASE("app_theme_value free function delegates to resolve (double)",
          "[mock][theme][binding][free_fn]") {
    // Arrange
    app_theme_binding<double> b{.light = 1.0, .dark = 2.0, .default_value = 5.0};

    // Act / Assert
    CHECK(app_theme_value(b, app_theme::light)       == 1.0);
    CHECK(app_theme_value(b, app_theme::dark)        == 2.0);
    CHECK(app_theme_value(b, app_theme::unspecified) == 5.0);
}

TEST_CASE("app_theme_value free function delegates to resolve (std::string)",
          "[mock][theme][binding][free_fn]") {
    // Arrange
    app_theme_binding<std::string> b{.light = "L", .dark = "D"};

    // Act / Assert
    CHECK(app_theme_value(b, app_theme::light)       == "L");
    CHECK(app_theme_value(b, app_theme::dark)        == "D");
    CHECK(app_theme_value(b, app_theme::unspecified) == "L");
}
