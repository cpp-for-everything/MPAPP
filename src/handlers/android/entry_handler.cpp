// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 follow-up — Android entry handler implementation.

#include "mpapp/handlers/android/entry_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"

namespace mpapp {

namespace {

jobject make_edit_text(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/EditText");
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

void edit_text_set_text(JNIEnv* env, jobject et, const std::string& text) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/EditText");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setText", "(Ljava/lang/CharSequence;)V");
    if (m != nullptr) {
        jstring jstr = env->NewStringUTF(text.c_str());
        env->CallVoidMethod(et, m, jstr);
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->DeleteLocalRef(jstr);
    }
    env->DeleteLocalRef(cls);
}

void edit_text_set_hint(JNIEnv* env, jobject et, const std::string& hint) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/TextView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setHint", "(Ljava/lang/CharSequence;)V");
    if (m != nullptr) {
        jstring jstr = env->NewStringUTF(hint.c_str());
        env->CallVoidMethod(et, m, jstr);
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->DeleteLocalRef(jstr);
    }
    env->DeleteLocalRef(cls);
}

void edit_text_set_enabled(JNIEnv* env, jobject et, bool enabled) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/View");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setEnabled", "(Z)V");
    if (m != nullptr) {
        env->CallVoidMethod(et, m, static_cast<jboolean>(enabled ? JNI_TRUE : JNI_FALSE));
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

jobject install_text_watcher(JNIEnv* env, jobject edit_text, jlong handler_ptr) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass watcher_cls = env->FindClass("io/mpapp/MppTextWatcher");
    if (watcher_cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(watcher_cls, "<init>", "(J)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(watcher_cls); return nullptr; }
    jobject local = env->NewObject(watcher_cls, ctor, handler_ptr);
    if (local == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(watcher_cls);
        return nullptr;
    }

    jclass et_cls = env->FindClass("android/widget/EditText");
    if (et_cls != nullptr) {
        jmethodID add_watcher = env->GetMethodID(
            et_cls, "addTextChangedListener",
            "(Landroid/text/TextWatcher;)V");
        if (add_watcher != nullptr) {
            env->CallVoidMethod(edit_text, add_watcher, local);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        env->DeleteLocalRef(et_cls);
    }

    jobject global = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    env->DeleteLocalRef(watcher_cls);
    return global;
}

} // namespace

entry_handler<platform::android>::entry_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env != nullptr) {
        native_ = make_edit_text(env, detail::get_activity());
    }
}

entry_handler<platform::android>::~entry_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (watcher_ != nullptr) {
            env->DeleteGlobalRef(watcher_);
            watcher_ = nullptr;
        }
        if (native_ != nullptr) {
            env->DeleteGlobalRef(native_);
            native_ = nullptr;
        }
    }
}

void entry_handler<platform::android>::apply_text(const std::string& text) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    suppress_echo_ = true;
    edit_text_set_text(env, native_, text);
    suppress_echo_ = false;
}

void entry_handler<platform::android>::apply_placeholder(const std::string& text) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    edit_text_set_hint(env, native_, text);
}

void entry_handler<platform::android>::apply_is_read_only(bool ro) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    edit_text_set_enabled(env, native_, !ro);
}

void entry_handler<platform::android>::map_text(entry& e) {
    bound_ = &e;
    apply_text(e.text.get());
    e.text.changed.subscribe(text_slot_, text_cb_);

    if (native_ != nullptr && watcher_ == nullptr) {
        JNIEnv* env = detail::attach_current_thread();
        if (env != nullptr) {
            watcher_ = install_text_watcher(env, native_,
                                            reinterpret_cast<jlong>(this));
        }
    }
}

void entry_handler<platform::android>::map_placeholder(entry& e) {
    apply_placeholder(e.placeholder.get());
    e.placeholder.changed.subscribe(placeholder_slot_, placeholder_cb_);
}

void entry_handler<platform::android>::map_is_read_only(entry& e) {
    apply_is_read_only(e.is_read_only.get());
    e.is_read_only.changed.subscribe(readonly_slot_, readonly_cb_);
}

void entry_handler<platform::android>::on_native_text_changed(const std::string& text) {
    if (suppress_echo_ || bound_ == nullptr) return;
    if (bound_->text.get() != text) {
        bound_->text.set(text);
    }
}

void android_entry_dispatch_text_changed(entry_handler<platform::android>* h,
                                         const std::string& text) {
    if (h != nullptr) {
        h->on_native_text_changed(text);
    }
}

} // namespace mpapp

extern "C" JNIEXPORT void JNICALL
Java_io_mpapp_MppTextWatcher_nativeDispatchTextChanged(
    JNIEnv* env, jclass /*cls*/, jlong handler_ptr, jstring text) {
    if (handler_ptr == 0) return;
    const char* utf8 = (text != nullptr) ? env->GetStringUTFChars(text, nullptr) : nullptr;
    std::string s = (utf8 != nullptr) ? std::string{utf8} : std::string{};
    if (utf8 != nullptr) env->ReleaseStringUTFChars(text, utf8);
    mpapp::android_entry_dispatch_text_changed(
        reinterpret_cast<mpapp::entry_handler<mpapp::platform::android>*>(handler_ptr),
        s);
}

#endif // __ANDROID__
