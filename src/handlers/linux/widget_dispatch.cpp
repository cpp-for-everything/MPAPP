// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Per-ADR-0013 data-driven widget dispatch — GTK4 impl.

#include "mpapp/handlers/linux/widget_dispatch.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <vector>

namespace mpapp::detail::linux_dispatch {

namespace {
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

GtkWidget* dispatch(::mpapp::view* v) {
    if (v == nullptr) return nullptr;
    for (dispatcher_fn fn : registry()) {
        if (GtkWidget* native = fn(v); native != nullptr) {
            return native;
        }
    }
    return nullptr;
}

} // namespace mpapp::detail::linux_dispatch

#endif // __linux__ && !__ANDROID__
