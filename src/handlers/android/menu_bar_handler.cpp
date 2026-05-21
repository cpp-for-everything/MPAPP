// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android menu_bar handler implementation.

#include "mpapp/handlers/android/menu_bar_handler.hpp"

#if defined(__ANDROID__)

#include <string>
#include <vector>

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/menu_bar_item_handler.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"
#include "mpapp/menu_bar_item.hpp"

namespace mpapp {

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

// Clear the toolbar's menu and add one MenuItem per child menu_bar_item.
// Returns silently on any JNI exception. Mirrors toolbar_handler's
// approach.
void rebuild_menu(JNIEnv* env, jobject toolbar_obj,
                  const std::vector<view*>& items) {
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

    jmethodID add_m = env->GetMethodID(
        menu_cls, "add", "(Ljava/lang/CharSequence;)Landroid/view/MenuItem;");
    if (add_m == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(menu_cls);
        env->DeleteLocalRef(menu);
        return;
    }

    for (view* child : items) {
        if (child == nullptr) continue;
        auto* mbi = dynamic_cast<menu_bar_item*>(child);
        if (mbi == nullptr || !mbi->has_handler()) continue;
        const std::string& title = mbi->handler().current_title();
        jstring js = env->NewStringUTF(title.c_str());
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

menu_bar_handler<platform::android>::menu_bar_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_toolbar(env, detail::get_activity());
}

menu_bar_handler<platform::android>::~menu_bar_handler() {
    if (native_ != nullptr) {
        if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
            env->DeleteGlobalRef(native_);
        }
        native_ = nullptr;
    }
}

void menu_bar_handler<platform::android>::apply_items(const std::vector<view*>& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    rebuild_menu(env, native_, v);
}

void menu_bar_handler<platform::android>::map_items(menu_bar& b) {
    apply_items(b.items.get());
    b.items.changed.subscribe(items_slot_, items_cb_);
}

} // namespace mpapp

// --- ADR-0013 self-registration --------------------------------------------

namespace {

jobject dispatch_menu_bar(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::menu_bar*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar_menu_bar {
    registrar_menu_bar() {
        ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_menu_bar);
    }
};

[[maybe_unused]] registrar_menu_bar _reg_mb;

} // namespace

#endif // __ANDROID__
