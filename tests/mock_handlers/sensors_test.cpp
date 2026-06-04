// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for the RFC-0013 Essentials sensor APIs.
//
// Covers: accelerometer, gyroscope, magnetometer, barometer, compass,
// orientation_sensor — including sensors_common types/enums.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/accelerometer.hpp>
#include <mpapp/essentials/barometer.hpp>
#include <mpapp/essentials/compass.hpp>
#include <mpapp/essentials/gyroscope.hpp>
#include <mpapp/essentials/magnetometer.hpp>
#include <mpapp/essentials/orientation_sensor.hpp>
#include <mpapp/essentials/sensors_common.hpp>
#include <mpapp/signal.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// sensors_common — value types and enum to_string
// ---------------------------------------------------------------------------

TEST_CASE("vector3 default construction and equality", "[mock][sensors][common]") {
    // Arrange
    vector3 a;
    vector3 b;

    // Act / Assert
    CHECK(a == b);
    CHECK(a.x == 0.0);
    CHECK(a.y == 0.0);
    CHECK(a.z == 0.0);
}

TEST_CASE("vector3 inequality when fields differ", "[mock][sensors][common]") {
    // Arrange
    vector3 a{ 1.0, 2.0, 3.0 };
    vector3 b{ 1.0, 2.0, 4.0 };

    // Act / Assert
    CHECK_FALSE(a == b);
}

TEST_CASE("quaternion default construction and equality", "[mock][sensors][common]") {
    // Arrange
    quaternion a;
    quaternion b;

    // Act / Assert
    CHECK(a == b);
    CHECK(a.x == 0.0);
    CHECK(a.y == 0.0);
    CHECK(a.z == 0.0);
    CHECK(a.w == 1.0);
}

TEST_CASE("quaternion inequality when fields differ", "[mock][sensors][common]") {
    // Arrange
    quaternion a{ 0.0, 0.0, 0.0, 1.0 };
    quaternion b{ 0.1, 0.0, 0.0, 1.0 };

    // Act / Assert
    CHECK_FALSE(a == b);
}

TEST_CASE("sensor_speed to_string covers all enumerators", "[mock][sensors][common]") {
    CHECK(to_string(sensor_speed::ui)       == "ui");
    CHECK(to_string(sensor_speed::game)     == "game");
    CHECK(to_string(sensor_speed::default_) == "default");
    CHECK(to_string(sensor_speed::fastest)  == "fastest");
}

// ---------------------------------------------------------------------------
// accelerometer
// ---------------------------------------------------------------------------

TEST_CASE("accelerometer: initially not monitoring", "[mock][sensors][accelerometer]") {
    // Arrange
    mock_accelerometer acc;

    // Act / Assert
    CHECK_FALSE(acc.is_monitoring());
}

TEST_CASE("accelerometer: start begins monitoring and records speed",
          "[mock][sensors][accelerometer]") {
    // Arrange
    mock_accelerometer acc;

    // Act
    acc.start(sensor_speed::game);

    // Assert
    CHECK(acc.is_monitoring());
    CHECK(acc.last_speed() == sensor_speed::game);
}

TEST_CASE("accelerometer: start while already monitoring is a no-op",
          "[mock][sensors][accelerometer]") {
    // Arrange
    mock_accelerometer acc;
    acc.start(sensor_speed::game);

    // Act — second start with a different speed; speed must not change
    acc.start(sensor_speed::fastest);

    // Assert
    CHECK(acc.is_monitoring());
    CHECK(acc.last_speed() == sensor_speed::game);
}

TEST_CASE("accelerometer: stop ends monitoring", "[mock][sensors][accelerometer]") {
    // Arrange
    mock_accelerometer acc;
    acc.start(sensor_speed::default_);

    // Act
    acc.stop();

    // Assert
    CHECK_FALSE(acc.is_monitoring());
}

TEST_CASE("accelerometer: stop while not monitoring is a no-op",
          "[mock][sensors][accelerometer]") {
    // Arrange
    mock_accelerometer acc;

    // Act / Assert — must not throw or crash
    acc.stop();
    CHECK_FALSE(acc.is_monitoring());
}

