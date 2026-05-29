// SPDX-License-Identifier: Apache-2.0
// JNI trampoline for io.mpapp.MppWebViewClient — routes
// onPageStarted / onPageFinished into the native web_view_handler.

#if defined(__ANDROID__)

#include <jni.h>
#include <string>

#include "mpapp/handlers/android/web_view_handler.hpp"

extern "C" JNIEXPORT void JNICALL
Java_io_mpapp_MppWebViewClient_nativeDispatchPageStarted(
    JNIEnv* env, jclass /*cls*/, jlong handler_ptr, jstring url) {
    if (handler_ptr == 0) return;
    const char* utf8 = (url != nullptr) ? env->GetStringUTFChars(url, nullptr) : nullptr;
    std::string s = (utf8 != nullptr) ? std::string{utf8} : std::string{};
    if (utf8 != nullptr) env->ReleaseStringUTFChars(url, utf8);
    mpapp::internal::android_web_view_dispatch_page_started(
        reinterpret_cast<mpapp::internal::web_view_handler<mpapp::platform::android>*>(handler_ptr),
        s);
}

extern "C" JNIEXPORT void JNICALL
Java_io_mpapp_MppWebViewClient_nativeDispatchPageFinished(
    JNIEnv* env, jclass /*cls*/, jlong handler_ptr, jstring url, jboolean success) {
    if (handler_ptr == 0) return;
    const char* utf8 = (url != nullptr) ? env->GetStringUTFChars(url, nullptr) : nullptr;
    std::string s = (utf8 != nullptr) ? std::string{utf8} : std::string{};
    if (utf8 != nullptr) env->ReleaseStringUTFChars(url, utf8);
    mpapp::internal::android_web_view_dispatch_page_finished(
        reinterpret_cast<mpapp::internal::web_view_handler<mpapp::platform::android>*>(handler_ptr),
        s,
        success == JNI_TRUE);
}

#endif // __ANDROID__
