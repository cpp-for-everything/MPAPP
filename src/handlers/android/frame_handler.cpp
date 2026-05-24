// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_frame handler implementation. `mpapp::internal::basic_frame` is
// the deprecated MAUI-9 alias for `Border`; kept for one-to-one XAML
// migration parity. Implemented as `FrameLayout` + `GradientDrawable`
// background — same native shape as the Border handler.

#if defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable: 4996)
#endif

#include "mpapp/handlers/android/frame_handler.hpp"

#if defined(__ANDROID__)

#include <cstdlib>
#include <string>

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"
#include "mpapp/internal/basic_frame.hpp"
#include "mpapp/view.hpp"

namespace mpapp::internal {

namespace {

jint color_to_argb(const color& c) {
    auto byte = [](double v) -> unsigned long {
        if (!(v == v)) return 0;
        if (v <= 0.0) return 0;
        if (v >= 1.0) return 255;
        return static_cast<unsigned long>(v * 255.0 + 0.5);
    };
    unsigned long a = byte(c.a);
    unsigned long r = byte(c.r);
    unsigned long g = byte(c.g);
    unsigned long b = byte(c.b);
    return static_cast<jint>((a << 24) | (r << 16) | (g << 8) | b);
}

jobject make_frame_layout(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/FrameLayout");
    if (cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(Landroid/content/Context;)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(cls); return nullptr; }
    jobject local = env->NewObject(cls, ctor, context);
    if (env->ExceptionCheck() || local == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(cls);
        return nullptr;
    }
    env->DeleteLocalRef(cls);
    jobject global = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    return global;
}

void view_group_remove_all(JNIEnv* env, jobject group) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/ViewGroup");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "removeAllViews", "()V");
    if (m != nullptr) { env->CallVoidMethod(group, m); if (env->ExceptionCheck()) env->ExceptionClear(); }
    env->DeleteLocalRef(cls);
}

void view_group_add(JNIEnv* env, jobject group, jobject child) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/ViewGroup");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "addView", "(Landroid/view/View;)V");
    if (m != nullptr) { env->CallVoidMethod(group, m, child); if (env->ExceptionCheck()) env->ExceptionClear(); }
    env->DeleteLocalRef(cls);
}

void view_set_padding(JNIEnv* env, jobject view, jint l, jint t, jint r, jint b) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/View");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setPadding", "(IIII)V");
    if (m != nullptr) { env->CallVoidMethod(view, m, l, t, r, b); if (env->ExceptionCheck()) env->ExceptionClear(); }
    env->DeleteLocalRef(cls);
}

void view_set_elevation(JNIEnv* env, jobject view, jfloat elevation) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/View");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setElevation", "(F)V");
    if (m != nullptr) { env->CallVoidMethod(view, m, elevation); if (env->ExceptionCheck()) env->ExceptionClear(); }
    env->DeleteLocalRef(cls);
}

void apply_background(JNIEnv* env, jobject view,
                      jint stroke_argb, float corner_radius) {
    if (env->ExceptionCheck()) env->ExceptionClear();

    jclass drawable_cls = env->FindClass("android/graphics/drawable/GradientDrawable");
    if (drawable_cls == nullptr) { env->ExceptionClear(); return; }

    jmethodID ctor = env->GetMethodID(drawable_cls, "<init>", "()V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(drawable_cls); return; }

    jobject drawable = env->NewObject(drawable_cls, ctor);
    if (env->ExceptionCheck() || drawable == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(drawable_cls);
        return;
    }

    // setStroke(int width, int color) — MAUI Frame uses a 1-dip basic_border.
    jmethodID set_stroke = env->GetMethodID(drawable_cls, "setStroke", "(II)V");
    if (set_stroke != nullptr) {
        env->CallVoidMethod(drawable, set_stroke, static_cast<jint>(1), stroke_argb);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }

    // setCornerRadius(float) — -1 means "platform default", which we map
    // to 0 corners (no system default exists on Android).
    jmethodID set_radius = env->GetMethodID(drawable_cls, "setCornerRadius", "(F)V");
    if (set_radius != nullptr) {
        jfloat r = (corner_radius < 0.0f) ? 0.0f : corner_radius;
        env->CallVoidMethod(drawable, set_radius, r);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }

    jclass view_cls = env->FindClass("android/view/View");
    if (view_cls != nullptr) {
        jmethodID set_bg = env->GetMethodID(
            view_cls, "setBackground", "(Landroid/graphics/drawable/Drawable;)V");
        if (set_bg != nullptr) {
            env->CallVoidMethod(view, set_bg, drawable);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        env->DeleteLocalRef(view_cls);
    }

    env->DeleteLocalRef(drawable);
    env->DeleteLocalRef(drawable_cls);
}

} // namespace

frame_handler<platform::android>::frame_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_frame_layout(env, detail::get_activity());
    if (native_ != nullptr) {
        rebuild_background();
    }
}

