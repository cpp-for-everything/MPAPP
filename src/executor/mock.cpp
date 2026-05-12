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
// CMake selects exactly one platform-native source for each target; this
// `mock.cpp` is linked unconditionally for now because no platform-native
// source exists yet.

#include <mpapp/executor.hpp>
#include <mpapp/test_dispatcher.hpp>

namespace mpapp {

namespace {

test_dispatcher& singleton_main_dispatcher() noexcept {
    static test_dispatcher inst;
    return inst;
}

} // namespace

dispatcher& main_dispatcher() noexcept {
    return singleton_main_dispatcher();
}

namespace detail {

void post_after_on_main(std::chrono::steady_clock::duration d,
                        std::function<void()>               work) {
    singleton_main_dispatcher().post_after(d, std::move(work));
}

} // namespace detail

} // namespace mpapp
