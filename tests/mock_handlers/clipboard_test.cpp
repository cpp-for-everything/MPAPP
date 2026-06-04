// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Catch2 tests for mpapp::clipboard (RFC-0013 Essentials).

#include <optional>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/clipboard.hpp>
#include <mpapp/signal.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// Initial / default state
// ---------------------------------------------------------------------------

TEST_CASE("mock_clipboard starts empty", "[mock][clipboard]") {
    // Arrange
    mock_clipboard cb;

    // Act / Assert
    CHECK_FALSE(cb.has_text());
    CHECK_FALSE(cb.get_text().has_value());
}

// ---------------------------------------------------------------------------
// set_text / get_text round-trip
// ---------------------------------------------------------------------------

TEST_CASE("mock_clipboard set_text stores text and get_text returns it",
          "[mock][clipboard]") {
    // Arrange
    mock_clipboard cb;

    // Act
    cb.set_text("hello clipboard");

    // Assert
    REQUIRE(cb.get_text().has_value());
    CHECK(*cb.get_text() == "hello clipboard");
    CHECK(cb.has_text());
}

TEST_CASE("mock_clipboard set_text with empty string clears clipboard",
          "[mock][clipboard]") {
    // Arrange
    mock_clipboard cb;
    cb.set_text("some text");
    REQUIRE(cb.has_text());

    // Act
    cb.set_text("");

    // Assert
    CHECK_FALSE(cb.has_text());
    CHECK_FALSE(cb.get_text().has_value());
}

TEST_CASE("mock_clipboard set_text overwrites previous content",
          "[mock][clipboard]") {
    // Arrange
    mock_clipboard cb;
    cb.set_text("first");

    // Act
    cb.set_text("second");

    // Assert
    REQUIRE(cb.get_text().has_value());
    CHECK(*cb.get_text() == "second");
}

// ---------------------------------------------------------------------------
// has_text
// ---------------------------------------------------------------------------

TEST_CASE("mock_clipboard has_text reflects clipboard state",
          "[mock][clipboard]") {
    // Arrange
    mock_clipboard cb;

    // Initially empty.
    CHECK_FALSE(cb.has_text());

    // After setting text.
    cb.set_text("data");
    CHECK(cb.has_text());

    // After clearing.
    cb.set_text("");
    CHECK_FALSE(cb.has_text());
}

// ---------------------------------------------------------------------------
// Signal emission
// ---------------------------------------------------------------------------

TEST_CASE("mock_clipboard set_text emits clipboard_content_changed(true) for non-empty text",
          "[mock][clipboard][signal]") {
    // Arrange
    mock_clipboard cb;
    bool last_value   = false;
    int  emit_count   = 0;
    signal_slot<bool> slot;
    auto handler = [&](bool has) { last_value = has; ++emit_count; };
    cb.clipboard_content_changed.subscribe(slot, handler);

    // Act
    cb.set_text("some data");

    // Assert
    CHECK(emit_count == 1);
    CHECK(last_value == true);
}

TEST_CASE("mock_clipboard set_text with empty string emits clipboard_content_changed(false)",
          "[mock][clipboard][signal]") {
    // Arrange
    mock_clipboard cb;
    cb.set_text("initial");

    bool last_value = true;
    int  emit_count = 0;
    signal_slot<bool> slot;
    auto handler = [&](bool has) { last_value = has; ++emit_count; };
    cb.clipboard_content_changed.subscribe(slot, handler);

    // Act
    cb.set_text("");

    // Assert
    CHECK(emit_count == 1);
    CHECK(last_value == false);
}

TEST_CASE("mock_clipboard emits signal on every set_text call",
          "[mock][clipboard][signal]") {
    // Arrange
    mock_clipboard cb;
    int emit_count = 0;
    signal_slot<bool> slot;
    auto handler = [&](bool) { ++emit_count; };
    cb.clipboard_content_changed.subscribe(slot, handler);

    // Act
    cb.set_text("a");
    cb.set_text("b");
    cb.set_text("");

    // Assert — three distinct calls, three emissions.
    CHECK(emit_count == 3);
}

TEST_CASE("mock_clipboard signal disconnects on slot destruction",
          "[mock][clipboard][signal]") {
    // Arrange
    mock_clipboard cb;
    int emit_count = 0;

    {
        signal_slot<bool> slot;
        auto handler = [&](bool) { ++emit_count; };
        cb.clipboard_content_changed.subscribe(slot, handler);

        cb.set_text("fire once");
        CHECK(emit_count == 1);
    } // slot destroyed here — auto-unsubscribes

    // Act: emit again after slot is gone.
    cb.set_text("no listener");

    // Assert: count stays at 1.
    CHECK(emit_count == 1);
}

TEST_CASE("mock_clipboard supports multiple subscribers",
          "[mock][clipboard][signal]") {
    // Arrange
    mock_clipboard cb;
    int count_a = 0;
    int count_b = 0;
    signal_slot<bool> slot_a;
    signal_slot<bool> slot_b;
    auto handler_a = [&](bool) { ++count_a; };
    auto handler_b = [&](bool) { ++count_b; };
    cb.clipboard_content_changed.subscribe(slot_a, handler_a);
    cb.clipboard_content_changed.subscribe(slot_b, handler_b);

    // Act
    cb.set_text("broadcast");

    // Assert: both subscribers received the signal.
    CHECK(count_a == 1);
    CHECK(count_b == 1);
}

// ---------------------------------------------------------------------------
// Abstract interface polymorphism
// ---------------------------------------------------------------------------

TEST_CASE("mock_clipboard is usable through the abstract clipboard interface",
          "[mock][clipboard]") {
    // Arrange
    mock_clipboard impl;
    clipboard& iface = impl;

    // Act
    iface.set_text("via interface");

    // Assert
    REQUIRE(iface.get_text().has_value());
    CHECK(*iface.get_text() == "via interface");
    CHECK(iface.has_text());
}

TEST_CASE("clipboard interface get_text returns nullopt when empty",
          "[mock][clipboard]") {
    // Arrange
    mock_clipboard impl;
    clipboard& iface = impl;

    // Assert — not set yet.
    CHECK_FALSE(iface.has_text());
    CHECK_FALSE(iface.get_text().has_value());
}

// ---------------------------------------------------------------------------
// Signal subscriber count helper (sanity)
// ---------------------------------------------------------------------------

TEST_CASE("clipboard_content_changed subscriber_count reflects connections",
          "[mock][clipboard][signal]") {
    // Arrange
    mock_clipboard cb;

    CHECK(cb.clipboard_content_changed.subscriber_count() == 0);

    signal_slot<bool> slot;
    auto handler = [](bool) {};
    cb.clipboard_content_changed.subscribe(slot, handler);

    CHECK(cb.clipboard_content_changed.subscriber_count() == 1);

    slot.disconnect();

    CHECK(cb.clipboard_content_changed.subscriber_count() == 0);
}