frame_handler<platform::android>::~frame_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (native_ != nullptr) { env->DeleteGlobalRef(native_); native_ = nullptr; }
    }
}

void frame_handler<platform::android>::rebuild_background() {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    apply_background(env, native_, color_to_argb(cached_border_color_), cached_corner_radius_);
    // MAUI maps HasShadow → CardElevation on Android; without a CardView
    // we approximate with View.setElevation.
    view_set_elevation(env, native_, cached_has_shadow_ ? 4.0f : 0.0f);
}

void frame_handler<platform::android>::apply_content(const std::shared_ptr<view>& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    view_group_remove_all(env, native_);
    jobject child = v ? detail::android_dispatch::dispatch(v.get()) : nullptr;
    if (child != nullptr) view_group_add(env, native_, child);
}

void frame_handler<platform::android>::apply_border_color(const color& c)  { cached_border_color_ = c; rebuild_background(); }
void frame_handler<platform::android>::apply_has_shadow(bool b)            { cached_has_shadow_   = b; rebuild_background(); }
void frame_handler<platform::android>::apply_corner_radius(float r)        { cached_corner_radius_ = r; rebuild_background(); }

void frame_handler<platform::android>::apply_padding(const thickness& t) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    view_set_padding(env, native_,
                     static_cast<jint>(t.left + 0.5),
                     static_cast<jint>(t.top + 0.5),
                     static_cast<jint>(t.right + 0.5),
                     static_cast<jint>(t.bottom + 0.5));
}

void frame_handler<platform::android>::map_content(basic_frame& f) {
    apply_content(f.content.get());
    f.content.changed.subscribe(content_slot_, content_cb_);
}

void frame_handler<platform::android>::map_border_color(basic_frame& f) {
    apply_border_color(f.border_color.get());
    f.border_color.changed.subscribe(border_color_slot_, border_color_cb_);
}

void frame_handler<platform::android>::map_has_shadow(basic_frame& f) {
    apply_has_shadow(f.has_shadow.get());
    f.has_shadow.changed.subscribe(has_shadow_slot_, has_shadow_cb_);
}

void frame_handler<platform::android>::map_corner_radius(basic_frame& f) {
    apply_corner_radius(f.corner_radius.get());
    f.corner_radius.changed.subscribe(corner_radius_slot_, corner_radius_cb_);
}

void frame_handler<platform::android>::map_padding(basic_frame& f) {
    apply_padding(f.padding.get());
    f.padding.changed.subscribe(padding_slot_, padding_cb_);
}

void frame_handler<platform::android>::bind_content(basic_frame& f, view& child) {
    f.content.set(std::shared_ptr<view>(&child, [](view*){}));
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --

namespace {

jobject dispatch_frame(::mpapp::view* v) {
    if (auto* fr = dynamic_cast<::mpapp::internal::basic_frame*>(v); fr && fr->has_handler()) {
        return fr->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_frame); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__

#if defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#  pragma warning(pop)
#endif
