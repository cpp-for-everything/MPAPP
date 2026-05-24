// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_search_bar handler implementation.

#include "mpapp/handlers/android/search_bar_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"

namespace mpapp::internal {

namespace {

jobject make_search_view(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/SearchView");
    if (cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(Landroid/content/Context;)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(cls); return nullptr; }
    jobject local = env->NewObject(cls, ctor, context);
    if (env->ExceptionCheck() || local == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(cls);
        return nullptr;
    }
    // Open the search field by default (no "tap to expand" affordance).
    jmethodID set_iconified = env->GetMethodID(cls, "setIconified", "(Z)V");
    if (set_iconified != nullptr) {
        env->CallVoidMethod(local, set_iconified, JNI_FALSE);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
    jobject global = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    return global;
}

} // namespace

search_bar_handler<platform::android>::search_bar_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_search_view(env, detail::get_activity());
}

search_bar_handler<platform::android>::~search_bar_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (native_ != nullptr) { env->DeleteGlobalRef(native_); native_ = nullptr; }
    }
}

void search_bar_handler<platform::android>::apply_text(const std::string& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/SearchView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID set_query = env->GetMethodID(
        cls, "setQuery", "(Ljava/lang/CharSequence;Z)V");
    if (set_query != nullptr) {
        jstring jstr = env->NewStringUTF(v.c_str());
        env->CallVoidMethod(native_, set_query, jstr, JNI_FALSE);
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->DeleteLocalRef(jstr);
    }
    env->DeleteLocalRef(cls);
}

void search_bar_handler<platform::android>::apply_placeholder(const std::string& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/SearchView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID set_hint = env->GetMethodID(
        cls, "setQueryHint", "(Ljava/lang/CharSequence;)V");
    if (set_hint != nullptr) {
        jstring jstr = env->NewStringUTF(v.c_str());
        env->CallVoidMethod(native_, set_hint, jstr);
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->DeleteLocalRef(jstr);
    }
    env->DeleteLocalRef(cls);
}

void search_bar_handler<platform::android>::map_text(basic_search_bar& s) {
    apply_text(s.text.get());
    s.text.changed.subscribe(text_slot_, text_cb_);
}
void search_bar_handler<platform::android>::map_placeholder(basic_search_bar& s) {
    apply_placeholder(s.placeholder.get());
    s.placeholder.changed.subscribe(placeholder_slot_, placeholder_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register basic_search_bar so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/android/widget_dispatch.hpp"
#include "mpapp/internal/basic_search_bar.hpp"

namespace {

jobject dispatch_search_bar(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_search_bar*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_search_bar); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
