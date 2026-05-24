// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — Android basic_button handler implementation.

#include "mpapp/handlers/android/button_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"

namespace mpapp::internal {

namespace {

jobject make_button(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/Button");
    if (cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(Landroid/content/Context;)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(cls); return nullptr; }
    jobject local = env->NewObject(cls, ctor, context);
    if (env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(cls); return nullptr; }
    env->DeleteLocalRef(cls);
    if (local == nullptr) return nullptr;
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

} // namespace

button_handler<platform::android>::button_handler() {
    JNIEnv* env = ::mpapp::detail::attach_current_thread();
    if (env != nullptr) {
        native_ = make_button(env, ::mpapp::detail::get_activity());
    }
}

button_handler<platform::android>::~button_handler() {
    if (native_ != nullptr) {
        if (JNIEnv* env = ::mpapp::detail::attach_current_thread(); env != nullptr) {
            env->DeleteGlobalRef(native_);
        }
        native_ = nullptr;
    }
}

void button_handler<platform::android>::apply_text(const std::string& text) {
    if (native_ == nullptr) return;
    JNIEnv* env = ::mpapp::detail::attach_current_thread();
    if (env == nullptr) return;
    button_set_text(env, native_, text);
}

void button_handler<platform::android>::map_text(basic_button& b) {
    apply_text(b.text.get());
    b.text.changed.subscribe(text_slot_, text_cb_);
}

void button_handler<platform::android>::map_clicked(basic_button& b) {
    if (native_ == nullptr) return;
    JNIEnv* env = ::mpapp::detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();

    // 1. Instantiate `io.mpapp.MppClickRouter(long buttonPtr)` — the
    //    Java-side OnClickListener that bridges back into native via
    //    `Java_io_mpapp_MppClickRouter_nativeDispatchClick`.
    jclass router_cls = env->FindClass("io/mpapp/MppClickRouter");
    if (router_cls == nullptr) {
        env->ExceptionClear();
        return;
    }
    jmethodID router_ctor = env->GetMethodID(router_cls, "<init>", "(J)V");
    if (router_ctor == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(router_cls);
        return;
    }
    const jlong button_ptr = reinterpret_cast<jlong>(&b);
    jobject router = env->NewObject(router_cls, router_ctor, button_ptr);
    if (router == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(router_cls);
        return;
    }

    // 2. Call View.setOnClickListener(router). The Button class
    //    inherits setOnClickListener from android.view.View, so we
    //    look up the method on the View class for portability.
    jclass view_cls = env->FindClass("android/view/View");
    if (view_cls != nullptr) {
        jmethodID set_listener = env->GetMethodID(
            view_cls, "setOnClickListener",
            "(Landroid/view/View$OnClickListener;)V");
        if (set_listener != nullptr) {
            env->CallVoidMethod(native_, set_listener, router);
            if (env->ExceptionCheck()) {
                env->ExceptionDescribe();
                env->ExceptionClear();
            }
        }
        env->DeleteLocalRef(view_cls);
    }

    env->DeleteLocalRef(router);
    env->DeleteLocalRef(router_cls);
}

void android_button_dispatch_click(basic_button* b) {
    if (b != nullptr) {
        b->clicked.emit();
    }
}

} // namespace mpapp::internal

extern "C" JNIEXPORT void JNICALL
Java_io_mpapp_MppClickRouter_nativeDispatchClick(
    JNIEnv* /*env*/, jclass /*cls*/, jlong button_ptr) {
    mpapp::internal::android_button_dispatch_click(
        reinterpret_cast<mpapp::internal::basic_button*>(button_ptr));
}

// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register basic_button so ADR-0013 fall-through
// dispatch can find its jobject without the legacy dynamic_cast chain.

#include "mpapp/handlers/android/widget_dispatch.hpp"
#include "mpapp/internal/basic_button.hpp"

namespace {

jobject dispatch_button(::mpapp::view* v) {
    if (auto* b = dynamic_cast<::mpapp::internal::basic_button*>(v); b && b->has_handler()) {
        return b->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_button); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
