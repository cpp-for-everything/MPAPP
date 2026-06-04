// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for the RFC-0013 Essentials device_display.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/device_display.hpp>
#include <mpapp/signal.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static display_info make_info(double w, double h, double density = 1.0,
                               double rate = 60.0,
                               display_orientation ori = display_orientation::portrait,
                               display_rotation   rot = display_rotation::rotation_0) {
    return display_info{ w, h, density, rate, ori, rot };
}

// ---------------------------------------------------------------------------
// display_orientation to_string
// ---------------------------------------------------------------------------

TEST_CASE("display_orientation to_string covers all values",
          "[mock][device_display][to_string]") {
    CHECK(to_string(display_orientation::unknown)   == "unknown");
    CHECK(to_string(display_orientation::portrait)  == "portrait");
    CHECK(to_string(display_orientation::landscape) == "landscape");
}

// ---------------------------------------------------------------------------
// display_rotation to_string
// ---------------------------------------------------------------------------

TEST_CASE("display_rotation to_string covers all values",
          "[mock][device_display][to_string]") {
    CHECK(to_string(display_rotation::rotation_0)   == "rotation_0");
    CHECK(to_string(display_rotation::rotation_90)  == "rotation_90");
    CHECK(to_string(display_rotation::rotation_180) == "rotation_180");
    CHECK(to_string(display_rotation::rotation_270) == "rotation_270");
}

// ---------------------------------------------------------------------------
// display_info value type
// ---------------------------------------------------------------------------

TEST_CASE("display_info default construction",
          "[mock][device_display][display_info]") {
    display_info d{};
    CHECK(d.width       == 0.0);
    CHECK(d.height      == 0.0);
    CHECK(d.density     == 1.0);
    CHECK(d.rate        == 60.0);
    CHECK(d.orientation == display_orientation::unknown);
    CHECK(d.rotation    == display_rotation::rotation_0);
}

TEST_CASE("display_info equality and inequality",
          "[mock][device_display][display_info]") {
    display_info a = make_info(1920.0, 1080.0);
    display_info b = a;

    CHECK(a == b);

    b.width = 2560.0;
    CHECK_FALSE(a == b);

    b = a;
    b.height = 1440.0;
    CHECK_FALSE(a == b);

    b = a;
    b.density = 2.0;
    CHECK_FALSE(a == b);

    b = a;
    b.rate = 120.0;
    CHECK_FALSE(a == b);

    b = a;
    b.orientation = display_orientation::landscape;
    CHECK_FALSE(a == b);

    b = a;
    b.rotation = display_rotation::rotation_90;
    CHECK_FALSE(a == b);
}

// ---------------------------------------------------------------------------
// mock_device_display — basic state
// ---------------------------------------------------------------------------

TEST_CASE("mock_device_display default-constructed has zero-size display",
          "[mock][device_display]") {
    mock_device_display dd{};

    const auto info = dd.main_display_info();
    CHECK(info.width       == 0.0);
    CHECK(info.height      == 0.0);
    CHECK(info.density     == 1.0);
    CHECK(info.rate        == 60.0);
    CHECK(info.orientation == display_orientation::unknown);
    CHECK(info.rotation    == display_rotation::rotation_0);
}

TEST_CASE("mock_device_display constructed with explicit info",
          "[mock][device_display]") {
    const display_info initial = make_info(2340.0, 1080.0, 3.0, 90.0,
                                           display_orientation::portrait,
                                           display_rotation::rotation_0);
    mock_device_display dd{ initial };
    CHECK(dd.main_display_info() == initial);
}

// ---------------------------------------------------------------------------
// mock_device_display — keep_screen_on
// ---------------------------------------------------------------------------

TEST_CASE("keep_screen_on defaults to false and is settable",
          "[mock][device_display]") {
    mock_device_display dd{};

    // Arrange: default state
    CHECK_FALSE(dd.keep_screen_on());

    // Act: turn on
    dd.set_keep_screen_on(true);

    // Assert: on
    CHECK(dd.keep_screen_on());

    // Act: turn off
    dd.set_keep_screen_on(false);

    // Assert: off
    CHECK_FALSE(dd.keep_screen_on());
}

// ---------------------------------------------------------------------------
// mock_device_display — set_main_display_info (no signal on same value)
// ---------------------------------------------------------------------------

TEST_CASE("set_main_display_info with same value does not fire signal",
          "[mock][device_display][signal]") {
    const display_info initial = make_info(1920.0, 1080.0);
    mock_device_display dd{ initial };

    int hits = 0;
    signal_slot<display_info> slot;
    auto cb = [&](display_info) { ++hits; };
    dd.main_display_info_changed.subscribe(slot, cb);

    // Act: set the exact same value
    dd.set_main_display_info(initial);

    // Assert: no emission
    CHECK(hits == 0);
    CHECK(dd.main_display_info() == initial);
}

// ---------------------------------------------------------------------------
// mock_device_display — set_main_display_info fires signal on change
// ---------------------------------------------------------------------------

TEST_CASE("set_main_display_info fires signal on new value",
          "[mock][device_display][signal]") {
    const display_info first  = make_info(1920.0, 1080.0);
    const display_info second = make_info(2560.0, 1440.0, 2.0, 144.0,
                                          display_orientation::landscape,
                                          display_rotation::rotation_90);
    mock_device_display dd{ first };

    display_info last{};
    int hits = 0;
    signal_slot<display_info> slot;
    auto cb = [&](display_info info) { last = info; ++hits; };
    dd.main_display_info_changed.subscribe(slot, cb);

    // Act
    dd.set_main_display_info(second);

    // Assert
    CHECK(hits == 1);
    CHECK(last == second);
    CHECK(dd.main_display_info() == second);
}