TEST_CASE("accelerometer: push_reading emits reading_changed while monitoring",
          "[mock][sensors][accelerometer]") {
    // Arrange
    mock_accelerometer acc;
    acc.start(sensor_speed::ui);

    vector3 received{};
    int hits = 0;
    signal_slot<vector3> slot;
    auto cb = [&](vector3 v) { received = v; ++hits; };
    acc.reading_changed.subscribe(slot, cb);

    // Act
    acc.push_reading({ 1.0, 2.0, 3.0 });

    // Assert
    CHECK(hits == 1);
    CHECK(received == vector3{ 1.0, 2.0, 3.0 });
}

TEST_CASE("accelerometer: push_reading does NOT emit when not monitoring",
          "[mock][sensors][accelerometer]") {
    // Arrange
    mock_accelerometer acc;

    int hits = 0;
    signal_slot<vector3> slot;
    auto cb = [&](vector3) { ++hits; };
    acc.reading_changed.subscribe(slot, cb);

    // Act
    acc.push_reading({ 1.0, 0.0, 0.0 });

    // Assert
    CHECK(hits == 0);
}

TEST_CASE("accelerometer: readings stop emitting after stop()",
          "[mock][sensors][accelerometer]") {
    // Arrange
    mock_accelerometer acc;
    acc.start(sensor_speed::fastest);

    int hits = 0;
    signal_slot<vector3> slot;
    auto cb = [&](vector3) { ++hits; };
    acc.reading_changed.subscribe(slot, cb);

    acc.push_reading({ 0.0, 0.0, 9.8 });
    CHECK(hits == 1);

    // Act
    acc.stop();
    acc.push_reading({ 0.0, 0.0, 9.8 });

    // Assert
    CHECK(hits == 1);
}

TEST_CASE("accelerometer: multiple readings while monitoring all emit",
          "[mock][sensors][accelerometer]") {
    // Arrange
    mock_accelerometer acc;
    acc.start(sensor_speed::game);

    int hits = 0;
    signal_slot<vector3> slot;
    auto cb = [&](vector3) { ++hits; };
    acc.reading_changed.subscribe(slot, cb);

    // Act
    acc.push_reading({ 1.0, 0.0, 0.0 });
    acc.push_reading({ 0.0, 1.0, 0.0 });
    acc.push_reading({ 0.0, 0.0, 1.0 });

    // Assert
    CHECK(hits == 3);
}

// ---------------------------------------------------------------------------
// gyroscope
// ---------------------------------------------------------------------------

TEST_CASE("gyroscope: initially not monitoring", "[mock][sensors][gyroscope]") {
    mock_gyroscope gyro;
    CHECK_FALSE(gyro.is_monitoring());
}

TEST_CASE("gyroscope: start begins monitoring and records speed",
          "[mock][sensors][gyroscope]") {
    // Arrange
    mock_gyroscope gyro;

    // Act
    gyro.start(sensor_speed::fastest);

    // Assert
    CHECK(gyro.is_monitoring());
    CHECK(gyro.last_speed() == sensor_speed::fastest);
}

TEST_CASE("gyroscope: start while already monitoring is a no-op",
          "[mock][sensors][gyroscope]") {
    // Arrange
    mock_gyroscope gyro;
    gyro.start(sensor_speed::ui);

    // Act
    gyro.start(sensor_speed::game);

    // Assert
    CHECK(gyro.last_speed() == sensor_speed::ui);
}

TEST_CASE("gyroscope: stop ends monitoring", "[mock][sensors][gyroscope]") {
    mock_gyroscope gyro;
    gyro.start(sensor_speed::default_);
    gyro.stop();
    CHECK_FALSE(gyro.is_monitoring());
}

TEST_CASE("gyroscope: stop while not monitoring is a no-op", "[mock][sensors][gyroscope]") {
    mock_gyroscope gyro;
    gyro.stop();
    CHECK_FALSE(gyro.is_monitoring());
}

TEST_CASE("gyroscope: push_reading emits while monitoring", "[mock][sensors][gyroscope]") {
    // Arrange
    mock_gyroscope gyro;
    gyro.start(sensor_speed::game);

    vector3 received{};
    int hits = 0;
    signal_slot<vector3> slot;
    auto cb = [&](vector3 v) { received = v; ++hits; };
    gyro.reading_changed.subscribe(slot, cb);

    // Act
    gyro.push_reading({ 0.1, 0.2, 0.3 });

    // Assert
    CHECK(hits == 1);
    CHECK(received == vector3{ 0.1, 0.2, 0.3 });
}

