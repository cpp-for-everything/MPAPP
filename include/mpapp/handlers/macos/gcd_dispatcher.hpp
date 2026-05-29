// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. AppKit real main-thread dispatcher (blind write — compiled +
// run on a Mac: PENDING).
//
// `mpapp::dispatcher` backed by Grand Central Dispatch's main queue: post() →
// dispatch_async(dispatch_get_main_queue(), ...), post_after() →
// dispatch_after(...). Installing it routes async_sleep / ui_task
// continuations / animation ticks onto the real CFRunLoop the AppKit app
// pumps. The macOS application handler installs it in
// applicationDidFinishLaunching (on the main thread). Lives in the macos
// handler lib so mpapp-core stays platform-neutral (T-0032).

#ifndef MPAPP_HANDLERS_MACOS_GCD_DISPATCHER_HPP
#define MPAPP_HANDLERS_MACOS_GCD_DISPATCHER_HPP

#include "../../platform.hpp"

#if defined(__APPLE__)
#  include <TargetConditionals.h>
#  if !TARGET_OS_IPHONE

namespace mpapp::detail {

// Install a process-wide GCD-main-queue dispatcher as mpapp::main_dispatcher().
// Call once on the main thread at app startup. Idempotent.
void install_macos_main_dispatcher();

} // namespace mpapp::detail

#  endif // !TARGET_OS_IPHONE
#endif // __APPLE__
#endif // MPAPP_HANDLERS_MACOS_GCD_DISPATCHER_HPP
