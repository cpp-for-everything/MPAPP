// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Per-ADR-0013 data-driven widget dispatch — WinUI 3.
//
// Returns the WinUI common base `winrt::Microsoft::UI::Xaml::UIElement`.
// All concrete control types (Button, Image, Border, ContentControl,
// StackPanel, …) derive from UIElement; container "Content" / "Children"
// setters accept it implicitly.

#ifndef MPAPP_HANDLERS_WINDOWS_WIDGET_DISPATCH_HPP
#define MPAPP_HANDLERS_WINDOWS_WIDGET_DISPATCH_HPP

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.h>

namespace mpapp { class view; }

namespace mpapp::detail::windows_dispatch {

using dispatcher_fn = ::winrt::Microsoft::UI::Xaml::UIElement (*)(::mpapp::view*);

void register_dispatcher(dispatcher_fn fn);

::winrt::Microsoft::UI::Xaml::UIElement dispatch(::mpapp::view* v);

} // namespace mpapp::detail::windows_dispatch

#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_WIDGET_DISPATCH_HPP
