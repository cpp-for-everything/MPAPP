// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — Android basic_stack_layout handler implementation.

#include "mpapp/handlers/android/stack_layout_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

#include "mpapp/internal/basic_button.hpp"
#include "mpapp/internal/basic_check_box.hpp"
#include "mpapp/internal/basic_activity_indicator.hpp"
#include "mpapp/border.hpp"
#include "mpapp/internal/basic_box_view.hpp"
#include "mpapp/internal/basic_date_picker.hpp"
#include "mpapp/editor.hpp"
#include "mpapp/internal/basic_image.hpp"
#include "mpapp/internal/basic_image_button.hpp"
#include "mpapp/internal/basic_time_picker.hpp"
#include "mpapp/internal/basic_picker.hpp"
#include "mpapp/internal/basic_progress_bar.hpp"
#include "mpapp/internal/basic_search_bar.hpp"
#include "mpapp/internal/basic_entry.hpp"
#include "mpapp/handlers/android/button_handler.hpp"
#include "mpapp/handlers/android/check_box_handler.hpp"
#include "mpapp/handlers/android/activity_indicator_handler.hpp"
#include "mpapp/handlers/android/border_handler.hpp"
#include "mpapp/handlers/android/picker_handler.hpp"
#include "mpapp/handlers/android/progress_bar_handler.hpp"
#include "mpapp/handlers/android/search_bar_handler.hpp"
#include "mpapp/handlers/android/box_view_handler.hpp"
#include "mpapp/handlers/android/date_picker_handler.hpp"
#include "mpapp/handlers/android/editor_handler.hpp"
#include "mpapp/handlers/android/image_handler.hpp"
#include "mpapp/handlers/android/image_button_handler.hpp"
#include "mpapp/handlers/android/time_picker_handler.hpp"
#include "mpapp/handlers/android/entry_handler.hpp"
#include "mpapp/handlers/android/label_handler.hpp"
#include "mpapp/handlers/android/radio_button_handler.hpp"
#include "mpapp/handlers/android/slider_handler.hpp"
#include "mpapp/handlers/android/stepper_handler.hpp"
#include "mpapp/handlers/android/switch_handler.hpp"
#include "mpapp/internal/basic_label.hpp"
#include "mpapp/internal/basic_radio_button.hpp"
#include "mpapp/internal/basic_slider.hpp"
#include "mpapp/internal/basic_stepper.hpp"
#include "mpapp/internal/basic_switch_.hpp"

namespace mpapp::internal {

namespace {

constexpr int LINEAR_LAYOUT_HORIZONTAL = 0;
constexpr int LINEAR_LAYOUT_VERTICAL   = 1;

// Construct an android.widget.LinearLayout(Context). Returns a global
// ref or nullptr on failure.
jobject make_linear_layout(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/LinearLayout");
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

void linear_layout_set_orientation(JNIEnv* env, jobject ll, int orient_native) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/LinearLayout");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setOrientation", "(I)V");
    if (m != nullptr) {
        env->CallVoidMethod(ll, m, orient_native);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void linear_layout_set_padding(JNIEnv* env, jobject ll, int l, int t, int r, int b) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/View");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setPadding", "(IIII)V");
    if (m != nullptr) {
        env->CallVoidMethod(ll, m, l, t, r, b);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void view_set_gravity(JNIEnv* env, jobject layout, int gravity) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/LinearLayout");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setGravity", "(I)V");
    if (m != nullptr) {
        env->CallVoidMethod(layout, m, gravity);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

// android.view.Gravity constants.
constexpr int GRAVITY_START                = 0x00800003;
constexpr int GRAVITY_CENTER_HORIZONTAL    = 0x00000001;
constexpr int GRAVITY_END                  = 0x00800005;
constexpr int GRAVITY_FILL_HORIZONTAL      = 0x00000007;
constexpr int GRAVITY_TOP                  = 0x00000030;
constexpr int GRAVITY_CENTER_VERTICAL      = 0x00000010;
constexpr int GRAVITY_BOTTOM               = 0x00000050;
constexpr int GRAVITY_FILL_VERTICAL        = 0x00000070;

int to_horizontal_gravity(h_align a) {
    switch (a) {
        case h_align::start:   return GRAVITY_START;
        case h_align::center:  return GRAVITY_CENTER_HORIZONTAL;
        case h_align::end:     return GRAVITY_END;
        case h_align::stretch: return GRAVITY_FILL_HORIZONTAL;
    }
    return GRAVITY_FILL_HORIZONTAL;
}

int to_vertical_gravity(v_align a) {
    switch (a) {
        case v_align::start:   return GRAVITY_TOP;
        case v_align::center:  return GRAVITY_CENTER_VERTICAL;
        case v_align::end:     return GRAVITY_BOTTOM;
        case v_align::stretch: return GRAVITY_FILL_VERTICAL;
    }
    return GRAVITY_FILL_VERTICAL;
}

} // namespace

stack_layout_handler<platform::android>::stack_layout_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env != nullptr) {
        native_ = make_linear_layout(env, detail::get_activity());
    }
}

