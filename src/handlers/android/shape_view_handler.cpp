// SPDX-License-Identifier: Apache-2.0
// Android shape_view handler implementation.

#include "mpapp/handlers/android/shape_view_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

namespace mpapp {

namespace {

// Parse #RRGGBB / #AARRGGBB / #RGB into a 32-bit ARGB int; 0 sentinel = no color.
int parse_argb(const std::string& s) {
    if (s.empty() || s[0] != '#') return 0;
    auto hex_val = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };
    auto byte = [&](std::size_t i) -> int {
        int hi = hex_val(s[i]);
        int lo = hex_val(s[i + 1]);
        if (hi < 0 || lo < 0) return -1;
        return (hi << 4) | lo;
    };
    if (s.size() == 7) {
        int r = byte(1), g = byte(3), b = byte(5);
        if (r < 0 || g < 0 || b < 0) return 0;
        return (0xFF << 24) | (r << 16) | (g << 8) | b;
    }
    if (s.size() == 9) {
        int a = byte(1), r = byte(3), g = byte(5), b = byte(7);
        if (a < 0 || r < 0 || g < 0 || b < 0) return 0;
        return (a << 24) | (r << 16) | (g << 8) | b;
    }
    if (s.size() == 4) {
        int r = hex_val(s[1]), g = hex_val(s[2]), b = hex_val(s[3]);
        if (r < 0 || g < 0 || b < 0) return 0;
        return (0xFF << 24) | ((r * 0x11) << 16) | ((g * 0x11) << 8) | (b * 0x11);
    }
    return 0;
}

jobject make_shape_view(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("io/mpapp/MppShapeView");
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

void call_void_int(JNIEnv* env, jobject obj, const char* method, jint v) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("io/mpapp/MppShapeView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, method, "(I)V");
    if (m != nullptr) {
        env->CallVoidMethod(obj, m, v);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void call_void_float(JNIEnv* env, jobject obj, const char* method, jfloat v) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("io/mpapp/MppShapeView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, method, "(F)V");
    if (m != nullptr) {
        env->CallVoidMethod(obj, m, v);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void call_void_string(JNIEnv* env, jobject obj, const char* method, const char* v) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("io/mpapp/MppShapeView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, method, "(Ljava/lang/String;)V");
    if (m != nullptr) {
        jstring s = env->NewStringUTF(v);
        env->CallVoidMethod(obj, m, s);
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->DeleteLocalRef(s);
    }
    env->DeleteLocalRef(cls);
}

} // namespace

shape_view_handler<platform::android>::shape_view_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_shape_view(env, detail::get_activity());
}

shape_view_handler<platform::android>::~shape_view_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (native_ != nullptr) { env->DeleteGlobalRef(native_); native_ = nullptr; }
    }
}

void shape_view_handler<platform::android>::apply_kind(shape_kind v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    call_void_int(env, native_, "setShapeKind", static_cast<jint>(v));
}

void shape_view_handler<platform::android>::apply_data(const std::string& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    call_void_string(env, native_, "setShapeData", v.c_str());
}

void shape_view_handler<platform::android>::apply_fill(const std::string& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    call_void_int(env, native_, "setFillColor", static_cast<jint>(parse_argb(v)));
}

void shape_view_handler<platform::android>::apply_stroke(const std::string& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    call_void_int(env, native_, "setStrokeColor", static_cast<jint>(parse_argb(v)));
}

void shape_view_handler<platform::android>::apply_stroke_thickness(double v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    call_void_float(env, native_, "setStrokeWidth", static_cast<jfloat>(v));
}

void shape_view_handler<platform::android>::apply_opacity(double v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    call_void_float(env, native_, "setShapeOpacity", static_cast<jfloat>(v));
}

void shape_view_handler<platform::android>::map_kind(shape_view& s) {
    bound_ = &s;
    apply_kind(s.kind.get());
    s.kind.changed.subscribe(kind_slot_, kind_cb_);
}
void shape_view_handler<platform::android>::map_data(shape_view& s) {
    apply_data(s.data.get());
    s.data.changed.subscribe(data_slot_, data_cb_);
}
void shape_view_handler<platform::android>::map_fill(shape_view& s) {
    apply_fill(s.fill.get());
    s.fill.changed.subscribe(fill_slot_, fill_cb_);
}
void shape_view_handler<platform::android>::map_stroke(shape_view& s) {
    apply_stroke(s.stroke.get());
    s.stroke.changed.subscribe(stroke_slot_, stroke_cb_);
}
void shape_view_handler<platform::android>::map_stroke_thickness(shape_view& s) {
    apply_stroke_thickness(s.stroke_thickness.get());
    s.stroke_thickness.changed.subscribe(stroke_thick_slot_, stroke_thick_cb_);
}
void shape_view_handler<platform::android>::map_opacity(shape_view& s) {
    apply_opacity(s.opacity.get());
    s.opacity.changed.subscribe(opacity_slot_, opacity_cb_);
}

} // namespace mpapp

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
