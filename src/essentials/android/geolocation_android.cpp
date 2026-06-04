// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// Android geolocation backend implementation. Confines all JNI usage to this
// translation unit; the header (geolocation_android.hpp) stays JNI-free.

#include "mpapp/essentials/android/geolocation_android.hpp"

#if defined(__ANDROID__)

#include <cstdint>

#include "mpapp/handlers/android/jni_bridge.hpp"

namespace mpapp {

namespace {

// ---------------------------------------------------------------------------
// Helper: query LocationManager for a location using the given provider name.
// Returns a local ref to an android.location.Location, or nullptr.
// Caller must DeleteLocalRef the result.
// ---------------------------------------------------------------------------
jobject query_provider(JNIEnv* env, jobject location_manager,
                       jclass  lm_cls,    const char* provider_utf8) noexcept {
    if (env->ExceptionCheck()) { env->ExceptionClear(); }

    jstring provider = env->NewStringUTF(provider_utf8);
    if (provider == nullptr) { env->ExceptionClear(); return nullptr; }

    jmethodID get_last = env->GetMethodID(
        lm_cls, "getLastKnownLocation",
        "(Ljava/lang/String;)Landroid/location/Location;");
    if (get_last == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(provider);
        return nullptr;
    }

    jobject loc = env->CallObjectMethod(location_manager, get_last, provider);
    if (env->ExceptionCheck()) { env->ExceptionClear(); loc = nullptr; }
    env->DeleteLocalRef(provider);
    return loc;   // local ref or nullptr
}

// ---------------------------------------------------------------------------
// Helper: extract double from android.location.Location via methodName().
// Returns 0.0 on any error.
// ---------------------------------------------------------------------------
double call_double_method(JNIEnv* env, jobject loc_obj,
                          jclass loc_cls, const char* method_name) noexcept {
    jmethodID mid = env->GetMethodID(loc_cls, method_name, "()D");
    if (mid == nullptr) { env->ExceptionClear(); return 0.0; }
    double val = env->CallDoubleMethod(loc_obj, mid);
    if (env->ExceptionCheck()) { env->ExceptionClear(); return 0.0; }
    return val;
}

// ---------------------------------------------------------------------------
// Helper: extract float from android.location.Location via methodName().
// Returns 0.0f on any error.
// ---------------------------------------------------------------------------
float call_float_method(JNIEnv* env, jobject loc_obj,
                        jclass loc_cls, const char* method_name) noexcept {
    jmethodID mid = env->GetMethodID(loc_cls, method_name, "()F");
    if (mid == nullptr) { env->ExceptionClear(); return 0.0f; }
    float val = env->CallFloatMethod(loc_obj, mid);
    if (env->ExceptionCheck()) { env->ExceptionClear(); return 0.0f; }
    return val;
}

// ---------------------------------------------------------------------------
// Helper: query Location.hasAltitude() / hasBearing() / hasSpeed() /
//         hasAccuracy().  Returns false on any error.
// ---------------------------------------------------------------------------
bool call_bool_method(JNIEnv* env, jobject loc_obj,
                      jclass loc_cls, const char* method_name) noexcept {
    jmethodID mid = env->GetMethodID(loc_cls, method_name, "()Z");
    if (mid == nullptr) { env->ExceptionClear(); return false; }
    jboolean val = env->CallBooleanMethod(loc_obj, mid);
    if (env->ExceptionCheck()) { env->ExceptionClear(); return false; }
    return val == JNI_TRUE;
}

// ---------------------------------------------------------------------------
// Helper: Location.getTime() -> long (Unix epoch ms).
// ---------------------------------------------------------------------------
long long call_long_method(JNIEnv* env, jobject loc_obj,
                           jclass loc_cls, const char* method_name) noexcept {
    jmethodID mid = env->GetMethodID(loc_cls, method_name, "()J");
    if (mid == nullptr) { env->ExceptionClear(); return 0LL; }
    jlong val = env->CallLongMethod(loc_obj, mid);
    if (env->ExceptionCheck()) { env->ExceptionClear(); return 0LL; }
    return static_cast<long long>(val);
}

// ---------------------------------------------------------------------------
// Core: obtain a LocationManager from the stored Context and perform a
// getLastKnownLocation query, trying GPS first then NETWORK.
// Returns std::nullopt on any failure or when no fix is cached.
// ---------------------------------------------------------------------------
std::optional<geo_location> fetch_last_known() noexcept {
    JNIEnv*  env     = detail::attach_current_thread();
    if (env == nullptr) return std::nullopt;
    jobject  context = detail::get_activity();
    if (context == nullptr) return std::nullopt;

    if (env->ExceptionCheck()) { env->ExceptionClear(); }

    // Context.getSystemService("location") -> LocationManager
    jclass ctx_cls = env->FindClass("android/content/Context");
    if (ctx_cls == nullptr) { env->ExceptionClear(); return std::nullopt; }

    jmethodID get_service = env->GetMethodID(
        ctx_cls, "getSystemService",
        "(Ljava/lang/String;)Ljava/lang/Object;");
    env->DeleteLocalRef(ctx_cls);
    if (get_service == nullptr) { env->ExceptionClear(); return std::nullopt; }

    jstring svc_name = env->NewStringUTF("location");
    if (svc_name == nullptr) { env->ExceptionClear(); return std::nullopt; }

    jobject lm_obj = env->CallObjectMethod(context, get_service, svc_name);
    env->DeleteLocalRef(svc_name);
    if (env->ExceptionCheck()) { env->ExceptionClear(); lm_obj = nullptr; }
    if (lm_obj == nullptr) return std::nullopt;

    jclass lm_cls = env->GetObjectClass(lm_obj);
    if (lm_cls == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(lm_obj);
        return std::nullopt;
    }

    // Try GPS_PROVIDER first, then NETWORK_PROVIDER.
    jobject loc_obj = query_provider(env, lm_obj, lm_cls, "gps");
    if (loc_obj == nullptr) {
        loc_obj = query_provider(env, lm_obj, lm_cls, "network");
    }

    env->DeleteLocalRef(lm_cls);
    env->DeleteLocalRef(lm_obj);

    if (loc_obj == nullptr) return std::nullopt;

    // Populate geo_location from android.location.Location fields.
    jclass loc_cls = env->GetObjectClass(loc_obj);
    if (loc_cls == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(loc_obj);
        return std::nullopt;
    }

    geo_location fix{};

    fix.latitude  = call_double_method(env, loc_obj, loc_cls, "getLatitude");
    fix.longitude = call_double_method(env, loc_obj, loc_cls, "getLongitude");
    fix.timestamp = call_long_method  (env, loc_obj, loc_cls, "getTime");

    if (call_bool_method(env, loc_obj, loc_cls, "hasAltitude")) {
        fix.altitude = call_double_method(env, loc_obj, loc_cls, "getAltitude");
    }

    if (call_bool_method(env, loc_obj, loc_cls, "hasAccuracy")) {
        fix.accuracy = static_cast<double>(
            call_float_method(env, loc_obj, loc_cls, "getAccuracy"));
    }

    if (call_bool_method(env, loc_obj, loc_cls, "hasSpeed")) {
        fix.speed = static_cast<double>(
            call_float_method(env, loc_obj, loc_cls, "getSpeed"));
    }

    if (call_bool_method(env, loc_obj, loc_cls, "hasBearing")) {
        fix.course = static_cast<double>(
            call_float_method(env, loc_obj, loc_cls, "getBearing"));
    }

    env->DeleteLocalRef(loc_cls);
    env->DeleteLocalRef(loc_obj);

    return fix;
}

} // namespace

// ---------------------------------------------------------------------------
// android_geolocation public interface
// ---------------------------------------------------------------------------

std::optional<geo_location> android_geolocation::get_last_known() const {
    return fetch_last_known();
}

std::optional<geo_location>
android_geolocation::get_location(const geolocation_request& req) {
    last_request_ = req;
    return fetch_last_known();
}

bool android_geolocation::is_listening() const {
    return listening_;
}

void android_geolocation::start_listening(const geolocation_request& req) {
    if (listening_) return;
    last_request_ = req;
    listening_    = true;
    // TODO(follow-up): call LocationManager.requestLocationUpdates() with a
    // LocationListener registered on a looper thread. The listener callback
    // should call location_changed.emit(fix) and update the last_known cache.
    // Requires a persistent GlobalRef to a Java LocationListener object
    // (created once in this method) and removal via removeUpdates in
    // stop_listening().
}

void android_geolocation::stop_listening() {
    if (!listening_) return;
    listening_ = false;
    // TODO(follow-up): call LocationManager.removeUpdates(listener_global_ref)
    // and DeleteGlobalRef the listener object.
}

} // namespace mpapp

#endif // __ANDROID__
