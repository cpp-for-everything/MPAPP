// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Catch2 tests for mpapp::battery (RFC-0013 Essentials).
//
// Coverage targets: every public method, every signal emission path, the
// not-supported / default paths, and all to_string helpers.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/battery.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// to_string helpers
// ---------------------------------------------------------------------------

TEST_CASE("battery_state to_string covers every enumerator + fallback",
          "[mock][battery][enum]") {
    CHECK(to_string(battery_state::unknown)      == "unknown");
    CHECK(to_string(battery_state::charging)     == "charging");
    CHECK(to_string(battery_state::discharging)  == "discharging");
    CHECK(to_string(battery_state::full)         == "full");
    CHECK(to_string(battery_state::not_charging) == "not_charging");
    CHECK(to_string(battery_state::not_present)  == "not_present");
    CHECK(to_string(static_cast<battery_state>(99)) == "?");
}

TEST_CASE("battery_power_source to_string covers every enumerator + fallback",
          "[mock][battery][enum]") {
    CHECK(to_string(battery_power_source::unknown)  == "unknown");
    CHECK(to_string(battery_power_source::battery)  == "battery");
    CHECK(to_string(battery_power_source::ac)       == "ac");
    CHECK(to_string(battery_power_source::usb)      == "usb");
    CHECK(to_string(battery_power_source::wireless) == "wireless");
    CHECK(to_string(static_cast<battery_power_source>(99)) == "?");
}

TEST_CASE("energy_saver_status to_string covers every enumerator + fallback",
          "[mock][battery][enum]") {
    CHECK(to_string(energy_saver_status::unknown) == "unknown");
    CHECK(to_string(energy_saver_status::on)      == "on");
    CHECK(to_string(energy_saver_status::off)     == "off");
    CHECK(to_string(static_cast<energy_saver_status>(99)) == "?");
}

// ---------------------------------------------------------------------------
// battery_info value type
// ---------------------------------------------------------------------------

TEST_CASE("battery_info default-constructs to all-unknown zeros and compares by value",
          "[mock][battery][battery_info]") {
    battery_info a{};
    CHECK(a.charge_level  == 0.0);
    CHECK(a.state         == battery_state::unknown);
    CHECK(a.power_source  == battery_power_source::unknown);
    CHECK(a == battery_info{});

    battery_info b{ 0.5, battery_state::charging, battery_power_source::ac };
    CHECK_FALSE(a == b);

    battery_info c = b;
    CHECK(c == b);
}

// ---------------------------------------------------------------------------
// mock_battery — default (supported) construction
// ---------------------------------------------------------------------------

TEST_CASE("mock_battery default construction yields unknown / zero state",
          "[mock][battery]") {
    // Arrange
    mock_battery bat;

    // Act + Assert
    CHECK(bat.is_supported());
    CHECK(bat.charge_level() == 0.0);
    CHECK(bat.state()        == battery_state::unknown);
    CHECK(bat.power_source() == battery_power_source::unknown);
    CHECK(bat.energy_saver() == energy_saver_status::unknown);
}

// ---------------------------------------------------------------------------
// mock_battery — set_charge_level
// ---------------------------------------------------------------------------

TEST_CASE("set_charge_level updates charge_level and emits battery_info_changed",
          "[mock][battery]") {
    // Arrange
    mock_battery bat;
    battery_info last{};
    int hits = 0;
    signal_slot<battery_info> slot;
    auto cb = [&](battery_info info) { last = info; ++hits; };
    bat.battery_info_changed.subscribe(slot, cb);

    // Act
    bat.set_charge_level(0.75);

    // Assert
    CHECK(hits == 1);
    CHECK(bat.charge_level() == 0.75);
    CHECK(last.charge_level  == 0.75);
    CHECK(last.state         == battery_state::unknown);
}

TEST_CASE("set_charge_level with same value does NOT emit",
          "[mock][battery]") {
    // Arrange
    mock_battery bat;
    bat.set_charge_level(0.5);
    int hits = 0;
    signal_slot<battery_info> slot;
    auto cb = [&](battery_info) { ++hits; };
    bat.battery_info_changed.subscribe(slot, cb);

    // Act — set same value again
    bat.set_charge_level(0.5);

    // Assert
    CHECK(hits == 0);
}

// ---------------------------------------------------------------------------
// mock_battery — set_state
// ---------------------------------------------------------------------------

