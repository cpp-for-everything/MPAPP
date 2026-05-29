// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 real main-thread dispatcher.
//
// `mpapp::dispatcher` backed by the UI-thread DispatcherQueue: post() →
// DispatcherQueue.TryEnqueue, post_after() → a one-shot DispatcherQueueTimer.
// Installing it makes async_sleep / ui_task continuations / animation frame
// ticks run on the real WinUI message loop instead of the deterministic
// test_dispatcher. The Windows application handler installs it in OnLaunched
// (on the UI thread, where DispatcherQueue::GetForCurrentThread() is valid).
//
// Lives in the windows handler lib (which links the Windows App SDK) so
// mpapp-core stays platform-neutral (T-0032).

#ifndef MPAPP_HANDLERS_WINDOWS_DISPATCHER_QUEUE_HPP
#define MPAPP_HANDLERS_WINDOWS_DISPATCHER_QUEUE_HPP

#include "../../platform.hpp"

#if defined(_WIN32)

namespace mpapp::detail {

// Install a process-wide DispatcherQueue-backed dispatcher as
// mpapp::main_dispatcher(). MUST be called on the UI thread (it captures
// DispatcherQueue::GetForCurrentThread()). Idempotent.
void install_dispatcher_queue_main_dispatcher();

} // namespace mpapp::detail

#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_DISPATCHER_QUEUE_HPP
