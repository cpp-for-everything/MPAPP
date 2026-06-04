// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Catch2 tests for the RFC-0013 AppActions Essentials module.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/app_actions.hpp>
#include <mpapp/signal.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static app_action make_action(std::string id, std::string title,
                               std::string subtitle = {}, std::string icon = {}) {
    return app_action{ std::move(id), std::move(title),
                       std::move(subtitle), std::move(icon) };
}

// ---------------------------------------------------------------------------
// app_action value type
// ---------------------------------------------------------------------------

TEST_CASE("app_action default construction yields empty strings", "[mock][app_actions][struct]") {
    // Arrange / Act
    app_action a;

    // Assert
    CHECK(a.id.empty());
    CHECK(a.title.empty());
    CHECK(a.subtitle.empty());
    CHECK(a.icon.empty());
}

TEST_CASE("app_action equality operator", "[mock][app_actions][struct]") {
    // Arrange
    app_action a = make_action("share", "Share", "Share this", "share.png");
    app_action b = make_action("share", "Share", "Share this", "share.png");
    app_action c = make_action("open",  "Open");

    // Assert
    CHECK(a == b);
    CHECK_FALSE(a == c);

    // Mutate one field and ensure equality is broken
    b.title = "Different";
    CHECK_FALSE(a == b);
}

// ---------------------------------------------------------------------------
// mock_app_actions - is_supported
// ---------------------------------------------------------------------------

TEST_CASE("mock_app_actions defaults to supported", "[mock][app_actions]") {
    // Arrange / Act
    mock_app_actions m;

    // Assert
    CHECK(m.is_supported());
}

TEST_CASE("mock_app_actions can be constructed as not-supported", "[mock][app_actions]") {
    // Arrange / Act
    mock_app_actions m{ false };

    // Assert
    CHECK_FALSE(m.is_supported());
}

TEST_CASE("mock_app_actions set_supported changes the flag", "[mock][app_actions]") {
    // Arrange
    mock_app_actions m{ true };

    // Act
    m.set_supported(false);

    // Assert
    CHECK_FALSE(m.is_supported());

    // Act again
    m.set_supported(true);

    // Assert
    CHECK(m.is_supported());
}

// ---------------------------------------------------------------------------
// mock_app_actions - get / set
// ---------------------------------------------------------------------------

TEST_CASE("get returns empty list when no actions set", "[mock][app_actions]") {
    // Arrange
    mock_app_actions m;

    // Act / Assert
    CHECK(m.get().empty());
}

TEST_CASE("set stores actions and get returns them (supported)", "[mock][app_actions]") {
    // Arrange
    mock_app_actions m;
    std::vector<app_action> actions = {
        make_action("new",   "New",   "Create new item", "new.png"),
        make_action("open",  "Open",  "Open a file",     "open.png"),
        make_action("share", "Share", "",                "share.png"),
    };

    // Act
    m.set(actions);

    // Assert
    const auto stored = m.get();
    REQUIRE(stored.size() == 3);
    CHECK(stored[0] == actions[0]);
    CHECK(stored[1] == actions[1]);
    CHECK(stored[2] == actions[2]);
}

TEST_CASE("set replaces the previous list", "[mock][app_actions]") {
    // Arrange
    mock_app_actions m;
    m.set({ make_action("old", "Old Action") });
    REQUIRE(m.get().size() == 1);

    std::vector<app_action> replacement = {
        make_action("new1", "First"),
        make_action("new2", "Second"),
    };

    // Act
    m.set(replacement);

    // Assert
    const auto stored = m.get();
    REQUIRE(stored.size() == 2);
    CHECK(stored[0].id == "new1");
    CHECK(stored[1].id == "new2");
}

TEST_CASE("set with empty list clears all actions", "[mock][app_actions]") {
    // Arrange
    mock_app_actions m;
    m.set({ make_action("a", "A"), make_action("b", "B") });
    REQUIRE(m.get().size() == 2);

    // Act
    m.set({});

    // Assert
    CHECK(m.get().empty());
}

// ---------------------------------------------------------------------------
// mock_app_actions - not-supported path
// ---------------------------------------------------------------------------

TEST_CASE("set is ignored (no storage) when not supported", "[mock][app_actions]") {
    // Arrange
    mock_app_actions m{ false };
    std::vector<app_action> actions = { make_action("x", "X") };

    // Act
    m.set(actions);

    // Assert - actions are NOT stored because not supported
    CHECK(m.get().empty());
}

TEST_CASE("last_set records the argument even when not supported", "[mock][app_actions]") {
    // Arrange
    mock_app_actions m{ false };
    std::vector<app_action> actions = { make_action("y", "Y") };

    // Act
    m.set(actions);

    // Assert - last_set_ captures the call regardless
    REQUIRE(m.last_set().has_value());
    const auto& recorded = *m.last_set();
    REQUIRE(recorded.size() == 1);
    CHECK(recorded[0].id == "y");
}

// ---------------------------------------------------------------------------
// mock_app_actions - last_set
// ---------------------------------------------------------------------------

