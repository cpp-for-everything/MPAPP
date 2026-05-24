// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_menu_bar handler — wraps `mux::Controls::MenuBar`.
//
// On `apply_items()` we clear the bar's `Items` collection and re-append
// each child resolved via the ADR-0013 dispatch registry. A child
// basic_menu_bar_item resolves to a `mux::Controls::MenuBarItem`; any other
// view* type is skipped (the registry returns nullptr and we drop it).
// The full granular `MenuBarHandlerUpdate(Add/Insert/Remove)` surface
// MAUI exposes is out of scope for M-04b; clear+repopulate is the
// baseline.

#ifndef MPAPP_HANDLERS_WINDOWS_MENU_BAR_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_MENU_BAR_HANDLER_HPP

#include <vector>

#include "../../internal/basic_menu_bar.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../view.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp::internal {

template <>
class menu_bar_handler<platform::windows> {
public:
    menu_bar_handler();
    ~menu_bar_handler();
    menu_bar_handler(const menu_bar_handler&)            = delete;
    menu_bar_handler& operator=(const menu_bar_handler&) = delete;

    void map_items(basic_menu_bar& b);

    winrt::Microsoft::UI::Xaml::Controls::MenuBar&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::MenuBar& native() const noexcept { return native_; }

private:
    void apply_items(const std::vector<view*>& v);

    struct items_cb_t {
        menu_bar_handler<platform::windows>* self;
        void operator()(const std::vector<view*>& v) const { self->apply_items(v); }
    };

    winrt::Microsoft::UI::Xaml::Controls::MenuBar native_{nullptr};

    items_cb_t                              items_cb_{this};
    signal_slot<std::vector<view*> const&>  items_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_MENU_BAR_HANDLER_HPP
