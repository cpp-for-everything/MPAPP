// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for the RFC-0013 Essentials geolocation API.
//
// Covers: geolocation_accuracy to_string, geo_location value type,
// geolocation_request defaults, mock_geolocation — every public method,
// signal emission, and default/not-supported paths.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/geolocation.hpp>
#include <mpapp/signal.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// geolocation_accuracy — to_string
// ---------------------------------------------------------------------------

TEST_CASE("geolocation_accuracy to_string covers all enumerators",
          "[mock][geolocation][enum]") {
    CHECK(to_string(geolocation_accuracy::lowest) == "lowest");
    CHECK(to_string(geolocation_accuracy::low)    == "low");
    CHECK(to_string(geolocation_accuracy::medium) == "medium");
    CHECK(to_string(geolocation_accuracy::high)   == "high");
    CHECK(to_string(geolocation_accuracy::best)   == "best");
}

// ---------------------------------------------------------------------------
// geo_location — value type
// ---------------------------------------------------------------------------

TEST_CASE("geo_location default construction has zeroed fields",
          "[mock][geolocation][value]") {
    // Arrange / Act
    geo_location loc;

    // Assert
    CHECK(loc.latitude  == 0.0);
    CHECK(loc.longitude == 0.0);
    CHECK_FALSE(loc.altitude.has_value());
    CHECK_FALSE(loc.accuracy.has_value());
    CHECK_FALSE(loc.speed.has_value());
    CHECK_FALSE(loc.course.has_value());
    CHECK(loc.timestamp == 0);
}

TEST_CASE("geo_location equality when all fields match", "[mock][geolocation][value]") {
    // Arrange
    geo_location a;
    a.latitude  = 51.5074;
    a.longitude = -0.1278;
    a.altitude  = 11.0;
    a.accuracy  = 5.0;
    a.speed     = 1.5;
    a.course    = 270.0;
    a.timestamp = 1000000LL;

    geo_location b = a;

    // Act / Assert
    CHECK(a == b);
}

TEST_CASE("geo_location inequality when latitude differs", "[mock][geolocation][value]") {
    // Arrange
    geo_location a;
    a.latitude = 51.5;
    geo_location b = a;
    b.latitude = 48.8;

    // Act / Assert
    CHECK_FALSE(a == b);
}

TEST_CASE("geo_location inequality when optional altitude differs",
          "[mock][geolocation][value]") {
    // Arrange
    geo_location a;
    a.altitude = 100.0;
    geo_location b;
    // b.altitude is std::nullopt

    // Act / Assert
    CHECK_FALSE(a == b);
}

TEST_CASE("geo_location optional fields hold the values set",
          "[mock][geolocation][value]") {
    // Arrange
    geo_location loc;
    loc.altitude = 200.5;
    loc.accuracy = 3.0;
    loc.speed    = 12.5;
    loc.course   = 90.0;

    // Act / Assert
    REQUIRE(loc.altitude.has_value());
    CHECK(*loc.altitude == 200.5);
    REQUIRE(loc.accuracy.has_value());
    CHECK(*loc.accuracy == 3.0);
    REQUIRE(loc.speed.has_value());
    CHECK(*loc.speed == 12.5);
    REQUIRE(loc.course.has_value());
    CHECK(*loc.course == 90.0);
}

// ---------------------------------------------------------------------------
// geolocation_request — defaults
// ---------------------------------------------------------------------------

TEST_CASE("geolocation_request default accuracy is medium and timeout is 0",
          "[mock][geolocation][request]") {
    // Arrange / Act
    geolocation_request req;

    // Assert
    CHECK(req.accuracy        == geolocation_accuracy::medium);
    CHECK(req.timeout_seconds == 0.0);
}

// ---------------------------------------------------------------------------
// mock_geolocation — initial state
// ---------------------------------------------------------------------------

