// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — Android window handler implementation.

#include "mpapp/handlers/android/window_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"

#include "mpapp/button.hpp"
#include "mpapp/handlers/android/button_handler.hpp"
#include "mpapp/handlers/android/label_handler.hpp"
#include "mpapp/handlers/android/stack_layout_handler.hpp"
#include "mpapp/label.hpp"
#include "mpapp/stack_layout.hpp"

namespace mpapp {

namespace {

jobject child_jobject(view* v) {
    if (auto* sl = dynamic_cast<stack_layout*>(v); sl && sl->has_handler()) {
        return sl->handler().native();
    }
    if (auto* b = dynamic_cast<button*>(v); b && b->has_handler()) {
        return b->handler().native();
    }
    if (auto* l = dynamic_cast<label*>(v); l && l->has_handler()) {
        return l->handler().native();
    }
    return nullptr;
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
    jclass activity_cls = env->GetObjectClass(activity);
    jmethodID set_title = env->GetMethodID(activity_cls, "setTitle",
                                            "(Ljava/lang/CharSequence;)V");
    if (set_title != nullptr) {
        jstring jstr = env->NewStringUTF(v.c_str());
        env->CallVoidMethod(activity, set_title, jstr);
        env->DeleteLocalRef(jstr);
    }
    env->DeleteLocalRef(activity_cls);
}

void window_handler<platform::android>::apply_content(view* v) {
    jobject activity = get_activity();
    if (activity == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    jobject child = (v != nullptr) ? child_jobject(v) : nullptr;
    jclass activity_cls = env->GetObjectClass(activity);
    jmethodID set_content_view = env->GetMethodID(
        activity_cls, "setContentView", "(Landroid/view/View;)V");
    if (set_content_view != nullptr) {
        env->CallVoidMethod(activity, set_content_view, child);
    }
    env->DeleteLocalRef(activity_cls);
}

void window_handler<platform::android>::apply_is_visible(bool /*v*/) {
    // Activity visibility is managed by the OS; the cross-platform
    // is_visible flag is informational only on Android. Promoting the
    // app to foreground via Intent flags lands in the M-05 surface.
}

void window_handler<platform::android>::bind(window& w) {
    bound_ = &w;

    apply_title(w.title.get());
    w.title.changed.subscribe(title_slot_, title_cb_);

    apply_content(w.content.get());
    w.content.changed.subscribe(content_slot_, content_cb_);

    apply_is_visible(w.is_visible.get());
    w.is_visible.changed.subscribe(visible_slot_, visible_cb_);
}

} // namespace mpapp

#endif // __ANDROID__
