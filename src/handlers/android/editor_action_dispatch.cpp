// SPDX-License-Identifier: Apache-2.0
// Shared JNI trampoline for io.mpapp.MppEditorActionListener.
// Routes IME-action events to the appropriate native handler based on
// the `kind` discriminator passed at listener construction.

#if defined(__ANDROID__)

#include <jni.h>

#include "mpapp/handlers/android/entry_cell_handler.hpp"

extern "C" JNIEXPORT void JNICALL
Java_io_mpapp_MppEditorActionListener_nativeDispatchEditorAction(
    JNIEnv* /*env*/, jclass /*cls*/, jlong owner_ptr, jint kind, jint action_id) {
    if (owner_ptr == 0) return;
    switch (kind) {
        case 1:  // entry_cell_handler
            mpapp::internal::android_entry_cell_dispatch_editor_action(
                reinterpret_cast<mpapp::internal::entry_cell_handler<mpapp::platform::android>*>(owner_ptr),
                static_cast<int>(action_id));
            return;
        default:
            return;
    }
}

#endif // __ANDROID__
