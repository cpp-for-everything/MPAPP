// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for the RFC-0014 asset_catalog subsystem.
//
// Covers: asset_kind to_string, register_asset, find, by_kind, resolve_path,
// missing-key behaviour, overwrite semantics, asset_registered signal, and
// count().  Follows the AAA pattern used throughout the mock_handlers suite.

#include <optional>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/assets/asset_catalog.hpp>
#include <mpapp/signal.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// to_string helpers
// ---------------------------------------------------------------------------

TEST_CASE("asset_kind to_string covers every case and out-of-range fallback",
          "[mock][asset_catalog][enum]") {
    CHECK(to_string(asset_kind::image)    == "image");
    CHECK(to_string(asset_kind::font)     == "font");
    CHECK(to_string(asset_kind::app_icon) == "app_icon");
    CHECK(to_string(asset_kind::splash)   == "splash");
    CHECK(to_string(static_cast<asset_kind>(99)) == "?");
}

// ---------------------------------------------------------------------------
// Default state
// ---------------------------------------------------------------------------

TEST_CASE("asset_catalog is empty after default construction",
          "[mock][asset_catalog][defaults]") {
    // Arrange + Assert
    asset_catalog cat;
    CHECK(cat.count() == 0);
}

// ---------------------------------------------------------------------------
// register_asset + find
// ---------------------------------------------------------------------------

TEST_CASE("register_asset stores an entry retrievable by find",
          "[mock][asset_catalog][register][find]") {
    // Arrange
    asset_catalog cat;
    asset_entry e;
    e.key   = "logo";
    e.path  = "Resources/Images/logo.png";
    e.kind  = asset_kind::image;
    e.alias = "Logo";

    // Act
    cat.register_asset(e);

    // Assert
    auto result = cat.find("logo");
    REQUIRE(result.has_value());
    CHECK(result->key   == "logo");
    CHECK(result->path  == "Resources/Images/logo.png");
    CHECK(result->kind  == asset_kind::image);
    CHECK(result->alias == "Logo");
    CHECK(cat.count()   == 1);
}

TEST_CASE("find returns nullopt for an unregistered key",
          "[mock][asset_catalog][find][missing]") {
    // Arrange
    asset_catalog cat;

    // Assert
    CHECK_FALSE(cat.find("nonexistent").has_value());
}

TEST_CASE("register_asset with empty alias stores empty alias",
          "[mock][asset_catalog][register]") {
    // Arrange
    asset_catalog cat;
    asset_entry e;
    e.key  = "icon";
    e.path = "Resources/icon.png";
    e.kind = asset_kind::app_icon;
    // alias left default (empty)

    // Act
    cat.register_asset(e);

    // Assert
    auto result = cat.find("icon");
    REQUIRE(result.has_value());
    CHECK(result->alias.empty());
}

// ---------------------------------------------------------------------------
// Overwrite semantics
// ---------------------------------------------------------------------------

TEST_CASE("registering an existing key overwrites the previous entry",
          "[mock][asset_catalog][register][overwrite]") {
    // Arrange
    asset_catalog cat;
    asset_entry first;
    first.key  = "splash";
    first.path = "Resources/splash_v1.png";
    first.kind = asset_kind::splash;
    cat.register_asset(first);

    asset_entry second;
    second.key  = "splash";
    second.path = "Resources/splash_v2.png";
    second.kind = asset_kind::splash;

    // Act
    cat.register_asset(second);

    // Assert
    REQUIRE(cat.count() == 1);
    auto result = cat.find("splash");
    REQUIRE(result.has_value());
    CHECK(result->path == "Resources/splash_v2.png");
}

// ---------------------------------------------------------------------------
// by_kind
// ---------------------------------------------------------------------------

TEST_CASE("by_kind returns only entries matching the requested kind",
          "[mock][asset_catalog][by_kind]") {
    // Arrange
    asset_catalog cat;

    asset_entry img;
    img.key  = "logo";
    img.path = "Resources/Images/logo.png";
    img.kind = asset_kind::image;
    cat.register_asset(img);

    asset_entry fnt;
    fnt.key  = "opensans";
    fnt.path = "Resources/Fonts/OpenSans-Regular.ttf";
    fnt.kind = asset_kind::font;
    cat.register_asset(fnt);

    asset_entry splash;
    splash.key  = "splash";
    splash.path = "Resources/splash.png";
    splash.kind = asset_kind::splash;
    cat.register_asset(splash);

    // Act
    auto images  = cat.by_kind(asset_kind::image);
    auto fonts   = cat.by_kind(asset_kind::font);
    auto icons   = cat.by_kind(asset_kind::app_icon);
    auto splashes = cat.by_kind(asset_kind::splash);

    // Assert
    REQUIRE(images.size()   == 1);
    CHECK(images[0].key     == "logo");

    REQUIRE(fonts.size()    == 1);
    CHECK(fonts[0].key      == "opensans");

    CHECK(icons.empty());

    REQUIRE(splashes.size() == 1);
    CHECK(splashes[0].key   == "splash");
}

