// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Catch2 tests for the RFC-0013 Essentials permissions API.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/permissions.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// to_string helpers
// ---------------------------------------------------------------------------

TEST_CASE("to_string(permission_status) covers every enumerator", "[mock][permissions][to_string]") {
    CHECK(to_string(permission_status::unknown)    == "unknown");
    CHECK(to_string(permission_status::denied)     == "denied");
    CHECK(to_string(permission_status::disabled)   == "disabled");
    CHECK(to_string(permission_status::granted)    == "granted");
    CHECK(to_string(permission_status::restricted) == "restricted");
    CHECK(to_string(permission_status::limited)    == "limited");

    // Out-of-range (cast to an unused value) must return "?" not UB.
    CHECK(to_string(static_cast<permission_status>(0xFF)) == "?");
}

TEST_CASE("to_string(permission_type) covers every enumerator", "[mock][permissions][to_string]") {
    CHECK(to_string(permission_type::location_when_in_use) == "location_when_in_use");
    CHECK(to_string(permission_type::location_always)      == "location_always");
    CHECK(to_string(permission_type::camera)               == "camera");
    CHECK(to_string(permission_type::microphone)           == "microphone");
    CHECK(to_string(permission_type::photos)               == "photos");
    CHECK(to_string(permission_type::contacts)             == "contacts");
    CHECK(to_string(permission_type::calendar)             == "calendar");
    CHECK(to_string(permission_type::reminders)            == "reminders");
    CHECK(to_string(permission_type::sensors)              == "sensors");
    CHECK(to_string(permission_type::storage_read)         == "storage_read");
    CHECK(to_string(permission_type::storage_write)        == "storage_write");
    CHECK(to_string(permission_type::phone)                == "phone");
    CHECK(to_string(permission_type::sms)                  == "sms");
    CHECK(to_string(permission_type::bluetooth)            == "bluetooth");
    CHECK(to_string(permission_type::network_state)        == "network_state");

    // Out-of-range fallback.
    CHECK(to_string(static_cast<permission_type>(0xFF)) == "?");
}

// ---------------------------------------------------------------------------
// check_status - default and preset paths
// ---------------------------------------------------------------------------

TEST_CASE("check_status returns unknown by default for every type", "[mock][permissions][check_status]") {
    // Arrange
    mock_permissions p;

    // Act + Assert
    CHECK(p.check_status(permission_type::camera)               == permission_status::unknown);
    CHECK(p.check_status(permission_type::microphone)           == permission_status::unknown);
    CHECK(p.check_status(permission_type::location_when_in_use) == permission_status::unknown);
    CHECK(p.check_status(permission_type::location_always)      == permission_status::unknown);
    CHECK(p.check_status(permission_type::photos)               == permission_status::unknown);
    CHECK(p.check_status(permission_type::contacts)             == permission_status::unknown);
    CHECK(p.check_status(permission_type::calendar)             == permission_status::unknown);
    CHECK(p.check_status(permission_type::reminders)            == permission_status::unknown);
    CHECK(p.check_status(permission_type::sensors)              == permission_status::unknown);
    CHECK(p.check_status(permission_type::storage_read)         == permission_status::unknown);
    CHECK(p.check_status(permission_type::storage_write)        == permission_status::unknown);
    CHECK(p.check_status(permission_type::phone)                == permission_status::unknown);
    CHECK(p.check_status(permission_type::sms)                  == permission_status::unknown);
    CHECK(p.check_status(permission_type::bluetooth)            == permission_status::unknown);
    CHECK(p.check_status(permission_type::network_state)        == permission_status::unknown);
}

TEST_CASE("set_status is reflected by check_status", "[mock][permissions][check_status]") {
    // Arrange
    mock_permissions p;

    // Act
    p.set_status(permission_type::camera, permission_status::granted);
    p.set_status(permission_type::microphone, permission_status::denied);
    p.set_status(permission_type::location_when_in_use, permission_status::restricted);
    p.set_status(permission_type::photos, permission_status::limited);
    p.set_status(permission_type::contacts, permission_status::disabled);

    // Assert
    CHECK(p.check_status(permission_type::camera)               == permission_status::granted);
    CHECK(p.check_status(permission_type::microphone)           == permission_status::denied);
    CHECK(p.check_status(permission_type::location_when_in_use) == permission_status::restricted);
    CHECK(p.check_status(permission_type::photos)               == permission_status::limited);
    CHECK(p.check_status(permission_type::contacts)             == permission_status::disabled);
    // Unset types still return unknown.
    CHECK(p.check_status(permission_type::calendar)             == permission_status::unknown);
}

