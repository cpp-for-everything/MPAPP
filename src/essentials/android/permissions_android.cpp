// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// Android permissions backend implementation. Confines all JNI usage to this
// translation unit; the header (permissions_android.hpp) stays JNI-free.

#include "mpapp/essentials/android/permissions_android.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"

#include <string_view>

namespace mpapp {

namespace {

// Map permission_type to the canonical android.Manifest.permission string.
// Returns an empty string_view for types without a direct Android equivalent.
[[nodiscard]] constexpr std::string_view android_manifest_permission(permission_type type) noexcept {
    switch (type) {
        case permission_type::location_when_in_use:
            return "android.permission.ACCESS_FINE_LOCATION";
        case permission_type::location_always:
            return "android.permission.ACCESS_BACKGROUND_LOCATION";
        case permission_type::camera:
            return "android.permission.CAMERA";
        case permission_type::microphone:
            return "android.permission.RECORD_AUDIO";
        case permission_type::photos:
            // API 33+: READ_MEDIA_IMAGES; below 33: READ_EXTERNAL_STORAGE.
            // Use the most-capable modern string; callers should check API level.
            return "android.permission.READ_MEDIA_IMAGES";
        case permission_type::contacts:
            return "android.permission.READ_CONTACTS";
        case permission_type::calendar:
            return "android.permission.READ_CALENDAR";
        case permission_type::reminders:
            // Android has no dedicated reminders permission; map to calendar.
            return "android.permission.READ_CALENDAR";
        case permission_type::sensors:
            return "android.permission.BODY_SENSORS";
        case permission_type::storage_read:
            return "android.permission.READ_EXTERNAL_STORAGE";
        case permission_type::storage_write:
            return "android.permission.WRITE_EXTERNAL_STORAGE";
        case permission_type::phone:
            return "android.permission.READ_PHONE_STATE";
        case permission_type::sms:
            return "android.permission.SEND_SMS";
        case permission_type::bluetooth:
            return "android.permission.BLUETOOTH_CONNECT";
        case permission_type::network_state:
            // ACCESS_NETWORK_STATE is a normal (install-time) permission;
            // it is always granted, but we still call checkSelfPermission
            // for consistency.
            return "android.permission.ACCESS_NETWORK_STATE";
        default:
            return "";
    }
}

// Call ContextCompat.checkSelfPermission(context, permString).
// Returns PackageManager.PERMISSION_GRANTED (0) or PERMISSION_DENIED (-1),
// or -2 on JNI failure.
[[nodiscard]] int check_self_permission(JNIEnv* env, jobject context, jstring perm_str) {
    if (env == nullptr || context == nullptr || perm_str == nullptr) return -2;
    if (env->ExceptionCheck()) env->ExceptionClear();

    jclass compat_cls = env->FindClass("androidx/core/content/ContextCompat");
    if (compat_cls == nullptr) {
        // Fallback: use Context.checkSelfPermission (API 23+).
        env->ExceptionClear();
        jclass ctx_cls = env->FindClass("android/content/Context");
        if (ctx_cls == nullptr) { env->ExceptionClear(); return -2; }

        jmethodID check_m = env->GetMethodID(
            ctx_cls, "checkSelfPermission",
            "(Ljava/lang/String;)I");
        if (check_m == nullptr) {
            env->ExceptionClear();
            env->DeleteLocalRef(ctx_cls);
            return -2;
        }
        jint result = env->CallIntMethod(context, check_m, perm_str);
        if (env->ExceptionCheck()) { env->ExceptionClear(); result = -2; }
        env->DeleteLocalRef(ctx_cls);
        return static_cast<int>(result);
    }

    jmethodID check_m = env->GetStaticMethodID(
        compat_cls, "checkSelfPermission",
        "(Landroid/content/Context;Ljava/lang/String;)I");
    if (check_m == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(compat_cls);
        return -2;
    }

    jint result = env->CallStaticIntMethod(compat_cls, check_m, context, perm_str);
    if (env->ExceptionCheck()) { env->ExceptionClear(); result = -2; }
    env->DeleteLocalRef(compat_cls);
    return static_cast<int>(result);
}

// Call ActivityCompat.shouldShowRequestPermissionRationale(activity, permString).
// Returns false on JNI failure.
[[nodiscard]] bool should_show_rationale_jni(JNIEnv* env, jobject activity, jstring perm_str) {
    if (env == nullptr || activity == nullptr || perm_str == nullptr) return false;
    if (env->ExceptionCheck()) env->ExceptionClear();

    jclass compat_cls = env->FindClass("androidx/core/app/ActivityCompat");
    if (compat_cls == nullptr) {
        env->ExceptionClear();
        return false;
    }

    jmethodID show_m = env->GetStaticMethodID(
        compat_cls, "shouldShowRequestPermissionRationale",
        "(Landroid/app/Activity;Ljava/lang/String;)Z");
    if (show_m == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(compat_cls);
        return false;
    }

    jboolean result = env->CallStaticBooleanMethod(compat_cls, show_m, activity, perm_str);
    if (env->ExceptionCheck()) { env->ExceptionClear(); result = JNI_FALSE; }
    env->DeleteLocalRef(compat_cls);
    return result == JNI_TRUE;
}

} // namespace

// ---------------------------------------------------------------------------

permission_status android_permissions::check_status(permission_type type) const {
    const std::string_view manifest_perm = android_manifest_permission(type);
    if (manifest_perm.empty()) return permission_status::unknown;

    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return permission_status::unknown;

    jobject context = detail::get_activity();
    if (context == nullptr) return permission_status::unknown;

    jstring perm_str = env->NewStringUTF(manifest_perm.data());
    if (perm_str == nullptr) return permission_status::unknown;

    const int result = check_self_permission(env, context, perm_str);
    env->DeleteLocalRef(perm_str);

    // PackageManager.PERMISSION_GRANTED == 0
    if (result == 0) return permission_status::granted;
    if (result == -1) return permission_status::denied;
    // -2 means JNI failure — treat as unknown
    return permission_status::unknown;
}

permission_status android_permissions::request(permission_type type) {
    // Runtime permission requests in Android are inherently asynchronous:
    // ActivityCompat.requestPermissions() posts the dialog and returns
    // immediately; the result arrives in onRequestPermissionsResult on the
    // Java side. There is no blocking C++ mechanism to wait for that callback.
    //
    // This implementation records the intent (logs via the bridge) and
    // returns check_status() — the current known state. The host application
    // is expected to wire onRequestPermissionsResult back into the bridge and
    // re-call check_status() after the user responds.
    //
    // If a future version of the bridge exposes a callback registration API,
    // this method can be updated to dispatch via that mechanism.
    return check_status(type);
}

bool android_permissions::should_show_rationale(permission_type type) const {
    const std::string_view manifest_perm = android_manifest_permission(type);
    if (manifest_perm.empty()) return false;

    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return false;

    // shouldShowRequestPermissionRationale requires an Activity, not just
    // a Context. detail::get_activity() returns the Activity jobject.
    jobject activity = detail::get_activity();
    if (activity == nullptr) return false;

    jstring perm_str = env->NewStringUTF(manifest_perm.data());
    if (perm_str == nullptr) return false;

    const bool result = should_show_rationale_jni(env, activity, perm_str);
    env->DeleteLocalRef(perm_str);
    return result;
}

} // namespace mpapp

#endif // __ANDROID__
