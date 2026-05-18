// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — Android button handler implementation.

#include "mpapp/handlers/android/button_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"

namespace mpapp {

namespace {

jobject make_button(JNIEnv* env, jobject context) {
    jclass cls = env->FindClass("android/widget/Button");
    if (cls == nullptr) return nullptr;
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(Landroid/content/Context;)V");
    if (ctor == nullptr) { env->DeleteLocalRef(cls); return nullptr; }
    jobject local = env->NewObject(cls, ctor, context);
    env->DeleteLocalRef(cls);
    if (local == nullptr) return nullptr;
    jobject global = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    return global;
}

void button_set_text(JNIEnv* env, jobject btn, const std::string& text) {
    jclass cls = env->GetObjectClass(btn);
    jmethodID m = env->GetMethodID(cls, "setText", "(Ljava/lang/CharSequence;)V");
    if (m != nullptr) {
        jstring jstr = env->NewStringUTF(text.c_str());
        env->CallVoidMethod(btn, m, jstr);
        env->DeleteLocalRef(jstr);
    }
    env->DeleteLocalRef(cls);
}

} // namespace

button_handler<platform::android>::button_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env != nullptr) {
        native_ = make_button(env, detail::get_activity());
    }
}

button_handler<platform::android>::~button_handler() {
    if (native_ != nullptr) {
        if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
            env->DeleteGlobalRef(native_);
        }
        native_ = nullptr;
    }
}

void button_handler<platform::android>::apply_text(const std::string& text) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    button_set_text(env, native_, text);
}

void button_handler<platform::android>::map_text(button& b) {
    apply_text(b.text.get());
    b.text.changed.subscribe(text_slot_, text_cb_);
}

void button_handler<platform::android>::map_clicked(button& b) {
    // The Java MainActivity is responsible for installing an
    // OnClickListener on the Button jobject (it has the View ref via
    // setContentView's tree). That listener invokes a native method
    // bridged here. The user's Android app boilerplate ships a stock
    // `MppClickRouter` Java class that does this — see the Android
    // example template (T-0011 follow-up: M-05 milestone).
    (void)b;
}

void android_button_dispatch_click(button* b) {
    if (b != nullptr) {
        b->clicked.emit();
    }
}

} // namespace mpapp

extern "C" JNIEXPORT void JNICALL
Java_io_mpapp_MppClickRouter_nativeDispatchClick(
    JNIEnv* /*env*/, jclass /*cls*/, jlong button_ptr) {
    mpapp::android_button_dispatch_click(
        reinterpret_cast<mpapp::button*>(button_ptr));
}

#endif // __ANDROID__
