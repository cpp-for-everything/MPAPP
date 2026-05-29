// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Async Executor and Event Loops.md
//
// Mock (deterministic) backing implementation for `mpapp::main_dispatcher()`
// and timer scheduling. This file provides the cross-platform skeleton that
// lets the rest of the framework (handlers, hot-reload, tests) talk to the
// async surface without depending on a real platform event loop.
//
// Real platform-native implementations land alongside this file:
//
//   src/executor/windows_iocp.cpp     // TODO P4+: IOCP-backed main_dispatcher
//   src/executor/linux_iouring.cpp    // TODO P4+: io_uring (kernel >= 5.6)
//   src/executor/linux_epoll.cpp      // TODO P4+: epoll fallback
//   src/executor/apple_kqueue.cpp     // TODO P4+: kqueue + CFRunLoop bridge
//   src/executor/android_alooper.cpp  // TODO P4+: epoll + ALooper bridge
//
// This file is the dispatcher *registry*: it stays in `mpapp-core`
// unconditionally and owns `main_dispatcher()` + the install hook. By
// default `main_dispatcher()` returns a deterministic `test_dispatcher`
// (virtual time, `advance()`), which is what the test build relies on. Apps
// call `install_main_dispatcher()` at startup with a real per-platform
// dispatcher — those live in the per-platform handler libs (which link the
// native toolkit) so `mpapp-core` stays platform-neutral (T-0032):
//
//   src/handlers/linux/glib_dispatcher.cpp      // g_idle_add / g_timeout_add
//   src/handlers/windows/dispatcher_queue.cpp   // DispatcherQueue(+Timer)
//   src/handlers/android/looper_dispatcher.cpp  // Handler on the main Looper

#include <mpapp/executor.hpp>
#include <mpapp/test_dispatcher.hpp>

#include <atomic>

namespace mpapp {

namespace {

test_dispatcher& fallback_dispatcher() noexcept {
    static test_dispatcher inst;
    return inst;
}

std::atomic<dispatcher*>& installed_dispatcher() noexcept {
    static std::atomic<dispatcher*> inst{nullptr};
    return inst;
}

} // namespace

dispatcher& main_dispatcher() noexcept {
    if (dispatcher* d = installed_dispatcher().load(std::memory_order_acquire)) {
        return *d;
    }
    return fallback_dispatcher();
}

void install_main_dispatcher(dispatcher* d) noexcept {
    installed_dispatcher().store(d, std::memory_order_release);
}

namespace detail {

void post_after_on_main(std::chrono::steady_clock::duration d,
                        std::function<void()>               work) {
    main_dispatcher().post_after(d, std::move(work));
}

} // namespace detail

} // namespace mpapp
