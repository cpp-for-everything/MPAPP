// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android border handler implementation.

#include "mpapp/handlers/android/border_handler.hpp"

#if defined(__ANDROID__)

#include <cstdlib>
#include <string>

#include "mpapp/box_view.hpp"
#include "mpapp/button.hpp"
#include "mpapp/check_box.hpp"
#include "mpapp/editor.hpp"
#include "mpapp/entry.hpp"
#include "mpapp/handlers/android/box_view_handler.hpp"
#include "mpapp/handlers/android/button_handler.hpp"
#include "mpapp/handlers/android/check_box_handler.hpp"
#include "mpapp/handlers/android/editor_handler.hpp"
#include "mpapp/handlers/android/entry_handler.hpp"
#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"
#include "mpapp/handlers/android/label_handler.hpp"
#include "mpapp/handlers/android/radio_button_handler.hpp"
#include "mpapp/handlers/android/slider_handler.hpp"
#include "mpapp/handlers/android/stack_layout_handler.hpp"
#include "mpapp/handlers/android/stepper_handler.hpp"
#include "mpapp/handlers/android/switch_handler.hpp"
#include "mpapp/label.hpp"
#include "mpapp/radio_button.hpp"
#include "mpapp/slider.hpp"
#include "mpapp/stack_layout.hpp"
#include "mpapp/stepper.hpp"
#include "mpapp/switch_.hpp"