TEST_CASE("set_main_display_info fires signal on each distinct change",
          "[mock][device_display][signal]") {
    mock_device_display dd{};

    int hits = 0;
    signal_slot<display_info> slot;
    auto cb = [&](display_info) { ++hits; };
    dd.main_display_info_changed.subscribe(slot, cb);

    const display_info a = make_info(800.0, 600.0);
    const display_info b = make_info(1024.0, 768.0);
    const display_info c = make_info(1280.0, 720.0);

    dd.set_main_display_info(a);
    CHECK(hits == 1);

    dd.set_main_display_info(b);
    CHECK(hits == 2);

    dd.set_main_display_info(b);   // same — no fire
    CHECK(hits == 2);

    dd.set_main_display_info(c);
    CHECK(hits == 3);
}

// ---------------------------------------------------------------------------
// mock_device_display — multiple subscribers
// ---------------------------------------------------------------------------

TEST_CASE("multiple subscribers all receive the changed display_info",
          "[mock][device_display][signal]") {
    mock_device_display dd{};

    const display_info updated = make_info(3840.0, 2160.0, 4.0, 60.0,
                                           display_orientation::landscape,
                                           display_rotation::rotation_270);

    display_info recv_a{};
    display_info recv_b{};
    signal_slot<display_info> slot_a;
    signal_slot<display_info> slot_b;
    auto cb_a = [&](display_info info) { recv_a = info; };
    auto cb_b = [&](display_info info) { recv_b = info; };
    dd.main_display_info_changed.subscribe(slot_a, cb_a);
    dd.main_display_info_changed.subscribe(slot_b, cb_b);

    dd.set_main_display_info(updated);

    CHECK(recv_a == updated);
    CHECK(recv_b == updated);
}

// ---------------------------------------------------------------------------
// mock_device_display — subscriber disconnect stops receiving
// ---------------------------------------------------------------------------

TEST_CASE("disconnected slot stops receiving signal",
          "[mock][device_display][signal]") {
    mock_device_display dd{};

    int hits = 0;
    signal_slot<display_info> slot;
    auto cb = [&](display_info) { ++hits; };
    dd.main_display_info_changed.subscribe(slot, cb);

    dd.set_main_display_info(make_info(1280.0, 720.0));
    CHECK(hits == 1);

    slot.disconnect();

    dd.set_main_display_info(make_info(1920.0, 1080.0));
    CHECK(hits == 1);   // still 1 — slot was disconnected
}

// ---------------------------------------------------------------------------
// display_orientation all enum values round-trip via uint8_t underlying type
// ---------------------------------------------------------------------------

TEST_CASE("display_orientation underlying values are stable",
          "[mock][device_display][enum]") {
    CHECK(static_cast<std::uint8_t>(display_orientation::unknown)   == 0u);
    CHECK(static_cast<std::uint8_t>(display_orientation::portrait)  == 1u);
    CHECK(static_cast<std::uint8_t>(display_orientation::landscape) == 2u);
}

TEST_CASE("display_rotation underlying values are stable",
          "[mock][device_display][enum]") {
    CHECK(static_cast<std::uint8_t>(display_rotation::rotation_0)   == 0u);
    CHECK(static_cast<std::uint8_t>(display_rotation::rotation_90)  == 1u);
    CHECK(static_cast<std::uint8_t>(display_rotation::rotation_180) == 2u);
    CHECK(static_cast<std::uint8_t>(display_rotation::rotation_270) == 3u);
}

// ---------------------------------------------------------------------------
// mock_device_display — all display_rotation and display_orientation combos
// ---------------------------------------------------------------------------

TEST_CASE("display_info stores landscape orientation",
          "[mock][device_display][display_info]") {
    display_info info = make_info(1920.0, 1080.0, 1.0, 60.0,
                                  display_orientation::landscape,
                                  display_rotation::rotation_90);
    mock_device_display dd{ info };
    const auto got = dd.main_display_info();
    CHECK(got.orientation == display_orientation::landscape);
    CHECK(got.rotation    == display_rotation::rotation_90);
}

TEST_CASE("display_info stores rotation_180 and rotation_270",
          "[mock][device_display][display_info]") {
    display_info info_180 = make_info(360.0, 800.0, 3.0, 60.0,
                                       display_orientation::portrait,
                                       display_rotation::rotation_180);
    mock_device_display dd{ info_180 };
    CHECK(dd.main_display_info().rotation == display_rotation::rotation_180);

    display_info info_270 = info_180;
    info_270.rotation = display_rotation::rotation_270;
    dd.set_main_display_info(info_270);
    CHECK(dd.main_display_info().rotation == display_rotation::rotation_270);
}

// ---------------------------------------------------------------------------
// signal subscriber count helper (sanity)
// ---------------------------------------------------------------------------

TEST_CASE("subscriber_count reflects active slots",
          "[mock][device_display][signal]") {
    mock_device_display dd{};

    CHECK(dd.main_display_info_changed.subscriber_count() == 0u);

    signal_slot<display_info> s1;
    signal_slot<display_info> s2;
    auto noop = [](display_info) {};
    dd.main_display_info_changed.subscribe(s1, noop);
    CHECK(dd.main_display_info_changed.subscriber_count() == 1u);

    dd.main_display_info_changed.subscribe(s2, noop);
    CHECK(dd.main_display_info_changed.subscriber_count() == 2u);

    s1.disconnect();
    CHECK(dd.main_display_info_changed.subscriber_count() == 1u);

    s2.disconnect();
    CHECK(dd.main_display_info_changed.subscriber_count() == 0u);
}
