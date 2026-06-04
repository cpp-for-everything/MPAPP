// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for the MVVM typed message bus (messenger).

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/mvvm/messenger.hpp>

using namespace mpapp;

namespace {

struct ping_message {
    int value = 0;
};

struct text_message {
    std::string text;
};

} // namespace

TEST_CASE("messenger delivers a message to a registered recipient",
          "[mock][messenger]") {
    // Arrange
    messenger bus;
    int recipient_a = 0;
    int received    = 0;
    bus.register_handler<ping_message>(
        &recipient_a, [&received](const ping_message& m) { received = m.value; });

    // Act
    bus.send(ping_message{ 42 });

    // Assert
    CHECK(received == 42);
}

TEST_CASE("messenger delivers to multiple recipients of the same type",
          "[mock][messenger]") {
    // Arrange
    messenger bus;
    int       recipient_a = 0;
    int       recipient_b = 0;
    int       hits_a       = 0;
    int       hits_b       = 0;
    bus.register_handler<ping_message>(
        &recipient_a, [&hits_a](const ping_message&) { ++hits_a; });
    bus.register_handler<ping_message>(
        &recipient_b, [&hits_b](const ping_message&) { ++hits_b; });

    // Act
    bus.send(ping_message{ 1 });

    // Assert
    CHECK(hits_a == 1);
    CHECK(hits_b == 1);
}

TEST_CASE("messenger unregister removes only the targeted recipient",
          "[mock][messenger]") {
    // Arrange
    messenger bus;
    int       recipient_a = 0;
    int       recipient_b = 0;
    int       hits_a       = 0;
    int       hits_b       = 0;
    bus.register_handler<ping_message>(
        &recipient_a, [&hits_a](const ping_message&) { ++hits_a; });
    bus.register_handler<ping_message>(
        &recipient_b, [&hits_b](const ping_message&) { ++hits_b; });

    // Act
    bus.unregister<ping_message>(&recipient_a);
    bus.send(ping_message{ 1 });

    // Assert
    CHECK(hits_a == 0);
    CHECK(hits_b == 1);
}

TEST_CASE("messenger is_registered reflects registration state",
          "[mock][messenger]") {
    // Arrange
    messenger bus;
    int       recipient = 0;

    // Assert: not registered initially
    CHECK_FALSE(bus.is_registered<ping_message>(&recipient));

    // Act + Assert: registered after register_handler
    bus.register_handler<ping_message>(
        &recipient, [](const ping_message&) {});
    CHECK(bus.is_registered<ping_message>(&recipient));

    // is_registered is type-specific
    CHECK_FALSE(bus.is_registered<text_message>(&recipient));

    // Act + Assert: cleared after unregister
    bus.unregister<ping_message>(&recipient);
    CHECK_FALSE(bus.is_registered<ping_message>(&recipient));
}

TEST_CASE("messenger send to a type with no recipients is a no-op",
          "[mock][messenger]") {
    // Arrange
    messenger bus;

    // Act + Assert: must not throw or crash
    REQUIRE_NOTHROW(bus.send(ping_message{ 99 }));
    REQUIRE_NOTHROW(bus.send(text_message{ "unheard" }));
}

TEST_CASE("messenger keeps message types isolated", "[mock][messenger]") {
    // Arrange
    messenger   bus;
    int         recipient = 0;
    int         ping_hits = 0;
    std::string last_text;
    bus.register_handler<ping_message>(
        &recipient, [&ping_hits](const ping_message&) { ++ping_hits; });
    bus.register_handler<text_message>(
        &recipient, [&last_text](const text_message& m) { last_text = m.text; });

    // Act
    bus.send(text_message{ "hello" });

    // Assert: only the text handler fired
    CHECK(ping_hits == 0);
    CHECK(last_text == "hello");
}

TEST_CASE("messenger re-registration replaces the existing handler",
          "[mock][messenger]") {
    // Arrange
    messenger bus;
    int       recipient = 0;
    int       first     = 0;
    int       second    = 0;
    bus.register_handler<ping_message>(
        &recipient, [&first](const ping_message&) { ++first; });
    bus.register_handler<ping_message>(
        &recipient, [&second](const ping_message&) { ++second; });

    // Act
    bus.send(ping_message{ 0 });

    // Assert: only the latest handler runs, and there is still one entry
    CHECK(first == 0);
    CHECK(second == 1);
    CHECK(bus.is_registered<ping_message>(&recipient));
}

TEST_CASE("messenger unregister_all removes every subscription for a recipient",
          "[mock][messenger]") {
    // Arrange
    messenger bus;
    int       recipient_a = 0;
    int       recipient_b = 0;
    int       ping_b       = 0;
    bus.register_handler<ping_message>(
        &recipient_a, [](const ping_message&) {});
    bus.register_handler<text_message>(
        &recipient_a, [](const text_message&) {});
    bus.register_handler<ping_message>(
        &recipient_b, [&ping_b](const ping_message&) { ++ping_b; });

    // Act
    bus.unregister_all(&recipient_a);

    // Assert: recipient_a is gone everywhere; recipient_b survives
    CHECK_FALSE(bus.is_registered<ping_message>(&recipient_a));
    CHECK_FALSE(bus.is_registered<text_message>(&recipient_a));
    CHECK(bus.is_registered<ping_message>(&recipient_b));

    bus.send(ping_message{ 7 });
    CHECK(ping_b == 1);
}

TEST_CASE("messenger unregister on an unknown type/recipient is a safe no-op",
          "[mock][messenger]") {
    // Arrange
    messenger bus;
    int       known   = 0;
    int       unknown = 0;
    bus.register_handler<ping_message>(&known, [](const ping_message&) {});

    // Act + Assert: unregistering a type that exists but a recipient that does
    // not, and a type that does not exist at all, must both be safe.
    REQUIRE_NOTHROW(bus.unregister<ping_message>(&unknown));
    REQUIRE_NOTHROW(bus.unregister<text_message>(&unknown));
    REQUIRE_NOTHROW(bus.unregister_all(&unknown));

    CHECK(bus.is_registered<ping_message>(&known));
}

TEST_CASE("messenger handler may unregister itself during dispatch",
          "[mock][messenger]") {
    // Arrange
    messenger bus;
    int       recipient = 0;
    int       hits      = 0;
    bus.register_handler<ping_message>(
        &recipient, [&bus, &recipient, &hits](const ping_message&) {
            ++hits;
            bus.unregister<ping_message>(&recipient);
        });

    // Act
    bus.send(ping_message{ 1 });
    bus.send(ping_message{ 2 }); // recipient already removed

    // Assert
    CHECK(hits == 1);
    CHECK_FALSE(bus.is_registered<ping_message>(&recipient));
}
