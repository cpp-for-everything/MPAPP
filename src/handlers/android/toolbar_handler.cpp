// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_toolbar handler implementation.

#include "mpapp/handlers/android/toolbar_handler.hpp"

#if defined(__ANDROID__)

#include <string>
#include <vector>

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

namespace mpapp::internal {

namespace {

jobject make_toolbar(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/Toolbar");
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

// Clear the basic_toolbar's menu and add one MenuItem per toolbar_item.
// Returns silently on any JNI exception, mirroring the rest of the
// Android handler conventions.
void rebuild_menu(JNIEnv* env, jobject toolbar_obj,
                  const std::vector<toolbar_item>& items) {
    if (env->ExceptionCheck()) env->ExceptionClear();

    jclass toolbar_cls = env->FindClass("android/widget/Toolbar");
    if (toolbar_cls == nullptr) { env->ExceptionClear(); return; }

    jmethodID get_menu = env->GetMethodID(
        toolbar_cls, "getMenu", "()Landroid/view/Menu;");
    if (get_menu == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(toolbar_cls);
        return;
    }
    jobject menu = env->CallObjectMethod(toolbar_obj, get_menu);
    if (env->ExceptionCheck() || menu == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(toolbar_cls);
        return;
    }
    env->DeleteLocalRef(toolbar_cls);

    jclass menu_cls = env->FindClass("android/view/Menu");
    if (menu_cls == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(menu);
        return;
    }

    jmethodID clear_m = env->GetMethodID(menu_cls, "clear", "()V");
    if (clear_m != nullptr) {
        env->CallVoidMethod(menu, clear_m);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }

    // Menu.add(CharSequence): MenuItem
    jmethodID add_m = env->GetMethodID(
        menu_cls, "add", "(Ljava/lang/CharSequence;)Landroid/view/MenuItem;");
    if (add_m == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(menu_cls);
        env->DeleteLocalRef(menu);
        return;
    }

    for (const auto& it : items) {
        jstring js = env->NewStringUTF(it.text.c_str());
        jobject menu_item = env->CallObjectMethod(menu, add_m, js);
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->DeleteLocalRef(js);
        if (menu_item != nullptr) {
            env->DeleteLocalRef(menu_item);
        }
    }

    env->DeleteLocalRef(menu_cls);
    env->DeleteLocalRef(menu);
}

} // namespace

toolbar_handler<platform::android>::toolbar_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_toolbar(env, detail::get_activity());
}

toolbar_handler<platform::android>::~toolbar_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (native_ != nullptr) { env->DeleteGlobalRef(native_); native_ = nullptr; }
    }
}

void toolbar_handler<platform::android>::apply_items(const std::vector<toolbar_item>& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    rebuild_menu(env, native_, v);
}

void toolbar_handler<platform::android>::apply_title(const std::string& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();

    jclass cls = env->FindClass("android/widget/Toolbar");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID set_title = env->GetMethodID(
        cls, "setTitle", "(Ljava/lang/CharSequence;)V");
    if (set_title != nullptr) {
        jstring js = env->NewStringUTF(v.c_str());
        env->CallVoidMethod(native_, set_title, js);
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->DeleteLocalRef(js);
    }
    env->DeleteLocalRef(cls);
}

void toolbar_handler<platform::android>::map_items(basic_toolbar& t) {
    apply_items(t.items.get());
    t.items.changed.subscribe(items_slot_, items_cb_);
}
void toolbar_handler<platform::android>::map_title(basic_toolbar& t) {
    apply_title(t.title.get());
    t.title.changed.subscribe(title_slot_, title_cb_);
}

} // namespace mpapp::internal
namespace {

// Per ADR-0013 — self-register so the container dispatch surfaces resolve
// `view*` → `jobject` without a per-widget dynamic_cast branch.
jobject dispatch_toolbar(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_toolbar*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() {
        ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_toolbar);
    }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
