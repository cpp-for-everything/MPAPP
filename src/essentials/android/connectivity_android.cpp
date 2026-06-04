// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// Android connectivity backend implementation. Confines all JNI usage to this
// translation unit; the header (connectivity_android.hpp) stays JNI-free.

#include "mpapp/essentials/android/connectivity_android.hpp"

#if defined(__ANDROID__)

#include <jni.h>

#include "mpapp/handlers/android/jni_bridge.hpp"

namespace mpapp {

namespace {

// android.net.NetworkCapabilities integer constants (API 21+).
// Defined here to avoid depending on android/net/NetworkCapabilities.h which
// may not be present in all NDK configurations.
constexpr jint k_net_capability_internet   = 12;
constexpr jint k_net_capability_validated  = 16;

// Obtain ConnectivityManager via Context.getSystemService("connectivity").
// Returns a local ref the caller must DeleteLocalRef, or nullptr on failure.
jobject get_connectivity_manager(JNIEnv* env, jobject context) {
    if (context == nullptr) return nullptr;
    if (env->ExceptionCheck()) env->ExceptionClear();

    jclass ctx_cls = env->FindClass("android/content/Context");
    if (ctx_cls == nullptr) { env->ExceptionClear(); return nullptr; }

    jmethodID get_service = env->GetMethodID(
        ctx_cls, "getSystemService",
        "(Ljava/lang/String;)Ljava/lang/Object;");
    if (get_service == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(ctx_cls);
        return nullptr;
    }

    // Context.CONNECTIVITY_SERVICE == "connectivity"
    jstring service_name = env->NewStringUTF("connectivity");
    jobject manager = env->CallObjectMethod(context, get_service, service_name);
    if (env->ExceptionCheck()) { env->ExceptionClear(); manager = nullptr; }
    env->DeleteLocalRef(service_name);
    env->DeleteLocalRef(ctx_cls);
    return manager;
}

// Returns true when the given NetworkCapabilities object (local ref) reports
// the requested integer capability via hasCapability(int).
bool has_capability(JNIEnv* env, jobject caps, jint capability) {
    if (caps == nullptr) return false;

    jclass caps_cls = env->GetObjectClass(caps);
    if (caps_cls == nullptr) return false;

    jmethodID has_cap = env->GetMethodID(caps_cls, "hasCapability", "(I)Z");
    env->DeleteLocalRef(caps_cls);
    if (has_cap == nullptr) { env->ExceptionClear(); return false; }

    jboolean result = env->CallBooleanMethod(caps, has_cap, capability);
    if (env->ExceptionCheck()) { env->ExceptionClear(); return false; }
    return result == JNI_TRUE;
}

} // namespace

network_access android_connectivity::access() const {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return network_access::none;

    jobject context = detail::get_activity();
    jobject manager = get_connectivity_manager(env, context);
    if (manager == nullptr) return network_access::none;

    network_access result = network_access::none;

    // ConnectivityManager.getActiveNetwork() -> Network (API 23+).
    jclass mgr_cls = env->GetObjectClass(manager);
    if (mgr_cls == nullptr) {
        env->DeleteLocalRef(manager);
        return network_access::none;
    }

    jmethodID get_active = env->GetMethodID(
        mgr_cls, "getActiveNetwork", "()Landroid/net/Network;");
    if (get_active == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(mgr_cls);
        env->DeleteLocalRef(manager);
        return network_access::none;
    }

    jobject network = env->CallObjectMethod(manager, get_active);
    if (env->ExceptionCheck()) { env->ExceptionClear(); network = nullptr; }

    if (network != nullptr) {
        // ConnectivityManager.getNetworkCapabilities(Network) -> NetworkCapabilities.
        jmethodID get_caps = env->GetMethodID(
            mgr_cls, "getNetworkCapabilities",
            "(Landroid/net/Network;)Landroid/net/NetworkCapabilities;");
        if (get_caps == nullptr) {
            env->ExceptionClear();
            // Active network exists but we cannot inspect capabilities.
            result = network_access::local;
        } else {
            jobject caps = env->CallObjectMethod(manager, get_caps, network);
            if (env->ExceptionCheck()) { env->ExceptionClear(); caps = nullptr; }

            if (caps != nullptr) {
                const bool has_internet  = has_capability(env, caps, k_net_capability_internet);
                const bool has_validated = has_capability(env, caps, k_net_capability_validated);

                if (has_internet && has_validated) {
                    result = network_access::internet;
                } else {
                    // Network present but not fully validated or no internet capability.
                    result = network_access::local;
                }
                env->DeleteLocalRef(caps);
            } else {
                // Active network exists but capabilities could not be retrieved.
                result = network_access::local;
            }
        }
        env->DeleteLocalRef(network);
    }
    // else: result stays network_access::none — no active network.

    env->DeleteLocalRef(mgr_cls);
    env->DeleteLocalRef(manager);
    return result;
}

} // namespace mpapp

#endif // __ANDROID__
