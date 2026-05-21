// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android bindable_layout handler implementation.

#include "mpapp/handlers/android/bindable_layout_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

namespace mpapp {

namespace {

constexpr int LINEAR_LAYOUT_VERTICAL = 1;

// Construct an android.widget.LinearLayout(Context). Returns a global
// ref or nullptr on failure. JNI rules: ExceptionClear + delete every
// local ref + return a global ref (caller deletes it).
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
    // Set orientation = VERTICAL.
    jmethodID set_orient = env->GetMethodID(cls, "setOrientation", "(I)V");
    if (set_orient != nullptr) {
        env->CallVoidMethod(local, set_orient, static_cast<jint>(LINEAR_LAYOUT_VERTICAL));
        if (env->ExceptionCheck()) env->ExceptionClear();
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

bindable_layout_handler<platform::android>::bindable_layout_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_linear_layout(env, detail::get_activity());
}

bindable_layout_handler<platform::android>::~bindable_layout_handler() {
    if (native_ != nullptr) {
        if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
            env->DeleteGlobalRef(native_);
        }
        native_ = nullptr;
    }
}

void bindable_layout_handler<platform::android>::rebuild_children(layout& host) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    view_group_remove_all(env, native_);
    // M-04b: item_template instantiation deferred. clear half of the
    // contract is sufficient for now — future template wiring will start
    // from a known-empty state.
    (void)host;
}

void bindable_layout_handler<platform::android>::map_items_source(layout& host) {
    rebuild_children(host);
}

void bindable_layout_handler<platform::android>::map_item_template(layout& /*host*/) {
    // Recorded but not yet driving instantiation — see header comment.
}

void bindable_layout_handler<platform::android>::map_empty_view(layout& host) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    const auto& items = bindable_layout::get_items_source(host);
    if (!items.items.empty()) return;
    auto empty = bindable_layout::get_empty_view(host);
    view* raw = empty.get();
    if (raw == nullptr) return;
    if (jobject child = detail::android_dispatch::dispatch(raw); child != nullptr) {
        view_group_add(env, native_, child);
    }
}

} // namespace mpapp

// ----- ADR-0013 self-registration --------------------------------------------
//
// `bindable_layout` is an attached-property facility, not a `view`
// subclass — there is no instance to `dynamic_cast` to. The dispatcher
// is therefore a defensive no-op: returns nullptr so the registry
// simply skips it. Kept to satisfy ADR-0013's self-registration
// contract.

namespace {

jobject dispatch_bindable_layout(::mpapp::view* /*v*/) {
    return nullptr;  // see comment above
}

struct registrar {
    registrar() {
        ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_bindable_layout);
    }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
