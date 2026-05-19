// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android check_box handler implementation.

#include "mpapp/handlers/android/check_box_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"

namespace mpapp {

namespace {

jobject make_check_box(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/CheckBox");
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

void check_box_set_checked(JNIEnv* env, jobject cb, bool checked) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/CompoundButton");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setChecked", "(Z)V");
    if (m != nullptr) {
        env->CallVoidMethod(cb, m, static_cast<jboolean>(checked ? JNI_TRUE : JNI_FALSE));
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void check_box_set_text(JNIEnv* env, jobject cb, const char* text) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/TextView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setText", "(Ljava/lang/CharSequence;)V");
    if (m != nullptr) {
        jstring jstr = env->NewStringUTF(text);
        env->CallVoidMethod(cb, m, jstr);
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->DeleteLocalRef(jstr);
    }
    env->DeleteLocalRef(cls);
}

jobject install_checked_change_listener(JNIEnv* env, jobject cb, jlong handler_ptr) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass listener_cls = env->FindClass("io/mpapp/MppCheckedChangeListener");
    if (listener_cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(listener_cls, "<init>", "(JI)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(listener_cls); return nullptr; }
    // kind=2 → check_box_handler routing.
    jobject local = env->NewObject(listener_cls, ctor, handler_ptr,
                                   static_cast<jint>(check_box_handler<platform::android>::kind));
    if (local == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(listener_cls);
        return nullptr;
    }
    jclass cb_cls = env->FindClass("android/widget/CompoundButton");
    if (cb_cls != nullptr) {
        jmethodID set_listener = env->GetMethodID(
            cb_cls, "setOnCheckedChangeListener",
            "(Landroid/widget/CompoundButton$OnCheckedChangeListener;)V");
        if (set_listener != nullptr) {
            env->CallVoidMethod(cb, set_listener, local);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        env->DeleteLocalRef(cb_cls);
    }
    jobject global = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    env->DeleteLocalRef(listener_cls);
    return global;
}

} // namespace

check_box_handler<platform::android>::check_box_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env != nullptr) {
        native_ = make_check_box(env, detail::get_activity());
        if (native_ != nullptr) {
            // Default to no text label — user code controls the
            // label content via a sibling mpapp::label if needed.
            check_box_set_text(env, native_, "");
        }
    }
}

check_box_handler<platform::android>::~check_box_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (listener_ != nullptr) { env->DeleteGlobalRef(listener_); listener_ = nullptr; }
        if (native_   != nullptr) { env->DeleteGlobalRef(native_);   native_   = nullptr; }
    }
}

void check_box_handler<platform::android>::apply_is_checked(bool v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    suppress_echo_ = true;
    check_box_set_checked(env, native_, v);
    suppress_echo_ = false;
}

void check_box_handler<platform::android>::map_is_checked(check_box& c) {
    bound_ = &c;
    apply_is_checked(c.is_checked.get());
    c.is_checked.changed.subscribe(slot_, cb_);

    if (native_ != nullptr && listener_ == nullptr) {
        JNIEnv* env = detail::attach_current_thread();
        if (env != nullptr) {
            listener_ = install_checked_change_listener(env, native_,
                            reinterpret_cast<jlong>(this));
        }
    }
}

void check_box_handler<platform::android>::on_native_checked_changed(bool checked) {
    if (suppress_echo_ || bound_ == nullptr) return;
    if (bound_->is_checked.get() != checked) {
        bound_->is_checked.set(checked);
    }
}

void android_check_box_dispatch_checked_changed(check_box_handler<platform::android>* h, bool checked) {
    if (h != nullptr) h->on_native_checked_changed(checked);
}

} // namespace mpapp

#endif // __ANDROID__
