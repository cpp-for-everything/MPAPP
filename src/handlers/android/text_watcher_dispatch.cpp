// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Shared JNI trampoline for io.mpapp.MppTextWatcher.
// Routes to either entry_handler or editor_handler based on the
// `kind` discriminator passed by the Java side at watcher construction.

#if defined(__ANDROID__)

#include <jni.h>
#include <string>

#include "mpapp/handlers/android/editor_handler.hpp"
#include "mpapp/handlers/android/entry_cell_handler.hpp"
#include "mpapp/handlers/android/entry_handler.hpp"

extern "C" JNIEXPORT void JNICALL
Java_io_mpapp_MppTextWatcher_nativeDispatchTextChanged(
    JNIEnv* env, jclass /*cls*/, jlong handler_ptr, jint kind, jstring text) {
    if (handler_ptr == 0) return;
    const char* utf8 = (text != nullptr) ? env->GetStringUTFChars(text, nullptr) : nullptr;
    std::string s = (utf8 != nullptr) ? std::string{utf8} : std::string{};
    if (utf8 != nullptr) env->ReleaseStringUTFChars(text, utf8);

    switch (kind) {
        case 1:  // entry_handler
            mpapp::android_entry_dispatch_text_changed(
                reinterpret_cast<mpapp::entry_handler<mpapp::platform::android>*>(handler_ptr),
                s);
            return;
        case 2:  // editor_handler
            mpapp::android_editor_dispatch_text_changed(
                reinterpret_cast<mpapp::editor_handler<mpapp::platform::android>*>(handler_ptr),
                s);
            return;
        case 3:  // entry_cell_handler
            mpapp::android_entry_cell_dispatch_text_changed(
                reinterpret_cast<mpapp::entry_cell_handler<mpapp::platform::android>*>(handler_ptr),
                s);
            return;
        default:
            return;
    }
}

#endif // __ANDROID__
