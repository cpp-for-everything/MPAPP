// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Tests for the in-memory string_localizer.
//
// Covers: add/get round-trip, culture switching + signal, invariant/fallback
// resolution, positional format() substitution, missing-key pass-through,
// and disconnect behaviour on culture_changed.

#include <string>
#include <string_view>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/localization/string_localizer.hpp>
#include <mpapp/signal.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// add / get -- basic round-trip
// ---------------------------------------------------------------------------

TEST_CASE("get returns registered string for current culture",
          "[mock][localization]") {
    // Arrange
    string_localizer loc;
    loc.add("en-US", "greeting", "Hello");
    loc.set_current_culture("en-US");

    // Act + Assert
    CHECK(loc.get("greeting") == "Hello");
}

TEST_CASE("get returns key verbatim when key is absent everywhere",
          "[mock][localization]") {
    // Arrange
    string_localizer loc;
    loc.set_current_culture("en-US");

    // Act + Assert -- no entry registered at all
    CHECK(loc.get("missing.key") == "missing.key");
}

TEST_CASE("get falls back to invariant culture when current culture lacks key",
          "[mock][localization]") {
    // Arrange
    string_localizer loc;
    loc.add("", "app_name", "MyApp");        // invariant
    loc.add("en-US", "greeting", "Hello");   // en-US only
    loc.set_current_culture("en-US");

    // Act + Assert -- "app_name" not in "en-US", should fall to invariant
    CHECK(loc.get("app_name") == "MyApp");
    CHECK(loc.get("greeting") == "Hello");
}

TEST_CASE("get with explicit fallback_culture resolves that culture first",
          "[mock][localization]") {
    // Arrange
    string_localizer loc;
    loc.add("fr-FR", "greeting", "Bonjour");
    loc.add("en-US", "greeting", "Hello");
    loc.set_current_culture("en-US");

    // Act -- ask for "fr-FR" explicitly even though current is "en-US"
    CHECK(loc.get("greeting", "fr-FR") == "Bonjour");
}

TEST_CASE("get with explicit fallback then invariant when key absent in both",
          "[mock][localization]") {
    // Arrange
    string_localizer loc;
    loc.add("", "shared", "Shared");         // invariant only
    loc.set_current_culture("en-US");        // current culture has nothing

    // Act -- fallback_culture="fr-FR" also empty; should land on invariant
    CHECK(loc.get("shared", "fr-FR") == "Shared");
}

TEST_CASE("get returns key verbatim when fallback culture and invariant both lack it",
          "[mock][localization]") {
    // Arrange
    string_localizer loc;
    loc.set_current_culture("en-US");

    // Act
    CHECK(loc.get("no.such.key", "fr-FR") == "no.such.key");
}

// ---------------------------------------------------------------------------
// Multiple cultures
// ---------------------------------------------------------------------------

TEST_CASE("multiple cultures stored independently",
          "[mock][localization]") {
    // Arrange
    string_localizer loc;
    loc.add("en-US", "color", "color");
    loc.add("en-GB", "color", "colour");

    // Act + Assert
    loc.set_current_culture("en-US");
    CHECK(loc.get("color") == "color");

    loc.set_current_culture("en-GB");
    CHECK(loc.get("color") == "colour");
}

TEST_CASE("add overwrites an existing entry in the same culture",
          "[mock][localization]") {
    // Arrange
    string_localizer loc;
    loc.add("en-US", "item", "first");
    loc.add("en-US", "item", "second");
    loc.set_current_culture("en-US");

    // Act + Assert
    CHECK(loc.get("item") == "second");
}

// ---------------------------------------------------------------------------
// set_current_culture / current_culture
// ---------------------------------------------------------------------------

TEST_CASE("current_culture reflects the last set_current_culture call",
          "[mock][localization]") {
    // Arrange
    string_localizer loc;

    // Assert default
    CHECK(loc.current_culture() == "");

    // Act
    loc.set_current_culture("de-DE");

    // Assert
    CHECK(loc.current_culture() == "de-DE");
}

// ---------------------------------------------------------------------------
// culture_changed signal
// ---------------------------------------------------------------------------

TEST_CASE("culture_changed fires when culture actually changes",
          "[mock][localization][signal]") {
    // Arrange
    string_localizer loc;
    std::string last_value;
    int call_count = 0;

    signal_slot<std::string> slot;
    auto cb = [&](std::string v) {
        last_value = std::move(v);
        ++call_count;
    };
    loc.culture_changed.subscribe(slot, cb);

    // Act
    loc.set_current_culture("fr-FR");

    // Assert
    CHECK(call_count == 1);
    CHECK(last_value == "fr-FR");

    // Act again -- different value
    loc.set_current_culture("ja-JP");

    // Assert
    CHECK(call_count == 2);
    CHECK(last_value == "ja-JP");
}

TEST_CASE("culture_changed does NOT fire when culture is set to the same value",
          "[mock][localization][signal]") {
    // Arrange
    string_localizer loc;
    loc.set_current_culture("en-US");

    int call_count = 0;
    signal_slot<std::string> slot;
    auto cb = [&](std::string) { ++call_count; };
    loc.culture_changed.subscribe(slot, cb);

    // Act -- same culture
    loc.set_current_culture("en-US");

    // Assert
    CHECK(call_count == 0);

    // Act -- different culture -- must fire
    loc.set_current_culture("en-GB");
    CHECK(call_count == 1);
}

