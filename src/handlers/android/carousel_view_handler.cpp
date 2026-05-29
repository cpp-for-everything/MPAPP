// SPDX-License-Identifier: Apache-2.0
// Android basic_carousel_view handler implementation (android.widget.ViewFlipper).

#include "mpapp/handlers/android/carousel_view_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/gesture_attach.hpp"

namespace mpapp::internal {

namespace {

jobject make_view_flipper(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/ViewFlipper");
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

jobject make_text_view(JNIEnv* env, jobject context, const std::string& text) {
    jclass cls = env->FindClass("android/widget/TextView");
    if (cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(Landroid/content/Context;)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(cls); return nullptr; }
    jobject tv = env->NewObject(cls, ctor, context);
    if (env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(cls); return nullptr; }
    if (jmethodID set = env->GetMethodID(cls, "setText", "(Ljava/lang/CharSequence;)V")) {
        jstring js = env->NewStringUTF(text.c_str());
        env->CallVoidMethod(tv, set, js);
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->DeleteLocalRef(js);
    }
    env->DeleteLocalRef(cls);
    return tv;  // local ref — consumed by addView immediately
}

void view_group_remove_all(JNIEnv* env, jobject group) {
    jclass cls = env->FindClass("android/view/ViewGroup");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    if (jmethodID m = env->GetMethodID(cls, "removeAllViews", "()V")) {
        env->CallVoidMethod(group, m);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void view_group_add(JNIEnv* env, jobject group, jobject child) {
    jclass cls = env->FindClass("android/view/ViewGroup");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    if (jmethodID m = env->GetMethodID(cls, "addView", "(Landroid/view/View;)V")) {
        env->CallVoidMethod(group, m, child);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

// setDisplayedChild(int) is declared on android.widget.ViewAnimator and
// inherited by ViewFlipper; GetMethodID resolves inherited methods, so we
// look it up on the ViewFlipper class directly.
void set_displayed_child(JNIEnv* env, jobject flipper, int idx) {
    jclass cls = env->FindClass("android/widget/ViewFlipper");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    if (jmethodID m = env->GetMethodID(cls, "setDisplayedChild", "(I)V")) {
        env->CallVoidMethod(flipper, m, static_cast<jint>(idx));
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

} // namespace

carousel_view_handler<platform::android>::carousel_view_handler() {
    JNIEnv* env = ::mpapp::detail::attach_current_thread();
    if (env != nullptr) {
        native_ = make_view_flipper(env, ::mpapp::detail::get_activity());
    }
}

carousel_view_handler<platform::android>::~carousel_view_handler() {
    if (native_ != nullptr) {
        if (JNIEnv* env = ::mpapp::detail::attach_current_thread(); env != nullptr) {
            env->DeleteGlobalRef(native_);
        }
        native_ = nullptr;
    }
}

void carousel_view_handler<platform::android>::rebuild_items(const std::vector<std::string>& v) {
    if (native_ == nullptr) return;
    JNIEnv* env = ::mpapp::detail::attach_current_thread();
    if (env == nullptr) return;
    view_group_remove_all(env, native_);
    jobject ctx = ::mpapp::detail::get_activity();
    for (const auto& s : v) {
        jobject tv = make_text_view(env, ctx, s);
        if (tv != nullptr) {
            view_group_add(env, native_, tv);
            env->DeleteLocalRef(tv);
        }
    }
    if (bound_ != nullptr) apply_position(bound_->position.get());
}

void carousel_view_handler<platform::android>::apply_position(int idx) {
    if (native_ == nullptr || idx < 0) return;
    JNIEnv* env = ::mpapp::detail::attach_current_thread();
    if (env == nullptr) return;
    set_displayed_child(env, native_, idx);
}

void carousel_view_handler<platform::android>::map_items_source(basic_carousel_view& c) {
    bound_ = &c;
    rebuild_items(c.items_source.get());
    c.items_source.changed.subscribe(items_slot_, items_cb_);
}

void carousel_view_handler<platform::android>::map_position(basic_carousel_view& c) {
    apply_position(c.position.get());
    c.position.changed.subscribe(pos_slot_, pos_cb_);
}

void carousel_view_handler<platform::android>::map_loop(basic_carousel_view& /*c*/) {
    // Loop/clamp handled in basic_carousel_view::scroll_to.
}

void carousel_view_handler<platform::android>::map_is_swipe_enabled(basic_carousel_view& /*c*/) {
    // ViewFlipper has no built-in swipe; fling-to-page is a follow-up
    // (GestureDetector + a router shim). v1 is programmatic + tap-driven.
}

void carousel_view_handler<platform::android>::map_peek_count(basic_carousel_view& /*c*/) {
    // ViewFlipper shows exactly one child — peek is a v1 no-op.
}

void carousel_view_handler<platform::android>::map_gestures(basic_carousel_view& c) {
    android_gestures::attach(native_, c);
}

} // namespace mpapp::internal

// ---------- Self-registration with the per-platform dispatch registry --
#include "mpapp/handlers/android/widget_dispatch.hpp"

namespace {

jobject dispatch_carousel_view(::mpapp::view* v) {
    if (auto* c = dynamic_cast<::mpapp::internal::basic_carousel_view*>(v);
        c && c->has_handler()) {
        return c->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_carousel_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
