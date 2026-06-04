// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::sensors_common` — shared types for all motion/environment sensor
// headers (accelerometer, gyroscope, magnetometer, barometer, compass,
// orientation_sensor). Counterpart to the common value types used across
// MAUI Essentials sensor APIs. No macros; header-only.

#ifndef MPAPP_ESSENTIALS_SENSORS_COMMON_HPP
#define MPAPP_ESSENTIALS_SENSORS_COMMON_HPP

#include <cstdint>
#include <string_view>

namespace mpapp {

// Three-axis vector: used by accelerometer, gyroscope, magnetometer.
struct vector3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    bool operator==(const vector3&) const = default;
};

// Quaternion: used by orientation_sensor.
struct quaternion {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double w = 1.0;

    bool operator==(const quaternion&) const = default;
};

// Requested sensor update rate. Mirrors MAUI SensorSpeed.
enum class sensor_speed : std::uint8_t {
    ui       = 0,  // suitable for UI updates (~60 ms)
    game     = 1,  // suitable for games (~20 ms)
    default_ = 2,  // platform default
    fastest  = 3,  // as fast as possible
};

[[nodiscard]] constexpr std::string_view to_string(sensor_speed s) noexcept {
    switch (s) {
        case sensor_speed::ui:       return "ui";
        case sensor_speed::game:     return "game";
        case sensor_speed::default_: return "default";
        case sensor_speed::fastest:  return "fastest";
    }
    return "unknown";
}

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_SENSORS_COMMON_HPP