// ---------------------------------------------------------------------------
// request - fallback and override paths
// ---------------------------------------------------------------------------

TEST_CASE("request without a request_result echoes check_status", "[mock][permissions][request]") {
    // Arrange
    mock_permissions p;
    p.set_status(permission_type::camera, permission_status::denied);

    // Act
    permission_status result = p.request(permission_type::camera);

    // Assert
    CHECK(result == permission_status::denied);
    CHECK(p.last_requested().has_value());
    CHECK(p.last_requested().value() == permission_type::camera);
}

TEST_CASE("request uses set_request_result and updates check_status", "[mock][permissions][request]") {
    // Arrange
    mock_permissions p;
    p.set_status(permission_type::microphone, permission_status::unknown);
    p.set_request_result(permission_type::microphone, permission_status::granted);

    // Act
    permission_status result = p.request(permission_type::microphone);

    // Assert - request() returns the preset result.
    CHECK(result == permission_status::granted);
    // check_status() is promoted to match the request result.
    CHECK(p.check_status(permission_type::microphone) == permission_status::granted);
}

TEST_CASE("request on type with no status and no request_result returns unknown", "[mock][permissions][request]") {
    // Arrange
    mock_permissions p;

    // Act
    permission_status result = p.request(permission_type::bluetooth);

    // Assert
    CHECK(result == permission_status::unknown);
}

TEST_CASE("request records all types in order", "[mock][permissions][request]") {
    // Arrange
    mock_permissions p;

    // Act
    p.request(permission_type::camera);
    p.request(permission_type::microphone);
    p.request(permission_type::location_always);

    // Assert
    const auto& history = p.requested_types();
    REQUIRE(history.size() == 3);
    CHECK(history[0] == permission_type::camera);
    CHECK(history[1] == permission_type::microphone);
    CHECK(history[2] == permission_type::location_always);
    CHECK(p.last_requested().value() == permission_type::location_always);
}

TEST_CASE("request can be called multiple times on same type", "[mock][permissions][request]") {
    // Arrange
    mock_permissions p;
    p.set_request_result(permission_type::photos, permission_status::granted);

    // Act
    p.request(permission_type::photos);
    p.request(permission_type::photos);

    // Assert
    CHECK(p.requested_types().size() == 2);
    CHECK(p.last_requested().value() == permission_type::photos);
    CHECK(p.check_status(permission_type::photos) == permission_status::granted);
}

// ---------------------------------------------------------------------------
// last_requested - before and after
// ---------------------------------------------------------------------------

TEST_CASE("last_requested returns nullopt before any request", "[mock][permissions][last_requested]") {
    // Arrange
    mock_permissions p;

    // Act + Assert
    CHECK_FALSE(p.last_requested().has_value());
}

// ---------------------------------------------------------------------------
// should_show_rationale
// ---------------------------------------------------------------------------

TEST_CASE("should_show_rationale returns false by default", "[mock][permissions][rationale]") {
    // Arrange
    mock_permissions p;

    // Assert - all types default to false.
    CHECK_FALSE(p.should_show_rationale(permission_type::camera));
    CHECK_FALSE(p.should_show_rationale(permission_type::microphone));
    CHECK_FALSE(p.should_show_rationale(permission_type::location_when_in_use));
    CHECK_FALSE(p.should_show_rationale(permission_type::storage_read));
}

TEST_CASE("set_rationale true makes should_show_rationale return true", "[mock][permissions][rationale]") {
    // Arrange
    mock_permissions p;

    // Act
    p.set_rationale(permission_type::camera, true);
    p.set_rationale(permission_type::microphone, true);

    // Assert
    CHECK(p.should_show_rationale(permission_type::camera));
    CHECK(p.should_show_rationale(permission_type::microphone));
    // Unset types remain false.
    CHECK_FALSE(p.should_show_rationale(permission_type::contacts));
}

TEST_CASE("set_rationale false reverts to false", "[mock][permissions][rationale]") {
    // Arrange
    mock_permissions p;
    p.set_rationale(permission_type::sms, true);

    // Act
    p.set_rationale(permission_type::sms, false);

    // Assert
    CHECK_FALSE(p.should_show_rationale(permission_type::sms));
}

