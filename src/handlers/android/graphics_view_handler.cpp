// SPDX-License-Identifier: Apache-2.0
// Android basic_graphics_view handler implementation.
//
// Renders basic_graphics_view::drawable through the ADR-0015 canvas facade
// into an android.graphics.Bitmap whose pixels are hosted in an
// ImageView. See the header for the full paint-cycle description.

#include "mpapp/handlers/android/graphics_view_handler.hpp"

#if defined(__ANDROID__)

#include <android/bitmap.h>
#include <cstdint>
#include <cstring>

#include "mpapp/detail/graphics/canvas.hpp"
#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

namespace mpapp::internal {

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

// android.graphics.Bitmap.createBitmap(int w, int h, Bitmap.Config c).
// Returns a global ref to the freshly allocated Bitmap, or nullptr on
// failure. Config.ARGB_8888 maps to ANDROID_BITMAP_FORMAT_RGBA_8888 at
// the NDK layer (R, G, B, A byte order in memory) — see header for the
// byte-swap rationale.
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

// Copy a BGRA32-premultiplied source buffer into an RGBA-byte-ordered
// destination buffer, swapping bytes 0 and 2 per pixel. Handles
// non-tight source strides (the facade canvas may pad rows).
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
            dr[x * 4 + 0] = r;   // R from source's B position
            dr[x * 4 + 1] = g;
            dr[x * 4 + 2] = b;   // B from source's R position
            dr[x * 4 + 3] = a;
        }
    }
}

} // namespace

graphics_view_handler<platform::android>::graphics_view_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_object(env, "android/widget/ImageView", detail::get_activity());
}

graphics_view_handler<platform::android>::~graphics_view_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (bitmap_ != nullptr) { env->DeleteGlobalRef(bitmap_); bitmap_ = nullptr; }
        if (native_ != nullptr) { env->DeleteGlobalRef(native_); native_ = nullptr; }
    }
}

void graphics_view_handler<platform::android>::apply_width(int w) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    view_set_min_size(env, native_, "setMinimumWidth", w);
    repaint();
}

void graphics_view_handler<platform::android>::apply_height(int h) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    view_set_min_size(env, native_, "setMinimumHeight", h);
    repaint();
}

void graphics_view_handler<platform::android>::ensure_bitmap(int w, int h) {
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

void graphics_view_handler<platform::android>::repaint() {
    if (native_ == nullptr || bound_ == nullptr) return;
    const int w = bound_->width.get();
    const int h = bound_->height.get();
    if (w <= 0 || h <= 0) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;

    const auto& cb = bound_->drawable.get();
    if (!cb) return;  // No callback installed — leave whatever was there.

    auto canvas = detail::graphics::make_canvas(w, h);
    if (canvas == nullptr) return;
    cb(*canvas);

    ensure_bitmap(w, h);
    if (bitmap_ == nullptr) return;

    const std::uint8_t* src        = canvas->pixel_data();
    const int           src_stride = canvas->pixel_stride_bytes();
    if (src == nullptr || src_stride <= 0) return;

    void* dst_void = nullptr;
    if (AndroidBitmap_lockPixels(env, bitmap_, &dst_void) < 0 || dst_void == nullptr) {
        return;
    }
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

    // setImageBitmap on the same bitmap object forces ImageView to
    // re-render the now-updated pixels. The reference is unchanged so
    // no allocation happens here — just an invalidate.
    imageview_set_bitmap(env, native_, bitmap_);
}

void graphics_view_handler<platform::android>::map_size(basic_graphics_view& gv) {
    bound_ = &gv;
    apply_width(gv.width.get());
    apply_height(gv.height.get());
    gv.width.changed.subscribe(w_slot_, w_cb_);
    gv.height.changed.subscribe(h_slot_, h_cb_);
}

void graphics_view_handler<platform::android>::map_draw_count(basic_graphics_view& gv) {
    bound_ = &gv;
    gv.draw_count.changed.subscribe(count_slot_, count_cb_);
}

void graphics_view_handler<platform::android>::map_drawable(basic_graphics_view& gv) {
    bound_ = &gv;
    gv.drawable.changed.subscribe(drawable_slot_, drawable_cb_);
    repaint();
}

} // namespace mpapp::internal
// ---------- Self-registration --------------------------------------------
namespace {

jobject dispatch_graphics_view(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_graphics_view*>(v); w && w->has_gv_handler()) {
        return w->gv_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_graphics_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