TEST_CASE("set_state updates state and emits battery_info_changed",
          "[mock][battery]") {
    // Arrange
    mock_battery bat;
    battery_state last_state = battery_state::unknown;
    int hits = 0;
    signal_slot<battery_info> slot;
    auto cb = [&](battery_info info) { last_state = info.state; ++hits; };
    bat.battery_info_changed.subscribe(slot, cb);

    // Act
    bat.set_state(battery_state::charging);

    // Assert
    CHECK(hits == 1);
    CHECK(bat.state()  == battery_state::charging);
    CHECK(last_state   == battery_state::charging);
}

TEST_CASE("set_state with same value does NOT emit",
          "[mock][battery]") {
    // Arrange
    mock_battery bat;
    bat.set_state(battery_state::full);
    int hits = 0;
    signal_slot<battery_info> slot;
    auto cb = [&](battery_info) { ++hits; };
    bat.battery_info_changed.subscribe(slot, cb);

    // Act
    bat.set_state(battery_state::full);

    // Assert
    CHECK(hits == 0);
}

// ---------------------------------------------------------------------------
// mock_battery — set_power_source
// ---------------------------------------------------------------------------

TEST_CASE("set_power_source updates power_source and emits battery_info_changed",
          "[mock][battery]") {
    // Arrange
    mock_battery bat;
    battery_power_source last_ps = battery_power_source::unknown;
    int hits = 0;
    signal_slot<battery_info> slot;
    auto cb = [&](battery_info info) { last_ps = info.power_source; ++hits; };
    bat.battery_info_changed.subscribe(slot, cb);

    // Act
    bat.set_power_source(battery_power_source::ac);

    // Assert
    CHECK(hits == 1);
    CHECK(bat.power_source() == battery_power_source::ac);
    CHECK(last_ps            == battery_power_source::ac);
}

TEST_CASE("set_power_source with same value does NOT emit",
          "[mock][battery]") {
    // Arrange
    mock_battery bat;
    bat.set_power_source(battery_power_source::usb);
    int hits = 0;
    signal_slot<battery_info> slot;
    auto cb = [&](battery_info) { ++hits; };
    bat.battery_info_changed.subscribe(slot, cb);

    // Act
    bat.set_power_source(battery_power_source::usb);

    // Assert
    CHECK(hits == 0);
}

// ---------------------------------------------------------------------------
// mock_battery — set_energy_saver
// ---------------------------------------------------------------------------

TEST_CASE("set_energy_saver updates status and emits energy_saver_status_changed",
          "[mock][battery]") {
    // Arrange
    mock_battery bat;
    energy_saver_status last = energy_saver_status::unknown;
    int hits = 0;
    signal_slot<energy_saver_status> slot;
    auto cb = [&](energy_saver_status es) { last = es; ++hits; };
    bat.energy_saver_status_changed.subscribe(slot, cb);

    // Act
    bat.set_energy_saver(energy_saver_status::on);

    // Assert
    CHECK(hits == 1);
    CHECK(bat.energy_saver() == energy_saver_status::on);
    CHECK(last               == energy_saver_status::on);
}

TEST_CASE("set_energy_saver with same value does NOT emit",
          "[mock][battery]") {
    // Arrange
    mock_battery bat;
    bat.set_energy_saver(energy_saver_status::off);
    int hits = 0;
    signal_slot<energy_saver_status> slot;
    auto cb = [&](energy_saver_status) { ++hits; };
    bat.energy_saver_status_changed.subscribe(slot, cb);

    // Act
    bat.set_energy_saver(energy_saver_status::off);

    // Assert
    CHECK(hits == 0);
}

// ---------------------------------------------------------------------------
// mock_battery — not-supported path
// ---------------------------------------------------------------------------

TEST_CASE("mock_battery constructed with supported=false returns not-present defaults",
          "[mock][battery][not_supported]") {
    // Arrange
    mock_battery bat{ false };

    // Assert
    CHECK_FALSE(bat.is_supported());
    CHECK(bat.charge_level()  < 0.0);
    CHECK(bat.state()         == battery_state::not_present);
    CHECK(bat.power_source()  == battery_power_source::unknown);
    // energy_saver is still reported (OS-level, independent of battery HW)
    CHECK(bat.energy_saver()  == energy_saver_status::unknown);
}

TEST_CASE("set_supported(false) then set_* calls are silently ignored",
          "[mock][battery][not_supported]") {
    // Arrange
    mock_battery bat;
    bat.set_charge_level(0.5);
    bat.set_state(battery_state::discharging);
    bat.set_power_source(battery_power_source::battery);

    int hits = 0;
    signal_slot<battery_info> slot;
    auto cb = [&](battery_info) { ++hits; };
    bat.battery_info_changed.subscribe(slot, cb);

    // Act — disable support, then try to mutate
    bat.set_supported(false);
    bat.set_charge_level(0.9);
    bat.set_state(battery_state::full);
    bat.set_power_source(battery_power_source::wireless);

    // Assert — no signal fired, getters return not-present defaults
    CHECK(hits == 0);
    CHECK(bat.charge_level() < 0.0);
    CHECK(bat.state()        == battery_state::not_present);
    CHECK(bat.power_source() == battery_power_source::unknown);
}

