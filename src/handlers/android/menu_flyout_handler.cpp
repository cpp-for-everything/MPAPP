// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_menu_flyout handler implementation.

#include "mpapp/handlers/android/menu_flyout_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

#include "mpapp/internal/basic_menu_flyout.hpp"
#include "mpapp/view.hpp"

namespace mpapp::internal {

namespace {

constexpr int LINEAR_LAYOUT_VERTICAL = 1;

// View visibility constants — match android.view.View.
constexpr int VIEW_VISIBLE = 0;
constexpr int VIEW_GONE    = 8;

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
        env->CallVoidMethod(local, set_orient, LINEAR_LAYOUT_VERTICAL);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
    jobject global = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    return global;
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

void view_set_visibility(JNIEnv* env, jobject view, int vis) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/View");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setVisibility", "(I)V");
    if (m != nullptr) {
        env->CallVoidMethod(view, m, vis);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

} // namespace

menu_flyout_handler<platform::android>::menu_flyout_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    native_ = make_linear_layout(env, detail::get_activity());
    if (native_ != nullptr) {
        // Start hidden — `is_open` defaults to false. GONE keeps the
        // popup out of the layout flow until the host shows it.
        view_set_visibility(env, native_, VIEW_GONE);
    }
}

menu_flyout_handler<platform::android>::~menu_flyout_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    if (native_ != nullptr) { env->DeleteGlobalRef(native_); native_ = nullptr; }
}

void menu_flyout_handler<platform::android>::apply_items(const std::vector<view*>& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    view_group_remove_all(env, native_);
    for (view* child : v) {
        if (child == nullptr) continue;
        if (jobject jv = detail::android_dispatch::dispatch(child); jv != nullptr) {
            view_group_add(env, native_, jv);
        }
    }
}

void menu_flyout_handler<platform::android>::apply_is_open(bool v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    view_set_visibility(env, native_, v ? VIEW_VISIBLE : VIEW_GONE);
}

void menu_flyout_handler<platform::android>::map_items(basic_menu_flyout& f) {
    apply_items(f.items.get());
    f.items.changed.subscribe(items_slot_, items_cb_);
}

void menu_flyout_handler<platform::android>::map_is_open(basic_menu_flyout& f) {
    apply_is_open(f.is_open.get());
    f.is_open.changed.subscribe(is_open_slot_, is_open_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --

namespace {

// basic_menu_flyout is a popup surface, not a regular container child. The
// dispatcher returns nullptr so container dispatch sites skip it
// cleanly. The registrar is still installed for ADR-0013 uniformity.
jobject dispatch_menu_flyout(::mpapp::view* /*v*/) {
    return nullptr;
}

struct registrar_mf {
    registrar_mf() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_menu_flyout); }
};

[[maybe_unused]] registrar_mf _reg;

} // namespace

#endif // __ANDROID__
