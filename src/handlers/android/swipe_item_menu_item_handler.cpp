// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_swipe_item_menu_item handler implementation.

#include "mpapp/handlers/android/swipe_item_menu_item_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

namespace mpapp::internal {

namespace {

jobject make_button(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/Button");
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

void button_set_text(JNIEnv* env, jobject btn, const std::string& text) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/Button");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setText", "(Ljava/lang/CharSequence;)V");
    if (m != nullptr) {
        jstring jstr = env->NewStringUTF(text.c_str());
        env->CallVoidMethod(btn, m, jstr);
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->DeleteLocalRef(jstr);
    }
    env->DeleteLocalRef(cls);
}

void button_set_content_description(JNIEnv* env, jobject btn, const std::string& text) {
    // Capture the icon URI on the basic_button's content description so a future
    // basic_image-source resolver can read it back without round-tripping through
    // the C++ Observable. android.view.View.setContentDescription(CharSequence).
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/View");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setContentDescription", "(Ljava/lang/CharSequence;)V");
    if (m != nullptr) {
        jstring jstr = env->NewStringUTF(text.c_str());
        env->CallVoidMethod(btn, m, jstr);
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->DeleteLocalRef(jstr);
    }
    env->DeleteLocalRef(cls);
}

} // namespace

swipe_item_menu_item_handler<platform::android>::swipe_item_menu_item_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    native_ = make_button(env, detail::get_activity());
}

swipe_item_menu_item_handler<platform::android>::~swipe_item_menu_item_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    if (native_ != nullptr) { env->DeleteGlobalRef(native_); native_ = nullptr; }
}

void swipe_item_menu_item_handler<platform::android>::apply_text(const std::string& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    button_set_text(env, native_, v);
}

void swipe_item_menu_item_handler<platform::android>::apply_icon_uri(const std::string& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    // Symbolic for M-04b — stash on the content description so future
    // basic_image-source resolution can pick it up.
    button_set_content_description(env, native_, v);
}

void swipe_item_menu_item_handler<platform::android>::map_text(basic_swipe_item_menu_item& m) {
    apply_text(m.text.get());
    m.text.changed.subscribe(text_slot_, text_cb_);
}

void swipe_item_menu_item_handler<platform::android>::map_icon_uri(basic_swipe_item_menu_item& m) {
    apply_icon_uri(m.icon_uri.get());
    m.icon_uri.changed.subscribe(icon_slot_, icon_cb_);
}

void swipe_item_menu_item_handler<platform::android>::map_invoked(basic_swipe_item_menu_item& m) {
    // Java-side OnClickListener routing is deferred until the shared
    // MppClickRouter pattern is generalised beyond `basic_button` (it currently
    // hardcodes a `basic_button*` cookie). Programmatic `invoked.emit()` works
    // unconditionally on the C++ side. Touch the parameter so the unused-
    // var warning doesn't trip /WX.
    (void)m;
}

} // namespace mpapp::internal
// ----- ADR-0013 self-registration --------------------------------------

namespace {

jobject dispatch_swipe_item_menu_item(::mpapp::view* v) {
    if (auto* m = dynamic_cast<::mpapp::internal::basic_swipe_item_menu_item*>(v); m && m->has_handler()) {
        return m->handler().native();
    }
    return nullptr;
}

struct swipe_item_menu_item_registrar {
    swipe_item_menu_item_registrar() {
        ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_swipe_item_menu_item);
    }
};

[[maybe_unused]] swipe_item_menu_item_registrar _swipe_item_menu_item_reg;

} // namespace

#endif // __ANDROID__
