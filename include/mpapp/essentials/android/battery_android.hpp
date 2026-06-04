// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::android_battery` — Android BatteryManager backend.
// Implements `mpapp::battery` using android.os.BatteryManager for the
// capacity query and a sticky Intent broadcast
// (Intent.ACTION_BATTERY_CHANGED) for status and plugged-type. The app
// Context is obtained from the JNI bridge (mpapp::detail::get_activity()).
//
// All JNI details (<jni.h>, FindClass, CallObjectMethod, DeleteLocalRef,
// AttachCurrentThread) are confined to the .cpp translation unit; this
// header is intentionally JNI-free and OS-header-free, mirroring the
// pattern of clipboard_android.hpp.  No macros in the public API.

#ifndef MPAPP_ESSENTIALS_ANDROID_BATTERY_ANDROID_HPP
#define MPAPP_ESSENTIALS_ANDROID_BATTERY_ANDROID_HPP

#include "../../essentials/battery.hpp"

namespace mpapp {

// Android battery backend.  Implements `mpapp::battery` via:
//
//   charge_level()   — BatteryManager.getIntProperty(BATTERY_PROPERTY_CAPACITY)
//                      divided by 100.0; returns -1.0 when unavailable.
//   state()          — sticky ACTION_BATTERY_CHANGED extra EXTRA_STATUS mapped
//                      to battery_state (charging / discharging / full /
//                      not_charging / unknown).
//   power_source()   — sticky ACTION_BATTERY_CHANGED extra EXTRA_PLUGGED mapped
//                      to battery_power_source (battery / ac / usb / wireless).
//   energy_saver()   — always energy_saver_status::unknown (Android does not
//                      expose a reliable cross-version energy-saver query
//                      through a single stable API; a future revision may add
//                      PowerManager.isPowerSaveMode() for API 21+).
//
// The Context is taken from the JNI bridge (detail::get_activity()), which
// the host MainActivity sets once during native init.  No Context is
// required in the constructor.
//
// Thread safety: each public method attaches the calling thread to the JVM
// independently; concurrent calls are safe but may perform redundant work.
// The signal members inherited from mpapp::battery are not mutex-protected;
// callers should emit signals from a single thread.
class android_battery final : public battery {
public:
    android_battery()  = default;
    ~android_battery() = default;

    android_battery(const android_battery&)            = delete;
    android_battery& operator=(const android_battery&) = delete;
    android_battery(android_battery&&)                 = delete;
    android_battery& operator=(android_battery&&)      = delete;

    // Current charge level in [0.0, 1.0].  Returns a negative value when
    // the BatteryManager service or BATTERY_PROPERTY_CAPACITY is unavailable.
    [[nodiscard]] double              charge_level()  const override;

    // Current charging state derived from ACTION_BATTERY_CHANGED EXTRA_STATUS.
    [[nodiscard]] battery_state       state()         const override;

    // Current power source derived from ACTION_BATTERY_CHANGED EXTRA_PLUGGED.
    [[nodiscard]] battery_power_source power_source() const override;

    // Always returns energy_saver_status::unknown on this backend.
    [[nodiscard]] energy_saver_status  energy_saver() const override;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_ANDROID_BATTERY_ANDROID_HPP
