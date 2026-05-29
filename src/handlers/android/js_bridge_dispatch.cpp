// SPDX-License-Identifier: Apache-2.0
// JNI trampoline for io.mpapp.MppJsBridge — routes JavaScriptInterface
// calls from window.mpapp.send(payload) into the native
// hybrid_web_view_handler.

#if defined(__ANDROID__)

#include <jni.h>
#include <string>

#include "mpapp/handlers/android/hybrid_web_view_handler.hpp"

extern "C" JNIEXPORT void JNICALL
Java_io_mpapp_MppJsBridge_nativeDispatchInbound(
    JNIEnv* env, jclass /*cls*/, jlong handler_ptr, jstring payload) {
    if (handler_ptr == 0) return;
    const char* utf8 = (payload != nullptr) ? env->GetStringUTFChars(payload, nullptr) : nullptr;
    std::string s = (utf8 != nullptr) ? std::string{utf8} : std::string{};
    if (utf8 != nullptr) env->ReleaseStringUTFChars(payload, utf8);
    mpapp::internal::android_hybrid_web_view_dispatch_inbound(
        reinterpret_cast<mpapp::internal::hybrid_web_view_handler<mpapp::platform::android>*>(handler_ptr),
        s);
}

#endif // __ANDROID__
