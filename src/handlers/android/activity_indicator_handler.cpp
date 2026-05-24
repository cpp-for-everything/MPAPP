// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_activity_indicator handler implementation.

#include "mpapp/handlers/android/activity_indicator_handler.hpp"

#if defined(__ANDROID__)

#include <cstdlib>
#include <string>

#include "mpapp/handlers/android/jni_bridge.hpp"

namespace mpapp::internal {

namespace {

jint parse_argb(const brush_ref& br) {
    const std::string& name = br.name;
    if (name.empty()) return static_cast<jint>(0xFF0078D7U);  // Win accent
    if (name[0] == '#') {
        unsigned long v = std::strtoul(name.c_str() + 1, nullptr, 16);
        if (name.size() == 7) return static_cast<jint>(0xFF000000U | v);
        if (name.size() == 9) return static_cast<jint>(v);
        return static_cast<jint>(0xFF0078D7U);
    }
    if (name == "Red")   return static_cast<jint>(0xFFDC3232U);
    if (name == "Green") return static_cast<jint>(0xFF50B450U);
    if (name == "Blue")  return static_cast<jint>(0xFF3C78DCU);
    if (name == "Black") return static_cast<jint>(0xFF000000U);
    if (name == "White") return static_cast<jint>(0xFFFFFFFFU);
    if (name == "Teal")  return static_cast<jint>(0xFF0096A5U);
    return static_cast<jint>(0xFF0078D7U);
}

jobject make_progress_bar(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/ProgressBar");
    if (cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(Landroid/content/Context;)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(cls); return nullptr; }
    jobject local = env->NewObject(cls, ctor, context);
    if (env->ExceptionCheck() || local == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(cls);
        return nullptr;
    }
    // ProgressBar defaults to indeterminate; ensure flag is set.
    jmethodID set_indet = env->GetMethodID(cls, "setIndeterminate", "(Z)V");
    if (set_indet != nullptr) {
        env->CallVoidMethod(local, set_indet, JNI_TRUE);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
    jobject global = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    return global;
}

} // namespace

activity_indicator_handler<platform::android>::activity_indicator_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_progress_bar(env, detail::get_activity());
}

activity_indicator_handler<platform::android>::~activity_indicator_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (native_ != nullptr) { env->DeleteGlobalRef(native_); native_ = nullptr; }
    }
}

void activity_indicator_handler<platform::android>::apply_is_running(bool v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/View");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID set_vis = env->GetMethodID(cls, "setVisibility", "(I)V");
    if (set_vis != nullptr) {
        // View.VISIBLE = 0, View.GONE = 8.
        env->CallVoidMethod(native_, set_vis, static_cast<jint>(v ? 0 : 8));
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void activity_indicator_handler<platform::android>::apply_color(const brush_ref& b) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();

    // ColorStateList.valueOf(int color) → single-state list.
    jclass csl_cls = env->FindClass("android/content/res/ColorStateList");
    if (csl_cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID value_of = env->GetStaticMethodID(
        csl_cls, "valueOf", "(I)Landroid/content/res/ColorStateList;");
    if (value_of == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(csl_cls); return; }
    jobject csl = env->CallStaticObjectMethod(csl_cls, value_of, parse_argb(b));
    env->DeleteLocalRef(csl_cls);
    if (csl == nullptr || env->ExceptionCheck()) {
        if (env->ExceptionCheck()) env->ExceptionClear();
        if (csl != nullptr) env->DeleteLocalRef(csl);
        return;
    }

    jclass pb_cls = env->FindClass("android/widget/ProgressBar");
    if (pb_cls != nullptr) {
        jmethodID set_tint = env->GetMethodID(
            pb_cls, "setIndeterminateTintList", "(Landroid/content/res/ColorStateList;)V");
        if (set_tint != nullptr) {
            env->CallVoidMethod(native_, set_tint, csl);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        env->DeleteLocalRef(pb_cls);
    }
    env->DeleteLocalRef(csl);
}

void activity_indicator_handler<platform::android>::map_is_running(basic_activity_indicator& a) {
    apply_is_running(a.is_running.get());
    a.is_running.changed.subscribe(is_running_slot_, is_running_cb_);
}

void activity_indicator_handler<platform::android>::map_color(basic_activity_indicator& a) {
    apply_color(a.color.get());
    a.color.changed.subscribe(color_slot_, color_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register basic_activity_indicator so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/android/widget_dispatch.hpp"
#include "mpapp/internal/basic_activity_indicator.hpp"

namespace {

jobject dispatch_activity_indicator(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_activity_indicator*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_activity_indicator); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