TEST_CASE("mock_geolocation: initially not listening", "[mock][geolocation]") {
    // Arrange / Act
    mock_geolocation geo;

    // Assert
    CHECK_FALSE(geo.is_listening());
}

TEST_CASE("mock_geolocation: get_last_known returns nullopt by default",
          "[mock][geolocation]") {
    // Arrange
    mock_geolocation geo;

    // Act / Assert
    CHECK_FALSE(geo.get_last_known().has_value());
}

TEST_CASE("mock_geolocation: get_location returns nullopt by default",
          "[mock][geolocation]") {
    // Arrange
    mock_geolocation geo;

    // Act / Assert
    CHECK_FALSE(geo.get_location().has_value());
}

// ---------------------------------------------------------------------------
// mock_geolocation — set_last_known / set_current
// ---------------------------------------------------------------------------

TEST_CASE("mock_geolocation: set_last_known makes get_last_known return value",
          "[mock][geolocation]") {
    // Arrange
    mock_geolocation geo;
    geo_location     loc;
    loc.latitude  = 40.7128;
    loc.longitude = -74.0060;

    // Act
    geo.set_last_known(loc);

    // Assert
    REQUIRE(geo.get_last_known().has_value());
    CHECK(*geo.get_last_known() == loc);
}

TEST_CASE("mock_geolocation: set_last_known with nullopt clears the cache",
          "[mock][geolocation]") {
    // Arrange
    mock_geolocation geo;
    geo_location     loc;
    loc.latitude = 1.0;
    geo.set_last_known(loc);
    REQUIRE(geo.get_last_known().has_value());

    // Act
    geo.set_last_known(std::nullopt);

    // Assert
    CHECK_FALSE(geo.get_last_known().has_value());
}

TEST_CASE("mock_geolocation: set_current makes get_location return value",
          "[mock][geolocation]") {
    // Arrange
    mock_geolocation geo;
    geo_location     loc;
    loc.latitude  = 48.8566;
    loc.longitude = 2.3522;

    // Act
    geo.set_current(loc);

    // Assert
    REQUIRE(geo.get_location().has_value());
    CHECK(*geo.get_location() == loc);
}

TEST_CASE("mock_geolocation: set_current with nullopt returns nullopt from get_location",
          "[mock][geolocation]") {
    // Arrange
    mock_geolocation geo;
    geo_location     loc;
    loc.latitude = 2.0;
    geo.set_current(loc);

    // Act
    geo.set_current(std::nullopt);

    // Assert
    CHECK_FALSE(geo.get_location().has_value());
}

// ---------------------------------------------------------------------------
// mock_geolocation — get_location records the request
// ---------------------------------------------------------------------------

TEST_CASE("mock_geolocation: get_location records the supplied request",
          "[mock][geolocation]") {
    // Arrange
    mock_geolocation    geo;
    geolocation_request req;
    req.accuracy        = geolocation_accuracy::best;
    req.timeout_seconds = 10.0;

    // Act
    (void)geo.get_location(req);

    // Assert
    CHECK(geo.last_request().accuracy        == geolocation_accuracy::best);
    CHECK(geo.last_request().timeout_seconds == 10.0);
}

TEST_CASE("mock_geolocation: get_location with default request records medium accuracy",
          "[mock][geolocation]") {
    // Arrange
    mock_geolocation geo;

    // Act
    (void)geo.get_location();

    // Assert
    CHECK(geo.last_request().accuracy == geolocation_accuracy::medium);
}

// ---------------------------------------------------------------------------
// mock_geolocation — start_listening / stop_listening
// ---------------------------------------------------------------------------

TEST_CASE("mock_geolocation: start_listening sets is_listening true",
          "[mock][geolocation]") {
    // Arrange
    mock_geolocation geo;

    // Act
    geo.start_listening();

    // Assert
    CHECK(geo.is_listening());
}

