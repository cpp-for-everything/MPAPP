// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for `mpapp::mock_dialog_service` (Page dialog services).
//
// The mock dialog service is the platform-independent surface the wrapper
// `mpapp::page` forwards to. Tests drive it directly so they stay off the
// per-platform handler library while still covering every public method and
// the programmed-response / recorded-request semantics.

#include <optional>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/dialogs.hpp>

using namespace mpapp;

TEST_CASE("dialog service starts with no recorded requests",
          "[mock][dialogs]") {
    mock_dialog_service d;
    CHECK_FALSE(d.last_alert().has_value());
    CHECK_FALSE(d.last_action_sheet().has_value());
    CHECK_FALSE(d.last_prompt().has_value());
}

TEST_CASE("single-button display_alert records request with empty accept",
          "[mock][dialogs]") {
    // Arrange
    mock_dialog_service d;

    // Act
    d.display_alert("Title", "Body", "OK");

    // Assert
    REQUIRE(d.last_alert().has_value());
    CHECK(d.last_alert()->title   == "Title");
    CHECK(d.last_alert()->message == "Body");
    CHECK(d.last_alert()->accept  == "");
    CHECK(d.last_alert()->cancel  == "OK");
}

TEST_CASE("two-button display_alert replays programmed accept result",
          "[mock][dialogs]") {
    mock_dialog_service d;
    d.set_next_alert_result(true);

    bool seen = false;
    bool got  = false;
    d.display_alert("Delete?", "Sure?", "Yes", "No",
                    [&](bool accepted) { seen = true; got = accepted; });

    CHECK(seen);
    CHECK(got);
    REQUIRE(d.last_alert().has_value());
    CHECK(d.last_alert()->accept == "Yes");
    CHECK(d.last_alert()->cancel == "No");
}

TEST_CASE("two-button display_alert defaults to false when un-primed",
          "[mock][dialogs]") {
    mock_dialog_service d;

    bool got = true;
    d.display_alert("Q", "M", "Yes", "No", [&](bool accepted) { got = accepted; });

    CHECK_FALSE(got);
}

TEST_CASE("programmed alert result is consumed once",
          "[mock][dialogs]") {
    mock_dialog_service d;
    d.set_next_alert_result(true);

    bool first = false;
    bool second = true;
    d.display_alert("a", "b", "ok", "no", [&](bool v) { first = v; });
    d.display_alert("a", "b", "ok", "no", [&](bool v) { second = v; });

    CHECK(first);          // primed value
    CHECK_FALSE(second);   // reverted to default
}

TEST_CASE("two-button display_alert tolerates an empty callback",
          "[mock][dialogs]") {
    mock_dialog_service d;
    d.set_next_alert_result(true);
    // No callback supplied - must not crash, request still recorded.
    d.display_alert("a", "b", "ok", "no", {});
    REQUIRE(d.last_alert().has_value());
    CHECK(d.last_alert()->title == "a");
}

TEST_CASE("display_action_sheet records request and replays choice",
          "[mock][dialogs]") {
    mock_dialog_service d;
    d.set_next_action_sheet_result("Email");

    std::string chosen;
    d.display_action_sheet("Share via", "Cancel", "Delete",
                           {"Email", "SMS"},
                           [&](std::string c) { chosen = std::move(c); });

    CHECK(chosen == "Email");
    REQUIRE(d.last_action_sheet().has_value());
    CHECK(d.last_action_sheet()->title       == "Share via");
    CHECK(d.last_action_sheet()->cancel      == "Cancel");
    CHECK(d.last_action_sheet()->destruction == "Delete");
    REQUIRE(d.last_action_sheet()->buttons.size() == 2);
    CHECK(d.last_action_sheet()->buttons[0] == "Email");
    CHECK(d.last_action_sheet()->buttons[1] == "SMS");
}

TEST_CASE("display_action_sheet defaults to the cancel label when un-primed",
          "[mock][dialogs]") {
    mock_dialog_service d;

    std::string chosen = "unset";
    d.display_action_sheet("T", "Cancel", "", {"A", "B"},
                           [&](std::string c) { chosen = std::move(c); });

    CHECK(chosen == "Cancel");
}

TEST_CASE("action sheet programmed result is consumed once",
          "[mock][dialogs]") {
    mock_dialog_service d;
    d.set_next_action_sheet_result("A");

    std::string first = "?", second = "?";
    d.display_action_sheet("t", "Cancel", "", {"A"}, [&](std::string c) { first = c; });
    d.display_action_sheet("t", "Cancel", "", {"A"}, [&](std::string c) { second = c; });

    CHECK(first  == "A");
    CHECK(second == "Cancel");
}

TEST_CASE("display_action_sheet tolerates an empty callback",
          "[mock][dialogs]") {
    mock_dialog_service d;
    d.display_action_sheet("t", "Cancel", "", {"A"}, {});
    REQUIRE(d.last_action_sheet().has_value());
}

