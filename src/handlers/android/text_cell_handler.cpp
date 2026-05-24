// SPDX-License-Identifier: Apache-2.0
// Android basic_text_cell handler implementation.

#include "mpapp/handlers/android/text_cell_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

namespace mpapp::internal {

namespace {

constexpr int LINEAR_LAYOUT_VERTICAL = 1;
constexpr int VIEW_VISIBLE = 0;
constexpr int VIEW_GONE    = 8;

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

void view_set_visibility(JNIEnv* env, jobject view_obj, int v) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/View");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setVisibility", "(I)V");
    if (m != nullptr) {
        env->CallVoidMethod(view_obj, m, static_cast<jint>(v));
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

} // namespace

text_cell_handler<platform::android>::text_cell_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    jobject ctx = detail::get_activity();

    native_      = make_object(env, "android/widget/LinearLayout", ctx);
    text_view_   = make_object(env, "android/widget/TextView",     ctx);
    detail_view_ = make_object(env, "android/widget/TextView",     ctx);

    if (native_ != nullptr) {
        ll_set_orientation(env, native_, LINEAR_LAYOUT_VERTICAL);
        view_set_padding(env, native_, 24, 12, 24, 12);  // approx dp; raw px in v1
        vg_add(env, native_, text_view_);
        vg_add(env, native_, detail_view_);
    }
    if (detail_view_ != nullptr) view_set_visibility(env, detail_view_, VIEW_GONE);
}

text_cell_handler<platform::android>::~text_cell_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (detail_view_ != nullptr) { env->DeleteGlobalRef(detail_view_); detail_view_ = nullptr; }
        if (text_view_   != nullptr) { env->DeleteGlobalRef(text_view_);   text_view_   = nullptr; }
        if (native_      != nullptr) { env->DeleteGlobalRef(native_);      native_      = nullptr; }
    }
}

void text_cell_handler<platform::android>::apply_text(const std::string& v) {
    if (text_view_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    tv_set_text(env, text_view_, v.c_str());
}

void text_cell_handler<platform::android>::apply_detail(const std::string& v) {
    if (detail_view_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    tv_set_text(env, detail_view_, v.c_str());
    view_set_visibility(env, detail_view_, v.empty() ? VIEW_GONE : VIEW_VISIBLE);
}

void text_cell_handler<platform::android>::map_text(basic_text_cell& c) {
    apply_text(c.text.get());
    c.text.changed.subscribe(text_slot_, text_cb_);
}

void text_cell_handler<platform::android>::map_detail(basic_text_cell& c) {
    apply_detail(c.detail.get());
    c.detail.changed.subscribe(detail_slot_, detail_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration --------------------------------------------
namespace {

jobject dispatch_text_cell(::mpapp::view* v) {
    if (auto* c = dynamic_cast<::mpapp::internal::basic_text_cell*>(v); c && c->has_tc_handler()) {
        return c->tc_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_text_cell); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
