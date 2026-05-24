// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_flyout_view handler implementation.

#include "mpapp/handlers/android/flyout_view_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

namespace mpapp::internal {

namespace {

constexpr int LINEAR_LAYOUT_HORIZONTAL = 0;

// android.view.Gravity.START — composed of LEFT (0x3) + a relative flag
// (0x800000). Used by DrawerLayout's openDrawer(int) / closeDrawer(int)
// overloads as well as the LayoutParams gravity field.
constexpr int GRAVITY_START = 0x800003;

// Try to create a DrawerLayout; returns nullptr if androidx is not on
// the runtime classpath. Caller owns the returned global ref.
jobject try_make_drawer_layout(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("androidx/drawerlayout/widget/DrawerLayout");
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
    jmethodID set_orient = env->GetMethodID(cls, "setOrientation", "(I)V");
    if (set_orient != nullptr) {
        env->CallVoidMethod(local, set_orient, LINEAR_LAYOUT_HORIZONTAL);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
    jobject global = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    return global;
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

void view_group_remove(JNIEnv* env, jobject group, jobject child) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/ViewGroup");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "removeView", "(Landroid/view/View;)V");
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

// Build a DrawerLayout.LayoutParams configured for the flyout pane
// (gravity = START). Returns a local ref the caller should attach
// to the View via setLayoutParams. Used only on the DrawerLayout path.
jobject make_drawer_layout_params_for_flyout(JNIEnv* env) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass lp_cls = env->FindClass("androidx/drawerlayout/widget/DrawerLayout$LayoutParams");
    if (lp_cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID lp_ctor = env->GetMethodID(lp_cls, "<init>", "(II)V");
    if (lp_ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(lp_cls); return nullptr; }
    // width = WRAP_CONTENT (-2), height = MATCH_PARENT (-1).
    jobject lp = env->NewObject(lp_cls, lp_ctor,
                                static_cast<jint>(-2), static_cast<jint>(-1));
    if (env->ExceptionCheck() || lp == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(lp_cls);
        return nullptr;
    }
    jfieldID f_grav = env->GetFieldID(lp_cls, "gravity", "I");
    if (f_grav != nullptr) {
        env->SetIntField(lp, f_grav, static_cast<jint>(GRAVITY_START));
    }
    env->DeleteLocalRef(lp_cls);
    return lp;
}

void view_set_layout_params(JNIEnv* env, jobject child, jobject lp) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/View");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setLayoutParams",
                                   "(Landroid/view/ViewGroup$LayoutParams;)V");
    if (m != nullptr) {
        env->CallVoidMethod(child, m, lp);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void drawer_open(JNIEnv* env, jobject drawer, bool open) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("androidx/drawerlayout/widget/DrawerLayout");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    const char* name = open ? "openDrawer" : "closeDrawer";
    jmethodID m = env->GetMethodID(cls, name, "(I)V");
    if (m != nullptr) {
        env->CallVoidMethod(drawer, m, static_cast<jint>(GRAVITY_START));
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

} // namespace

flyout_view_handler<platform::android>::flyout_view_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    jobject context = detail::get_activity();

    // Path A — preferred: androidx.drawerlayout.widget.DrawerLayout.
    native_ = try_make_drawer_layout(env, context);
    if (native_ != nullptr) {
        is_drawer_ = true;
        return;
    }

    // Path B — fallback: hand-rolled horizontal LinearLayout host.
    is_drawer_ = false;
    native_ = make_linear_layout(env, context);
}

flyout_view_handler<platform::android>::~flyout_view_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (current_flyout_ != nullptr) { env->DeleteGlobalRef(current_flyout_); current_flyout_ = nullptr; }
    if (current_detail_ != nullptr) { env->DeleteGlobalRef(current_detail_); current_detail_ = nullptr; }
    if (native_         != nullptr) { env->DeleteGlobalRef(native_);         native_         = nullptr; }
}

void flyout_view_handler<platform::android>::apply_flyout(const std::shared_ptr<view>& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;

    // Detach the previous flyout child (if any) from the host and drop
    // our extra global ref.
    if (current_flyout_ != nullptr) {
        view_group_remove(env, native_, current_flyout_);
        env->DeleteGlobalRef(current_flyout_);
        current_flyout_ = nullptr;
    }

    // ADR-0013 registry first; if no widget is registered for the child
    // type, leave the flyout pane empty.
    jobject child = v ? detail::android_dispatch::dispatch(v.get()) : nullptr;
    if (child == nullptr) return;

    // Take our own global ref so we can openDrawer / set visibility on
    // it directly later, independent of the parent's bookkeeping.
    current_flyout_ = env->NewGlobalRef(child);

    if (is_drawer_) {
        // Apply START-gravity LayoutParams before attaching — DrawerLayout
        // needs them to recognise the child as a drawer.
        if (jobject lp = make_drawer_layout_params_for_flyout(env); lp != nullptr) {
            view_set_layout_params(env, current_flyout_, lp);
            env->DeleteLocalRef(lp);
        }
    }
    view_group_add(env, native_, current_flyout_);
}

void flyout_view_handler<platform::android>::apply_detail(const std::shared_ptr<view>& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;

    if (current_detail_ != nullptr) {
        view_group_remove(env, native_, current_detail_);
        env->DeleteGlobalRef(current_detail_);
        current_detail_ = nullptr;
    }

    jobject child = v ? detail::android_dispatch::dispatch(v.get()) : nullptr;
    if (child == nullptr) return;

    current_detail_ = env->NewGlobalRef(child);
    view_group_add(env, native_, current_detail_);
}

void flyout_view_handler<platform::android>::apply_is_presented(bool v) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (is_drawer_) {
        drawer_open(env, native_, v);
    } else if (current_flyout_ != nullptr) {
        view_set_visibility(env, current_flyout_, v);
    }
}

void flyout_view_handler<platform::android>::map_flyout(basic_flyout_view& f) {
    apply_flyout(f.flyout.get());
    f.flyout.changed.subscribe(flyout_slot_, flyout_cb_);
}

void flyout_view_handler<platform::android>::map_detail(basic_flyout_view& f) {
    apply_detail(f.detail.get());
    f.detail.changed.subscribe(detail_slot_, detail_cb_);
}

void flyout_view_handler<platform::android>::map_is_presented(basic_flyout_view& f) {
    apply_is_presented(f.is_presented.get());
    f.is_presented.changed.subscribe(is_presented_slot_, is_presented_cb_);
}

void flyout_view_handler<platform::android>::bind_flyout(basic_flyout_view& f, view& child) {
    f.flyout.set(std::shared_ptr<view>(&child, [](view*){}));
}

void flyout_view_handler<platform::android>::bind_detail(basic_flyout_view& f, view& child) {
    f.detail.set(std::shared_ptr<view>(&child, [](view*){}));
}

// ----- ADR-0013 self-registration --------------------------------------

namespace {

jobject dispatch_flyout_view(::mpapp::view* v) {
    if (auto* f = dynamic_cast<::mpapp::internal::basic_flyout_view*>(v); f && f->has_handler()) {
        return f->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() {
        ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_flyout_view);
    }
};

[[maybe_unused]] registrar _reg;

} // namespace

} // namespace mpapp::internal
#endif // __ANDROID__
