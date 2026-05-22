// SPDX-License-Identifier: Apache-2.0
// Android entry_cell handler implementation.

#include "mpapp/handlers/android/entry_cell_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

namespace mpapp {

namespace {

constexpr int LINEAR_LAYOUT_HORIZONTAL = 0;
constexpr int WRAP_CONTENT = -2;

// EditorInfo.IME_ACTION_* values — kept verbatim from android.view.inputmethod.EditorInfo.
constexpr int IME_ACTION_DONE   = 6;
constexpr int IME_ACTION_GO     = 2;
constexpr int IME_ACTION_NEXT   = 5;
constexpr int IME_ACTION_SEARCH = 3;
constexpr int IME_ACTION_SEND   = 4;

// InputType constants (android.text.InputType).
constexpr int TYPE_CLASS_TEXT                 = 0x00000001;
constexpr int TYPE_CLASS_NUMBER               = 0x00000002;
constexpr int TYPE_CLASS_PHONE                = 0x00000003;
constexpr int TYPE_TEXT_VARIATION_EMAIL_ADDR  = 0x00000020;
constexpr int TYPE_TEXT_VARIATION_URI         = 0x00000010;
constexpr int TYPE_TEXT_VARIATION_SHORT_MSG   = 0x00000040;

int keyboard_to_input_type(keyboard_kind k) {
    switch (k) {
        case keyboard_kind::email:     return TYPE_CLASS_TEXT | TYPE_TEXT_VARIATION_EMAIL_ADDR;
        case keyboard_kind::numeric:   return TYPE_CLASS_NUMBER;
        case keyboard_kind::telephone: return TYPE_CLASS_PHONE;
        case keyboard_kind::url:       return TYPE_CLASS_TEXT | TYPE_TEXT_VARIATION_URI;
        case keyboard_kind::chat:      return TYPE_CLASS_TEXT | TYPE_TEXT_VARIATION_SHORT_MSG;
        case keyboard_kind::text:
        case keyboard_kind::default_:
        default:                       return TYPE_CLASS_TEXT;
    }
}

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

void tv_set_hint(JNIEnv* env, jobject tv, const char* text) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/TextView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setHint", "(Ljava/lang/CharSequence;)V");
    if (m != nullptr) {
        jstring s = env->NewStringUTF(text);
        env->CallVoidMethod(tv, m, s);
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->DeleteLocalRef(s);
    }
    env->DeleteLocalRef(cls);
}

void tv_set_input_type(JNIEnv* env, jobject tv, int input_type) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/TextView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setInputType", "(I)V");
    if (m != nullptr) {
        env->CallVoidMethod(tv, m, static_cast<jint>(input_type));
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

void apply_linear_weight_lp(JNIEnv* env, jobject child, int w, int h, float weight) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass lp_cls = env->FindClass("android/widget/LinearLayout$LayoutParams");
    if (lp_cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID ctor = env->GetMethodID(lp_cls, "<init>", "(IIF)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(lp_cls); return; }
    jobject lp = env->NewObject(lp_cls, ctor,
                                static_cast<jint>(w),
                                static_cast<jint>(h),
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

jobject install_text_watcher(JNIEnv* env, jobject edit_text, jlong handler_ptr) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass watcher_cls = env->FindClass("io/mpapp/MppTextWatcher");
    if (watcher_cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(watcher_cls, "<init>", "(JI)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(watcher_cls); return nullptr; }
    // kind=3 → entry_cell_handler text routing
    jobject local = env->NewObject(watcher_cls, ctor, handler_ptr, static_cast<jint>(3));
    if (local == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(watcher_cls);
        return nullptr;
    }
    jclass et_cls = env->FindClass("android/widget/EditText");
    if (et_cls != nullptr) {
        jmethodID add_watcher = env->GetMethodID(
            et_cls, "addTextChangedListener", "(Landroid/text/TextWatcher;)V");
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

jobject install_editor_action_listener(JNIEnv* env, jobject edit_text, jlong handler_ptr) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass listener_cls = env->FindClass("io/mpapp/MppEditorActionListener");
    if (listener_cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(listener_cls, "<init>", "(JI)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(listener_cls); return nullptr; }
    // kind=1 → entry_cell_handler
    jobject local = env->NewObject(listener_cls, ctor, handler_ptr, static_cast<jint>(1));
    if (local == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(listener_cls);
        return nullptr;
    }
    jclass tv_cls = env->FindClass("android/widget/TextView");
    if (tv_cls != nullptr) {
        jmethodID set_l = env->GetMethodID(
            tv_cls, "setOnEditorActionListener",
            "(Landroid/widget/TextView$OnEditorActionListener;)V");
        if (set_l != nullptr) {
            env->CallVoidMethod(edit_text, set_l, local);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        env->DeleteLocalRef(tv_cls);
    }
    jobject global = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    env->DeleteLocalRef(listener_cls);
    return global;
}

} // namespace

entry_cell_handler<platform::android>::entry_cell_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    jobject ctx = detail::get_activity();

    native_     = make_object(env, "android/widget/LinearLayout", ctx);
    label_view_ = make_object(env, "android/widget/TextView",     ctx);
    edit_text_  = make_object(env, "android/widget/EditText",     ctx);

    if (native_ != nullptr) {
        ll_set_orientation(env, native_, LINEAR_LAYOUT_HORIZONTAL);
        view_set_padding(env, native_, 24, 12, 24, 12);
    }
    if (label_view_ != nullptr) {
        apply_linear_weight_lp(env, label_view_, WRAP_CONTENT, WRAP_CONTENT, 0.0f);
        vg_add(env, native_, label_view_);
    }
    if (edit_text_ != nullptr) {
        apply_linear_weight_lp(env, edit_text_, 0, WRAP_CONTENT, 1.0f);
        vg_add(env, native_, edit_text_);
    }
}

entry_cell_handler<platform::android>::~entry_cell_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (ime_listener_ != nullptr) { env->DeleteGlobalRef(ime_listener_); ime_listener_ = nullptr; }
        if (watcher_      != nullptr) { env->DeleteGlobalRef(watcher_);      watcher_      = nullptr; }
        if (edit_text_    != nullptr) { env->DeleteGlobalRef(edit_text_);    edit_text_    = nullptr; }
        if (label_view_   != nullptr) { env->DeleteGlobalRef(label_view_);   label_view_   = nullptr; }
        if (native_       != nullptr) { env->DeleteGlobalRef(native_);       native_       = nullptr; }
    }
}

void entry_cell_handler<platform::android>::apply_label(const std::string& v) {
    if (label_view_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    tv_set_text(env, label_view_, v.c_str());
}

void entry_cell_handler<platform::android>::apply_text(const std::string& v) {
    if (edit_text_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    suppress_echo_ = true;
    tv_set_text(env, edit_text_, v.c_str());
    suppress_echo_ = false;
}

void entry_cell_handler<platform::android>::apply_placeholder(const std::string& v) {
    if (edit_text_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    tv_set_hint(env, edit_text_, v.c_str());
}

void entry_cell_handler<platform::android>::apply_keyboard(keyboard_kind k) {
    if (edit_text_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    tv_set_input_type(env, edit_text_, keyboard_to_input_type(k));
}

void entry_cell_handler<platform::android>::map_label(entry_cell& c) {
    apply_label(c.label.get());
    c.label.changed.subscribe(label_slot_, label_cb_);
}

void entry_cell_handler<platform::android>::map_text(entry_cell& c) {
    bound_ = &c;
    apply_text(c.text.get());
    c.text.changed.subscribe(text_slot_, text_cb_);

    if (edit_text_ != nullptr && watcher_ == nullptr) {
        JNIEnv* env = detail::attach_current_thread();
        if (env != nullptr) {
            watcher_ = install_text_watcher(env, edit_text_,
                            reinterpret_cast<jlong>(this));
        }
    }
    if (edit_text_ != nullptr && ime_listener_ == nullptr) {
        JNIEnv* env = detail::attach_current_thread();
        if (env != nullptr) {
            ime_listener_ = install_editor_action_listener(env, edit_text_,
                                reinterpret_cast<jlong>(this));
        }
    }
}

void entry_cell_handler<platform::android>::map_placeholder(entry_cell& c) {
    apply_placeholder(c.placeholder.get());
    c.placeholder.changed.subscribe(placeholder_slot_, placeholder_cb_);
}

void entry_cell_handler<platform::android>::map_keyboard(entry_cell& c) {
    apply_keyboard(c.keyboard.get());
    c.keyboard.changed.subscribe(keyboard_slot_, keyboard_cb_);
}

void entry_cell_handler<platform::android>::on_native_text_changed(const std::string& text) {
    if (suppress_echo_ || bound_ == nullptr) return;
    if (bound_->text.get() != text) {
        bound_->text.set(text);
    }
}

void entry_cell_handler<platform::android>::on_native_editor_action(int action_id) {
    if (bound_ == nullptr) return;
    // Only emit `completed` for IME terminal actions; ignore intermediates
    // and keyboard-shown notifications.
    switch (action_id) {
        case IME_ACTION_DONE:
        case IME_ACTION_GO:
        case IME_ACTION_NEXT:
        case IME_ACTION_SEARCH:
        case IME_ACTION_SEND:
            bound_->completed.emit(bound_->text.get());
            return;
        default:
            return;
    }
}

void android_entry_cell_dispatch_text_changed(entry_cell_handler<platform::android>* h,
                                              const std::string& text) {
    if (h != nullptr) h->on_native_text_changed(text);
}

void android_entry_cell_dispatch_editor_action(entry_cell_handler<platform::android>* h,
                                               int action_id) {
    if (h != nullptr) h->on_native_editor_action(action_id);
}

} // namespace mpapp

// ---------- Self-registration --------------------------------------------
namespace {

jobject dispatch_entry_cell(::mpapp::view* v) {
    if (auto* c = dynamic_cast<::mpapp::entry_cell*>(v); c && c->has_ec_handler()) {
        return c->ec_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_entry_cell); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
