// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_refresh_view handler implementation.

#include "mpapp/handlers/android/refresh_view_handler.hpp"

#if defined(__ANDROID__)

#include <cstdlib>
#include <string>

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

namespace mpapp::internal {

namespace {

jint parse_argb(const brush_ref& br, jint fallback) {
    const std::string& name = br.name;
    if (name.empty()) return fallback;
    if (name[0] == '#') {
        unsigned long v = std::strtoul(name.c_str() + 1, nullptr, 16);
        if (name.size() == 7) return static_cast<jint>(0xFF000000U | v);
        if (name.size() == 9) return static_cast<jint>(v);
        return fallback;
    }
    if (name == "Red")        return static_cast<jint>(0xFFDC3232U);
    if (name == "Green")      return static_cast<jint>(0xFF50B450U);
    if (name == "Blue")       return static_cast<jint>(0xFF3C78DCU);
    if (name == "Black")      return static_cast<jint>(0xFF000000U);
    if (name == "White")      return static_cast<jint>(0xFFFFFFFFU);
    if (name == "Teal")       return static_cast<jint>(0xFF0096A5U);
    if (name == "Gray")       return static_cast<jint>(0xFF808080U);
    if (name == "DodgerBlue") return static_cast<jint>(0xFF1E90FFU);
    return fallback;
}

// Try to create a SwipeRefreshLayout; returns nullptr if androidx is not
// on the classpath. Caller owns the returned global ref.
jobject try_make_swipe_refresh(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("androidx/swiperefreshlayout/widget/SwipeRefreshLayout");
    if (cls == nullptr) {
        env->ExceptionClear();  // class-not-found is expected on the fallback path
        return nullptr;
    }
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
    jmethodID set_indet = env->GetMethodID(cls, "setIndeterminate", "(Z)V");
    if (set_indet != nullptr) {
        env->CallVoidMethod(local, set_indet, JNI_TRUE);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
    jobject global = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    return global;
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

void view_set_visibility(JNIEnv* env, jobject view, bool visible) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/View");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID set_vis = env->GetMethodID(cls, "setVisibility", "(I)V");
    if (set_vis != nullptr) {
        env->CallVoidMethod(view, set_vis, static_cast<jint>(visible ? 0 : 8));  // VISIBLE=0, GONE=8
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

// SwipeRefreshLayout.setRefreshing(boolean).
void swipe_set_refreshing(JNIEnv* env, jobject swipe, bool v) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("androidx/swiperefreshlayout/widget/SwipeRefreshLayout");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setRefreshing", "(Z)V");
    if (m != nullptr) {
        env->CallVoidMethod(swipe, m, v ? JNI_TRUE : JNI_FALSE);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

// SwipeRefreshLayout.setColorSchemeColors(int...) — varargs become an
// int[] at the JNI boundary.
void swipe_set_color(JNIEnv* env, jobject swipe, jint argb) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("androidx/swiperefreshlayout/widget/SwipeRefreshLayout");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setColorSchemeColors", "([I)V");
    if (m != nullptr) {
        jintArray arr = env->NewIntArray(1);
        if (arr != nullptr) {
            env->SetIntArrayRegion(arr, 0, 1, &argb);
            env->CallVoidMethod(swipe, m, arr);
            if (env->ExceptionCheck()) env->ExceptionClear();
            env->DeleteLocalRef(arr);
        }
    }
    env->DeleteLocalRef(cls);
}

void progress_set_tint(JNIEnv* env, jobject pb, jint argb) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass csl_cls = env->FindClass("android/content/res/ColorStateList");
    if (csl_cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID value_of = env->GetStaticMethodID(
        csl_cls, "valueOf", "(I)Landroid/content/res/ColorStateList;");
    if (value_of == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(csl_cls); return; }
    jobject csl = env->CallStaticObjectMethod(csl_cls, value_of, argb);
    env->DeleteLocalRef(csl_cls);
    if (csl == nullptr || env->ExceptionCheck()) {
        if (env->ExceptionCheck()) env->ExceptionClear();
        if (csl != nullptr) env->DeleteLocalRef(csl);
        return;
    }
    jclass pb_cls = env->FindClass("android/widget/ProgressBar");
    if (pb_cls != nullptr) {
        jmethodID set_tint = env->GetMethodID(
            pb_cls, "setIndeterminateTintList", "(Landroid/content/res/ColorStateList;)V");
        if (set_tint != nullptr) {
            env->CallVoidMethod(pb, set_tint, csl);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        env->DeleteLocalRef(pb_cls);
    }
    env->DeleteLocalRef(csl);
}

} // namespace

refresh_view_handler<platform::android>::refresh_view_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    jobject context = detail::get_activity();

    // Path A — preferred: androidx.swiperefreshlayout.SwipeRefreshLayout.
    native_ = try_make_swipe_refresh(env, context);
    if (native_ != nullptr) {
        is_swipe_ = true;
        return;
    }

    // Path B — fallback: FrameLayout host + indeterminate ProgressBar
    // overlay, manually toggled by apply_is_refreshing.
    is_swipe_ = false;
    native_ = make_frame_layout(env, context);
    if (native_ == nullptr) return;
    spinner_ = make_progress_bar(env, context);
    if (spinner_ != nullptr) {
        view_set_visibility(env, spinner_, false);
        view_group_add(env, native_, spinner_);
    }
}

refresh_view_handler<platform::android>::~refresh_view_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (spinner_ != nullptr) { env->DeleteGlobalRef(spinner_); spinner_ = nullptr; }
    if (native_  != nullptr) { env->DeleteGlobalRef(native_);  native_  = nullptr; }
}

void refresh_view_handler<platform::android>::apply_content(const std::shared_ptr<view>& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;

    // Wipe any previous child. On the fallback path the ProgressBar
    // overlay needs to survive — re-add it after the wipe.
    view_group_remove_all(env, native_);
    if (!is_swipe_ && spinner_ != nullptr) {
        view_group_add(env, native_, spinner_);
    }

    // ADR-0013 registry first; if no widget is registered for the child
    // type, leave content empty.
    jobject child = v ? detail::android_dispatch::dispatch(v.get()) : nullptr;
    if (child != nullptr) {
        view_group_add(env, native_, child);
    }
}

void refresh_view_handler<platform::android>::apply_is_refreshing(bool v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (is_swipe_) {
        swipe_set_refreshing(env, native_, v);
    } else if (spinner_ != nullptr) {
        view_set_visibility(env, spinner_, v);
    }
}

void refresh_view_handler<platform::android>::apply_refresh_color(const brush_ref& b) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    const jint argb = parse_argb(b, static_cast<jint>(0xFF0078D7U));
    if (is_swipe_) {
        swipe_set_color(env, native_, argb);
    } else if (spinner_ != nullptr) {
        progress_set_tint(env, spinner_, argb);
    }
}

void refresh_view_handler<platform::android>::map_content(basic_refresh_view& r) {
    apply_content(r.content.get());
    r.content.changed.subscribe(content_slot_, content_cb_);
}

void refresh_view_handler<platform::android>::map_is_refreshing(basic_refresh_view& r) {
    apply_is_refreshing(r.is_refreshing.get());
    r.is_refreshing.changed.subscribe(is_refreshing_slot_, is_refreshing_cb_);
}

void refresh_view_handler<platform::android>::map_refresh_color(basic_refresh_view& r) {
    apply_refresh_color(r.refresh_color.get());
    r.refresh_color.changed.subscribe(refresh_color_slot_, refresh_color_cb_);
}

void refresh_view_handler<platform::android>::bind_content(basic_refresh_view& r, view& child) {
    r.content.set(std::shared_ptr<view>(&child, [](view*){}));
}

// ----- ADR-0013 self-registration --------------------------------------

namespace {

jobject dispatch_refresh_view(::mpapp::view* v) {
    if (auto* r = dynamic_cast<::mpapp::internal::basic_refresh_view*>(v); r && r->has_handler()) {
        return r->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() {
        ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_refresh_view);
    }
};

[[maybe_unused]] registrar _reg;

} // namespace

} // namespace mpapp::internal
#endif // __ANDROID__
