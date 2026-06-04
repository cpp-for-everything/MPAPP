// SPDX-License-Identifier: Apache-2.0
// Android basic_absolute_layout handler implementation.
//
// AbsoluteLayout maps to an android.widget.FrameLayout (the framework-level
// android.widget.AbsoluteLayout is deprecated). Each child is placed via a
// FrameLayout.LayoutParams whose leftMargin/topMargin encode the resolved
// (x, y) and whose width/height encode the resolved size. FrameLayout has no
// proportional placement, so proportional layout_flags are resolved here
// against the container's measured width/height (View.getWidth/getHeight) —
// mirroring the Linux GtkFixed handler — before the native addView call.

#include "mpapp/handlers/android/absolute_layout_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

namespace mpapp::internal {

namespace {

constexpr bool has_flag(absolute_layout_flags f, absolute_layout_flags bit) noexcept {
    return (static_cast<std::uint8_t>(f) & static_cast<std::uint8_t>(bit)) != 0;
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

// Read the container's current measured extent. Pre-layout this is zero,
// matching GtkFixed's pre-allocation behaviour — proportional children then
// resolve to 0 until a real measure pass runs.
jint view_get_int(JNIEnv* env, jobject v, const char* method_name) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/View");
    if (cls == nullptr) { env->ExceptionClear(); return 0; }
    jmethodID m = env->GetMethodID(cls, method_name, "()I");
    jint result = 0;
    if (m != nullptr) {
        result = env->CallIntMethod(v, m);
        if (env->ExceptionCheck()) { env->ExceptionClear(); result = 0; }
    }
    env->DeleteLocalRef(cls);
    return result;
}

// Construct a FrameLayout.LayoutParams(width, height) and stamp the resolved
// (x, y) into leftMargin/topMargin. width/height <= 0 fall back to
// WRAP_CONTENT (-2) so unsized children keep their intrinsic size.
constexpr jint kWrapContent = -2;

jobject make_layout_params(JNIEnv* env, int width, int height, int left, int top) {
    if (env->ExceptionCheck()) env->ExceptionClear();

    jclass lp_cls = env->FindClass("android/widget/FrameLayout$LayoutParams");
    if (lp_cls == nullptr) { env->ExceptionClear(); return nullptr; }

    jmethodID lp_ctor = env->GetMethodID(lp_cls, "<init>", "(II)V");
    if (lp_ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(lp_cls); return nullptr; }

    const jint w = width  > 0 ? static_cast<jint>(width)  : kWrapContent;
    const jint h = height > 0 ? static_cast<jint>(height) : kWrapContent;

    jobject lp = env->NewObject(lp_cls, lp_ctor, w, h);
    if (env->ExceptionCheck() || lp == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(lp_cls);
        return nullptr;
    }

    // ViewGroup.MarginLayoutParams.leftMargin / .topMargin are public int
    // fields; set them directly. FrameLayout.LayoutParams inherits them.
    jfieldID left_fid = env->GetFieldID(lp_cls, "leftMargin", "I");
    jfieldID top_fid  = env->GetFieldID(lp_cls, "topMargin", "I");
    if (left_fid != nullptr) env->SetIntField(lp, left_fid, static_cast<jint>(left));
    if (top_fid != nullptr)  env->SetIntField(lp, top_fid, static_cast<jint>(top));
    if (env->ExceptionCheck()) env->ExceptionClear();

    env->DeleteLocalRef(lp_cls);
    return lp;
}

void frame_add_view(JNIEnv* env, jobject frame, jobject child, jobject lp) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/ViewGroup");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "addView",
        "(Landroid/view/View;Landroid/view/ViewGroup$LayoutParams;)V");
    if (m != nullptr) {
        env->CallVoidMethod(frame, m, child, lp);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

// Update an already-attached child's LayoutParams via
// View.setLayoutParams(lp), then request a relayout.
void view_set_layout_params(JNIEnv* env, jobject child, jobject lp) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/View");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setLayoutParams",
        "(Landroid/view/ViewGroup$LayoutParams;)V");
    if (m != nullptr) {
        env->CallVoidMethod(child, m, lp);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

} // namespace

absolute_layout_handler<platform::android>::absolute_layout_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_frame_layout(env, detail::get_activity());
}

