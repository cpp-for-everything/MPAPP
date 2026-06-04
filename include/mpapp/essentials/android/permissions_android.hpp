// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::android_permissions` — Android runtime permission backend.
// Implements `mpapp::permissions` using ContextCompat.checkSelfPermission
// and ActivityCompat.requestPermissions, reached through the app Context
// obtained from the JNI bridge (mpapp::detail::get_activity()). All JNI
// details (<jni.h>, FindClass, CallStaticIntMethod, DeleteLocalRef,
// AttachCurrentThread) are confined to the .cpp translation unit; this
// header stays JNI-free. No macros in the public API.

#ifndef MPAPP_ESSENTIALS_ANDROID_PERMISSIONS_ANDROID_HPP
#define MPAPP_ESSENTIALS_ANDROID_PERMISSIONS_ANDROID_HPP

#include "../../essentials/permissions.hpp"

namespace mpapp {

// Android permissions backend. Implements `mpapp::permissions` using the
// Android runtime-permission APIs available from API level 23+.
//
// check_status():
//   Calls ContextCompat.checkSelfPermission(context, manifestPermission).
//   Returns granted when the result is PackageManager.PERMISSION_GRANTED (0),
//   denied otherwise. Returns unknown when the Context or JNI env is
//   unavailable, or when the permission_type has no Android mapping.
//
// request():
//   Runtime permission requests require an Activity and a callback
//   (ActivityCompat.requestPermissions + onRequestPermissionsResult). Since
//   there is no blocking callback mechanism available synchronously at the
//   C++ layer, this implementation records the intent and returns the current
//   check_status() result immediately. The host application must wire up
//   onRequestPermissionsResult to update the bridge state and re-query. This
//   matches the async nature of the Android permission model.
//
// should_show_rationale():
//   Calls ActivityCompat.shouldShowRequestPermissionRationale(activity,
//   manifestPermission). Returns false when the Activity or JNI env is
//   unavailable.
//
// The Context/Activity is taken from the JNI bridge (detail::get_activity()),
// which the host MainActivity sets once during native init.
class android_permissions final : public permissions {
public:
    android_permissions()  = default;
    ~android_permissions() = default;

    android_permissions(const android_permissions&)            = delete;
    android_permissions& operator=(const android_permissions&) = delete;
    android_permissions(android_permissions&&)                 = delete;
    android_permissions& operator=(android_permissions&&)      = delete;

    // Returns the current Android grant status for the given permission_type
    // without prompting the user. Uses ContextCompat.checkSelfPermission.
    [[nodiscard]] permission_status check_status(permission_type type) const override;

    // Records the intent to request the permission and returns the current
    // check_status() value. Full async request dispatch requires wiring
    // ActivityCompat.requestPermissions from the Java side.
    permission_status request(permission_type type) override;

    // Returns true when ActivityCompat.shouldShowRequestPermissionRationale
    // reports that a rationale UI should be shown before re-requesting.
    [[nodiscard]] bool should_show_rationale(permission_type type) const override;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_ANDROID_PERMISSIONS_ANDROID_HPP