TEST_CASE("last_set returns nullopt before any set call", "[mock][app_actions]") {
    // Arrange / Act
    mock_app_actions m;

    // Assert
    CHECK_FALSE(m.last_set().has_value());
}

TEST_CASE("last_set tracks the most recent set call", "[mock][app_actions]") {
    // Arrange
    mock_app_actions m;
    m.set({ make_action("first", "First") });

    // Act
    m.set({ make_action("second", "Second"), make_action("third", "Third") });

    // Assert
    REQUIRE(m.last_set().has_value());
    const auto& last = *m.last_set();
    REQUIRE(last.size() == 2);
    CHECK(last[0].id == "second");
    CHECK(last[1].id == "third");
}

// ---------------------------------------------------------------------------
// mock_app_actions - trigger + signal emission
// ---------------------------------------------------------------------------

TEST_CASE("trigger fires app_action_activated with the correct action", "[mock][app_actions][signal]") {
    // Arrange
    mock_app_actions m;
    m.set({
        make_action("share", "Share", "Share now", "share.png"),
        make_action("open",  "Open"),
    });

    std::optional<app_action> received;
    int hit_count = 0;
    signal_slot<app_action> slot;
    auto cb = [&](const app_action& a) {
        received = a;
        ++hit_count;
    };
    m.app_action_activated.subscribe(slot, cb);

    // Act
    m.trigger("share");

    // Assert
    REQUIRE(hit_count == 1);
    REQUIRE(received.has_value());
    CHECK(received->id == "share");
    CHECK(received->title == "Share");
    CHECK(received->subtitle == "Share now");
    CHECK(received->icon == "share.png");
}

TEST_CASE("trigger with unknown id does not emit", "[mock][app_actions][signal]") {
    // Arrange
    mock_app_actions m;
    m.set({ make_action("known", "Known") });

    int hit_count = 0;
    signal_slot<app_action> slot;
    auto cb = [&](const app_action&) { ++hit_count; };
    m.app_action_activated.subscribe(slot, cb);

    // Act
    m.trigger("unknown-id");

    // Assert
    CHECK(hit_count == 0);
}

TEST_CASE("trigger on empty action list does not emit", "[mock][app_actions][signal]") {
    // Arrange
    mock_app_actions m;  // no actions set

    int hit_count = 0;
    signal_slot<app_action> slot;
    auto cb = [&](const app_action&) { ++hit_count; };
    m.app_action_activated.subscribe(slot, cb);

    // Act
    m.trigger("any");

    // Assert
    CHECK(hit_count == 0);
}

TEST_CASE("multiple triggers each fire independently", "[mock][app_actions][signal]") {
    // Arrange
    mock_app_actions m;
    m.set({
        make_action("a", "Action A"),
        make_action("b", "Action B"),
    });

    std::vector<std::string> fired_ids;
    signal_slot<app_action> slot;
    auto cb = [&](const app_action& a) { fired_ids.push_back(a.id); };
    m.app_action_activated.subscribe(slot, cb);

    // Act
    m.trigger("a");
    m.trigger("b");
    m.trigger("a");

    // Assert
    REQUIRE(fired_ids.size() == 3);
    CHECK(fired_ids[0] == "a");
    CHECK(fired_ids[1] == "b");
    CHECK(fired_ids[2] == "a");
}

TEST_CASE("multiple subscribers all receive the signal", "[mock][app_actions][signal]") {
    // Arrange
    mock_app_actions m;
    m.set({ make_action("ping", "Ping") });

    int hits1 = 0, hits2 = 0;
    signal_slot<app_action> slot1, slot2;
    auto cb1 = [&](const app_action&) { ++hits1; };
    auto cb2 = [&](const app_action&) { ++hits2; };
    m.app_action_activated.subscribe(slot1, cb1);
    m.app_action_activated.subscribe(slot2, cb2);

    // Act
    m.trigger("ping");

    // Assert
    CHECK(hits1 == 1);
    CHECK(hits2 == 1);
}

TEST_CASE("disconnecting slot stops further emissions", "[mock][app_actions][signal]") {
    // Arrange
    mock_app_actions m;
    m.set({ make_action("go", "Go") });

    int hit_count = 0;
    signal_slot<app_action> slot;
    auto cb = [&](const app_action&) { ++hit_count; };
    m.app_action_activated.subscribe(slot, cb);

    m.trigger("go");
    REQUIRE(hit_count == 1);

    // Act - disconnect
    slot.disconnect();
    m.trigger("go");

    // Assert - no additional emissions
    CHECK(hit_count == 1);
}

// ---------------------------------------------------------------------------
// Interface polymorphism
// ---------------------------------------------------------------------------

TEST_CASE("app_actions interface pointer works correctly via mock", "[mock][app_actions][interface]") {
    // Arrange
    app_actions* api = new mock_app_actions{};
    std::vector<app_action> actions = {
        make_action("share", "Share"),
        make_action("open",  "Open"),
    };

    // Act
    api->set(actions);

    // Assert
    CHECK(api->is_supported());
    const auto result = api->get();
    REQUIRE(result.size() == 2);
    CHECK(result[0].id == "share");
    CHECK(result[1].id == "open");

    delete api;
}
