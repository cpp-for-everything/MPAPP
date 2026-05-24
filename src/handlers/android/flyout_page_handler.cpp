// SPDX-License-Identifier: Apache-2.0
// Android basic_flyout_page handler implementation.

#include "mpapp/handlers/android/flyout_page_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

namespace mpapp::internal {

namespace {

constexpr int LINEAR_LAYOUT_HORIZONTAL = 0;
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

flyout_page_handler<platform::android>::flyout_page_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    jobject ctx = detail::get_activity();

    native_      = make_object(env, "android/widget/LinearLayout", ctx);
    flyout_host_ = make_object(env, "android/widget/FrameLayout",  ctx);
    detail_host_ = make_object(env, "android/widget/FrameLayout",  ctx);

    if (native_ != nullptr) ll_set_orientation(env, native_, LINEAR_LAYOUT_HORIZONTAL);
    if (flyout_host_ != nullptr) view_set_visibility(env, flyout_host_, VIEW_GONE);

    if (native_ != nullptr) {
        vg_add(env, native_, flyout_host_);
        vg_add(env, native_, detail_host_);
    }
}

flyout_page_handler<platform::android>::~flyout_page_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (detail_host_ != nullptr) { env->DeleteGlobalRef(detail_host_); detail_host_ = nullptr; }
        if (flyout_host_ != nullptr) { env->DeleteGlobalRef(flyout_host_); flyout_host_ = nullptr; }
        if (native_      != nullptr) { env->DeleteGlobalRef(native_);      native_      = nullptr; }
    }
}

void flyout_page_handler<platform::android>::apply_flyout(basic_page* p) {
    if (flyout_host_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    vg_remove_all(env, flyout_host_);
    if (p != nullptr) {
        if (jobject n = detail::android_dispatch::dispatch(p); n != nullptr) {
            vg_add(env, flyout_host_, n);
        }
    }
}

void flyout_page_handler<platform::android>::apply_detail(basic_page* p) {
    if (detail_host_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    vg_remove_all(env, detail_host_);
    if (p != nullptr) {
        if (jobject n = detail::android_dispatch::dispatch(p); n != nullptr) {
            vg_add(env, detail_host_, n);
        }
    }
}

void flyout_page_handler<platform::android>::apply_is_presented(bool v) {
    if (flyout_host_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    view_set_visibility(env, flyout_host_, v ? VIEW_VISIBLE : VIEW_GONE);
}

void flyout_page_handler<platform::android>::map_flyout(basic_flyout_page& fp) {
    apply_flyout(fp.flyout.get());
    fp.flyout.changed.subscribe(flyout_slot_, flyout_cb_);
}

void flyout_page_handler<platform::android>::map_detail(basic_flyout_page& fp) {
    apply_detail(fp.detail.get());
    fp.detail.changed.subscribe(detail_slot_, detail_cb_);
}

void flyout_page_handler<platform::android>::map_is_presented(basic_flyout_page& fp) {
    apply_is_presented(fp.is_presented.get());
    fp.is_presented.changed.subscribe(presented_slot_, presented_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration --------------------------------------------
#include "mpapp/internal/basic_flyout_page.hpp"

namespace {

jobject dispatch_flyout_page(::mpapp::view* v) {
    if (auto* f = dynamic_cast<::mpapp::internal::basic_flyout_page*>(v); f && f->has_fp_handler()) {
        return f->fp_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_flyout_page); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
