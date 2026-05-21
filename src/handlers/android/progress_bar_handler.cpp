// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android progress_bar handler implementation.

#include "mpapp/handlers/android/progress_bar_handler.hpp"

#if defined(__ANDROID__)

#include <cstdlib>
#include <string>

#include "mpapp/handlers/android/jni_bridge.hpp"

namespace mpapp {

namespace {

constexpr jint kAndroidProgressMax = 10000;

jint parse_argb(const brush_ref& br, jint fallback) {
    const std::string& name = br.name;
    if (name.empty()) return fallback;
    if (name[0] == '#') {
        unsigned long v = std::strtoul(name.c_str() + 1, nullptr, 16);
        if (name.size() == 7) return static_cast<jint>(0xFF000000U | v);
        if (name.size() == 9) return static_cast<jint>(v);
        return fallback;
    }
    if (name == "Red")   return static_cast<jint>(0xFFDC3232U);
    if (name == "Green") return static_cast<jint>(0xFF50B450U);
    if (name == "Blue")  return static_cast<jint>(0xFF3C78DCU);
    if (name == "Black") return static_cast<jint>(0xFF000000U);
    if (name == "White") return static_cast<jint>(0xFFFFFFFFU);
    if (name == "Teal")  return static_cast<jint>(0xFF0096A5U);
    if (name == "Gray")  return static_cast<jint>(0xFF808080U);
    return fallback;
}

jobject make_progress_bar(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/ProgressBar");
    if (cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(Landroid/content/Context;)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(cls); return nullptr; }
    jobject local = env->NewObject(cls, ctor, context);
    if (env->ExceptionCheck() || local == nullptr) {
        env->ExceptionClear(); env->DeleteLocalRef(cls); return nullptr;
    }
    // Switch to determinate mode + set max.
    jmethodID set_indet = env->GetMethodID(cls, "setIndeterminate", "(Z)V");
    if (set_indet != nullptr) {
        env->CallVoidMethod(local, set_indet, JNI_FALSE);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    jmethodID set_max = env->GetMethodID(cls, "setMax", "(I)V");
    if (set_max != nullptr) {
        env->CallVoidMethod(local, set_max, kAndroidProgressMax);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
    jobject global = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    return global;
}

jobject color_state_list(JNIEnv* env, jint argb) {
    jclass cls = env->FindClass("android/content/res/ColorStateList");
    if (cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID value_of = env->GetStaticMethodID(
        cls, "valueOf", "(I)Landroid/content/res/ColorStateList;");
    if (value_of == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(cls); return nullptr; }
    jobject csl = env->CallStaticObjectMethod(cls, value_of, argb);
    env->DeleteLocalRef(cls);
    if (env->ExceptionCheck()) { env->ExceptionClear(); if (csl) env->DeleteLocalRef(csl); return nullptr; }
    return csl;
}

void set_tint(JNIEnv* env, jobject view, const char* method, jint argb) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jobject csl = color_state_list(env, argb);
    if (csl == nullptr) return;
    jclass cls = env->FindClass("android/widget/ProgressBar");
    if (cls != nullptr) {
        jmethodID m = env->GetMethodID(cls, method, "(Landroid/content/res/ColorStateList;)V");
        if (m != nullptr) {
            env->CallVoidMethod(view, m, csl);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        env->DeleteLocalRef(cls);
    }
    env->DeleteLocalRef(csl);
}

} // namespace

progress_bar_handler<platform::android>::progress_bar_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_progress_bar(env, detail::get_activity());
}

progress_bar_handler<platform::android>::~progress_bar_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (native_ != nullptr) { env->DeleteGlobalRef(native_); native_ = nullptr; }
    }
}

void progress_bar_handler<platform::android>::apply_progress(double v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    if (v < 0) v = 0; if (v > 1) v = 1;
    jclass cls = env->FindClass("android/widget/ProgressBar");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID set_prog = env->GetMethodID(cls, "setProgress", "(I)V");
    if (set_prog != nullptr) {
        env->CallVoidMethod(native_, set_prog, static_cast<jint>(v * kAndroidProgressMax + 0.5));
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void progress_bar_handler<platform::android>::apply_color(const brush_ref& b) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    set_tint(env, native_, "setProgressTintList", parse_argb(b, static_cast<jint>(0xFF0078D7U)));
}

void progress_bar_handler<platform::android>::apply_background_color(const brush_ref& b) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    set_tint(env, native_, "setProgressBackgroundTintList", parse_argb(b, static_cast<jint>(0xFFF0F0F0U)));
}

void progress_bar_handler<platform::android>::map_progress(progress_bar& p) {
    apply_progress(p.progress.get());
    p.progress.changed.subscribe(progress_slot_, progress_cb_);
}
void progress_bar_handler<platform::android>::map_color(progress_bar& p) {
    apply_color(p.color.get());
    p.color.changed.subscribe(color_slot_, color_cb_);
}
void progress_bar_handler<platform::android>::map_background_color(progress_bar& p) {
    apply_background_color(p.background_color.get());
    p.background_color.changed.subscribe(bg_slot_, bg_cb_);
}

} // namespace mpapp


// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register progress_bar so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/android/widget_dispatch.hpp"
#include "mpapp/progress_bar.hpp"

namespace {

jobject dispatch_progress_bar(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::progress_bar*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_progress_bar); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
