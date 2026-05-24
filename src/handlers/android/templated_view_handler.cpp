// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_templated_view handler implementation.

#include "mpapp/handlers/android/templated_view_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

#include "mpapp/internal/basic_templated_view.hpp"
#include "mpapp/view.hpp"

namespace mpapp::internal {

namespace {

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

templated_view_handler<platform::android>::templated_view_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    native_ = make_frame_layout(env, detail::get_activity());
}

templated_view_handler<platform::android>::~templated_view_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    if (native_ != nullptr) { env->DeleteGlobalRef(native_); native_ = nullptr; }
}

void templated_view_handler<platform::android>::apply_content(const std::shared_ptr<view>& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();

    view_group_remove_all(env, native_);

    // ADR-0013 registry first; if no widget is registered for the child
    // type, leave content empty (legacy widgets that haven't migrated
    // yet are not rendered by basic_templated_view).
    jobject child = v ? detail::android_dispatch::dispatch(v.get()) : nullptr;
    if (child != nullptr) view_group_add(env, native_, child);
}

void templated_view_handler<platform::android>::apply_template_id(const std::string& v) {
    // P3 templating engine is deferred — record the id for later wiring.
    template_id_ = v;
}

void templated_view_handler<platform::android>::map_content(basic_templated_view& t) {
    apply_content(t.content.get());
    t.content.changed.subscribe(content_slot_, content_cb_);
}

void templated_view_handler<platform::android>::map_template_id(basic_templated_view& t) {
    apply_template_id(t.template_id.get());
    t.template_id.changed.subscribe(template_id_slot_, template_id_cb_);
}

void templated_view_handler<platform::android>::bind_content(basic_templated_view& t, view& child) {
    t.content.set(std::shared_ptr<view>(&child, [](view*){}));
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --

namespace {

jobject dispatch_templated_view(::mpapp::view* v) {
    if (auto* t = dynamic_cast<::mpapp::internal::basic_templated_view*>(v); t && t->has_handler()) {
        return t->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_templated_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