namespace mpapp {

namespace {

jint parse_brush_to_argb(const brush_ref& br) {
    auto clamp_255 = [](double v) { return static_cast<jint>((v < 0 ? 0 : (v > 1 ? 1 : v)) * 255 + 0.5); };
    const std::string& name = br.name;
    if (name.empty()) return static_cast<jint>(0xFF000000U);
    if (name[0] == '#') {
        unsigned long v = std::strtoul(name.c_str() + 1, nullptr, 16);
        if (name.size() == 7) return static_cast<jint>(0xFF000000U | v);
        if (name.size() == 9) return static_cast<jint>(v);
        return static_cast<jint>(0xFF000000U);
    }
    if (name == "Red")   return static_cast<jint>(0xFFDC3232U);
    if (name == "Green") return static_cast<jint>(0xFF50B450U);
    if (name == "Blue")  return static_cast<jint>(0xFF3C78DCU);
    if (name == "Black") return static_cast<jint>(0xFF000000U);
    if (name == "White") return static_cast<jint>(0xFFFFFFFFU);
    if (name == "Gray")  return static_cast<jint>(0xFF808080U);
    if (name == "Teal")  return static_cast<jint>(0xFF0096A5U);
    (void)clamp_255;
    return static_cast<jint>(0xFF000000U);
}

struct corner4 { double tl=0, tr=0, br=0, bl=0; };
corner4 parse_corners(const stroke_shape_desc& s) {
    corner4 out{};
    auto paren = s.descriptor.find('(');
    if (paren == std::string::npos) return out;
    auto close = s.descriptor.find(')', paren + 1);
    if (close == std::string::npos) return out;
    std::string args = s.descriptor.substr(paren + 1, close - paren - 1);
    double values[4]{};
    int n = 0;
    std::string cur;
    for (char c : args) {
        if (c == ',') { if (n < 4) values[n++] = std::atof(cur.c_str()); cur.clear(); }
        else if (c != ' ') cur.push_back(c);
    }
    if (!cur.empty() && n < 4) values[n++] = std::atof(cur.c_str());
    if (n == 1)      out = {values[0], values[0], values[0], values[0]};
    else if (n == 4) out = {values[0], values[1], values[2], values[3]};
    return out;
}

jobject child_jobject(view* v) {
    // ADR-0013: registry dispatch only — each widget self-registers.
    return detail::android_dispatch::dispatch(v);
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

void apply_background(JNIEnv* env, jobject view,
                      double stroke_thickness, jint stroke_argb,
                      const corner4& r) {
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

    // setStroke(int width, int color)
    jmethodID set_stroke = env->GetMethodID(drawable_cls, "setStroke", "(II)V");
    if (set_stroke != nullptr) {
        env->CallVoidMethod(drawable, set_stroke, static_cast<jint>(stroke_thickness + 0.5), stroke_argb);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }

    // setCornerRadii(float[8])
    jmethodID set_radii = env->GetMethodID(drawable_cls, "setCornerRadii", "([F)V");
    if (set_radii != nullptr) {
        jfloatArray radii = env->NewFloatArray(8);
        if (radii != nullptr) {
            jfloat values[8] = {
                static_cast<jfloat>(r.tl), static_cast<jfloat>(r.tl),
                static_cast<jfloat>(r.tr), static_cast<jfloat>(r.tr),
                static_cast<jfloat>(r.br), static_cast<jfloat>(r.br),
                static_cast<jfloat>(r.bl), static_cast<jfloat>(r.bl),
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

border_handler<platform::android>::border_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_frame_layout(env, detail::get_activity());
    if (native_ != nullptr) {
        rebuild_background();
    }
}

border_handler<platform::android>::~border_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (native_ != nullptr) { env->DeleteGlobalRef(native_); native_ = nullptr; }
    }
}

void border_handler<platform::android>::rebuild_background() {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    apply_background(env, native_,
                     cached_stroke_thickness_,
                     parse_brush_to_argb(cached_stroke_),
                     parse_corners(cached_stroke_shape_));
}

void border_handler<platform::android>::apply_content(const std::shared_ptr<view>& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    view_group_remove_all(env, native_);
    jobject child = v ? child_jobject(v.get()) : nullptr;
    if (child != nullptr) view_group_add(env, native_, child);
}

void border_handler<platform::android>::apply_padding(const thickness& t) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    view_set_padding(env, native_,
                     static_cast<jint>(t.left + 0.5),
                     static_cast<jint>(t.top + 0.5),
                     static_cast<jint>(t.right + 0.5),
                     static_cast<jint>(t.bottom + 0.5));
}

void border_handler<platform::android>::apply_stroke(const brush_ref& b)              { cached_stroke_ = b; rebuild_background(); }
void border_handler<platform::android>::apply_stroke_thickness(double t)              { cached_stroke_thickness_ = t; rebuild_background(); }
void border_handler<platform::android>::apply_stroke_shape(const stroke_shape_desc& s){ cached_stroke_shape_ = s; rebuild_background(); }

void border_handler<platform::android>::map_content(border& b)          { apply_content(b.content.get()); b.content.changed.subscribe(content_slot_, content_cb_); }
void border_handler<platform::android>::map_padding(border& b)          { apply_padding(b.padding.get()); b.padding.changed.subscribe(padding_slot_, padding_cb_); }
void border_handler<platform::android>::map_stroke(border& b)           { apply_stroke(b.stroke.get()); b.stroke.changed.subscribe(stroke_slot_, stroke_cb_); }
void border_handler<platform::android>::map_stroke_thickness(border& b) { apply_stroke_thickness(b.stroke_thickness.get()); b.stroke_thickness.changed.subscribe(stroke_thick_slot_, stroke_thick_cb_); }
void border_handler<platform::android>::map_stroke_shape(border& b)     { apply_stroke_shape(b.stroke_shape.get()); b.stroke_shape.changed.subscribe(stroke_shape_slot_, stroke_shape_cb_); }

void border_handler<platform::android>::bind_content(border& b, view& child) {
    b.content.set(std::shared_ptr<view>(&child, [](view*){}));
}

} // namespace mpapp

// ---------- Self-registration with the per-platform dispatch registry --
namespace {

jobject dispatch_border(::mpapp::view* v) {
    if (auto* b = dynamic_cast<::mpapp::border*>(v); b && b->has_handler()) {
        return b->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_border); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
