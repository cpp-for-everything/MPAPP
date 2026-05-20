// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Per-ADR-0013 data-driven widget dispatch — WinUI 3 impl.

#include "mpapp/handlers/windows/widget_dispatch.hpp"

#if defined(_WIN32)

#include <vector>

namespace mpapp::detail::windows_dispatch {

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

::winrt::Microsoft::UI::Xaml::UIElement dispatch(::mpapp::view* v) {
    if (v == nullptr) return nullptr;
    for (dispatcher_fn fn : registry()) {
        if (::winrt::Microsoft::UI::Xaml::UIElement native = fn(v); native != nullptr) {
            return native;
        }
    }
    return nullptr;
}

} // namespace mpapp::detail::windows_dispatch

#endif // _WIN32