absolute_layout_handler<platform::android>::~absolute_layout_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (native_ != nullptr) { env->DeleteGlobalRef(native_); native_ = nullptr; }
    }
}

void absolute_layout_handler<platform::android>::apply_bounds(view& child,
                                                              const rect& r,
                                                              absolute_layout_flags f) {
    if (native_ == nullptr) return;

    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;

    jobject n = detail::android_dispatch::dispatch(&child);
    if (n == nullptr) return;

    // Resolve proportional components against the container's measured size.
    // A proportional value is a 0..1 fraction of the matching container
    // extent; otherwise it is an absolute device-independent pixel.
    const double cw = static_cast<double>(view_get_int(env, native_, "getWidth"));
    const double ch = static_cast<double>(view_get_int(env, native_, "getHeight"));

    const double x      = has_flag(f, absolute_layout_flags::x_proportional)      ? r.x * cw      : r.x;
    const double y      = has_flag(f, absolute_layout_flags::y_proportional)      ? r.y * ch      : r.y;
    const double width  = has_flag(f, absolute_layout_flags::width_proportional)  ? r.width * cw  : r.width;
    const double height = has_flag(f, absolute_layout_flags::height_proportional) ? r.height * ch : r.height;

    jobject lp = make_layout_params(env,
                                    static_cast<int>(width),
                                    static_cast<int>(height),
                                    static_cast<int>(x),
                                    static_cast<int>(y));
    if (lp == nullptr) return;
    view_set_layout_params(env, n, lp);
    env->DeleteLocalRef(lp);
}

void absolute_layout_handler<platform::android>::add_child(basic_absolute_layout& a, view& child) {
    if (native_ == nullptr) return;

    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;

    jobject n = detail::android_dispatch::dispatch(&child);
    if (n == nullptr) return;

    const rect                  r = a.get_layout_bounds(child);
    const absolute_layout_flags f = a.get_layout_flags(child);

    const double cw = static_cast<double>(view_get_int(env, native_, "getWidth"));
    const double ch = static_cast<double>(view_get_int(env, native_, "getHeight"));

    const double x      = has_flag(f, absolute_layout_flags::x_proportional)      ? r.x * cw      : r.x;
    const double y      = has_flag(f, absolute_layout_flags::y_proportional)      ? r.y * ch      : r.y;
    const double width  = has_flag(f, absolute_layout_flags::width_proportional)  ? r.width * cw  : r.width;
    const double height = has_flag(f, absolute_layout_flags::height_proportional) ? r.height * ch : r.height;

    jobject lp = make_layout_params(env,
                                    static_cast<int>(width),
                                    static_cast<int>(height),
                                    static_cast<int>(x),
                                    static_cast<int>(y));
    if (lp == nullptr) return;
    frame_add_view(env, native_, n, lp);
    env->DeleteLocalRef(lp);
}

void absolute_layout_handler<platform::android>::map_layout_bounds(basic_absolute_layout& a, view& child) {
    apply_bounds(child, a.get_layout_bounds(child), a.get_layout_flags(child));
}

void absolute_layout_handler<platform::android>::map_layout_flags(basic_absolute_layout& a, view& child) {
    apply_bounds(child, a.get_layout_bounds(child), a.get_layout_flags(child));
}

} // namespace mpapp::internal

// ---------- Self-registration --------------------------------------------
namespace {

jobject dispatch_absolute_layout(::mpapp::view* v) {
    if (auto* a = dynamic_cast<::mpapp::internal::basic_absolute_layout*>(v); a && a->has_handler()) {
        return a->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_absolute_layout); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
