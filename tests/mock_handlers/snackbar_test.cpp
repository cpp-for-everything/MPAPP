// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock-handler tests for `mpapp::internal::basic_snackbar`
// (CLAUDE Rule 6 / ADR-0008).
//
// Validates that the mock `snackbar_handler<platform::mock>` records initial
// values at mapper-attach time, one entry per real property change, and that
// show()/dismiss()/action_invoked signal emissions are captured correctly.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/internal/basic_snackbar.hpp>
#include <mpapp/handlers/mock/snackbar_handler.hpp>

namespace {

using snackbar_mock = mpapp::internal::snackbar_handler<mpapp::platform::mock>;

} // namespace

// ---------------------------------------------------------------------------
// Initial-value recording
// ---------------------------------------------------------------------------

TEST_CASE("snackbar mock handler records initial text on map", "[mock][snackbar]") {
    mpapp::internal::basic_snackbar s;
    snackbar_mock h;

    h.map_text(s);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"text="});
}

TEST_CASE("snackbar mock handler records initial action_text on map", "[mock][snackbar]") {
    mpapp::internal::basic_snackbar s;
    snackbar_mock h;

    h.map_action_text(s);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"action_text="});
}

TEST_CASE("snackbar mock handler records initial duration_ms on map", "[mock][snackbar]") {
    mpapp::internal::basic_snackbar s;
    snackbar_mock h;

    h.map_duration_ms(s);

    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "duration_ms");
    CHECK(h.calls()[0].value_repr    == "3000");
}

TEST_CASE("snackbar mock handler records initial is_shown on map", "[mock][snackbar]") {
    mpapp::internal::basic_snackbar s;
    snackbar_mock h;

    h.map_is_shown(s);

    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "is_shown");
    CHECK(h.calls()[0].value_repr    == "false");
}

// ---------------------------------------------------------------------------
// Property change recording
// ---------------------------------------------------------------------------

TEST_CASE("snackbar mock handler fires once per real text change", "[mock][snackbar]") {
    mpapp::internal::basic_snackbar s;
    snackbar_mock h;

    h.map_text(s);
    h.clear_calls();

    s.text = "Upload complete";

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"text=Upload complete"});
}

TEST_CASE("snackbar mock handler ignores same-value text writes", "[mock][snackbar]") {
    mpapp::internal::basic_snackbar s;
    snackbar_mock h;

    s.text = "stable";
    h.map_text(s);
    h.clear_calls();

    s.text = "stable"; // idempotent

    REQUIRE(h.calls().empty());
}

TEST_CASE("snackbar mock handler records action_text changes", "[mock][snackbar]") {
    mpapp::internal::basic_snackbar s;
    snackbar_mock h;

    h.map_action_text(s);
    h.clear_calls();

    s.action_text = "Undo";
    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"action_text=Undo"});

    s.action_text = "Undo"; // idempotent
    REQUIRE(h.calls().size() == 1);

    s.action_text = "Dismiss";
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "Dismiss");
}

TEST_CASE("snackbar mock handler records duration_ms changes", "[mock][snackbar]") {
    mpapp::internal::basic_snackbar s;
    snackbar_mock h;

    h.map_duration_ms(s);
    h.clear_calls();

    s.duration_ms = 5000.0;
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "duration_ms");
    CHECK(h.calls()[0].value_repr    == "5000");
}

TEST_CASE("snackbar mock handler records is_shown changes", "[mock][snackbar]") {
    mpapp::internal::basic_snackbar s;
    snackbar_mock h;

    h.map_is_shown(s);
    h.clear_calls();

    s.is_shown = true;
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "true");

    s.is_shown = true;  // idempotent
    REQUIRE(h.calls().size() == 1);

    s.is_shown = false;
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "false");
}

// ---------------------------------------------------------------------------
// show() / dismiss() methods
// ---------------------------------------------------------------------------

TEST_CASE("snackbar show sets is_shown true and emits shown signal", "[mock][snackbar]") {
    // Arrange
    mpapp::internal::basic_snackbar s;
    snackbar_mock h;

    h.map_is_shown(s);
    h.map_shown(s);
    h.clear_calls();

    // Act
    s.show();

    // Assert
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].property_name == "is_shown");
    CHECK(h.calls()[0].value_repr    == "true");
    CHECK(h.calls()[0].has_value     == true);
    CHECK(h.calls()[1].property_name == "shown");
    CHECK(h.calls()[1].has_value     == false);
}

