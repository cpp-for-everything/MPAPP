// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_menu_bar_item handler implementation.

#include "mpapp/handlers/android/menu_bar_item_handler.hpp"

#if defined(__ANDROID__)

#include <string>

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

namespace mpapp::internal {

namespace {

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

} // namespace

menu_bar_item_handler<platform::android>::menu_bar_item_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_text_view(env, detail::get_activity());
}

menu_bar_item_handler<platform::android>::~menu_bar_item_handler() {
    if (native_ != nullptr) {
        if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
            env->DeleteGlobalRef(native_);
        }
        native_ = nullptr;
    }
}

void menu_bar_item_handler<platform::android>::apply_title(const std::string& v) {
    current_title_ = v;
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    text_view_set_text(env, native_, v);
}

void menu_bar_item_handler<platform::android>::apply_items(const std::vector<view*>& /*v*/) {
    // Per-child rebuilds are flattened up into the parent basic_menu_bar's
    // Menu in `menu_bar_handler::apply_items`. The handler still wires
    // the signal so future granular updates have a hook.
}

void menu_bar_item_handler<platform::android>::map_title(basic_menu_bar_item& m) {
    apply_title(m.title.get());
    m.title.changed.subscribe(title_slot_, title_cb_);
}

void menu_bar_item_handler<platform::android>::map_items(basic_menu_bar_item& m) {
    apply_items(m.items.get());
    m.items.changed.subscribe(items_slot_, items_cb_);
}

} // namespace mpapp::internal
// --- ADR-0013 self-registration --------------------------------------------

namespace {

jobject dispatch_menu_bar_item(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_menu_bar_item*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar_menu_bar_item {
    registrar_menu_bar_item() {
        ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_menu_bar_item);
    }
};

[[maybe_unused]] registrar_menu_bar_item _reg_mbi;

} // namespace

#endif // __ANDROID__
