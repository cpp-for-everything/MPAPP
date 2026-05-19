// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android box_view handler implementation.

#include "mpapp/handlers/android/box_view_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"

namespace mpapp {

namespace {

jint to_argb(const color& c) {
    auto clamp_255 = [](double v) {
        if (!(v == v)) return 0;
        if (v <= 0.0)  return 0;
        if (v >= 1.0)  return 255;
        return static_cast<int>(v * 255.0 + 0.5);
    };
    jint a = clamp_255(c.a);
    jint r = clamp_255(c.r);
    jint g = clamp_255(c.g);
    jint b = clamp_255(c.b);
    return (a << 24) | (r << 16) | (g << 8) | b;
}

jobject make_view(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/View");
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

void set_min_size(JNIEnv* env, jobject view, jint px) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/View");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID set_w = env->GetMethodID(cls, "setMinimumWidth",  "(I)V");
    jmethodID set_h = env->GetMethodID(cls, "setMinimumHeight", "(I)V");
    if (set_w != nullptr) {
        env->CallVoidMethod(view, set_w, px);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    if (set_h != nullptr) {
        env->CallVoidMethod(view, set_h, px);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

// Build a GradientDrawable with the current fill + corner radii and
// apply it as the view's background. Returns nothing — failures are
// swallowed after clearing the JNI exception.
void apply_background(JNIEnv* env, jobject view,
                      const color& c, const corner_radius& r) {
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

    jmethodID set_color = env->GetMethodID(drawable_cls, "setColor", "(I)V");
    if (set_color != nullptr) {
        env->CallVoidMethod(drawable, set_color, to_argb(c));
        if (env->ExceptionCheck()) env->ExceptionClear();
    }

    // setCornerRadii(float[8]) — tl, tl, tr, tr, br, br, bl, bl
    jmethodID set_radii = env->GetMethodID(drawable_cls, "setCornerRadii", "([F)V");
    if (set_radii != nullptr) {
        jfloatArray radii = env->NewFloatArray(8);
        if (radii != nullptr) {
            jfloat values[8] = {
                static_cast<jfloat>(r.top_left),     static_cast<jfloat>(r.top_left),
                static_cast<jfloat>(r.top_right),    static_cast<jfloat>(r.top_right),
                static_cast<jfloat>(r.bottom_right), static_cast<jfloat>(r.bottom_right),
                static_cast<jfloat>(r.bottom_left),  static_cast<jfloat>(r.bottom_left),
            };
            env->SetFloatArrayRegion(radii, 0, 8, values);
            env->CallVoidMethod(drawable, set_radii, radii);
            if (env->ExceptionCheck()) env->ExceptionClear();
            env->DeleteLocalRef(radii);
        }
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

box_view_handler<platform::android>::box_view_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_view(env, detail::get_activity());
    if (native_ != nullptr) {
        // MAUI BoxView default measured size is 40dp. Treat as 40 px on the
        // spike; full dp conversion lands with the unit/density work in M-06.
        set_min_size(env, native_, 80);
        // Apply the (default) fill+corners so the view has a visible
        // background even before the user sets fill explicitly.
        apply_background(env, native_, cached_fill_, cached_corners_);
    }
}

box_view_handler<platform::android>::~box_view_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (native_ != nullptr) {
            env->DeleteGlobalRef(native_);
            native_ = nullptr;
        }
    }
}

void box_view_handler<platform::android>::rebuild_background() {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    apply_background(env, native_, cached_fill_, cached_corners_);
}

void box_view_handler<platform::android>::apply_fill(const color& c) {
    cached_fill_ = c;
    rebuild_background();
}

void box_view_handler<platform::android>::apply_corners(const corner_radius& r) {
    cached_corners_ = r;
    rebuild_background();
}

void box_view_handler<platform::android>::map_fill(box_view& b) {
    apply_fill(b.fill.get());
    b.fill.changed.subscribe(fill_slot_, fill_cb_);
}

void box_view_handler<platform::android>::map_corners(box_view& b) {
    apply_corners(b.corners.get());
    b.corners.changed.subscribe(corners_slot_, corners_cb_);
}

} // namespace mpapp

#endif // __ANDROID__
