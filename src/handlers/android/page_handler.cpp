// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_page handler implementation.

#include "mpapp/handlers/android/page_handler.hpp"

#if defined(__ANDROID__)

#include <string>

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

#include "mpapp/internal/basic_page.hpp"
#include "mpapp/view.hpp"

namespace mpapp::internal {

namespace {

constexpr int LINEAR_LAYOUT_VERTICAL = 1;

jobject make_linear_layout(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/LinearLayout");
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

jobject make_text_view(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/TextView");
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

jobject make_frame_layout(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/FrameLayout");
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

jobject make_progress_bar(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/ProgressBar");
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

void linear_layout_set_orientation(JNIEnv* env, jobject ll, int orient_native) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/LinearLayout");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setOrientation", "(I)V");
    if (m != nullptr) {
        env->CallVoidMethod(ll, m, orient_native);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void view_group_add(JNIEnv* env, jobject group, jobject child) {
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

void view_group_remove_all(JNIEnv* env, jobject group) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/ViewGroup");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "removeAllViews", "()V");
    if (m != nullptr) {
        env->CallVoidMethod(group, m);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void text_view_set_text(JNIEnv* env, jobject tv, const std::string& text) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/TextView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setText", "(Ljava/lang/CharSequence;)V");
    if (m != nullptr) {
        jstring jstr = env->NewStringUTF(text.c_str());
        env->CallVoidMethod(tv, m, jstr);
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->DeleteLocalRef(jstr);
    }
    env->DeleteLocalRef(cls);
}

void view_set_visibility(JNIEnv* env, jobject view, int vis) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/View");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setVisibility", "(I)V");
    if (m != nullptr) {
        env->CallVoidMethod(view, m, vis);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

} // namespace

page_handler<platform::android>::page_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();

    jobject context = detail::get_activity();

    native_       = make_linear_layout(env, context);
    title_view_   = make_text_view   (env, context);
    content_host_ = make_frame_layout(env, context);
    busy_bar_     = make_progress_bar(env, context);

    if (native_ != nullptr) {
        linear_layout_set_orientation(env, native_, LINEAR_LAYOUT_VERTICAL);
        if (title_view_   != nullptr) view_group_add(env, native_, title_view_);
        if (content_host_ != nullptr) view_group_add(env, native_, content_host_);
        if (busy_bar_     != nullptr) {
            view_group_add(env, native_, busy_bar_);
            // GONE = 8 (don't reserve layout space when not busy).
            view_set_visibility(env, busy_bar_, 8);
        }
    }
}

page_handler<platform::android>::~page_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    if (busy_bar_     != nullptr) { env->DeleteGlobalRef(busy_bar_);     busy_bar_     = nullptr; }
    if (content_host_ != nullptr) { env->DeleteGlobalRef(content_host_); content_host_ = nullptr; }
    if (title_view_   != nullptr) { env->DeleteGlobalRef(title_view_);   title_view_   = nullptr; }
    if (native_       != nullptr) { env->DeleteGlobalRef(native_);       native_       = nullptr; }
}

void page_handler<platform::android>::apply_title(const std::string& v) {
    if (title_view_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    text_view_set_text(env, title_view_, v);
}

void page_handler<platform::android>::apply_content(view* v) {
    if (content_host_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    view_group_remove_all(env, content_host_);
    // ADR-0013: query the registry.
    jobject child = (v != nullptr) ? detail::android_dispatch::dispatch(v) : nullptr;
    if (child != nullptr) view_group_add(env, content_host_, child);
}

void page_handler<platform::android>::apply_is_busy(bool v) {
    if (busy_bar_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    // View.VISIBLE = 0, View.GONE = 8
    view_set_visibility(env, busy_bar_, v ? 0 : 8);
}

void page_handler<platform::android>::map_title(basic_page& p) {
    apply_title(p.title.get());
    p.title.changed.subscribe(title_slot_, title_cb_);
}

void page_handler<platform::android>::map_content(basic_page& p) {
    apply_content(p.content.get());
    p.content.changed.subscribe(content_slot_, content_cb_);
}

void page_handler<platform::android>::map_is_busy(basic_page& p) {
    apply_is_busy(p.is_busy.get());
    p.is_busy.changed.subscribe(busy_slot_, busy_cb_);
}

void page_handler<platform::android>::bind_content(basic_page& p, view& child) {
    p.content.set(&child);
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --

namespace {

jobject dispatch_page(::mpapp::view* v) {
    if (auto* p = dynamic_cast<::mpapp::internal::basic_page*>(v); p && p->has_handler()) {
        return p->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_page); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