TEST_CASE("gyroscope: push_reading ignored when not monitoring",
          "[mock][sensors][gyroscope]") {
    mock_gyroscope gyro;
    int hits = 0;
    signal_slot<vector3> slot;
    auto cb = [&](vector3) { ++hits; };
    gyro.reading_changed.subscribe(slot, cb);
    gyro.push_reading({ 1.0, 1.0, 1.0 });
    CHECK(hits == 0);
}

TEST_CASE("gyroscope: readings stop after stop()", "[mock][sensors][gyroscope]") {
    mock_gyroscope gyro;
    gyro.start(sensor_speed::ui);
    int hits = 0;
    signal_slot<vector3> slot;
    auto cb = [&](vector3) { ++hits; };
    gyro.reading_changed.subscribe(slot, cb);
    gyro.push_reading({ 1.0, 0.0, 0.0 });
    gyro.stop();
    gyro.push_reading({ 1.0, 0.0, 0.0 });
    CHECK(hits == 1);
}

// ---------------------------------------------------------------------------
// magnetometer
// ---------------------------------------------------------------------------

TEST_CASE("magnetometer: initially not monitoring", "[mock][sensors][magnetometer]") {
    mock_magnetometer mag;
    CHECK_FALSE(mag.is_monitoring());
}

TEST_CASE("magnetometer: start begins monitoring and records speed",
          "[mock][sensors][magnetometer]") {
    mock_magnetometer mag;
    mag.start(sensor_speed::default_);
    CHECK(mag.is_monitoring());
    CHECK(mag.last_speed() == sensor_speed::default_);
}

TEST_CASE("magnetometer: start while already monitoring is a no-op",
          "[mock][sensors][magnetometer]") {
    mock_magnetometer mag;
    mag.start(sensor_speed::ui);
    mag.start(sensor_speed::fastest);
    CHECK(mag.last_speed() == sensor_speed::ui);
}

TEST_CASE("magnetometer: stop ends monitoring", "[mock][sensors][magnetometer]") {
    mock_magnetometer mag;
    mag.start(sensor_speed::game);
    mag.stop();
    CHECK_FALSE(mag.is_monitoring());
}

TEST_CASE("magnetometer: stop while not monitoring is a no-op",
          "[mock][sensors][magnetometer]") {
    mock_magnetometer mag;
    mag.stop();
    CHECK_FALSE(mag.is_monitoring());
}

TEST_CASE("magnetometer: push_reading emits while monitoring",
          "[mock][sensors][magnetometer]") {
    mock_magnetometer mag;
    mag.start(sensor_speed::fastest);

    vector3 received{};
    int hits = 0;
    signal_slot<vector3> slot;
    auto cb = [&](vector3 v) { received = v; ++hits; };
    mag.reading_changed.subscribe(slot, cb);

    mag.push_reading({ 10.0, -5.0, 3.0 });
    CHECK(hits == 1);
    CHECK(received == vector3{ 10.0, -5.0, 3.0 });
}

TEST_CASE("magnetometer: push_reading ignored when not monitoring",
          "[mock][sensors][magnetometer]") {
    mock_magnetometer mag;
    int hits = 0;
    signal_slot<vector3> slot;
    auto cb = [&](vector3) { ++hits; };
    mag.reading_changed.subscribe(slot, cb);
    mag.push_reading({ 1.0, 0.0, 0.0 });
    CHECK(hits == 0);
}

TEST_CASE("magnetometer: readings stop after stop()", "[mock][sensors][magnetometer]") {
    mock_magnetometer mag;
    mag.start(sensor_speed::game);
    int hits = 0;
    signal_slot<vector3> slot;
    auto cb = [&](vector3) { ++hits; };
    mag.reading_changed.subscribe(slot, cb);
    mag.push_reading({ 0.0, 1.0, 0.0 });
    mag.stop();
    mag.push_reading({ 0.0, 1.0, 0.0 });
    CHECK(hits == 1);
}

// ---------------------------------------------------------------------------
// barometer
// ---------------------------------------------------------------------------