TEST_CASE("by_kind returns empty vector when no entries match",
          "[mock][asset_catalog][by_kind][empty]") {
    // Arrange
    asset_catalog cat;

    asset_entry e;
    e.key  = "img";
    e.path = "a.png";
    e.kind = asset_kind::image;
    cat.register_asset(e);

    // Assert
    CHECK(cat.by_kind(asset_kind::font).empty());
    CHECK(cat.by_kind(asset_kind::app_icon).empty());
    CHECK(cat.by_kind(asset_kind::splash).empty());
}

// ---------------------------------------------------------------------------
// resolve_path
// ---------------------------------------------------------------------------

TEST_CASE("resolve_path returns the path for a registered key",
          "[mock][asset_catalog][resolve_path]") {
    // Arrange
    asset_catalog cat;
    asset_entry e;
    e.key  = "hero";
    e.path = "Resources/Images/hero.png";
    e.kind = asset_kind::image;
    cat.register_asset(e);

    // Act
    auto path = cat.resolve_path("hero");

    // Assert
    REQUIRE(path.has_value());
    CHECK(*path == "Resources/Images/hero.png");
}

TEST_CASE("resolve_path returns nullopt for a missing key",
          "[mock][asset_catalog][resolve_path][missing]") {
    // Arrange
    asset_catalog cat;

    // Assert
    CHECK_FALSE(cat.resolve_path("ghost").has_value());
}

// ---------------------------------------------------------------------------
// asset_registered signal
// ---------------------------------------------------------------------------

TEST_CASE("register_asset fires asset_registered with the stored entry",
          "[mock][asset_catalog][signal]") {
    // Arrange
    asset_catalog cat;

    std::string    received_key;
    asset_kind     received_kind = asset_kind::image;
    int            call_count    = 0;

    signal_slot<const asset_entry&> slot;
    auto cb = [&](const asset_entry& e) {
        received_key  = e.key;
        received_kind = e.kind;
        ++call_count;
    };
    cat.asset_registered.subscribe(slot, cb);

    asset_entry e;
    e.key  = "fonts/roboto";
    e.path = "Resources/Fonts/Roboto.ttf";
    e.kind = asset_kind::font;

    // Act
    cat.register_asset(e);

    // Assert
    CHECK(call_count    == 1);
    CHECK(received_key  == "fonts/roboto");
    CHECK(received_kind == asset_kind::font);
}

TEST_CASE("asset_registered fires again on overwrite",
          "[mock][asset_catalog][signal][overwrite]") {
    // Arrange
    asset_catalog cat;
    int call_count = 0;

    signal_slot<const asset_entry&> slot;
    auto cb = [&](const asset_entry&) { ++call_count; };
    cat.asset_registered.subscribe(slot, cb);

    asset_entry a;
    a.key  = "k";
    a.path = "v1.png";
    a.kind = asset_kind::image;

    asset_entry b;
    b.key  = "k";
    b.path = "v2.png";
    b.kind = asset_kind::image;

    // Act
    cat.register_asset(a);
    cat.register_asset(b);

    // Assert
    CHECK(call_count == 2);
}

TEST_CASE("disconnecting signal slot stops delivery",
          "[mock][asset_catalog][signal][disconnect]") {
    // Arrange
    asset_catalog cat;
    int call_count = 0;

    signal_slot<const asset_entry&> slot;
    auto cb = [&](const asset_entry&) { ++call_count; };
    cat.asset_registered.subscribe(slot, cb);

    asset_entry a;
    a.key  = "a";
    a.path = "a.png";
    a.kind = asset_kind::image;
    cat.register_asset(a);
    CHECK(call_count == 1);

    // Act - disconnect
    slot.disconnect();

    asset_entry b;
    b.key  = "b";
    b.path = "b.png";
    b.kind = asset_kind::image;
    cat.register_asset(b);

    // Assert
    CHECK(call_count == 1);
}

// ---------------------------------------------------------------------------
// count / multiple registrations
// ---------------------------------------------------------------------------

TEST_CASE("count reflects the number of unique keys registered",
          "[mock][asset_catalog][count]") {
    // Arrange
    asset_catalog cat;
    CHECK(cat.count() == 0);

    auto make = [](const char* key, asset_kind k) {
        asset_entry e;
        e.key  = key;
        e.path = "x";
        e.kind = k;
        return e;
    };

    // Act + Assert
    cat.register_asset(make("a", asset_kind::image));
    CHECK(cat.count() == 1);

    cat.register_asset(make("b", asset_kind::font));
    CHECK(cat.count() == 2);

    // Re-registering "a" must not increase count.
    cat.register_asset(make("a", asset_kind::splash));
    CHECK(cat.count() == 2);
}