TEST_CASE("display_prompt records request with defaulted labels",
          "[mock][dialogs]") {
    mock_dialog_service d;
    d.set_next_prompt_result(std::string{"hello"});

    std::optional<std::string> result;
    d.display_prompt("Name?", "Enter your name",
                     [&](std::optional<std::string> r) { result = std::move(r); });

    REQUIRE(result.has_value());
    CHECK(*result == "hello");
    REQUIRE(d.last_prompt().has_value());
    CHECK(d.last_prompt()->title         == "Name?");
    CHECK(d.last_prompt()->message       == "Enter your name");
    CHECK(d.last_prompt()->accept        == "OK");
    CHECK(d.last_prompt()->cancel        == "Cancel");
    CHECK(d.last_prompt()->placeholder   == "");
    CHECK(d.last_prompt()->initial_value == "");
}

TEST_CASE("display_prompt records overridden labels and field values",
          "[mock][dialogs]") {
    mock_dialog_service d;
    d.set_next_prompt_result(std::string{"42"});

    std::optional<std::string> result;
    d.display_prompt("Age", "How old?",
                     [&](std::optional<std::string> r) { result = std::move(r); },
                     "Submit", "Dismiss", "years", "18");

    REQUIRE(result.has_value());
    CHECK(*result == "42");
    REQUIRE(d.last_prompt().has_value());
    CHECK(d.last_prompt()->accept        == "Submit");
    CHECK(d.last_prompt()->cancel        == "Dismiss");
    CHECK(d.last_prompt()->placeholder   == "years");
    CHECK(d.last_prompt()->initial_value == "18");
}

TEST_CASE("display_prompt defaults to nullopt when un-primed (cancelled)",
          "[mock][dialogs]") {
    mock_dialog_service d;

    std::optional<std::string> result = std::string{"sentinel"};
    d.display_prompt("t", "m",
                     [&](std::optional<std::string> r) { result = std::move(r); });

    CHECK_FALSE(result.has_value());
}

TEST_CASE("display_prompt can be primed with an explicit nullopt cancel",
          "[mock][dialogs]") {
    mock_dialog_service d;
    d.set_next_prompt_result(std::nullopt);

    bool invoked = false;
    std::optional<std::string> result = std::string{"sentinel"};
    d.display_prompt("t", "m", [&](std::optional<std::string> r) {
        invoked = true;
        result = std::move(r);
    });

    CHECK(invoked);
    CHECK_FALSE(result.has_value());
}

TEST_CASE("prompt programmed result is consumed once",
          "[mock][dialogs]") {
    mock_dialog_service d;
    d.set_next_prompt_result(std::string{"primed"});

    std::optional<std::string> first, second = std::string{"x"};
    d.display_prompt("t", "m", [&](std::optional<std::string> r) { first = std::move(r); });
    d.display_prompt("t", "m", [&](std::optional<std::string> r) { second = std::move(r); });

    REQUIRE(first.has_value());
    CHECK(*first == "primed");
    CHECK_FALSE(second.has_value());   // reverted to default (nullopt)
}

TEST_CASE("display_prompt tolerates an empty callback",
          "[mock][dialogs]") {
    mock_dialog_service d;
    d.set_next_prompt_result(std::string{"x"});
    d.display_prompt("t", "m", {});
    REQUIRE(d.last_prompt().has_value());
}

TEST_CASE("clear() drops all recorded requests",
          "[mock][dialogs]") {
    mock_dialog_service d;
    d.display_alert("a", "b", "c");
    d.display_action_sheet("t", "Cancel", "", {"A"}, {});
    d.display_prompt("t", "m", {});
    REQUIRE(d.last_alert().has_value());
    REQUIRE(d.last_action_sheet().has_value());
    REQUIRE(d.last_prompt().has_value());

    d.clear();

    CHECK_FALSE(d.last_alert().has_value());
    CHECK_FALSE(d.last_action_sheet().has_value());
    CHECK_FALSE(d.last_prompt().has_value());
}

TEST_CASE("request value types compare by member equality",
          "[mock][dialogs]") {
    CHECK(alert_request{"t", "m", "", "c"} == alert_request{"t", "m", "", "c"});
    CHECK_FALSE(alert_request{"t", "m", "", "c"} == alert_request{"t", "m", "a", "c"});

    action_sheet_request a{"t", "Cancel", "", {"A", "B"}};
    CHECK(a == action_sheet_request{"t", "Cancel", "", {"A", "B"}});
    CHECK_FALSE(a == action_sheet_request{"t", "Cancel", "", {"A"}});

    prompt_request p{"t", "m", "OK", "Cancel", "", ""};
    CHECK(p == prompt_request{"t", "m", "OK", "Cancel", "", ""});
    CHECK_FALSE(p == prompt_request{"t", "m", "OK", "Cancel", "ph", ""});
}
