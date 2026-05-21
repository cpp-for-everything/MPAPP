// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android scroll_view handler implementation.

#include "mpapp/handlers/android/scroll_view_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

#include "mpapp/activity_indicator.hpp"
#include "mpapp/border.hpp"
#include "mpapp/box_view.hpp"
#include "mpapp/date_picker.hpp"
#include "mpapp/image.hpp"
#include "mpapp/picker.hpp"
#include "mpapp/time_picker.hpp"
#include "mpapp/progress_bar.hpp"
#include "mpapp/search_bar.hpp"
#include "mpapp/button.hpp"
#include "mpapp/check_box.hpp"
#include "mpapp/editor.hpp"
#include "mpapp/entry.hpp"
#include "mpapp/handlers/android/activity_indicator_handler.hpp"
#include "mpapp/handlers/android/border_handler.hpp"
#include "mpapp/handlers/android/box_view_handler.hpp"
#include "mpapp/handlers/android/date_picker_handler.hpp"
#include "mpapp/handlers/android/image_handler.hpp"
#include "mpapp/handlers/android/picker_handler.hpp"
#include "mpapp/handlers/android/time_picker_handler.hpp"
#include "mpapp/handlers/android/progress_bar_handler.hpp"
#include "mpapp/handlers/android/search_bar_handler.hpp"
#include "mpapp/handlers/android/button_handler.hpp"
#include "mpapp/handlers/android/check_box_handler.hpp"
#include "mpapp/handlers/android/editor_handler.hpp"
#include "mpapp/handlers/android/entry_handler.hpp"
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

jobject make_scroll_view(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/ScrollView");
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

jobject child_jobject(view* v) {
    // ADR-0013: registry dispatch only — each widget self-registers.
    return detail::android_dispatch::dispatch(v);
}

void view_group_remove_all(JNIEnv* env, jobject group) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/ViewGroup");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "removeAllViews", "()V");
    if (m != nullptr) {
        env->CallVoidMethod(group, m);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void view_group_add(JNIEnv* env, jobject group, jobject child) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/ViewGroup");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "addView", "(Landroid/view/View;)V");
    if (m != nullptr) {
        env->CallVoidMethod(group, m, child);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

} // namespace

scroll_view_handler<platform::android>::scroll_view_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env != nullptr) {
        native_ = make_scroll_view(env, detail::get_activity());
    }
}

scroll_view_handler<platform::android>::~scroll_view_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (native_ != nullptr) { env->DeleteGlobalRef(native_); native_ = nullptr; }
    }
}

void scroll_view_handler<platform::android>::apply_content(const std::shared_ptr<view>& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    view_group_remove_all(env, native_);
    jobject child = v ? child_jobject(v.get()) : nullptr;
    if (child != nullptr) {
        view_group_add(env, native_, child);
    }
}

void scroll_view_handler<platform::android>::apply_orientation(scroll_orientation /*o*/) {
    // android.widget.ScrollView is vertical-only. Horizontal scrolling
    // needs HorizontalScrollView (different widget). The
    // map_orientation hook is preserved for cross-platform parity but
    // a no-op on Android for the spike; M-05 polish swaps the native
    // widget per orientation.
}

void scroll_view_handler<platform::android>::map_content(scroll_view& s) {
    bound_ = &s;
    apply_content(s.content.get());
    s.content.changed.subscribe(content_slot_, content_cb_);
}

void scroll_view_handler<platform::android>::map_orientation(scroll_view& s) {
    apply_orientation(s.orientation.get());
    s.orientation.changed.subscribe(orient_slot_, orient_cb_);
}

void scroll_view_handler<platform::android>::bind_content(scroll_view& s, view& child) {
    s.content.set(std::shared_ptr<view>(&child, [](view*){}));
}

} // namespace mpapp

// ---------- Self-registration with the per-platform dispatch registry --
namespace {

jobject dispatch_scroll_view(::mpapp::view* v) {
    if (auto* s = dynamic_cast<::mpapp::scroll_view*>(v); s && s->has_handler()) {
        return s->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_scroll_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
