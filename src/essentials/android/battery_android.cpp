// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// Android battery backend implementation.  Confines <jni.h> and every JNI
// call to this translation unit behind an #if defined(__ANDROID__) guard,
// mirroring the project's Android handler pattern.  The public header
// (battery_android.hpp) stays JNI-free.

#include "mpapp/essentials/android/battery_android.hpp"

#if defined(__ANDROID__)

#include <jni.h>

#include "mpapp/handlers/android/jni_bridge.hpp"

namespace mpapp {

// ============================================================================
// Internal helpers
// ============================================================================

namespace {

// Obtain android.os.BatteryManager via Context.getSystemService.
// Uses the string literal "batterymanager" which equals
// Context.BATTERY_SERVICE on API 21+.
// Returns a local ref the caller must DeleteLocalRef, or nullptr on failure.
jobject acquire_battery_manager(JNIEnv* env, jobject context) {
    if (env == nullptr || context == nullptr) return nullptr;
    if (env->ExceptionCheck()) env->ExceptionClear();

    jclass ctx_cls = env->FindClass("android/content/Context");
    if (ctx_cls == nullptr) { env->ExceptionClear(); return nullptr; }

    jmethodID get_service = env->GetMethodID(
        ctx_cls, "getSystemService",
        "(Ljava/lang/String;)Ljava/lang/Object;");
    env->DeleteLocalRef(ctx_cls);
    if (get_service == nullptr) { env->ExceptionClear(); return nullptr; }

    // Context.BATTERY_SERVICE == "batterymanager" (API 21+, stable string).
    jstring svc_name = env->NewStringUTF("batterymanager");
    if (svc_name == nullptr) { env->ExceptionClear(); return nullptr; }

    jobject mgr = env->CallObjectMethod(context, get_service, svc_name);
    env->DeleteLocalRef(svc_name);
    if (env->ExceptionCheck() || mgr == nullptr) {
        env->ExceptionClear();
        if (mgr != nullptr) env->DeleteLocalRef(mgr);
        return nullptr;
    }
    return mgr;
}

// Query a BatteryManager.getIntProperty(int) value.
// Returns -1 on any JNI failure.
jint battery_manager_get_int_property(JNIEnv* env, jobject mgr, jint property) {
    if (env == nullptr || mgr == nullptr) return -1;

    jclass mgr_cls = env->GetObjectClass(mgr);
    if (mgr_cls == nullptr) { env->ExceptionClear(); return -1; }

    jmethodID get_prop = env->GetMethodID(mgr_cls, "getIntProperty", "(I)I");
    env->DeleteLocalRef(mgr_cls);
    if (get_prop == nullptr) { env->ExceptionClear(); return -1; }

    jint result = env->CallIntMethod(mgr, get_prop, property);
    if (env->ExceptionCheck()) { env->ExceptionClear(); return -1; }
    return result;
}

// Query a sticky broadcast intent extra (int) via Context.registerReceiver with
// a null BroadcastReceiver, which returns the last sticky broadcast as-is.
//
// action_class  — fully-qualified class owning the action constant string
//                 (e.g. "android/content/Intent").
// action_field  — static String field name (e.g. "ACTION_BATTERY_CHANGED").
// extra_class   — class owning the extra key constant
//                 (same as action_class for Intent extras on BatteryManager).
// extra_field   — static String field name (e.g. "EXTRA_STATUS").
// default_value — returned on any JNI failure.
jint query_sticky_intent_extra_int(
    JNIEnv*     env,
    jobject     context,
    const char* action_field,
    const char* extra_field,
    jint        default_value) {

    if (env == nullptr || context == nullptr) return default_value;
    if (env->ExceptionCheck()) env->ExceptionClear();

    // ---- Build Intent filter -------------------------------------------
    jclass intent_filter_cls = env->FindClass("android/content/IntentFilter");
    if (intent_filter_cls == nullptr) { env->ExceptionClear(); return default_value; }

    // Retrieve Intent.<action_field> static String.
    jclass intent_cls = env->FindClass("android/content/Intent");
    if (intent_cls == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(intent_filter_cls);
        return default_value;
    }

    jfieldID action_fid = env->GetStaticFieldID(
        intent_cls, action_field, "Ljava/lang/String;");
    if (action_fid == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(intent_cls);
        env->DeleteLocalRef(intent_filter_cls);
        return default_value;
    }

    auto action_str = static_cast<jstring>(
        env->GetStaticObjectField(intent_cls, action_fid));
    if (action_str == nullptr || env->ExceptionCheck()) {
        env->ExceptionClear();
        if (action_str != nullptr) env->DeleteLocalRef(action_str);
        env->DeleteLocalRef(intent_cls);
        env->DeleteLocalRef(intent_filter_cls);
        return default_value;
    }

    // IntentFilter(String action)
    jmethodID if_ctor = env->GetMethodID(
        intent_filter_cls, "<init>", "(Ljava/lang/String;)V");
    if (if_ctor == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(action_str);
        env->DeleteLocalRef(intent_cls);
        env->DeleteLocalRef(intent_filter_cls);
        return default_value;
    }

    jobject intent_filter = env->NewObject(intent_filter_cls, if_ctor, action_str);
    env->DeleteLocalRef(action_str);
    env->DeleteLocalRef(intent_filter_cls);
    if (env->ExceptionCheck() || intent_filter == nullptr) {
        env->ExceptionClear();
        if (intent_filter != nullptr) env->DeleteLocalRef(intent_filter);
        env->DeleteLocalRef(intent_cls);
        return default_value;
    }

    // ---- Context.registerReceiver(null, filter) -> sticky Intent ------
    jclass ctx_cls = env->FindClass("android/content/Context");
    if (ctx_cls == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(intent_filter);
        env->DeleteLocalRef(intent_cls);
        return default_value;
    }

    jmethodID reg_receiver = env->GetMethodID(
        ctx_cls, "registerReceiver",
        "(Landroid/content/BroadcastReceiver;"
        "Landroid/content/IntentFilter;)"
        "Landroid/content/Intent;");
    env->DeleteLocalRef(ctx_cls);
    if (reg_receiver == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(intent_filter);
        env->DeleteLocalRef(intent_cls);
        return default_value;
    }

    // Passing null as the BroadcastReceiver queries the last sticky intent.
    jobject sticky_intent = env->CallObjectMethod(
        context, reg_receiver, nullptr, intent_filter);
    env->DeleteLocalRef(intent_filter);
    if (env->ExceptionCheck() || sticky_intent == nullptr) {
        env->ExceptionClear();
        if (sticky_intent != nullptr) env->DeleteLocalRef(sticky_intent);
        env->DeleteLocalRef(intent_cls);
        return default_value;
    }

    // ---- Retrieve Intent.<extra_field> static String key --------------
    jfieldID extra_fid = env->GetStaticFieldID(
        intent_cls, extra_field, "Ljava/lang/String;");
    env->DeleteLocalRef(intent_cls);
    if (extra_fid == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(sticky_intent);
        return default_value;
    }

    // We need intent_cls again for the field — re-find with GetObjectClass.
    jclass sticky_cls = env->GetObjectClass(sticky_intent);
    if (sticky_cls == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(sticky_intent);
        return default_value;
    }

    // Re-acquire intent_cls to read the static field properly.
    // (extra_fid was obtained from intent_cls which was deleted; use a fresh
    //  FindClass to get the extra key string safely.)
    jclass intent_cls2 = env->FindClass("android/content/Intent");
    if (intent_cls2 == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(sticky_cls);
        env->DeleteLocalRef(sticky_intent);
        return default_value;
    }

    jfieldID extra_fid2 = env->GetStaticFieldID(
        intent_cls2, extra_field, "Ljava/lang/String;");
    if (extra_fid2 == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(intent_cls2);
        env->DeleteLocalRef(sticky_cls);
        env->DeleteLocalRef(sticky_intent);
        return default_value;
    }

    auto extra_key = static_cast<jstring>(
        env->GetStaticObjectField(intent_cls2, extra_fid2));
    env->DeleteLocalRef(intent_cls2);
    if (extra_key == nullptr || env->ExceptionCheck()) {
        env->ExceptionClear();
        if (extra_key != nullptr) env->DeleteLocalRef(extra_key);
        env->DeleteLocalRef(sticky_cls);
        env->DeleteLocalRef(sticky_intent);
        return default_value;
    }

    // ---- Intent.getIntExtra(String name, int defaultValue) ------------
    jmethodID get_int_extra = env->GetMethodID(
        sticky_cls, "getIntExtra", "(Ljava/lang/String;I)I");
    env->DeleteLocalRef(sticky_cls);
    if (get_int_extra == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(extra_key);
        env->DeleteLocalRef(sticky_intent);
        return default_value;
    }

    jint value = env->CallIntMethod(sticky_intent, get_int_extra,
                                     extra_key, default_value);
    env->DeleteLocalRef(extra_key);
    env->DeleteLocalRef(sticky_intent);
    if (env->ExceptionCheck()) { env->ExceptionClear(); return default_value; }
    return value;
}

// Map Android BatteryManager.BATTERY_STATUS_* int to battery_state.
// Constants from android.os.BatteryManager (stable API 5+):
//   BATTERY_STATUS_UNKNOWN    = 1
//   BATTERY_STATUS_CHARGING   = 2
//   BATTERY_STATUS_DISCHARGING= 3
//   BATTERY_STATUS_NOT_CHARGING=4
//   BATTERY_STATUS_FULL       = 5
battery_state map_battery_status(jint status) noexcept {
    switch (status) {
        case 2:  return battery_state::charging;
        case 3:  return battery_state::discharging;
        case 4:  return battery_state::not_charging;
        case 5:  return battery_state::full;
        default: return battery_state::unknown;
    }
}

// Map Android EXTRA_PLUGGED int to battery_power_source.
// 0 = on battery (not plugged); 1 = AC; 2 = USB; 4 = Wireless.
battery_power_source map_plugged(jint plugged) noexcept {
    switch (plugged) {
        case 0:  return battery_power_source::battery;
        case 1:  return battery_power_source::ac;
        case 2:  return battery_power_source::usb;
        case 4:  return battery_power_source::wireless;
        default: return battery_power_source::unknown;
    }
}

} // namespace

// ============================================================================
// android_battery — public interface implementation
// ============================================================================

double android_battery::charge_level() const {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return -1.0;

    jobject context = detail::get_activity();
    jobject mgr     = acquire_battery_manager(env, context);
    if (mgr == nullptr) return -1.0;

    // BatteryManager.BATTERY_PROPERTY_CAPACITY == 4 (API 21+, stable).
    constexpr jint BATTERY_PROPERTY_CAPACITY = 4;
    jint capacity = battery_manager_get_int_property(env, mgr, BATTERY_PROPERTY_CAPACITY);
    env->DeleteLocalRef(mgr);

    if (capacity < 0 || capacity > 100) return -1.0;
    return static_cast<double>(capacity) / 100.0;
}

battery_state android_battery::state() const {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return battery_state::unknown;

    jobject context = detail::get_activity();

    // Query EXTRA_STATUS from the sticky ACTION_BATTERY_CHANGED broadcast.
    jint status = query_sticky_intent_extra_int(
        env, context,
        "ACTION_BATTERY_CHANGED",  // Intent.ACTION_BATTERY_CHANGED field
        "EXTRA_STATUS",            // BatteryManager.EXTRA_STATUS field
        1                          // default = BATTERY_STATUS_UNKNOWN
    );

    return map_battery_status(status);
}

battery_power_source android_battery::power_source() const {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return battery_power_source::unknown;

    jobject context = detail::get_activity();

    // Query EXTRA_PLUGGED from the sticky ACTION_BATTERY_CHANGED broadcast.
    // Default -1 means "unknown / not set".
    jint plugged = query_sticky_intent_extra_int(
        env, context,
        "ACTION_BATTERY_CHANGED",  // Intent.ACTION_BATTERY_CHANGED field
        "EXTRA_PLUGGED",           // BatteryManager.EXTRA_PLUGGED field
        -1                         // default = unknown
    );

    if (plugged < 0) return battery_power_source::unknown;
    return map_plugged(plugged);
}

energy_saver_status android_battery::energy_saver() const {
    // PowerManager.isPowerSaveMode() would be ideal (API 21+) but requires
    // the PowerManager service and is deferred to a future iteration.
    return energy_saver_status::unknown;
}

} // namespace mpapp

#endif // __ANDROID__