TEST_CASE("barometer: initially not monitoring", "[mock][sensors][barometer]") {
    mock_barometer bar;
    CHECK_FALSE(bar.is_monitoring());
}

TEST_CASE("barometer: start begins monitoring and records speed",
          "[mock][sensors][barometer]") {
    mock_barometer bar;
    bar.start(sensor_speed::ui);
    CHECK(bar.is_monitoring());
    CHECK(bar.last_speed() == sensor_speed::ui);
}

TEST_CASE("barometer: start while already monitoring is a no-op",
          "[mock][sensors][barometer]") {
    mock_barometer bar;
    bar.start(sensor_speed::ui);
    bar.start(sensor_speed::fastest);
    CHECK(bar.last_speed() == sensor_speed::ui);
}

TEST_CASE("barometer: stop ends monitoring", "[mock][sensors][barometer]") {
    mock_barometer bar;
    bar.start(sensor_speed::default_);
    bar.stop();
    CHECK_FALSE(bar.is_monitoring());
}

TEST_CASE("barometer: stop while not monitoring is a no-op",
          "[mock][sensors][barometer]") {
    mock_barometer bar;
    bar.stop();
    CHECK_FALSE(bar.is_monitoring());
}

TEST_CASE("barometer: push_reading emits while monitoring", "[mock][sensors][barometer]") {
    // Arrange
    mock_barometer bar;
    bar.start(sensor_speed::game);

    double received = 0.0;
    int hits = 0;
    signal_slot<double> slot;
    auto cb = [&](double hpa) { received = hpa; ++hits; };
    bar.reading_changed.subscribe(slot, cb);

    // Act
    bar.push_reading(1013.25);

    // Assert
    CHECK(hits == 1);
    CHECK(received == 1013.25);
}

TEST_CASE("barometer: push_reading ignored when not monitoring",
          "[mock][sensors][barometer]") {
    mock_barometer bar;
    int hits = 0;
    signal_slot<double> slot;
    auto cb = [&](double) { ++hits; };
    bar.reading_changed.subscribe(slot, cb);
    bar.push_reading(1000.0);
    CHECK(hits == 0);
}

TEST_CASE("barometer: readings stop after stop()", "[mock][sensors][barometer]") {
    mock_barometer bar;
    bar.start(sensor_speed::fastest);
    int hits = 0;
    signal_slot<double> slot;
    auto cb = [&](double) { ++hits; };
    bar.reading_changed.subscribe(slot, cb);
    bar.push_reading(1013.0);
    bar.stop();
    bar.push_reading(1013.0);
    CHECK(hits == 1);
}

TEST_CASE("barometer: multiple readings all emit while monitoring",
          "[mock][sensors][barometer]") {
    mock_barometer bar;
    bar.start(sensor_speed::game);
    int hits = 0;
    signal_slot<double> slot;
    auto cb = [&](double) { ++hits; };
    bar.reading_changed.subscribe(slot, cb);
    bar.push_reading(1010.0);
    bar.push_reading(1011.0);
    bar.push_reading(1012.0);
    CHECK(hits == 3);
}

// ---------------------------------------------------------------------------
// compass
// ---------------------------------------------------------------------------

TEST_CASE("compass: initially not monitoring", "[mock][sensors][compass]") {
    mock_compass cmp;
    CHECK_FALSE(cmp.is_monitoring());
}

TEST_CASE("compass: start begins monitoring and records speed",
          "[mock][sensors][compass]") {
    mock_compass cmp;
    cmp.start(sensor_speed::game);
    CHECK(cmp.is_monitoring());
    CHECK(cmp.last_speed() == sensor_speed::game);
}

TEST_CASE("compass: start while already monitoring is a no-op",
          "[mock][sensors][compass]") {
    mock_compass cmp;
    cmp.start(sensor_speed::game);
    cmp.start(sensor_speed::ui);
    CHECK(cmp.last_speed() == sensor_speed::game);
}

TEST_CASE("compass: stop ends monitoring", "[mock][sensors][compass]") {
    mock_compass cmp;
    cmp.start(sensor_speed::default_);
    cmp.stop();
    CHECK_FALSE(cmp.is_monitoring());
}

