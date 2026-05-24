// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_menu_flyout_sub_item handler implementation.

#include "mpapp/handlers/android/menu_flyout_sub_item_handler.hpp"

#if defined(__ANDROID__)

#include <string>

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

#include "mpapp/internal/basic_menu_flyout_sub_item.hpp"
#include "mpapp/view.hpp"

namespace mpapp::internal {

namespace {

constexpr int LINEAR_LAYOUT_VERTICAL = 1;

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

jobject make_text_view(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/TextView");
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

void text_view_set_text(JNIEnv* env, jobject tv, const std::string& text) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/TextView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setText", "(Ljava/lang/CharSequence;)V");
    if (m != nullptr) {
        jstring jstr = env->NewStringUTF(text.c_str());
        env->CallVoidMethod(tv, m, jstr);
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->DeleteLocalRef(jstr);
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

} // namespace

menu_flyout_sub_item_handler<platform::android>::menu_flyout_sub_item_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();

    jobject context = detail::get_activity();
    native_     = make_linear_layout(env, context);
    label_      = make_text_view    (env, context);
    child_host_ = make_linear_layout(env, context);

    if (native_ != nullptr) {
        if (label_      != nullptr) view_group_add(env, native_, label_);
        if (child_host_ != nullptr) view_group_add(env, native_, child_host_);
    }
}

menu_flyout_sub_item_handler<platform::android>::~menu_flyout_sub_item_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    if (child_host_ != nullptr) { env->DeleteGlobalRef(child_host_); child_host_ = nullptr; }
    if (label_      != nullptr) { env->DeleteGlobalRef(label_);      label_      = nullptr; }
    if (native_     != nullptr) { env->DeleteGlobalRef(native_);     native_     = nullptr; }
}

void menu_flyout_sub_item_handler<platform::android>::apply_text(const std::string& v) {
    if (label_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    text_view_set_text(env, label_, v);
}

void menu_flyout_sub_item_handler<platform::android>::apply_items(const std::vector<view*>& v) {
    if (child_host_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    view_group_remove_all(env, child_host_);
    for (view* child : v) {
        if (child == nullptr) continue;
        if (jobject jv = detail::android_dispatch::dispatch(child); jv != nullptr) {
            view_group_add(env, child_host_, jv);
        }
    }
}

void menu_flyout_sub_item_handler<platform::android>::map_text(basic_menu_flyout_sub_item& s) {
    apply_text(s.text.get());
    s.text.changed.subscribe(text_slot_, text_cb_);
}

void menu_flyout_sub_item_handler<platform::android>::map_items(basic_menu_flyout_sub_item& s) {
    apply_items(s.items.get());
    s.items.changed.subscribe(items_slot_, items_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --

namespace {

jobject dispatch_menu_flyout_sub_item(::mpapp::view* v) {
    if (auto* s = dynamic_cast<::mpapp::internal::basic_menu_flyout_sub_item*>(v); s && s->has_handler()) {
        return s->handler().native();
    }
    return nullptr;
}

struct registrar_mfsi {
    registrar_mfsi() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_menu_flyout_sub_item); }
};

[[maybe_unused]] registrar_mfsi _reg;

} // namespace

#endif // __ANDROID__
