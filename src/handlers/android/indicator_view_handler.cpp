// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_indicator_view handler implementation.

#include "mpapp/handlers/android/indicator_view_handler.hpp"

#if defined(__ANDROID__)

#include <cstdlib>
#include <string>

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

namespace mpapp::internal {

namespace {

constexpr int LINEAR_LAYOUT_HORIZONTAL = 0;
// android.graphics.drawable.GradientDrawable.OVAL == 1.
constexpr int GRADIENT_DRAWABLE_OVAL   = 1;

// 8dp dots, 4dp spacing. The handler emits pixel sizes (px == dp at
// density 1; the JNI surface accepts plain ints and Android scales).
constexpr int kDotSizePx    = 24;   // ~8dp at xhdpi
constexpr int kDotSpacingPx = 12;   // ~4dp at xhdpi

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
    if (name == "Gray")  return static_cast<jint>(0xFFA0A0A0U);
    if (name == "Teal")  return static_cast<jint>(0xFF0096A5U);
    return fallback;
}

jobject make_linear_layout(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/LinearLayout");
    if (cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(Landroid/content/Context;)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(cls); return nullptr; }
    jobject local = env->NewObject(cls, ctor, context);
    if (env->ExceptionCheck() || local == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(cls);
        return nullptr;
    }
    jmethodID set_orient = env->GetMethodID(cls, "setOrientation", "(I)V");
    if (set_orient != nullptr) {
        env->CallVoidMethod(local, set_orient, LINEAR_LAYOUT_HORIZONTAL);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
    jobject global = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    return global;
}

// Build a GradientDrawable shaped as an oval with the given solid color.
// Returns a global ref or nullptr; caller must release.
jobject make_dot_drawable(JNIEnv* env, jint argb) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/graphics/drawable/GradientDrawable");
    if (cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(cls, "<init>", "()V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(cls); return nullptr; }
    jobject local = env->NewObject(cls, ctor);
    if (env->ExceptionCheck() || local == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(cls);
        return nullptr;
    }
    jmethodID set_shape = env->GetMethodID(cls, "setShape", "(I)V");
    if (set_shape != nullptr) {
        env->CallVoidMethod(local, set_shape, GRADIENT_DRAWABLE_OVAL);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    jmethodID set_color = env->GetMethodID(cls, "setColor", "(I)V");
    if (set_color != nullptr) {
        env->CallVoidMethod(local, set_color, argb);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
    jobject global = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    return global;
}

// Construct an android.view.View(context) and configure it as a single
// dot: fixed LayoutParams (kDotSizePx square + horizontal margins) and
// a GradientDrawable background. Returns a local ref attached to the
// parent LinearLayout — the caller passes ownership to addView.
jobject make_dot_view(JNIEnv* env, jobject context, jint argb,
                      bool with_left_margin) {
    if (env->ExceptionCheck()) env->ExceptionClear();

    jclass view_cls = env->FindClass("android/view/View");
    if (view_cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID view_ctor = env->GetMethodID(view_cls, "<init>", "(Landroid/content/Context;)V");
    if (view_ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(view_cls); return nullptr; }
    jobject dot = env->NewObject(view_cls, view_ctor, context);
    if (env->ExceptionCheck() || dot == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(view_cls);
        return nullptr;
    }

    // LayoutParams: LinearLayout.LayoutParams(width, height) then set margins.
    jclass lp_cls = env->FindClass("android/widget/LinearLayout$LayoutParams");
    if (lp_cls != nullptr) {
        jmethodID lp_ctor = env->GetMethodID(lp_cls, "<init>", "(II)V");
        if (lp_ctor != nullptr) {
            jobject lp = env->NewObject(lp_cls, lp_ctor,
                static_cast<jint>(kDotSizePx), static_cast<jint>(kDotSizePx));
            if (lp != nullptr && !env->ExceptionCheck()) {
                jfieldID f_left  = env->GetFieldID(lp_cls, "leftMargin",  "I");
                jfieldID f_right = env->GetFieldID(lp_cls, "rightMargin", "I");
                if (f_left  != nullptr) env->SetIntField(lp, f_left,
                    with_left_margin ? static_cast<jint>(kDotSpacingPx) : 0);
                if (f_right != nullptr) env->SetIntField(lp, f_right, 0);

                jmethodID set_lp = env->GetMethodID(view_cls, "setLayoutParams",
                    "(Landroid/view/ViewGroup$LayoutParams;)V");
                if (set_lp != nullptr) {
                    env->CallVoidMethod(dot, set_lp, lp);
                    if (env->ExceptionCheck()) env->ExceptionClear();
                }
                env->DeleteLocalRef(lp);
            } else if (env->ExceptionCheck()) {
                env->ExceptionClear();
            }
        }
        env->DeleteLocalRef(lp_cls);
    }

    // Background drawable.
    jobject drawable = make_dot_drawable(env, argb);
    if (drawable != nullptr) {
        jmethodID set_bg = env->GetMethodID(view_cls, "setBackground",
            "(Landroid/graphics/drawable/Drawable;)V");
        if (set_bg != nullptr) {
            env->CallVoidMethod(dot, set_bg, drawable);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        env->DeleteGlobalRef(drawable);
    }

    env->DeleteLocalRef(view_cls);
    return dot;
}

void linear_layout_remove_all_views(JNIEnv* env, jobject ll) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/ViewGroup");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "removeAllViews", "()V");
    if (m != nullptr) {
        env->CallVoidMethod(ll, m);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void linear_layout_add_view(JNIEnv* env, jobject ll, jobject child) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/ViewGroup");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "addView", "(Landroid/view/View;)V");
    if (m != nullptr) {
        env->CallVoidMethod(ll, m, child);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

} // namespace

indicator_view_handler<platform::android>::indicator_view_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_linear_layout(env, detail::get_activity());
}

indicator_view_handler<platform::android>::~indicator_view_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (native_ != nullptr) {
            env->DeleteGlobalRef(native_);
            native_ = nullptr;
        }
    }
}

void indicator_view_handler<platform::android>::rebuild_dots() {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();

    linear_layout_remove_all_views(env, native_);

    const jint argb_unsel = parse_argb(cached_color_,    static_cast<jint>(0xFFC8C8C8U));
    const jint argb_sel   = parse_argb(cached_selected_, static_cast<jint>(0xFF3C78DCU));
    jobject context = detail::get_activity();

    for (int i = 0; i < cached_count_; ++i) {
        const jint argb = (i == cached_position_) ? argb_sel : argb_unsel;
        jobject dot = make_dot_view(env, context, argb, /*with_left_margin=*/(i != 0));
        if (dot != nullptr) {
            linear_layout_add_view(env, native_, dot);
            env->DeleteLocalRef(dot);
        }
    }
}

void indicator_view_handler<platform::android>::recolor_dots() {
    // Cheapest correct implementation: rebuild. At realistic counts
    // (a handful of paged items) the cost is negligible and avoids the
    // complexity of walking native children to re-target their drawable.
    rebuild_dots();
}

void indicator_view_handler<platform::android>::apply_count(int v) {
    if (v < 0) v = 0;
    cached_count_ = v;
    rebuild_dots();
}

void indicator_view_handler<platform::android>::apply_position(int v) {
    cached_position_ = v;
    recolor_dots();
}

void indicator_view_handler<platform::android>::apply_indicator_color(const brush_ref& b) {
    cached_color_ = b;
    recolor_dots();
}

void indicator_view_handler<platform::android>::apply_selected_indicator_color(const brush_ref& b) {
    cached_selected_ = b;
    recolor_dots();
}

void indicator_view_handler<platform::android>::map_count(basic_indicator_view& iv) {
    apply_count(iv.count.get());
    iv.count.changed.subscribe(count_slot_, count_cb_);
}
void indicator_view_handler<platform::android>::map_position(basic_indicator_view& iv) {
    apply_position(iv.position.get());
    iv.position.changed.subscribe(position_slot_, position_cb_);
}
void indicator_view_handler<platform::android>::map_indicator_color(basic_indicator_view& iv) {
    apply_indicator_color(iv.indicator_color.get());
    iv.indicator_color.changed.subscribe(color_slot_, color_cb_);
}
void indicator_view_handler<platform::android>::map_selected_indicator_color(basic_indicator_view& iv) {
    apply_selected_indicator_color(iv.selected_indicator_color.get());
    iv.selected_indicator_color.changed.subscribe(sel_color_slot_, sel_color_cb_);
}

} // namespace mpapp::internal
// ----- ADR-0013 self-registration --------------------------------------------

namespace {
jobject dispatch_indicator_view(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_indicator_view*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}
struct registrar {
    registrar() {
        ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_indicator_view);
    }
};
[[maybe_unused]] registrar _reg;
} // namespace

#endif // __ANDROID__