TEST_CASE("compass: stop while not monitoring is a no-op", "[mock][sensors][compass]") {
    mock_compass cmp;
    cmp.stop();
    CHECK_FALSE(cmp.is_monitoring());
}

TEST_CASE("compass: push_reading emits while monitoring", "[mock][sensors][compass]") {
    // Arrange
    mock_compass cmp;
    cmp.start(sensor_speed::fastest);

    double received = -1.0;
    int hits = 0;
    signal_slot<double> slot;
    auto cb = [&](double deg) { received = deg; ++hits; };
    cmp.reading_changed.subscribe(slot, cb);

    // Act
    cmp.push_reading(270.0);

    // Assert
    CHECK(hits == 1);
    CHECK(received == 270.0);
}

TEST_CASE("compass: push_reading ignored when not monitoring",
          "[mock][sensors][compass]") {
    mock_compass cmp;
    int hits = 0;
    signal_slot<double> slot;
    auto cb = [&](double) { ++hits; };
    cmp.reading_changed.subscribe(slot, cb);
    cmp.push_reading(90.0);
    CHECK(hits == 0);
}

TEST_CASE("compass: readings stop after stop()", "[mock][sensors][compass]") {
    mock_compass cmp;
    cmp.start(sensor_speed::ui);
    int hits = 0;
    signal_slot<double> slot;
    auto cb = [&](double) { ++hits; };
    cmp.reading_changed.subscribe(slot, cb);
    cmp.push_reading(180.0);
    cmp.stop();
    cmp.push_reading(180.0);
    CHECK(hits == 1);
}

// ---------------------------------------------------------------------------
// orientation_sensor
// ---------------------------------------------------------------------------

TEST_CASE("orientation_sensor: initially not monitoring",
          "[mock][sensors][orientation]") {
    mock_orientation_sensor ori;
    CHECK_FALSE(ori.is_monitoring());
}

TEST_CASE("orientation_sensor: start begins monitoring and records speed",
          "[mock][sensors][orientation]") {
    mock_orientation_sensor ori;
    ori.start(sensor_speed::fastest);
    CHECK(ori.is_monitoring());
    CHECK(ori.last_speed() == sensor_speed::fastest);
}

TEST_CASE("orientation_sensor: start while already monitoring is a no-op",
          "[mock][sensors][orientation]") {
    mock_orientation_sensor ori;
    ori.start(sensor_speed::fastest);
    ori.start(sensor_speed::ui);
    CHECK(ori.last_speed() == sensor_speed::fastest);
}

TEST_CASE("orientation_sensor: stop ends monitoring", "[mock][sensors][orientation]") {
    mock_orientation_sensor ori;
    ori.start(sensor_speed::default_);
    ori.stop();
    CHECK_FALSE(ori.is_monitoring());
}

TEST_CASE("orientation_sensor: stop while not monitoring is a no-op",
          "[mock][sensors][orientation]") {
    mock_orientation_sensor ori;
    ori.stop();
    CHECK_FALSE(ori.is_monitoring());
}

TEST_CASE("orientation_sensor: push_reading emits while monitoring",
          "[mock][sensors][orientation]") {
    // Arrange
    mock_orientation_sensor ori;
    ori.start(sensor_speed::game);

    quaternion received{};
    int hits = 0;
    signal_slot<quaternion> slot;
    auto cb = [&](quaternion q) { received = q; ++hits; };
    ori.reading_changed.subscribe(slot, cb);

    // Act
    ori.push_reading({ 0.0, 0.707, 0.0, 0.707 });

    // Assert
    CHECK(hits == 1);
    CHECK(received == quaternion{ 0.0, 0.707, 0.0, 0.707 });
}

TEST_CASE("orientation_sensor: push_reading ignored when not monitoring",
          "[mock][sensors][orientation]") {
    mock_orientation_sensor ori;
    int hits = 0;
    signal_slot<quaternion> slot;
    auto cb = [&](quaternion) { ++hits; };
    ori.reading_changed.subscribe(slot, cb);
    ori.push_reading({ 0.0, 0.0, 0.0, 1.0 });
    CHECK(hits == 0);
}