TEST_CASE("culture_changed: multiple subscribers all receive the event",
          "[mock][localization][signal]") {
    // Arrange
    string_localizer loc;
    int hits_a = 0, hits_b = 0;

    signal_slot<std::string> slot_a, slot_b;
    auto cb_a = [&](std::string) { ++hits_a; };
    auto cb_b = [&](std::string) { ++hits_b; };
    loc.culture_changed.subscribe(slot_a, cb_a);
    loc.culture_changed.subscribe(slot_b, cb_b);

    // Act
    loc.set_current_culture("zh-CN");

    // Assert
    CHECK(hits_a == 1);
    CHECK(hits_b == 1);
}

TEST_CASE("culture_changed: disconnecting a slot stops delivery",
          "[mock][localization][signal]") {
    // Arrange
    string_localizer loc;
    int call_count = 0;

    signal_slot<std::string> slot;
    auto cb = [&](std::string) { ++call_count; };
    loc.culture_changed.subscribe(slot, cb);

    // Act -- fire while connected
    loc.set_current_culture("es-ES");
    CHECK(call_count == 1);

    // Act -- disconnect then fire
    slot.disconnect();
    loc.set_current_culture("pt-BR");

    // Assert -- count must not increase
    CHECK(call_count == 1);
}

// ---------------------------------------------------------------------------
// format() -- positional {N} substitution
// ---------------------------------------------------------------------------

TEST_CASE("format replaces {0} with the first argument",
          "[mock][localization][format]") {
    // Arrange
    string_localizer loc;
    loc.add("en-US", "hello_name", "Hello, {0}!");
    loc.set_current_culture("en-US");

    // Act + Assert
    CHECK(loc.format("hello_name", std::string("Alice")) == "Hello, Alice!");
}

TEST_CASE("format replaces multiple positional placeholders",
          "[mock][localization][format]") {
    // Arrange
    string_localizer loc;
    loc.add("en-US", "coords", "({0}, {1})");
    loc.set_current_culture("en-US");

    // Act + Assert
    CHECK(loc.format("coords", std::string("x"), std::string("y")) == "(x, y)");
}

TEST_CASE("format works with arithmetic arguments via to_string",
          "[mock][localization][format]") {
    // Arrange
    string_localizer loc;
    loc.add("en-US", "count_msg", "Count: {0}");
    loc.set_current_culture("en-US");

    // Act + Assert
    CHECK(loc.format("count_msg", 42) == "Count: 42");
}

TEST_CASE("format with string_view argument",
          "[mock][localization][format]") {
    // Arrange
    string_localizer loc;
    loc.add("en-US", "label", "Label: {0}");
    loc.set_current_culture("en-US");

    // Act + Assert
    std::string_view sv = "view";
    CHECK(loc.format("label", sv) == "Label: view");
}

TEST_CASE("format with const char* argument",
          "[mock][localization][format]") {
    // Arrange
    string_localizer loc;
    loc.add("en-US", "msg", "Msg: {0}");
    loc.set_current_culture("en-US");

    // Act + Assert
    CHECK(loc.format("msg", "literal") == "Msg: literal");
}

TEST_CASE("format passes through unknown index placeholders verbatim",
          "[mock][localization][format]") {
    // Arrange
    string_localizer loc;
    loc.add("en-US", "tmpl", "a{5}b");
    loc.set_current_culture("en-US");

    // Act + Assert -- {5} is out of range with zero args
    CHECK(loc.format("tmpl") == "a{5}b");
}

TEST_CASE("format passes through lone open brace without closing brace",
          "[mock][localization][format]") {
    // Arrange
    string_localizer loc;
    loc.add("en-US", "broken", "a{b");
    loc.set_current_culture("en-US");

    // Act + Assert -- no closing '}' so '{' is emitted verbatim
    CHECK(loc.format("broken") == "a{b");
}

TEST_CASE("format on missing key substitutes into the key itself",
          "[mock][localization][format]") {
    // Arrange
    string_localizer loc;
    loc.set_current_culture("en-US");

    // Act -- key not registered, returned verbatim, then format applied
    // The key has no {N}, so it comes back unchanged
    CHECK(loc.format("no.key", std::string("x")) == "no.key");
}

TEST_CASE("format uses the current culture for lookup",
          "[mock][localization][format]") {
    // Arrange
    string_localizer loc;
    loc.add("en-US", "greeting", "Hello, {0}!");
    loc.add("fr-FR", "greeting", "Bonjour, {0}!");
    loc.set_current_culture("fr-FR");

    // Act + Assert
    CHECK(loc.format("greeting", std::string("Marie")) == "Bonjour, Marie!");

    // Switch culture and reformat
    loc.set_current_culture("en-US");
    CHECK(loc.format("greeting", std::string("Alice")) == "Hello, Alice!");
}

TEST_CASE("format repeats the same placeholder multiple times",
          "[mock][localization][format]") {
    // Arrange
    string_localizer loc;
    loc.add("en-US", "echo", "{0} and {0}");
    loc.set_current_culture("en-US");

    // Act + Assert
    CHECK(loc.format("echo", std::string("ping")) == "ping and ping");
}

// ---------------------------------------------------------------------------
// Fallback chain integration
// ---------------------------------------------------------------------------

TEST_CASE("full fallback chain: current -> invariant -> key",
          "[mock][localization][fallback]") {
    // Arrange
    string_localizer loc;
    loc.add("", "shared",   "from-invariant");
    loc.add("en-US", "local", "from-en-US");
    loc.set_current_culture("en-US");

    // "local" -> found in en-US
    CHECK(loc.get("local")  == "from-en-US");
    // "shared" -> not in en-US, falls to invariant
    CHECK(loc.get("shared") == "from-invariant");
    // "gone" -> not in en-US, not in invariant, returns key
    CHECK(loc.get("gone")   == "gone");
}
