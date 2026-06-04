// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::magnetometer` — magnetic-field sensor. Counterpart to MAUI
// Essentials `Magnetometer`. Abstract interface + an in-memory mock whose
// monitoring state is test-controllable and that emits readings via
// `reading_changed` only while monitoring is active. Real per-platform
// backends implement the same interface and are injected via the DI
// container (RFC-0011). No macros; header-only.

#ifndef MPAPP_ESSENTIALS_MAGNETOMETER_HPP
#define MPAPP_ESSENTIALS_MAGNETOMETER_HPP

#include "../signal.hpp"
#include "sensors_common.hpp"

namespace mpapp {

class magnetometer {
public:
    virtual ~magnetometer() = default;

    // Whether the sensor is currently delivering readings.
    [[nodiscard]] virtual bool is_monitoring() const = 0;

    // Begin delivering readings at the requested speed. Calling start() while
    // already monitoring is a no-op.
    virtual void start(sensor_speed speed) = 0;

    // Stop delivering readings. Calling stop() while not monitoring is a
    // no-op.
    virtual void stop() = 0;

    // Fires with the latest vector3 reading (µT per axis) whenever a new
    // sample arrives (only while monitoring).
    mpapp::signal<vector3> reading_changed{};
};

// Mock / in-memory implementation.
//
// * start() / stop() toggle is_monitoring().
// * push_reading(v) emits reading_changed only if is_monitoring(); the
//   call is silently ignored otherwise.
// * last_speed() returns the sensor_speed passed to the most recent start().
class mock_magnetometer final : public magnetometer {
public:
    // ---- magnetometer interface -------------------------------------------

    [[nodiscard]] bool is_monitoring() const override { return monitoring_; }

    void start(sensor_speed speed) override {
        if (monitoring_) {
            return;
        }
        last_speed_ = speed;
        monitoring_ = true;
    }

    void stop() override {
        monitoring_ = false;
    }

    // ---- Test-control helpers ---------------------------------------------

    // Inject a reading. Emits reading_changed only while monitoring.
    void push_reading(vector3 reading) {
        if (!monitoring_) {
            return;
        }
        reading_changed.emit(reading);
    }

    // The speed passed to the last start() call.
    [[nodiscard]] sensor_speed last_speed() const noexcept { return last_speed_; }

private:
    bool         monitoring_ = false;
    sensor_speed last_speed_ = sensor_speed::default_;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_MAGNETOMETER_HPP
