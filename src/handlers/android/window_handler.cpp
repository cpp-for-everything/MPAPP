// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — Android basic_window handler implementation.

#include "mpapp/handlers/android/window_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

#include "mpapp/internal/basic_activity_indicator.hpp"
#include "mpapp/border.hpp"
#include "mpapp/internal/basic_box_view.hpp"
#include "mpapp/internal/basic_date_picker.hpp"
#include "mpapp/internal/basic_image.hpp"
#include "mpapp/internal/basic_image_button.hpp"
#include "mpapp/internal/basic_picker.hpp"
#include "mpapp/internal/basic_time_picker.hpp"
#include "mpapp/internal/basic_progress_bar.hpp"
#include "mpapp/internal/basic_search_bar.hpp"
#include "mpapp/internal/basic_button.hpp"
#include "mpapp/internal/basic_check_box.hpp"
#include "mpapp/editor.hpp"
#include "mpapp/internal/basic_entry.hpp"
#include "mpapp/handlers/android/activity_indicator_handler.hpp"
#include "mpapp/handlers/android/border_handler.hpp"
#include "mpapp/handlers/android/box_view_handler.hpp"
#include "mpapp/handlers/android/date_picker_handler.hpp"
#include "mpapp/handlers/android/image_handler.hpp"
#include "mpapp/handlers/android/image_button_handler.hpp"
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
#include "mpapp/handlers/android/scroll_view_handler.hpp"
#include "mpapp/handlers/android/slider_handler.hpp"
#include "mpapp/handlers/android/stack_layout_handler.hpp"
#include "mpapp/handlers/android/stepper_handler.hpp"
#include "mpapp/handlers/android/switch_handler.hpp"
#include "mpapp/internal/basic_label.hpp"
#include "mpapp/internal/basic_radio_button.hpp"
#include "mpapp/internal/basic_scroll_view.hpp"
#include "mpapp/internal/basic_slider.hpp"
#include "mpapp/internal/basic_stack_layout.hpp"
#include "mpapp/internal/basic_stepper.hpp"
#include "mpapp/internal/basic_switch_.hpp"

namespace mpapp::internal {

namespace {

jobject child_jobject(view* v) {
    // ADR-0013: registry dispatch only — each widget self-registers.
    return detail::android_dispatch::dispatch(v);
}

} // namespace

window_handler<platform::android>::window_handler() = default;
window_handler<platform::android>::~window_handler() = default;

jobject window_handler<platform::android>::native() noexcept {
    return detail::get_activity();
}
jobject window_handler<platform::android>::native() const noexcept {
    return detail::get_activity();
}

// Look up the Activity directly through the JNI bridge each time
// instead of holding a per-handler `NewGlobalRef`. The bridge's
// `g_activity` is already a global ref; double-NewGlobalRef-ing it
// has been seen to trip ART's CheckJNI on some Android-34 emulator
// images.
namespace {
jobject get_activity() noexcept { return detail::get_activity(); }
}

void window_handler<platform::android>::apply_title(const std::string& v) {
    jobject activity = get_activity();
    if (activity == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;

    if (env->ExceptionCheck()) {
        env->ExceptionClear();
    }

    jclass activity_cls = env->FindClass("android/app/Activity");
    if (activity_cls == nullptr) {
        env->ExceptionClear();
        return;
    }
    jmethodID set_title = env->GetMethodID(activity_cls, "setTitle",
                                            "(Ljava/lang/CharSequence;)V");
    if (set_title != nullptr) {
        jstring jstr = env->NewStringUTF(v.c_str());
        env->CallVoidMethod(activity, set_title, jstr);
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        env->DeleteLocalRef(jstr);
    }
    env->DeleteLocalRef(activity_cls);
}

void window_handler<platform::android>::apply_content(view* v) {
    jobject activity = get_activity();
    if (activity == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;

    // Clear any pending JNI exception from earlier calls so ART's
    // CheckJNI doesn't abort on the next JNI call. See
    // vault/50_Tasks/T-0011-app-basic_shell-abstraction/screenshots/evidence.md
    // for the diagnosis that surfaced this requirement.
    if (env->ExceptionCheck()) env->ExceptionClear();

    jobject child = (v != nullptr) ? child_jobject(v) : nullptr;

    // FindClass on android/app/Activity is more robust than
    // GetObjectClass(activity) — the latter has been observed to
    // abort ART's CheckJNI on Android-34 emulator images.
    jclass activity_cls = env->FindClass("android/app/Activity");
    if (activity_cls == nullptr) {
        env->ExceptionClear();
        return;
    }
    jmethodID set_content_view = env->GetMethodID(
        activity_cls, "setContentView", "(Landroid/view/View;)V");
    if (set_content_view != nullptr) {
        env->CallVoidMethod(activity, set_content_view, child);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(activity_cls);
}

void window_handler<platform::android>::apply_is_visible(bool /*v*/) {
    // Activity visibility is managed by the OS; the cross-platform
    // is_visible flag is informational only on Android. Promoting the
    // app to foreground via Intent flags lands in the M-05 surface.
}

void window_handler<platform::android>::bind(basic_window& w) {
    bound_ = &w;

    apply_title(w.title.get());
    w.title.changed.subscribe(title_slot_, title_cb_);

    apply_content(w.content.get());
    w.content.changed.subscribe(content_slot_, content_cb_);

    apply_is_visible(w.is_visible.get());
    w.is_visible.changed.subscribe(visible_slot_, visible_cb_);
}

} // namespace mpapp::internal
#endif // __ANDROID__