TEST_CASE("set_supported(true) re-enables mutations and getters",
          "[mock][battery][not_supported]") {
    // Arrange
    mock_battery bat{ false };

    int hits = 0;
    signal_slot<battery_info> slot;
    auto cb = [&](battery_info) { ++hits; };
    bat.battery_info_changed.subscribe(slot, cb);

    // Act — re-enable, then set values
    bat.set_supported(true);
    bat.set_charge_level(0.3);
    bat.set_state(battery_state::charging);

    // Assert
    CHECK(hits == 2);
    CHECK(bat.charge_level() == 0.3);
    CHECK(bat.state()        == battery_state::charging);
}

// ---------------------------------------------------------------------------
// multiple subscribers + slot disconnect
// ---------------------------------------------------------------------------

TEST_CASE("battery_info_changed supports multiple subscribers",
          "[mock][battery][signal]") {
    // Arrange
    mock_battery bat;
    int hits_a = 0, hits_b = 0;
    signal_slot<battery_info> slot_a, slot_b;
    auto cb_a = [&](battery_info) { ++hits_a; };
    auto cb_b = [&](battery_info) { ++hits_b; };
    bat.battery_info_changed.subscribe(slot_a, cb_a);
    bat.battery_info_changed.subscribe(slot_b, cb_b);

    // Act
    bat.set_charge_level(0.4);

    // Assert
    CHECK(hits_a == 1);
    CHECK(hits_b == 1);
}

TEST_CASE("disconnecting a slot stops further battery_info_changed delivery",
          "[mock][battery][signal]") {
    // Arrange
    mock_battery bat;
    int hits = 0;
    signal_slot<battery_info> slot;
    auto cb = [&](battery_info) { ++hits; };
    bat.battery_info_changed.subscribe(slot, cb);

    bat.set_charge_level(0.2);
    CHECK(hits == 1);

    // Act
    slot.disconnect();
    bat.set_charge_level(0.8);

    // Assert
    CHECK(hits == 1);
}

TEST_CASE("disconnecting a slot stops further energy_saver_status_changed delivery",
          "[mock][battery][signal]") {
    // Arrange
    mock_battery bat;
    int hits = 0;
    signal_slot<energy_saver_status> slot;
    auto cb = [&](energy_saver_status) { ++hits; };
    bat.energy_saver_status_changed.subscribe(slot, cb);

    bat.set_energy_saver(energy_saver_status::on);
    CHECK(hits == 1);

    // Act
    slot.disconnect();
    bat.set_energy_saver(energy_saver_status::off);

    // Assert
    CHECK(hits == 1);
}

// ---------------------------------------------------------------------------
// interface polymorphism
// ---------------------------------------------------------------------------

TEST_CASE("battery* base pointer dispatches correctly to mock_battery",
          "[mock][battery][polymorphism]") {
    // Arrange
    mock_battery concrete;
    concrete.set_charge_level(1.0);
    concrete.set_state(battery_state::full);
    concrete.set_power_source(battery_power_source::ac);
    concrete.set_energy_saver(energy_saver_status::off);

    battery* base = &concrete;

    // Assert via interface
    CHECK(base->charge_level()  == 1.0);
    CHECK(base->state()         == battery_state::full);
    CHECK(base->power_source()  == battery_power_source::ac);
    CHECK(base->energy_saver()  == energy_saver_status::off);
}

// ---------------------------------------------------------------------------
// snapshot consistency: emitted payload matches current getters
// ---------------------------------------------------------------------------

TEST_CASE("emitted battery_info snapshot is consistent with getter values",
          "[mock][battery][consistency]") {
    // Arrange
    mock_battery bat;
    bat.set_charge_level(0.6);
    bat.set_state(battery_state::discharging);
    bat.set_power_source(battery_power_source::battery);

    battery_info emitted{};
    signal_slot<battery_info> slot;
    auto cb = [&](battery_info info) { emitted = info; };
    bat.battery_info_changed.subscribe(slot, cb);

    // Act — one more change to fire the signal
    bat.set_charge_level(0.55);

    // Assert — emitted snapshot agrees with getters
    CHECK(emitted.charge_level  == bat.charge_level());
    CHECK(emitted.state         == bat.state());
    CHECK(emitted.power_source  == bat.power_source());
}
