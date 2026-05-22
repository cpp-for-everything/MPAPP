// SPDX-License-Identifier: Apache-2.0
// Native side of MppActionRouter (see
// examples/android_hello/app/src/main/java/io/mpapp/MppActionRouter.java).
// Switches on `kind` to dispatch the click to the right widget surface.

#if defined(__ANDROID__)

#include <jni.h>

#include "mpapp/navigation_page.hpp"
#include "mpapp/shell.hpp"
#include "mpapp/tabbed_page.hpp"

namespace mpapp::detail {

// Kind codes — keep in sync with MppActionRouter.java.
enum class action_kind : int {
    nav_back        = 0,   // ownerPtr = navigation_page*, payload unused
    shell_tab       = 1,   // ownerPtr = shell*,           payload = tab_index
    tabbed_page_tab = 2,   // ownerPtr = tabbed_page*,     payload = tab_index
};

void dispatch_android_action(jlong owner_ptr, jint kind, jint payload) {
    switch (static_cast<action_kind>(kind)) {
        case action_kind::nav_back: {
            auto* np = reinterpret_cast<navigation_page*>(owner_ptr);
            if (np != nullptr && np->stack().depth() > 1) np->pop();
            break;
        }
        case action_kind::shell_tab: {
            auto* s = reinterpret_cast<shell*>(owner_ptr);
            if (s != nullptr) s->current_tab_index.set(static_cast<int>(payload));
            break;
        }
        case action_kind::tabbed_page_tab: {
            auto* tp = reinterpret_cast<tabbed_page*>(owner_ptr);
            if (tp != nullptr) tp->selected_index.set(static_cast<int>(payload));
            break;
        }
    }
}

} // namespace mpapp::detail

extern "C" JNIEXPORT void JNICALL
Java_io_mpapp_MppActionRouter_nativeDispatchAction(
    JNIEnv* /*env*/,
    jclass  /*cls*/,
    jlong   owner_ptr,
    jint    kind,
    jint    payload) {
    mpapp::detail::dispatch_android_action(owner_ptr, kind, payload);
}

#endif // __ANDROID__