// ---------------------------------------------------------------------------
// reset
// ---------------------------------------------------------------------------

TEST_CASE("reset clears all state and history", "[mock][permissions][reset]") {
    // Arrange
    mock_permissions p;
    p.set_status(permission_type::camera, permission_status::granted);
    p.set_request_result(permission_type::camera, permission_status::granted);
    p.set_rationale(permission_type::camera, true);
    p.request(permission_type::camera);

    // Act
    p.reset();

    // Assert
    CHECK(p.check_status(permission_type::camera)         == permission_status::unknown);
    CHECK_FALSE(p.should_show_rationale(permission_type::camera));
    CHECK(p.requested_types().empty());
    CHECK_FALSE(p.last_requested().has_value());
    // request() after reset echoes the now-unknown status.
    CHECK(p.request(permission_type::camera) == permission_status::unknown);
}

// ---------------------------------------------------------------------------
// Interface polymorphism (use via base pointer)
// ---------------------------------------------------------------------------

TEST_CASE("permissions interface is usable via base pointer", "[mock][permissions][interface]") {
    // Arrange
    mock_permissions mock;
    mock.set_status(permission_type::bluetooth, permission_status::granted);
    mock.set_rationale(permission_type::bluetooth, false);

    permissions& p = mock;

    // Act + Assert via interface
    CHECK(p.check_status(permission_type::bluetooth) == permission_status::granted);
    CHECK_FALSE(p.should_show_rationale(permission_type::bluetooth));

    permission_status result = p.request(permission_type::bluetooth);
    CHECK(result == permission_status::granted);
}

// ---------------------------------------------------------------------------
// All remaining permission_type values - request + check_status coverage
// ---------------------------------------------------------------------------

TEST_CASE("all permission types can be set and requested", "[mock][permissions][all_types]") {
    // Arrange
    mock_permissions p;

    const permission_type all_types[] = {
        permission_type::location_when_in_use,
        permission_type::location_always,
        permission_type::camera,
        permission_type::microphone,
        permission_type::photos,
        permission_type::contacts,
        permission_type::calendar,
        permission_type::reminders,
        permission_type::sensors,
        permission_type::storage_read,
        permission_type::storage_write,
        permission_type::phone,
        permission_type::sms,
        permission_type::bluetooth,
        permission_type::network_state,
    };

    // Act - preset each type to granted and request it.
    for (permission_type t : all_types) {
        p.set_status(t, permission_status::granted);
        p.set_request_result(t, permission_status::granted);
    }

    for (permission_type t : all_types) {
        CHECK(p.check_status(t) == permission_status::granted);
        CHECK(p.request(t)      == permission_status::granted);
    }

    // Assert - history contains one entry per type.
    CHECK(p.requested_types().size() == 15);
}

// ---------------------------------------------------------------------------
// request_result independence from initial check_status
// ---------------------------------------------------------------------------

TEST_CASE("request_result may differ from initial check_status", "[mock][permissions][request]") {
    // Arrange - start denied, request grants.
    mock_permissions p;
    p.set_status(permission_type::contacts, permission_status::denied);
    p.set_request_result(permission_type::contacts, permission_status::granted);

    // Act
    permission_status before = p.check_status(permission_type::contacts);
    permission_status after  = p.request(permission_type::contacts);
    permission_status post   = p.check_status(permission_type::contacts);

    // Assert
    CHECK(before == permission_status::denied);
    CHECK(after  == permission_status::granted);
    CHECK(post   == permission_status::granted);
}

// ---------------------------------------------------------------------------
// All permission_status values exercised via set_status
// ---------------------------------------------------------------------------

TEST_CASE("all permission_status values round-trip through set_status", "[mock][permissions][status_values]") {
    mock_permissions p;
    const permission_type t = permission_type::calendar;

    p.set_status(t, permission_status::unknown);
    CHECK(p.check_status(t) == permission_status::unknown);

    p.set_status(t, permission_status::denied);
    CHECK(p.check_status(t) == permission_status::denied);

    p.set_status(t, permission_status::disabled);
    CHECK(p.check_status(t) == permission_status::disabled);

    p.set_status(t, permission_status::granted);
    CHECK(p.check_status(t) == permission_status::granted);

    p.set_status(t, permission_status::restricted);
    CHECK(p.check_status(t) == permission_status::restricted);

    p.set_status(t, permission_status::limited);
    CHECK(p.check_status(t) == permission_status::limited);
}