TEST_CASE("mock_geolocation: start_listening records the supplied request",
          "[mock][geolocation]") {
    // Arrange
    mock_geolocation    geo;
    geolocation_request req;
    req.accuracy        = geolocation_accuracy::high;
    req.timeout_seconds = 30.0;

    // Act
    geo.start_listening(req);

    // Assert
    CHECK(geo.last_request().accuracy        == geolocation_accuracy::high);
    CHECK(geo.last_request().timeout_seconds == 30.0);
}

TEST_CASE("mock_geolocation: start_listening while already listening is a no-op",
          "[mock][geolocation]") {
    // Arrange
    mock_geolocation    geo;
    geolocation_request req1;
    req1.accuracy = geolocation_accuracy::low;
    geo.start_listening(req1);

    // Act — second start with different accuracy; request must not change
    geolocation_request req2;
    req2.accuracy = geolocation_accuracy::best;
    geo.start_listening(req2);

    // Assert
    CHECK(geo.is_listening());
    CHECK(geo.last_request().accuracy == geolocation_accuracy::low);
}

TEST_CASE("mock_geolocation: stop_listening sets is_listening false",
          "[mock][geolocation]") {
    // Arrange
    mock_geolocation geo;
    geo.start_listening();
    REQUIRE(geo.is_listening());

    // Act
    geo.stop_listening();

    // Assert
    CHECK_FALSE(geo.is_listening());
}

TEST_CASE("mock_geolocation: stop_listening while not listening is a no-op",
          "[mock][geolocation]") {
    // Arrange
    mock_geolocation geo;
    REQUIRE_FALSE(geo.is_listening());

    // Act / Assert — must not throw or crash
    geo.stop_listening();
    CHECK_FALSE(geo.is_listening());
}

// ---------------------------------------------------------------------------
// mock_geolocation — push_location signal emission
// ---------------------------------------------------------------------------

TEST_CASE("mock_geolocation: push_location emits location_changed while listening",
          "[mock][geolocation]") {
    // Arrange
    mock_geolocation geo;
    geo.start_listening();

    geo_location received{};
    int hits = 0;
    signal_slot<geo_location> slot;
    auto cb = [&](geo_location loc) { received = loc; ++hits; };
    geo.location_changed.subscribe(slot, cb);

    geo_location fix;
    fix.latitude  = 35.6762;
    fix.longitude = 139.6503;
    fix.timestamp = 999LL;

    // Act
    geo.push_location(fix);

    // Assert
    CHECK(hits == 1);
    CHECK(received == fix);
}

TEST_CASE("mock_geolocation: push_location does NOT emit when not listening",
          "[mock][geolocation]") {
    // Arrange
    mock_geolocation geo;
    REQUIRE_FALSE(geo.is_listening());

    int hits = 0;
    signal_slot<geo_location> slot;
    auto cb = [&](geo_location) { ++hits; };
    geo.location_changed.subscribe(slot, cb);

    // Act
    geo_location fix;
    fix.latitude = 1.0;
    geo.push_location(fix);

    // Assert
    CHECK(hits == 0);
}

TEST_CASE("mock_geolocation: push_location updates last_known even when not listening",
          "[mock][geolocation]") {
    // Arrange
    mock_geolocation geo;
    REQUIRE_FALSE(geo.is_listening());

    geo_location fix;
    fix.latitude  = 55.7558;
    fix.longitude = 37.6173;

    // Act
    geo.push_location(fix);

    // Assert
    REQUIRE(geo.get_last_known().has_value());
    CHECK(*geo.get_last_known() == fix);
}

TEST_CASE("mock_geolocation: push_location updates last_known while listening",
          "[mock][geolocation]") {
    // Arrange
    mock_geolocation geo;
    geo.start_listening();

    geo_location fix;
    fix.latitude  = -33.8688;
    fix.longitude = 151.2093;

    signal_slot<geo_location> slot;
    auto cb = [](geo_location) {};
    geo.location_changed.subscribe(slot, cb);

    // Act
    geo.push_location(fix);

    // Assert
    REQUIRE(geo.get_last_known().has_value());
    CHECK(*geo.get_last_known() == fix);
}

