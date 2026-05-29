// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android implementation of RFC-0003 gesture recognizers.
//
// Tap: make the View clickable + install an io.mpapp.MppGestureRouter
// (a View.OnClickListener) whose onClick JNI-dispatches back to the
// recognizer's `tapped` signal — same shim pattern as the button handler's
// MppClickRouter. The Java class ships with the app's gradle project
// (examples/android_hello/.../io/mpapp/MppGestureRouter.java); it's
// resolved via FindClass at runtime, so this TU cross-compiles without it.

#include "mpapp/handlers/android/gesture_attach.hpp"

#if defined(__ANDROID__)

#include "mpapp/gestures/tap_gesture_recognizer.hpp"
#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/view.hpp"

namespace mpapp::internal::android_gestures {

namespace {

void install_tap(JNIEnv* env, jobject view_obj, tap_gesture_recognizer& tap) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass view_cls = env->FindClass("android/view/View");
    if (view_cls == nullptr) { env->ExceptionClear(); return; }

    if (jmethodID set_clickable = env->GetMethodID(view_cls, "setClickable", "(Z)V")) {
        env->CallVoidMethod(view_obj, set_clickable, static_cast<jboolean>(JNI_TRUE));
        if (env->ExceptionCheck()) env->ExceptionClear();
    }

    jclass router_cls = env->FindClass("io/mpapp/MppGestureRouter");
    if (router_cls == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(view_cls); return; }
    jmethodID ctor = env->GetMethodID(router_cls, "<init>", "(J)V");
    if (ctor == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(router_cls);
        env->DeleteLocalRef(view_cls);
        return;
    }
    jobject router = env->NewObject(router_cls, ctor, reinterpret_cast<jlong>(&tap));
    if (router != nullptr) {
        if (jmethodID set_listener = env->GetMethodID(
                view_cls, "setOnClickListener",
                "(Landroid/view/View$OnClickListener;)V")) {
            env->CallVoidMethod(view_obj, set_listener, router);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        env->DeleteLocalRef(router);
    }
    env->DeleteLocalRef(router_cls);
    env->DeleteLocalRef(view_cls);
}

} // namespace

void attach(jobject view_obj, view& v) {
    if (view_obj == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    for (const auto& r : v.gesture_recognizers) {
        if (r->kind() == gesture_kind::tap) {
            install_tap(env, view_obj, static_cast<tap_gesture_recognizer&>(*r));
        }
        // pan / pinch / swipe / pointer: follow-up (OnTouchListener +
        // ScaleGestureDetector + GestureDetector).
    }
}

} // namespace mpapp::internal::android_gestures

// JNI trampoline for the Java MppGestureRouter's onClick.
extern "C" JNIEXPORT void JNICALL
Java_io_mpapp_MppGestureRouter_nativeDispatchTap(JNIEnv* /*env*/, jclass /*cls*/,
                                                 jlong recognizer_ptr) {
    if (auto* tap = reinterpret_cast<mpapp::tap_gesture_recognizer*>(recognizer_ptr)) {
        tap->tapped.emit(mpapp::tapped_event_args{0.0, 0.0, mpapp::button_mask::primary});
    }
}

#endif // __ANDROID__