TEST_CASE("orientation_sensor: readings stop after stop()",
          "[mock][sensors][orientation]") {
    mock_orientation_sensor ori;
    ori.start(sensor_speed::fastest);
    int hits = 0;
    signal_slot<quaternion> slot;
    auto cb = [&](quaternion) { ++hits; };
    ori.reading_changed.subscribe(slot, cb);
    ori.push_reading({ 0.0, 0.0, 0.0, 1.0 });
    ori.stop();
    ori.push_reading({ 0.0, 0.0, 0.0, 1.0 });
    CHECK(hits == 1);
}

TEST_CASE("orientation_sensor: multiple readings all emit while monitoring",
          "[mock][sensors][orientation]") {
    mock_orientation_sensor ori;
    ori.start(sensor_speed::game);
    int hits = 0;
    signal_slot<quaternion> slot;
    auto cb = [&](quaternion) { ++hits; };
    ori.reading_changed.subscribe(slot, cb);
    ori.push_reading({ 1.0, 0.0, 0.0, 0.0 });
    ori.push_reading({ 0.0, 1.0, 0.0, 0.0 });
    ori.push_reading({ 0.0, 0.0, 1.0, 0.0 });
    CHECK(hits == 3);
}

// ---------------------------------------------------------------------------
// Cross-sensor: start→read→stop→no-emit cycle for each sensor
// ---------------------------------------------------------------------------

TEST_CASE("all sensors: start-read-stop-no-emit cycle", "[mock][sensors][cycle]") {
    // accelerometer
    {
        mock_accelerometer acc;
        int hits = 0;
        signal_slot<vector3> slot;
        auto cb = [&](vector3) { ++hits; };
        acc.reading_changed.subscribe(slot, cb);
        acc.start(sensor_speed::ui);
        acc.push_reading({ 0.0, 0.0, 9.8 });
        CHECK(hits == 1);
        acc.stop();
        acc.push_reading({ 0.0, 0.0, 9.8 });
        CHECK(hits == 1);
    }

    // gyroscope
    {
        mock_gyroscope gyro;
        int hits = 0;
        signal_slot<vector3> slot;
        auto cb = [&](vector3) { ++hits; };
        gyro.reading_changed.subscribe(slot, cb);
        gyro.start(sensor_speed::game);
        gyro.push_reading({ 0.1, 0.0, 0.0 });
        CHECK(hits == 1);
        gyro.stop();
        gyro.push_reading({ 0.1, 0.0, 0.0 });
        CHECK(hits == 1);
    }

    // magnetometer
    {
        mock_magnetometer mag;
        int hits = 0;
        signal_slot<vector3> slot;
        auto cb = [&](vector3) { ++hits; };
        mag.reading_changed.subscribe(slot, cb);
        mag.start(sensor_speed::fastest);
        mag.push_reading({ 5.0, 0.0, 0.0 });
        CHECK(hits == 1);
        mag.stop();
        mag.push_reading({ 5.0, 0.0, 0.0 });
        CHECK(hits == 1);
    }

    // barometer
    {
        mock_barometer bar;
        int hits = 0;
        signal_slot<double> slot;
        auto cb = [&](double) { ++hits; };
        bar.reading_changed.subscribe(slot, cb);
        bar.start(sensor_speed::default_);
        bar.push_reading(1013.0);
        CHECK(hits == 1);
        bar.stop();
        bar.push_reading(1013.0);
        CHECK(hits == 1);
    }

    // compass
    {
        mock_compass cmp;
        int hits = 0;
        signal_slot<double> slot;
        auto cb = [&](double) { ++hits; };
        cmp.reading_changed.subscribe(slot, cb);
        cmp.start(sensor_speed::ui);
        cmp.push_reading(45.0);
        CHECK(hits == 1);
        cmp.stop();
        cmp.push_reading(45.0);
        CHECK(hits == 1);
    }

    // orientation_sensor
    {
        mock_orientation_sensor ori;
        int hits = 0;
        signal_slot<quaternion> slot;
        auto cb = [&](quaternion) { ++hits; };
        ori.reading_changed.subscribe(slot, cb);
        ori.start(sensor_speed::game);
        ori.push_reading({ 0.0, 0.0, 0.0, 1.0 });
        CHECK(hits == 1);
        ori.stop();
        ori.push_reading({ 0.0, 0.0, 0.0, 1.0 });
        CHECK(hits == 1);
    }
}
