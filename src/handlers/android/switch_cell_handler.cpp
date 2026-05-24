// SPDX-License-Identifier: Apache-2.0
// Android basic_switch_cell handler implementation.

#include "mpapp/handlers/android/switch_cell_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

namespace mpapp::internal {

namespace {

constexpr int LINEAR_LAYOUT_HORIZONTAL = 0;

constexpr int MATCH_PARENT = -1;
constexpr int WRAP_CONTENT = -2;

jobject make_object(JNIEnv* env, const char* cls_name, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass(cls_name);
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

void ll_set_orientation(JNIEnv* env, jobject ll, int orientation) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/LinearLayout");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setOrientation", "(I)V");
    if (m != nullptr) {
        env->CallVoidMethod(ll, m, static_cast<jint>(orientation));
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void tv_set_text(JNIEnv* env, jobject tv, const char* text) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/TextView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setText", "(Ljava/lang/CharSequence;)V");
    if (m != nullptr) {
        jstring s = env->NewStringUTF(text);
        env->CallVoidMethod(tv, m, s);
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->DeleteLocalRef(s);
    }
    env->DeleteLocalRef(cls);
}

void compound_set_checked(JNIEnv* env, jobject sw, bool checked) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/CompoundButton");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setChecked", "(Z)V");
    if (m != nullptr) {
        env->CallVoidMethod(sw, m, static_cast<jboolean>(checked ? JNI_TRUE : JNI_FALSE));
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void view_set_padding(JNIEnv* env, jobject view_obj, int left, int top, int right, int bottom) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/View");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setPadding", "(IIII)V");
    if (m != nullptr) {
        env->CallVoidMethod(view_obj, m,
                            static_cast<jint>(left), static_cast<jint>(top),
                            static_cast<jint>(right), static_cast<jint>(bottom));
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

// Build LinearLayout.LayoutParams(width, height, weight) and assign it
// to the given child via View.setLayoutParams. This gives the basic_label its
// weight=1 so the switch is pushed to the trailing edge.
void apply_linear_lp_with_weight(JNIEnv* env, jobject child,
                                  int width, int height, float weight) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass lp_cls = env->FindClass("android/widget/LinearLayout$LayoutParams");
    if (lp_cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID ctor = env->GetMethodID(lp_cls, "<init>", "(IIF)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(lp_cls); return; }
    jobject lp = env->NewObject(lp_cls, ctor,
                                static_cast<jint>(width),
                                static_cast<jint>(height),
                                static_cast<jfloat>(weight));
    env->DeleteLocalRef(lp_cls);
    if (lp == nullptr) { env->ExceptionClear(); return; }

    jclass view_cls = env->FindClass("android/view/View");
    if (view_cls != nullptr) {
        jmethodID set_lp = env->GetMethodID(view_cls, "setLayoutParams",
            "(Landroid/view/ViewGroup$LayoutParams;)V");
        if (set_lp != nullptr) {
            env->CallVoidMethod(child, set_lp, lp);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        env->DeleteLocalRef(view_cls);
    }
    env->DeleteLocalRef(lp);
}

void vg_add(JNIEnv* env, jobject parent, jobject child) {
    if (child == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/ViewGroup");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID add = env->GetMethodID(cls, "addView", "(Landroid/view/View;)V");
    if (add != nullptr) {
        env->CallVoidMethod(parent, add, child);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

jobject install_checked_change_listener(JNIEnv* env, jobject sw, jlong handler_ptr) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass listener_cls = env->FindClass("io/mpapp/MppCheckedChangeListener");
    if (listener_cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(listener_cls, "<init>", "(JI)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(listener_cls); return nullptr; }
    // kind=4 → switch_cell_handler routing
    jobject local = env->NewObject(listener_cls, ctor, handler_ptr, static_cast<jint>(4));
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
            env->CallVoidMethod(sw, set_listener, local);
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

switch_cell_handler<platform::android>::switch_cell_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    jobject ctx = detail::get_activity();

    native_     = make_object(env, "android/widget/LinearLayout", ctx);
    text_view_  = make_object(env, "android/widget/TextView",     ctx);
    switch_obj_ = make_object(env, "android/widget/Switch",       ctx);

    if (native_ != nullptr) {
        ll_set_orientation(env, native_, LINEAR_LAYOUT_HORIZONTAL);
        view_set_padding(env, native_, 24, 12, 24, 12);
        if (text_view_ != nullptr) {
            // weight=1 so the basic_label takes the remaining width; switch hugs
            // the trailing edge.
            apply_linear_lp_with_weight(env, text_view_, 0, WRAP_CONTENT, 1.0f);
            vg_add(env, native_, text_view_);
        }
        if (switch_obj_ != nullptr) {
            apply_linear_lp_with_weight(env, switch_obj_, WRAP_CONTENT, WRAP_CONTENT, 0.0f);
            vg_add(env, native_, switch_obj_);
        }
    }
}

switch_cell_handler<platform::android>::~switch_cell_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (listener_    != nullptr) { env->DeleteGlobalRef(listener_);    listener_    = nullptr; }
        if (switch_obj_  != nullptr) { env->DeleteGlobalRef(switch_obj_);  switch_obj_  = nullptr; }
        if (text_view_   != nullptr) { env->DeleteGlobalRef(text_view_);   text_view_   = nullptr; }
        if (native_      != nullptr) { env->DeleteGlobalRef(native_);      native_      = nullptr; }
    }
}

void switch_cell_handler<platform::android>::apply_text(const std::string& v) {
    if (text_view_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    tv_set_text(env, text_view_, v.c_str());
}

void switch_cell_handler<platform::android>::apply_on(bool v) {
    if (switch_obj_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    suppress_echo_ = true;
    compound_set_checked(env, switch_obj_, v);
    suppress_echo_ = false;
}

void switch_cell_handler<platform::android>::map_text(basic_switch_cell& c) {
    apply_text(c.text.get());
    c.text.changed.subscribe(text_slot_, text_cb_);
}

void switch_cell_handler<platform::android>::map_on(basic_switch_cell& c) {
    bound_ = &c;
    apply_on(c.on.get());
    c.on.changed.subscribe(on_slot_, on_cb_);

    if (switch_obj_ != nullptr && listener_ == nullptr) {
        JNIEnv* env = detail::attach_current_thread();
        if (env != nullptr) {
            listener_ = install_checked_change_listener(env, switch_obj_,
                            reinterpret_cast<jlong>(this));
        }
    }
}

void switch_cell_handler<platform::android>::on_native_checked_changed(bool checked) {
    if (suppress_echo_ || bound_ == nullptr) return;
    if (bound_->on.get() != checked) {
        bound_->on.set(checked);
    }
    bound_->on_changed.emit(checked);
}

void android_switch_cell_dispatch_checked_changed(switch_cell_handler<platform::android>* h,
                                                  bool checked) {
    if (h != nullptr) h->on_native_checked_changed(checked);
}

} // namespace mpapp::internal
// ---------- Self-registration --------------------------------------------
namespace {

jobject dispatch_switch_cell(::mpapp::view* v) {
    if (auto* c = dynamic_cast<::mpapp::internal::basic_switch_cell*>(v); c && c->has_sc_handler()) {
        return c->sc_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_switch_cell); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
