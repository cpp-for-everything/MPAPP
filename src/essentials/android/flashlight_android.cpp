// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// Android flashlight backend implementation. Confines all JNI usage to this
// translation unit; the header (flashlight_android.hpp) stays JNI-free.

#include "mpapp/essentials/android/flashlight_android.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"

#include <string>

namespace mpapp {

namespace {

// Obtain the CameraManager via Context.getSystemService("camera").
// Returns a local ref the caller must DeleteLocalRef, or nullptr on failure.
jobject get_camera_manager(JNIEnv* env, jobject context) {
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

    // Context.CAMERA_SERVICE == "camera"
    jstring service_name = env->NewStringUTF("camera");
    jobject manager = env->CallObjectMethod(context, get_service, service_name);
    if (env->ExceptionCheck()) { env->ExceptionClear(); manager = nullptr; }
    env->DeleteLocalRef(service_name);
    env->DeleteLocalRef(ctx_cls);
    return manager;
}

// Obtain the first camera ID string from CameraManager.getCameraIdList().
// Returns an empty string on failure. Caller owns no refs (all consumed here).
std::string get_first_camera_id(JNIEnv* env, jobject camera_manager) {
    if (camera_manager == nullptr) return {};
    if (env->ExceptionCheck()) env->ExceptionClear();

    jclass mgr_cls = env->GetObjectClass(camera_manager);
    if (mgr_cls == nullptr) return {};

    jmethodID get_ids = env->GetMethodID(
        mgr_cls, "getCameraIdList", "()[Ljava/lang/String;");
    env->DeleteLocalRef(mgr_cls);
    if (get_ids == nullptr) { env->ExceptionClear(); return {}; }

    auto id_array = static_cast<jobjectArray>(
        env->CallObjectMethod(camera_manager, get_ids));
    if (env->ExceptionCheck()) { env->ExceptionClear(); return {}; }
    if (id_array == nullptr) return {};

    std::string result;
    jsize len = env->GetArrayLength(id_array);
    if (len > 0) {
        auto jstr = static_cast<jstring>(env->GetObjectArrayElement(id_array, 0));
        if (jstr != nullptr) {
            const char* utf8 = env->GetStringUTFChars(jstr, nullptr);
            if (utf8 != nullptr) {
                result = utf8;
                env->ReleaseStringUTFChars(jstr, utf8);
            }
            env->DeleteLocalRef(jstr);
        }
    }
    env->DeleteLocalRef(id_array);
    return result;
}

// Call CameraManager.setTorchMode(cameraId, enabled).
// Returns true on success.
bool set_torch_mode(JNIEnv* env, jobject camera_manager,
                    const std::string& camera_id, bool enabled) {
    if (camera_manager == nullptr || camera_id.empty()) return false;
    if (env->ExceptionCheck()) env->ExceptionClear();

    jclass mgr_cls = env->GetObjectClass(camera_manager);
    if (mgr_cls == nullptr) return false;

    jmethodID set_torch = env->GetMethodID(
        mgr_cls, "setTorchMode", "(Ljava/lang/String;Z)V");
    env->DeleteLocalRef(mgr_cls);
    if (set_torch == nullptr) { env->ExceptionClear(); return false; }

    jstring id_str = env->NewStringUTF(camera_id.c_str());
    if (id_str == nullptr) return false;

    env->CallVoidMethod(camera_manager, set_torch, id_str,
                        enabled ? JNI_TRUE : JNI_FALSE);
    bool ok = !env->ExceptionCheck();
    if (!ok) env->ExceptionClear();
    env->DeleteLocalRef(id_str);
    return ok;
}

} // namespace

// ---------------------------------------------------------------------------
// android_flashlight
// ---------------------------------------------------------------------------

void android_flashlight::turn_on() {
    if (!is_supported()) return;

    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;

    jobject context = detail::get_activity();
    jobject mgr     = get_camera_manager(env, context);
    if (mgr == nullptr) return;

    std::string id = get_first_camera_id(env, mgr);
    if (set_torch_mode(env, mgr, id, true)) {
        on_ = true;
    }
    env->DeleteLocalRef(mgr);
}

void android_flashlight::turn_off() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) { on_ = false; return; }

    jobject context = detail::get_activity();
    jobject mgr     = get_camera_manager(env, context);
    if (mgr == nullptr) { on_ = false; return; }

    std::string id = get_first_camera_id(env, mgr);
    set_torch_mode(env, mgr, id, false);
    env->DeleteLocalRef(mgr);
    on_ = false;
}

bool android_flashlight::is_on() const {
    return on_;
}

bool android_flashlight::is_supported() const {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return false;

    jobject context = detail::get_activity();
    if (context == nullptr) return false;
    if (env->ExceptionCheck()) env->ExceptionClear();

    // Context.getPackageManager()
    jclass ctx_cls = env->FindClass("android/content/Context");
    if (ctx_cls == nullptr) { env->ExceptionClear(); return false; }

    jmethodID get_pm = env->GetMethodID(
        ctx_cls, "getPackageManager",
        "()Landroid/content/pm/PackageManager;");
    env->DeleteLocalRef(ctx_cls);
    if (get_pm == nullptr) { env->ExceptionClear(); return false; }

    jobject pm = env->CallObjectMethod(context, get_pm);
    if (env->ExceptionCheck()) { env->ExceptionClear(); return false; }
    if (pm == nullptr) return false;

    // PackageManager.hasSystemFeature(PackageManager.FEATURE_CAMERA_FLASH)
    // FEATURE_CAMERA_FLASH == "android.hardware.camera.flash"
    jclass pm_cls = env->GetObjectClass(pm);
    if (pm_cls == nullptr) {
        env->DeleteLocalRef(pm);
        return false;
    }

    jmethodID has_feature = env->GetMethodID(
        pm_cls, "hasSystemFeature", "(Ljava/lang/String;)Z");
    env->DeleteLocalRef(pm_cls);
    if (has_feature == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(pm);
        return false;
    }

    jstring feature = env->NewStringUTF("android.hardware.camera.flash");
    if (feature == nullptr) {
        env->DeleteLocalRef(pm);
        return false;
    }

    jboolean result = env->CallBooleanMethod(pm, has_feature, feature);
    if (env->ExceptionCheck()) { env->ExceptionClear(); result = JNI_FALSE; }

    env->DeleteLocalRef(feature);
    env->DeleteLocalRef(pm);

    return result == JNI_TRUE;
}

} // namespace mpapp

#endif // __ANDROID__
