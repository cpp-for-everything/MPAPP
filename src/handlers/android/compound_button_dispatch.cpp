// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Single JNI trampoline for every CompoundButton-derived
// MPAPP handler. The Java `MppCheckedChangeListener` carries a `kind`
// discriminator so multiple handler types (switch_, check_box,
// future radio_button, …) can share the listener class.

#if defined(__ANDROID__)

#include <jni.h>

#include "mpapp/handlers/android/check_box_handler.hpp"
#include "mpapp/handlers/android/radio_button_handler.hpp"
#include "mpapp/handlers/android/switch_cell_handler.hpp"
#include "mpapp/handlers/android/switch_handler.hpp"

extern "C" JNIEXPORT void JNICALL
Java_io_mpapp_MppCheckedChangeListener_nativeDispatchCheckedChanged(
    JNIEnv* /*env*/, jclass /*cls*/, jlong handler_ptr, jint kind, jboolean checked) {
    if (handler_ptr == 0) return;
    const bool v = checked == JNI_TRUE;
    switch (kind) {
        case 1:  // switch_handler
            mpapp::android_switch_dispatch_checked_changed(
                reinterpret_cast<mpapp::switch_handler<mpapp::platform::android>*>(handler_ptr),
                v);
            return;
        case 2:  // check_box_handler
            mpapp::android_check_box_dispatch_checked_changed(
                reinterpret_cast<mpapp::check_box_handler<mpapp::platform::android>*>(handler_ptr),
                v);
            return;
        case 3:  // radio_button_handler
            mpapp::android_radio_button_dispatch_checked_changed(
                reinterpret_cast<mpapp::radio_button_handler<mpapp::platform::android>*>(handler_ptr),
                v);
            return;
        case 4:  // switch_cell_handler
            mpapp::android_switch_cell_dispatch_checked_changed(
                reinterpret_cast<mpapp::switch_cell_handler<mpapp::platform::android>*>(handler_ptr),
                v);
            return;
        default:
            return;
    }
}

#endif // __ANDROID__
