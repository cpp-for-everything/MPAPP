// SPDX-License-Identifier: Apache-2.0
// Android tabbed_page handler implementation.

#include "mpapp/handlers/android/tabbed_page_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"
#include "mpapp/page.hpp"

namespace mpapp {

namespace {

constexpr int LINEAR_LAYOUT_VERTICAL   = 1;
constexpr int LINEAR_LAYOUT_HORIZONTAL = 0;

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

void vg_remove_all(JNIEnv* env, jobject parent) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/ViewGroup");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "removeAllViews", "()V");
    if (m != nullptr) {
        env->CallVoidMethod(parent, m);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

} // namespace

tabbed_page_handler<platform::android>::tabbed_page_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    jobject ctx = detail::get_activity();

    native_       = make_object(env, "android/widget/LinearLayout", ctx);
    tab_strip_    = make_object(env, "android/widget/LinearLayout", ctx);
    content_host_ = make_object(env, "android/widget/FrameLayout",  ctx);

    if (native_    != nullptr) ll_set_orientation(env, native_,    LINEAR_LAYOUT_VERTICAL);
    if (tab_strip_ != nullptr) ll_set_orientation(env, tab_strip_, LINEAR_LAYOUT_HORIZONTAL);

    if (native_ != nullptr) {
        vg_add(env, native_, tab_strip_);
        vg_add(env, native_, content_host_);
    }
}

tabbed_page_handler<platform::android>::~tabbed_page_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (content_host_ != nullptr) { env->DeleteGlobalRef(content_host_); content_host_ = nullptr; }
        if (tab_strip_    != nullptr) { env->DeleteGlobalRef(tab_strip_);    tab_strip_    = nullptr; }
        if (native_       != nullptr) { env->DeleteGlobalRef(native_);       native_       = nullptr; }
    }
}

void tabbed_page_handler<platform::android>::rebuild_children(const std::vector<page*>& kids) {
    current_kids_ = kids;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    vg_remove_all(env, tab_strip_);
    jobject ctx = detail::get_activity();
    for (page* p : kids) {
        if (p == nullptr) continue;
        jobject label = make_object(env, "android/widget/TextView", ctx);
        if (label != nullptr) {
            tv_set_text(env, label, p->title.get().c_str());
            vg_add(env, tab_strip_, label);
            env->DeleteGlobalRef(label);
        }
    }
    // Render current selection content.
    if (bound_ != nullptr) apply_selection(bound_->selected_index.get());
}

void tabbed_page_handler<platform::android>::apply_selection(int idx) {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    vg_remove_all(env, content_host_);
    if (idx < 0 || idx >= static_cast<int>(current_kids_.size())) return;
    page* sel = current_kids_[static_cast<std::size_t>(idx)];
    if (sel == nullptr) return;
    if (jobject native = detail::android_dispatch::dispatch(sel); native != nullptr) {
        vg_add(env, content_host_, native);
    }
}

void tabbed_page_handler<platform::android>::map_children(tabbed_page& tp) {
    bound_ = &tp;
    rebuild_children(tp.children.get());
    tp.children.changed.subscribe(children_slot_, children_cb_);
}

void tabbed_page_handler<platform::android>::map_selected_index(tabbed_page& tp) {
    apply_selection(tp.selected_index.get());
    tp.selected_index.changed.subscribe(selection_slot_, selection_cb_);
}

} // namespace mpapp

// ---------- Self-registration --------------------------------------------
namespace {

jobject dispatch_tabbed_page(::mpapp::view* v) {
    if (auto* t = dynamic_cast<::mpapp::tabbed_page*>(v); t && t->has_tp_handler()) {
        return t->tp_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_tabbed_page); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