stack_layout_handler<platform::android>::~stack_layout_handler() {
    if (native_ != nullptr) {
        if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
            env->DeleteGlobalRef(native_);
        }
        native_ = nullptr;
    }
}

void stack_layout_handler<platform::android>::apply_orientation(orientation o) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    linear_layout_set_orientation(env, native_,
        (o == orientation::horizontal) ? LINEAR_LAYOUT_HORIZONTAL
                                       : LINEAR_LAYOUT_VERTICAL);
}

void stack_layout_handler<platform::android>::apply_spacing(double /*s*/) {
    // LinearLayout doesn't have a direct spacing property — emulated
    // via per-child layout margins in M-05.
}

void stack_layout_handler<platform::android>::apply_padding(thickness t) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    linear_layout_set_padding(env, native_,
        static_cast<int>(t.left), static_cast<int>(t.top),
        static_cast<int>(t.right), static_cast<int>(t.bottom));
}

void stack_layout_handler<platform::android>::apply_horizontal_alignment(h_align a) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    // Compose with existing vertical gravity.
    int g = to_horizontal_gravity(a) | (bound_
              ? to_vertical_gravity(bound_->vertical_alignment.get())
              : GRAVITY_FILL_VERTICAL);
    view_set_gravity(env, native_, g);
}

void stack_layout_handler<platform::android>::apply_vertical_alignment(v_align a) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    int g = to_vertical_gravity(a) | (bound_
              ? to_horizontal_gravity(bound_->horizontal_alignment.get())
              : GRAVITY_FILL_HORIZONTAL);
    view_set_gravity(env, native_, g);
}

void stack_layout_handler<platform::android>::bind(basic_stack_layout& s) {
    bound_ = &s;

    apply_orientation(s.stack_orientation.get());
    s.stack_orientation.changed.subscribe(orient_slot_, orient_cb_);

    apply_spacing(s.spacing.get());
    s.spacing.changed.subscribe(spacing_slot_, spacing_cb_);

    apply_padding(s.padding.get());
    s.padding.changed.subscribe(padding_slot_, padding_cb_);

    apply_horizontal_alignment(s.horizontal_alignment.get());
    s.horizontal_alignment.changed.subscribe(h_align_slot_, h_align_cb_);

    apply_vertical_alignment(s.vertical_alignment.get());
    s.vertical_alignment.changed.subscribe(v_align_slot_, v_align_cb_);

    for (std::size_t i = 0; i < s.child_count(); ++i) {
        if (view* child = s.child_at(i); child != nullptr) {
            add_child(*child);
        }
    }
}

void stack_layout_handler<platform::android>::add_child(view& child) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();

    // ADR-0013: registry dispatch only — each widget self-registers.
    jobject child_obj = detail::android_dispatch::dispatch(&child);
    if (child_obj == nullptr) return;

    jclass cls = env->FindClass("android/view/ViewGroup");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID add_view = env->GetMethodID(cls, "addView", "(Landroid/view/View;)V");
    if (add_view != nullptr) {
        env->CallVoidMethod(native_, add_view, child_obj);
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
    }
    env->DeleteLocalRef(cls);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
namespace {

jobject dispatch_stack_layout(::mpapp::view* v) {
    if (auto* s = dynamic_cast<::mpapp::internal::basic_stack_layout*>(v); s && s->has_handler()) {
        return s->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_stack_layout); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