TEST_CASE("snackbar dismiss sets is_shown false and emits dismissed signal", "[mock][snackbar]") {
    // Arrange
    mpapp::internal::basic_snackbar s;
    snackbar_mock h;

    s.is_shown = true; // prime to true first
    h.map_is_shown(s);
    h.map_dismissed(s);
    h.clear_calls();

    // Act
    s.dismiss();

    // Assert
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].property_name == "is_shown");
    CHECK(h.calls()[0].value_repr    == "false");
    CHECK(h.calls()[1].property_name == "dismissed");
    CHECK(h.calls()[1].has_value     == false);
}

TEST_CASE("snackbar show then dismiss produces correct is_shown sequence", "[mock][snackbar]") {
    // Arrange
    mpapp::internal::basic_snackbar s;
    snackbar_mock h;

    h.map_is_shown(s);
    h.clear_calls();

    // Act
    s.show();
    s.dismiss();

    // Assert
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].value_repr == "true");
    CHECK(h.calls()[1].value_repr == "false");
}

TEST_CASE("snackbar calling show twice only records one is_shown change", "[mock][snackbar]") {
    // Arrange
    mpapp::internal::basic_snackbar s;
    snackbar_mock h;

    h.map_is_shown(s);
    h.map_shown(s);
    h.clear_calls();

    // Act
    s.show();
    s.show(); // is_shown already true -- Observable idempotence suppresses second change

    // Assert: is_shown fires once; shown fires twice (signal, not Observable)
    const auto strs = h.calls_as_strings();
    int is_shown_count = 0;
    int shown_count    = 0;
    for (const auto& e : strs) {
        if (e == "is_shown=true") { ++is_shown_count; }
        if (e == "shown")         { ++shown_count;    }
    }
    CHECK(is_shown_count == 1);
    CHECK(shown_count    == 2);
}

// ---------------------------------------------------------------------------
// action_invoked signal
// ---------------------------------------------------------------------------

TEST_CASE("snackbar mock handler records action_invoked on simulate_action", "[mock][snackbar]") {
    // Arrange
    mpapp::internal::basic_snackbar s;
    snackbar_mock h;

    h.map_action_invoked(s);

    // Act
    h.simulate_action(s);
    h.simulate_action(s);

    // Assert
    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"action_invoked", "action_invoked"});
}

TEST_CASE("snackbar action_invoked signal can be observed externally", "[mock][snackbar]") {
    // Arrange
    mpapp::internal::basic_snackbar s;
    int hits = 0;
    mpapp::signal_slot<> slot;
    auto cb = [&hits]() { ++hits; };
    s.action_invoked.subscribe(slot, cb);

    // Act
    s.action_invoked.emit();
    s.action_invoked.emit();

    // Assert
    CHECK(hits == 2);
}

// ---------------------------------------------------------------------------
// Signal slot auto-disconnect
// ---------------------------------------------------------------------------

TEST_CASE("snackbar shown signal slot disconnects on handler destruction", "[mock][snackbar]") {
    // Arrange
    mpapp::internal::basic_snackbar s;
    int shown_count = 0;
    mpapp::signal_slot<> ext_slot;
    auto ext_cb = [&shown_count]() { ++shown_count; };
    s.shown.subscribe(ext_slot, ext_cb);

    {
        snackbar_mock h;
        h.map_shown(s);
        s.show();
        CHECK(shown_count == 1); // external subscriber fires
    } // h destroyed -- mock slot disconnects

    // Dismiss resets state; then show again
    s.dismiss();
    s.show(); // shown emits; mock slot is gone, external still connected
    CHECK(shown_count == 2);
}

// ---------------------------------------------------------------------------
// Multi-property sequence
// ---------------------------------------------------------------------------

TEST_CASE("snackbar multi-property sequence records in order", "[mock][snackbar]") {
    // Arrange
    mpapp::internal::basic_snackbar s;
    snackbar_mock h;

    h.map_text(s);
    h.map_action_text(s);
    h.map_duration_ms(s);
    h.clear_calls();

    // Act
    s.text        = "File saved";
    s.action_text = "Open";
    s.duration_ms = 4000.0;
    s.text        = "File saved"; // idempotent

    // Assert
    REQUIRE(h.calls().size() == 3);
    CHECK(h.calls()[0].property_name == "text");
    CHECK(h.calls()[0].value_repr    == "File saved");
    CHECK(h.calls()[1].property_name == "action_text");
    CHECK(h.calls()[1].value_repr    == "Open");
    CHECK(h.calls()[2].property_name == "duration_ms");
    CHECK(h.calls()[2].value_repr    == "4000");
}
