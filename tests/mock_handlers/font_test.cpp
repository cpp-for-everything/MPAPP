// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for the RFC-0012 fonts subsystem.

#include <string>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/fonts/font.hpp>
#include <mpapp/fonts/font_registry.hpp>

using namespace mpapp;

TEST_CASE("font value type + builder helpers", "[mock][font]") {
    font f = font::of_size("OpenSans", 18.0);
    CHECK(f.family == "OpenSans");
    CHECK(f.size == 18.0);
    CHECK(f.weight == font_weight::regular);
    CHECK(f.slant == font_slant::normal);
    CHECK_FALSE(f.is_bold());
    CHECK_FALSE(f.is_italic());

    font bold_italic = f.with_weight(font_weight::bold).with_slant(font_slant::italic);
    CHECK(bold_italic.is_bold());
    CHECK(bold_italic.is_italic());
    CHECK(bold_italic.size == 18.0);     // unchanged
    CHECK(f.weight == font_weight::regular); // original immutable

    CHECK(font::of_size("A", 10.0).with_size(12.0).size == 12.0);
}

TEST_CASE("font equality", "[mock][font]") {
    CHECK(font::of_size("Roboto", 14.0) == font::of_size("Roboto", 14.0));
    CHECK_FALSE(font::of_size("Roboto", 14.0) == font::of_size("Roboto", 15.0));
    CHECK_FALSE(font::of_size("Roboto", 14.0)
                == font::of_size("Roboto", 14.0).with_weight(font_weight::bold));
}

TEST_CASE("font_registry registers + resolves aliases", "[mock][font][registry]") {
    font_registry r;
    r.add_font("OpenSans-Regular.ttf", "OpenSans")
     .add_font("MaterialIcons.ttf", "Icons");

    CHECK(r.count() == 2);
    CHECK(r.has_alias("OpenSans"));
    CHECK(r.resolve("OpenSans") == "OpenSans-Regular.ttf");
    CHECK(r.resolve("Icons")    == "MaterialIcons.ttf");

    CHECK_FALSE(r.has_alias("Missing"));
    CHECK_FALSE(r.resolve("Missing").has_value());

    // Re-registering overwrites.
    r.add_font("OpenSans-v2.ttf", "OpenSans");
    CHECK(r.resolve("OpenSans") == "OpenSans-v2.ttf");
    CHECK(r.count() == 2);
}

TEST_CASE("configure_fonts applies a configuration callable",
          "[mock][font][registry]") {
    font_registry r;
    configure_fonts(r, [](font_registry& reg) {
        reg.add_font("A.ttf", "A").add_font("B.ttf", "B");
    });
    CHECK(r.count() == 2);
    CHECK(r.has_alias("A"));
    CHECK(r.has_alias("B"));
}
