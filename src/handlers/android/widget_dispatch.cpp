// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Per-ADR-0013 data-driven widget dispatch — Android impl.

#include "mpapp/handlers/android/widget_dispatch.hpp"

#if defined(__ANDROID__)

#include <vector>

namespace mpapp::detail::android_dispatch {

namespace {
// Local-static container — guaranteed initialised before first use.
// Registrars are global object constructors; they run before main()
// and `dispatch()` only runs at UI lifecycle time, well after init.
std::vector<dispatcher_fn>& registry() {
    static std::vector<dispatcher_fn> r;
    return r;
}
} // namespace

void register_dispatcher(dispatcher_fn fn) {
    if (fn != nullptr) {
        registry().push_back(fn);
    }
}

jobject dispatch(::mpapp::view* v) {
    if (v == nullptr) return nullptr;
    for (dispatcher_fn fn : registry()) {
        if (jobject native = fn(v); native != nullptr) {
            return native;
        }
    }
    return nullptr;
}

} // namespace mpapp::detail::android_dispatch

#endif // __ANDROID__
