// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4/GLib real main-thread dispatcher.
//
// `mpapp::dispatcher` backed by the GLib main loop: post() → g_idle_add_full,
// post_after() → g_timeout_add_full, both on the default main context (the
// one g_application_run pumps). Installing it makes async_sleep / ui_task
// continuations / animation frame ticks run on the real GTK event loop
// instead of the deterministic test_dispatcher. The Linux application handler
// installs it in its "activate" handler (on the UI thread).
//
// Lives in the linux handler lib (which links GTK/GLib) so mpapp-core stays
// platform-neutral (T-0032).

#ifndef MPAPP_HANDLERS_LINUX_GLIB_DISPATCHER_HPP
#define MPAPP_HANDLERS_LINUX_GLIB_DISPATCHER_HPP

#include "../../platform.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::detail {

// Install a process-wide GLib-backed dispatcher as mpapp::main_dispatcher().
// Idempotent — repeated calls reinstall the same singleton. Call once on the
// UI thread at app startup.
void install_glib_main_dispatcher();

} // namespace mpapp::detail

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_GLIB_DISPATCHER_HPP
