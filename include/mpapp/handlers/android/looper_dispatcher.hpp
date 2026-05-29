// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android real main-thread dispatcher.
//
// `mpapp::dispatcher` backed by an android.os.Handler bound to the main
// Looper: post() → Handler.post(Runnable), post_after() →
// Handler.postDelayed(Runnable, ms). The Runnable is the Java shim
// io.mpapp.MppDispatchRunnable, which JNI-dispatches back to the heap-owned
// std::function (same router-shim pattern as MppClickRouter). Installing it
// makes async_sleep / ui_task continuations / animation ticks run on the
// real Android main Looper instead of the deterministic test_dispatcher.
//
// The Android application handler installs it in run_app<App>() (on the Java
// main thread, where Looper.getMainLooper() is valid). Lives in the android
// handler lib so mpapp-core stays platform-neutral (T-0032).

#ifndef MPAPP_HANDLERS_ANDROID_LOOPER_DISPATCHER_HPP
#define MPAPP_HANDLERS_ANDROID_LOOPER_DISPATCHER_HPP

#include "../../platform.hpp"

#if defined(__ANDROID__)

namespace mpapp::detail {

// Install a process-wide Handler(mainLooper)-backed dispatcher as
// mpapp::main_dispatcher(). MUST be called on the Java main thread. Idempotent
// — if the JavaVM/main looper can't be reached it leaves the default in place.
void install_android_main_dispatcher();

} // namespace mpapp::detail

#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_LOOPER_DISPATCHER_HPP
