// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_menu_flyout_separator handler implementation.

#include "mpapp/handlers/windows/menu_flyout_separator_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

#include "mpapp/internal/basic_menu_flyout_separator.hpp"

namespace mpapp::internal {

namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

menu_flyout_separator_handler<platform::windows>::menu_flyout_separator_handler() {
    native_ = muxc::MenuFlyoutSeparator{};
}

menu_flyout_separator_handler<platform::windows>::~menu_flyout_separator_handler() = default;

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --

namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_menu_flyout_separator(::mpapp::view* v) {
    if (auto* s = dynamic_cast<::mpapp::internal::basic_menu_flyout_separator*>(v); s && s->has_handler()) {
        return s->handler().native();
    }
    return nullptr;
}

struct registrar_mfs {
    registrar_mfs() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_menu_flyout_separator); }
};

[[maybe_unused]] registrar_mfs _reg;

} // namespace

#endif // _WIN32
