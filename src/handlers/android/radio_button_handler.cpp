// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_radio_button handler implementation.

#include "mpapp/handlers/android/radio_button_handler.hpp"

#if defined(__ANDROID__)

#include <map>
#include <mutex>

#include "mpapp/handlers/android/jni_bridge.hpp"

namespace mpapp::internal {

namespace {

// Per-process group registry: group_name → global ref to the
// android.widget.RadioGroup container that owns the RadioButtons of
// that group. The first basic_radio_button to bind a non-empty group_name
// creates the RadioGroup; subsequent ones attach to it.
std::map<std::string, jobject>& group_registry() {
    static std::map<std::string, jobject> g;
    return g;
}
std::mutex& group_registry_mutex() {
    static std::mutex m;
    return m;
}

jobject make_radio_button(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/RadioButton");
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

jobject make_radio_group(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/RadioGroup");
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

void radio_button_set_checked(JNIEnv* env, jobject rb, bool checked) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/CompoundButton");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setChecked", "(Z)V");
    if (m != nullptr) {
        env->CallVoidMethod(rb, m, static_cast<jboolean>(checked ? JNI_TRUE : JNI_FALSE));
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void view_group_add_view(JNIEnv* env, jobject group, jobject child) {
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

jobject install_checked_change_listener(JNIEnv* env, jobject rb, jlong handler_ptr) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass listener_cls = env->FindClass("io/mpapp/MppCheckedChangeListener");
    if (listener_cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(listener_cls, "<init>", "(JI)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(listener_cls); return nullptr; }
    jobject local = env->NewObject(listener_cls, ctor, handler_ptr,
                                   static_cast<jint>(radio_button_handler<platform::android>::kind));
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
            env->CallVoidMethod(rb, set_listener, local);
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

radio_button_handler<platform::android>::radio_button_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env != nullptr) {
        native_ = make_radio_button(env, detail::get_activity());
    }
}

radio_button_handler<platform::android>::~radio_button_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (listener_ != nullptr) { env->DeleteGlobalRef(listener_); listener_ = nullptr; }
        if (native_   != nullptr) { env->DeleteGlobalRef(native_);   native_   = nullptr; }
    }
}

void radio_button_handler<platform::android>::apply_is_checked(bool v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    suppress_echo_ = true;
    radio_button_set_checked(env, native_, v);
    suppress_echo_ = false;
}

void radio_button_handler<platform::android>::apply_group_name(const std::string& v) {
    if (native_ == nullptr || v.empty() || v == attached_group_) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;

    jobject group_ref = nullptr;
    {
        std::lock_guard<std::mutex> g{group_registry_mutex()};
        auto& reg = group_registry();
        auto it = reg.find(v);
        if (it == reg.end()) {
            group_ref = make_radio_group(env, detail::get_activity());
            if (group_ref == nullptr) return;
            reg[v] = group_ref;
        } else {
            group_ref = it->second;
        }
    }
    if (group_ref != nullptr) {
        view_group_add_view(env, group_ref, native_);
        attached_group_ = v;
    }
}

void radio_button_handler<platform::android>::map_is_checked(basic_radio_button& r) {
    bound_ = &r;
    apply_is_checked(r.is_checked.get());
    r.is_checked.changed.subscribe(is_checked_slot_, is_checked_cb_);

    if (native_ != nullptr && listener_ == nullptr) {
        JNIEnv* env = detail::attach_current_thread();
        if (env != nullptr) {
            listener_ = install_checked_change_listener(env, native_,
                            reinterpret_cast<jlong>(this));
        }
    }
}

void radio_button_handler<platform::android>::map_group_name(basic_radio_button& r) {
    apply_group_name(r.group_name.get());
    r.group_name.changed.subscribe(group_name_slot_, group_name_cb_);
}

void radio_button_handler<platform::android>::on_native_checked_changed(bool checked) {
    if (suppress_echo_ || bound_ == nullptr) return;
    if (bound_->is_checked.get() != checked) {
        bound_->is_checked.set(checked);
    }
}

void android_radio_button_dispatch_checked_changed(radio_button_handler<platform::android>* h, bool checked) {
    if (h != nullptr) h->on_native_checked_changed(checked);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
// Phase 2 sweep per M-04b: register basic_radio_button so ADR-0013 fall-through
// dispatch can find its native handle without the legacy dynamic_cast chain.

#include "mpapp/handlers/android/widget_dispatch.hpp"
#include "mpapp/internal/basic_radio_button.hpp"

namespace {

jobject dispatch_radio_button(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_radio_button*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_radio_button); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
