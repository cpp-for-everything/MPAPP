// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// Android implementation of `mpapp::android_device_info`.
// All JNI usage (<jni.h>, FindClass, GetStaticFieldID, GetStaticObjectField,
// GetStringUTFChars, DeleteLocalRef, AttachCurrentThread) is confined to
// this translation unit. The header stays JNI-free.

#include "mpapp/essentials/android/device_info_android.hpp"

#if defined(__ANDROID__)

#include <jni.h>

#include <string>

#include "mpapp/handlers/android/jni_bridge.hpp"

namespace mpapp {

namespace {

// ---------------------------------------------------------------------------
// Read a static String field from a Java class and return it as a UTF-8
// std::string. Returns `fallback` when the JVM call fails at any step.
//
// Local references created inside this helper are deleted before it returns,
// except for `env` (not a local ref) and `cls` (owned by the caller).
// ---------------------------------------------------------------------------
[[nodiscard]] std::string get_static_string_field(
    JNIEnv*     env,
    jclass      cls,
    const char* field_name,
    const char* fallback)
{
    if (env == nullptr || cls == nullptr) {
        return fallback;
    }

    if (env->ExceptionCheck()) {
        env->ExceptionClear();
    }

    jfieldID fid = env->GetStaticFieldID(cls, field_name, "Ljava/lang/String;");
    if (fid == nullptr) {
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
        }
        return fallback;
    }

    auto jstr = static_cast<jstring>(env->GetStaticObjectField(cls, fid));
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        if (jstr != nullptr) {
            env->DeleteLocalRef(jstr);
        }
        return fallback;
    }
    if (jstr == nullptr) {
        return fallback;
    }

    const char* utf8 = env->GetStringUTFChars(jstr, nullptr);
    std::string result = (utf8 != nullptr) ? std::string(utf8) : fallback;
    if (utf8 != nullptr) {
        env->ReleaseStringUTFChars(jstr, utf8);
    }
    env->DeleteLocalRef(jstr);
    return result;
}

// ---------------------------------------------------------------------------
// Read android.os.Build.MODEL and android.os.Build.MANUFACTURER.
// ---------------------------------------------------------------------------
struct build_strings {
    std::string model;
    std::string manufacturer;
};

[[nodiscard]] build_strings query_build_fields(JNIEnv* env)
{
    build_strings result{"Android", "Unknown"};
    if (env == nullptr) {
        return result;
    }

    jclass build_cls = env->FindClass("android/os/Build");
    if (build_cls == nullptr) {
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
        }
        return result;
    }

    result.model        = get_static_string_field(env, build_cls, "MODEL",        "Android");
    result.manufacturer = get_static_string_field(env, build_cls, "MANUFACTURER", "Unknown");

    env->DeleteLocalRef(build_cls);
    return result;
}

// ---------------------------------------------------------------------------
// Read android.os.Build.VERSION.RELEASE.
// ---------------------------------------------------------------------------
[[nodiscard]] std::string query_os_version(JNIEnv* env)
{
    if (env == nullptr) {
        return "unknown";
    }

    jclass version_cls = env->FindClass("android/os/Build$VERSION");
    if (version_cls == nullptr) {
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
        }
        return "unknown";
    }

    std::string release =
        get_static_string_field(env, version_cls, "RELEASE", "unknown");

    env->DeleteLocalRef(version_cls);
    return release;
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// Public factory function
// ---------------------------------------------------------------------------
device_info android_device_info()
{
    device_info info;
    info.platform = device_platform::android;
    info.idiom    = device_idiom::phone;

    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) {
        // JVM unavailable — return safe defaults.
        info.model        = "Android";
        info.manufacturer = "Unknown";
        info.version      = "unknown";
        return info;
    }

    auto [model, manufacturer] = query_build_fields(env);
    info.model        = std::move(model);
    info.manufacturer = std::move(manufacturer);
    info.version      = query_os_version(env);
    return info;
}

} // namespace mpapp

#endif // defined(__ANDROID__)
