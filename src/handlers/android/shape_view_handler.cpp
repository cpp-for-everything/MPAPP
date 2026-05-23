// SPDX-License-Identifier: Apache-2.0
// Android shape_view handler implementation. See header for the
// design — ImageView + Bitmap fed by the shared
// detail::graphics::render_shape_view helper (T-0031 phase 2).

#include "mpapp/handlers/android/shape_view_handler.hpp"

#if defined(__ANDROID__)

#include <android/bitmap.h>
#include <cstdint>

#include "mpapp/detail/graphics/canvas.hpp"
#include "mpapp/detail/graphics/shape_renderer.hpp"
#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

namespace mpapp {

namespace {

jobject make_object(JNIEnv* env, const char* cls_name, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass(cls_name);
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

jobject create_bitmap(JNIEnv* env, int w, int h) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass bmp_cls = env->FindClass("android/graphics/Bitmap");
    if (bmp_cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jclass cfg_cls = env->FindClass("android/graphics/Bitmap$Config");
    if (cfg_cls == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(bmp_cls);
        return nullptr;
    }
    jfieldID argb8888 = env->GetStaticFieldID(
        cfg_cls, "ARGB_8888", "Landroid/graphics/Bitmap$Config;");
    if (argb8888 == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(bmp_cls);
        env->DeleteLocalRef(cfg_cls);
        return nullptr;
    }
    jobject cfg = env->GetStaticObjectField(cfg_cls, argb8888);
    env->DeleteLocalRef(cfg_cls);
    jmethodID create_m = env->GetStaticMethodID(
        bmp_cls,
        "createBitmap",
        "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
    if (create_m == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(bmp_cls);
        if (cfg != nullptr) env->DeleteLocalRef(cfg);
        return nullptr;
    }
    jobject local = env->CallStaticObjectMethod(
        bmp_cls, create_m, static_cast<jint>(w), static_cast<jint>(h), cfg);
    env->DeleteLocalRef(bmp_cls);
    if (cfg != nullptr) env->DeleteLocalRef(cfg);
    if (env->ExceptionCheck() || local == nullptr) {
        env->ExceptionClear();
        return nullptr;
    }
    jobject global = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    return global;
}

void imageview_set_bitmap(JNIEnv* env, jobject iv, jobject bmp) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/ImageView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setImageBitmap", "(Landroid/graphics/Bitmap;)V");
    if (m != nullptr) {
        env->CallVoidMethod(iv, m, bmp);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void view_set_min_size(JNIEnv* env, jobject v, const char* method, int px) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/View");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, method, "(I)V");
    if (m != nullptr) {
        env->CallVoidMethod(v, m, static_cast<jint>(px));
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

// ImageView default ScaleType is FIT_CENTER which would shrink small
// bitmaps inside a larger container. FIT_XY stretches the bitmap to
// fill the container — but here we re-render at the container size
// after each layout change, so the bitmap pixels and view pixels
// match 1:1. CENTER (no scaling) is the right choice.
void imageview_set_scaletype_center(JNIEnv* env, jobject iv) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass iv_cls = env->FindClass("android/widget/ImageView");
    if (iv_cls == nullptr) { env->ExceptionClear(); return; }
    jclass st_cls = env->FindClass("android/widget/ImageView$ScaleType");
    if (st_cls == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(iv_cls);
        return;
    }
    jfieldID center = env->GetStaticFieldID(
        st_cls, "CENTER", "Landroid/widget/ImageView$ScaleType;");
    jobject st_obj = (center != nullptr)
                     ? env->GetStaticObjectField(st_cls, center)
                     : nullptr;
    env->DeleteLocalRef(st_cls);
    if (st_obj == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(iv_cls);
        return;
    }
    jmethodID set_m = env->GetMethodID(
        iv_cls, "setScaleType", "(Landroid/widget/ImageView$ScaleType;)V");
    if (set_m != nullptr) {
        env->CallVoidMethod(iv, set_m, st_obj);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(st_obj);
    env->DeleteLocalRef(iv_cls);
}

void imageview_set_alpha(JNIEnv* env, jobject iv, jfloat alpha) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/View");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setAlpha", "(F)V");
    if (m != nullptr) {
        env->CallVoidMethod(iv, m, alpha);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

// Install the layout-change listener. owner_ptr is the C++ handler's
// `this`; the Java listener calls back into native with (w, h) when
// the layout assigns a new size.
void install_layout_listener(JNIEnv* env, jobject view, jlong owner_ptr) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass listener_cls = env->FindClass("io/mpapp/MppShapeViewLayoutListener");
    if (listener_cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID ctor = env->GetMethodID(listener_cls, "<init>", "(J)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(listener_cls); return; }
    jobject listener = env->NewObject(listener_cls, ctor, owner_ptr);
    if (env->ExceptionCheck() || listener == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(listener_cls);
        return;
    }
    env->DeleteLocalRef(listener_cls);

    jclass view_cls = env->FindClass("android/view/View");
    if (view_cls != nullptr) {
        jmethodID add_m = env->GetMethodID(
            view_cls,
            "addOnLayoutChangeListener",
            "(Landroid/view/View$OnLayoutChangeListener;)V");
        if (add_m != nullptr) {
            env->CallVoidMethod(view, add_m, listener);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        env->DeleteLocalRef(view_cls);
    }
    env->DeleteLocalRef(listener);
}

void blit_bgra_to_rgba(const std::uint8_t* src, int src_stride,
                       std::uint8_t* dst, int dst_stride,
                       int w, int h) {
    for (int y = 0; y < h; ++y) {
        const std::uint8_t* sr = src + y * src_stride;
        std::uint8_t*       dr = dst + y * dst_stride;
        for (int x = 0; x < w; ++x) {
            const std::uint8_t b = sr[x * 4 + 0];
            const std::uint8_t g = sr[x * 4 + 1];
            const std::uint8_t r = sr[x * 4 + 2];
            const std::uint8_t a = sr[x * 4 + 3];
            dr[x * 4 + 0] = r;
            dr[x * 4 + 1] = g;
            dr[x * 4 + 2] = b;
            dr[x * 4 + 3] = a;
        }
    }
}

} // namespace

shape_view_handler<platform::android>::shape_view_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_object(env, "android/widget/ImageView", detail::get_activity());
    if (native_ != nullptr) {
        imageview_set_scaletype_center(env, native_);
        // Default minimum size — matches the legacy MppShapeView's
        // 200x80 baseline so layouts that don't override sizing get
        // a non-zero allocation.
        view_set_min_size(env, native_, "setMinimumWidth",  200);
        view_set_min_size(env, native_, "setMinimumHeight", 80);
        install_layout_listener(env, native_, reinterpret_cast<jlong>(this));
    }
}

shape_view_handler<platform::android>::~shape_view_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (bitmap_ != nullptr) { env->DeleteGlobalRef(bitmap_); bitmap_ = nullptr; }
        if (native_ != nullptr) { env->DeleteGlobalRef(native_); native_ = nullptr; }
    }
}

void shape_view_handler<platform::android>::ensure_bitmap(int w, int h) {
    if (w <= 0 || h <= 0) return;
    if (bitmap_ != nullptr && bitmap_w_ == w && bitmap_h_ == h) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (bitmap_ != nullptr) {
        env->DeleteGlobalRef(bitmap_);
        bitmap_ = nullptr;
    }
    bitmap_ = create_bitmap(env, w, h);
    if (bitmap_ != nullptr) {
        bitmap_w_ = w;
        bitmap_h_ = h;
        imageview_set_bitmap(env, native_, bitmap_);
    }
}

void shape_view_handler<platform::android>::on_layout_changed(int w, int h) {
    layout_w_ = w;
    layout_h_ = h;
    repaint();
}

void shape_view_handler<platform::android>::repaint() {
    if (native_ == nullptr || bound_ == nullptr) return;
    // Use the latest layout-assigned dimensions. If layout hasn't run
    // yet, fall back to the default minimum (200x80, matching legacy).
    int w = layout_w_ > 0 ? layout_w_ : 200;
    int h = layout_h_ > 0 ? layout_h_ : 80;

    auto canvas = detail::graphics::make_canvas(w, h);
    if (canvas == nullptr) return;
    canvas->clear(detail::graphics::color::rgba(0.0f, 0.0f, 0.0f, 0.0f));
    detail::graphics::render_shape_view(*canvas, *bound_, w, h);

    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    ensure_bitmap(w, h);
    if (bitmap_ == nullptr) return;

    const std::uint8_t* src        = canvas->pixel_data();
    const int           src_stride = canvas->pixel_stride_bytes();
    if (src == nullptr || src_stride <= 0) return;

    void* dst_void = nullptr;
    if (AndroidBitmap_lockPixels(env, bitmap_, &dst_void) < 0 || dst_void == nullptr) return;
    AndroidBitmapInfo info{};
    if (AndroidBitmap_getInfo(env, bitmap_, &info) < 0) {
        AndroidBitmap_unlockPixels(env, bitmap_);
        return;
    }
    blit_bgra_to_rgba(src, src_stride,
                      static_cast<std::uint8_t*>(dst_void),
                      static_cast<int>(info.stride),
                      w, h);
    AndroidBitmap_unlockPixels(env, bitmap_);
    imageview_set_bitmap(env, native_, bitmap_);

    // Honor shape_view.opacity at the View level too (legacy
    // MppShapeView faded the whole view, not just the paint).
    imageview_set_alpha(env, native_, static_cast<jfloat>(bound_->opacity.get()));
}

void shape_view_handler<platform::android>::map_kind(shape_view& s) {
    bound_ = &s;
    repaint();
    s.kind.changed.subscribe(kind_slot_, cb_);
}
void shape_view_handler<platform::android>::map_data(shape_view& s) {
    bound_ = &s;
    s.data.changed.subscribe(data_slot_, cb_);
}
void shape_view_handler<platform::android>::map_fill(shape_view& s) {
    bound_ = &s;
    s.fill.changed.subscribe(fill_slot_, cb_);
}
void shape_view_handler<platform::android>::map_stroke(shape_view& s) {
    bound_ = &s;
    s.stroke.changed.subscribe(stroke_slot_, cb_);
}
void shape_view_handler<platform::android>::map_stroke_thickness(shape_view& s) {
    bound_ = &s;
    s.stroke_thickness.changed.subscribe(stroke_thick_slot_, cb_);
}
void shape_view_handler<platform::android>::map_opacity(shape_view& s) {
    bound_ = &s;
    if (native_ != nullptr) {
        JNIEnv* env = detail::attach_current_thread();
        if (env != nullptr) imageview_set_alpha(env, native_, static_cast<jfloat>(s.opacity.get()));
    }
    s.opacity.changed.subscribe(opacity_slot_, cb_);
}

} // namespace mpapp

// JNI trampoline for MppShapeViewLayoutListener.onLayoutChange. The
// Java listener supplies owner_ptr (the C++ handler `this`) plus the
// new (w, h); we reinterpret and route into on_layout_changed.
extern "C" JNIEXPORT void JNICALL
Java_io_mpapp_MppShapeViewLayoutListener_nativeOnLayoutChanged(
    JNIEnv* /*env*/,
    jclass  /*cls*/,
    jlong   owner_ptr,
    jint    w,
    jint    h) {
    auto* self = reinterpret_cast<mpapp::shape_view_handler<mpapp::platform::android>*>(owner_ptr);
    if (self != nullptr) self->on_layout_changed(static_cast<int>(w), static_cast<int>(h));
}

// ---------- Self-registration --------------------------------------------
namespace {

jobject dispatch_shape_view(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::shape_view*>(v); w && w->has_sv_handler()) {
        return w->sv_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_shape_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
