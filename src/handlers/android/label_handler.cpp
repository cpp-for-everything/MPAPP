// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — Android basic_label handler implementation.

#include "mpapp/handlers/android/label_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"

namespace mpapp::internal {

namespace {

jobject make_text_view(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/TextView");
    if (cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(Landroid/content/Context;)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(cls); return nullptr; }
    jobject local = env->NewObject(cls, ctor, context);
    if (env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(cls); return nullptr; }
    env->DeleteLocalRef(cls);
    if (local == nullptr) return nullptr;
    jobject global = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    return global;
}

void text_view_set_text(JNIEnv* env, jobject tv, const std::string& text) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/TextView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setText", "(Ljava/lang/CharSequence;)V");
    if (m != nullptr) {
        jstring jstr = env->NewStringUTF(text.c_str());
        env->CallVoidMethod(tv, m, jstr);
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->DeleteLocalRef(jstr);
    }
    env->DeleteLocalRef(cls);
}

// TypedValue.COMPLEX_UNIT_PT == 3 — interpret font_size as points so the
// sizing matches the desktop handlers' point semantics.
void text_view_set_text_size(JNIEnv* env, jobject tv, float pt) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/TextView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setTextSize", "(IF)V");
    if (m != nullptr) {
        env->CallVoidMethod(tv, m, /*COMPLEX_UNIT_PT*/ 3, pt);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

// Apply a typeface derived from family ("" = default) + bold flag.
// Typeface styles: NORMAL=0, BOLD=1.
void text_view_set_typeface(JNIEnv* env, jobject tv,
                            const std::string& family, bool bold) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass tvcls = env->FindClass("android/widget/TextView");
    if (tvcls == nullptr) { env->ExceptionClear(); return; }
    jclass tfcls = env->FindClass("android/graphics/Typeface");
    if (tfcls == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(tvcls); return; }

    jobject tf = nullptr;   // null -> default family
    if (!family.empty()) {
        jmethodID create = env->GetStaticMethodID(
            tfcls, "create", "(Ljava/lang/String;I)Landroid/graphics/Typeface;");
        if (create != nullptr) {
            jstring jfam = env->NewStringUTF(family.c_str());
            tf = env->CallStaticObjectMethod(tfcls, create, jfam, /*NORMAL*/ 0);
            if (env->ExceptionCheck()) { env->ExceptionClear(); tf = nullptr; }
            env->DeleteLocalRef(jfam);
        }
    }

    jmethodID setTf = env->GetMethodID(
        tvcls, "setTypeface", "(Landroid/graphics/Typeface;I)V");
    if (setTf != nullptr) {
        env->CallVoidMethod(tv, setTf, tf, bold ? 1 : 0);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    if (tf != nullptr) env->DeleteLocalRef(tf);
    env->DeleteLocalRef(tfcls);
    env->DeleteLocalRef(tvcls);
}

void text_view_set_text_color(JNIEnv* env, jobject tv, int argb) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/TextView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setTextColor", "(I)V");
    if (m != nullptr) {
        env->CallVoidMethod(tv, m, static_cast<jint>(argb));
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

} // namespace

label_handler<platform::android>::label_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env != nullptr) {
        native_ = make_text_view(env, detail::get_activity());
    }
}

label_handler<platform::android>::~label_handler() {
    if (native_ != nullptr) {
        if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
            env->DeleteGlobalRef(native_);
        }
        native_ = nullptr;
    }
}

void label_handler<platform::android>::apply_text(const std::string& text) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    text_view_set_text(env, native_, text);
}

void label_handler<platform::android>::apply_font_size(double pt) {
    if (native_ == nullptr || pt <= 0.0) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    text_view_set_text_size(env, native_, static_cast<float>(pt));
}

void label_handler<platform::android>::apply_typeface() {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    text_view_set_typeface(env, native_, font_family_, font_bold_);
}

void label_handler<platform::android>::map_text(basic_label& l) {
    apply_text(l.text.get());
    l.text.changed.subscribe(text_slot_, text_cb_);
}

void label_handler<platform::android>::map_font_size(basic_label& l) {
    apply_font_size(l.font_size.get());
    l.font_size.changed.subscribe(fsize_slot_, fsize_cb_);
}

void label_handler<platform::android>::map_font_bold(basic_label& l) {
    font_bold_ = l.font_bold.get();
    apply_typeface();
    l.font_bold.changed.subscribe(fbold_slot_, fbold_cb_);
}

void label_handler<platform::android>::apply_text_color(const color& c) {
    if (native_ == nullptr || c.a <= 0.0) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    auto to8 = [](double v) -> int {
        if (v < 0.0) v = 0.0;
        if (v > 1.0) v = 1.0;
        return static_cast<int>(v * 255.0 + 0.5);
    };
    const int argb = (to8(c.a) << 24) | (to8(c.r) << 16) |
                     (to8(c.g) << 8)  |  to8(c.b);
    text_view_set_text_color(env, native_, argb);
}

void label_handler<platform::android>::map_font_family(basic_label& l) {
    font_family_ = l.font_family.get();
    apply_typeface();
    l.font_family.changed.subscribe(ffamily_slot_, ffamily_cb_);
}

void label_handler<platform::android>::map_text_color(basic_label& l) {
    apply_text_color(l.text_color.get());
    l.text_color.changed.subscribe(tcolor_slot_, tcolor_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register basic_label so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/android/widget_dispatch.hpp"
#include "mpapp/internal/basic_label.hpp"

namespace {

jobject dispatch_label(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_label*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_label); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
