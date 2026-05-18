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

window_handler<platform::android>::window_handler() {
    // The Activity is set globally by the user's MainActivity native
    // bridge before run_app is called. Grab a per-window copy here so
    // the handler doesn't depend on global state after construction.
    JNIEnv* env = detail::attach_current_thread();
    if (env != nullptr) {
        jobject activity = detail::get_activity();
        if (activity != nullptr) {
            native_ = env->NewGlobalRef(activity);
        }
    }
}

window_handler<platform::android>::~window_handler() {
    if (native_ != nullptr) {
        if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
            env->DeleteGlobalRef(native_);
        }
        native_ = nullptr;
    }
}

void window_handler<platform::android>::apply_title(const std::string& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    jclass activity_cls = env->GetObjectClass(native_);
    jmethodID set_title = env->GetMethodID(activity_cls, "setTitle",
                                            "(Ljava/lang/CharSequence;)V");
    if (set_title != nullptr) {
        jstring jstr = env->NewStringUTF(v.c_str());
        env->CallVoidMethod(native_, set_title, jstr);
        env->DeleteLocalRef(jstr);
    }
    env->DeleteLocalRef(activity_cls);
}

void window_handler<platform::android>::apply_content(view* v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    jobject child = (v != nullptr) ? child_jobject(v) : nullptr;
    jclass activity_cls = env->GetObjectClass(native_);
    jmethodID set_content_view = env->GetMethodID(
        activity_cls, "setContentView", "(Landroid/view/View;)V");
    if (set_content_view != nullptr) {
        env->CallVoidMethod(native_, set_content_view, child);
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