TEST_CASE("mock_geolocation: push_location stops emitting after stop_listening",
          "[mock][geolocation]") {
    // Arrange
    mock_geolocation geo;
    geo.start_listening();

    int hits = 0;
    signal_slot<geo_location> slot;
    auto cb = [&](geo_location) { ++hits; };
    geo.location_changed.subscribe(slot, cb);

    geo_location fix;
    fix.latitude = 10.0;
    geo.push_location(fix);
    CHECK(hits == 1);

    // Act
    geo.stop_listening();
    geo.push_location(fix);

    // Assert
    CHECK(hits == 1);
}

TEST_CASE("mock_geolocation: multiple push_location calls all emit while listening",
          "[mock][geolocation]") {
    // Arrange
    mock_geolocation geo;
    geo.start_listening();

    int hits = 0;
    signal_slot<geo_location> slot;
    auto cb = [&](geo_location) { ++hits; };
    geo.location_changed.subscribe(slot, cb);

    // Act
    geo_location fix;
    fix.latitude  = 0.0;
    fix.longitude = 0.0;
    geo.push_location(fix);
    fix.latitude = 1.0;
    geo.push_location(fix);
    fix.latitude = 2.0;
    geo.push_location(fix);

    // Assert
    CHECK(hits == 3);
}

TEST_CASE("mock_geolocation: last push_location value reflected in get_last_known",
          "[mock][geolocation]") {
    // Arrange
    mock_geolocation geo;
    geo.start_listening();

    signal_slot<geo_location> slot;
    auto cb = [](geo_location) {};
    geo.location_changed.subscribe(slot, cb);

    geo_location fix1;
    fix1.latitude = 10.0;
    geo_location fix2;
    fix2.latitude = 20.0;

    // Act
    geo.push_location(fix1);
    geo.push_location(fix2);

    // Assert
    REQUIRE(geo.get_last_known().has_value());
    CHECK(geo.get_last_known()->latitude == 20.0);
}

// ---------------------------------------------------------------------------
// mock_geolocation — start → push → stop → no-emit cycle
// ---------------------------------------------------------------------------

TEST_CASE("mock_geolocation: start-push-stop-no-emit lifecycle",
          "[mock][geolocation]") {
    // Arrange
    mock_geolocation geo;

    int hits = 0;
    signal_slot<geo_location> slot;
    auto cb = [&](geo_location) { ++hits; };
    geo.location_changed.subscribe(slot, cb);

    geo_location fix;
    fix.latitude  = 52.52;
    fix.longitude = 13.405;

    // Act — start, push (should emit), stop, push (should not emit)
    geo.start_listening();
    geo.push_location(fix);
    CHECK(hits == 1);

    geo.stop_listening();
    geo.push_location(fix);

    // Assert
    CHECK(hits == 1);
    CHECK_FALSE(geo.is_listening());
}

// ---------------------------------------------------------------------------
// mock_geolocation — interface pointer polymorphism
// ---------------------------------------------------------------------------

TEST_CASE("mock_geolocation: usable via abstract geolocation pointer",
          "[mock][geolocation]") {
    // Arrange
    mock_geolocation     concrete;
    geolocation*         iface = &concrete;

    geo_location fix;
    fix.latitude  = 1.0;
    fix.longitude = 2.0;
    concrete.set_last_known(fix);
    concrete.set_current(fix);

    // Act / Assert via interface
    CHECK(iface->get_last_known().has_value());
    CHECK(iface->get_location().has_value());
    CHECK_FALSE(iface->is_listening());

    iface->start_listening();
    CHECK(iface->is_listening());

    iface->stop_listening();
    CHECK_FALSE(iface->is_listening());
}
