// SPDX-License-Identifier: Apache-2.0
// Android navigation_page handler implementation.

#include "mpapp/handlers/android/navigation_page_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"
#include "mpapp/page.hpp"

namespace mpapp {

namespace {

constexpr int LINEAR_LAYOUT_VERTICAL   = 1;
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

void linear_layout_set_orientation(JNIEnv* env, jobject ll, int orientation) {
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

void view_set_text(JNIEnv* env, jobject view_obj, const char* text) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/TextView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setText", "(Ljava/lang/CharSequence;)V");
    if (m != nullptr) {
        jstring s = env->NewStringUTF(text);
        env->CallVoidMethod(view_obj, m, s);
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

void view_group_add(JNIEnv* env, jobject parent, jobject child) {
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

void view_group_remove_all(JNIEnv* env, jobject parent) {
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

navigation_page_handler<platform::android>::navigation_page_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    jobject ctx = detail::get_activity();

    native_       = make_object(env, "android/widget/LinearLayout", ctx);
    bar_          = make_object(env, "android/widget/LinearLayout", ctx);
    back_button_  = make_object(env, "android/widget/Button",       ctx);
    title_view_   = make_object(env, "android/widget/TextView",     ctx);
    content_host_ = make_object(env, "android/widget/FrameLayout",  ctx);

    if (native_ != nullptr)
        linear_layout_set_orientation(env, native_, LINEAR_LAYOUT_VERTICAL);
    if (bar_ != nullptr)
        linear_layout_set_orientation(env, bar_, LINEAR_LAYOUT_HORIZONTAL);

    if (back_button_ != nullptr) {
        view_set_text(env, back_button_, "<");
        view_set_visibility(env, back_button_, VIEW_GONE);
    }
    if (title_view_ != nullptr) view_set_text(env, title_view_, "");

    if (bar_ != nullptr) {
        view_group_add(env, bar_, back_button_);
        view_group_add(env, bar_, title_view_);
    }
    if (native_ != nullptr) {
        view_group_add(env, native_, bar_);
        view_group_add(env, native_, content_host_);
    }
}

navigation_page_handler<platform::android>::~navigation_page_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (content_host_ != nullptr) { env->DeleteGlobalRef(content_host_); content_host_ = nullptr; }
        if (title_view_   != nullptr) { env->DeleteGlobalRef(title_view_);   title_view_   = nullptr; }
        if (back_button_  != nullptr) { env->DeleteGlobalRef(back_button_);  back_button_  = nullptr; }
        if (bar_          != nullptr) { env->DeleteGlobalRef(bar_);          bar_          = nullptr; }
        if (native_       != nullptr) { env->DeleteGlobalRef(native_);       native_       = nullptr; }
    }
}

void navigation_page_handler<platform::android>::apply_top(view* new_top) {
    if (content_host_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;

    view_group_remove_all(env, content_host_);
    if (new_top != nullptr) {
        if (jobject native = detail::android_dispatch::dispatch(new_top); native != nullptr) {
            view_group_add(env, content_host_, native);
        }
    }

    if (auto* p = dynamic_cast<page*>(new_top); p != nullptr) {
        apply_title(p->title.get());
    } else {
        apply_title("");
    }
}

void navigation_page_handler<platform::android>::apply_title(const std::string& v) {
    if (title_view_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    view_set_text(env, title_view_, v.c_str());
}

void navigation_page_handler<platform::android>::apply_back_visibility(std::size_t depth) {
    if (back_button_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    view_set_visibility(env, back_button_, depth > 1 ? VIEW_VISIBLE : VIEW_GONE);
}

void navigation_page_handler<platform::android>::map_stack(navigation_page& np) {
    bound_ = &np;
    apply_top(np.stack().top());
    apply_back_visibility(np.stack().depth());
    np.stack().page_did_appear.subscribe(did_appear_slot_, did_appear_cb_);
    np.stack_depth.changed.subscribe(depth_slot_, depth_cb_);
    // Back-button OnClickListener wiring is deferred to M-05 polish —
    // the mock surface still exposes nav.pop() directly to user code.
}

} // namespace mpapp

// ---------- Self-registration --------------------------------------------
namespace {

jobject dispatch_navigation_page(::mpapp::view* v) {
    if (auto* n = dynamic_cast<::mpapp::navigation_page*>(v); n && n->has_np_handler()) {
        return n->np_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_navigation_page); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
