// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 `basic_menu_flyout` handler — wraps
// `mux::Controls::MenuFlyout`. Items are appended into the flyout's
// `Items` collection by resolving each child `view*` through the
// ADR-0013 dispatch registry; basic_menu_flyout_item / basic_menu_flyout_separator
// / basic_menu_flyout_sub_item each contribute the appropriate child
// MenuFlyoutItemBase subclass. `is_open` is reflected by calling
// `ShowAt`/`Hide`; the flyout's `Closed` event flips the Observable
// back when the user dismisses it.

#ifndef MPAPP_HANDLERS_WINDOWS_MENU_FLYOUT_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_MENU_FLYOUT_HANDLER_HPP

#include <vector>

#include "../../internal/basic_menu_flyout.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../view.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>

namespace mpapp::internal {

template <>
class menu_flyout_handler<platform::windows> {
public:
    menu_flyout_handler();
    ~menu_flyout_handler();
    menu_flyout_handler(const menu_flyout_handler&)            = delete;
    menu_flyout_handler& operator=(const menu_flyout_handler&) = delete;
    menu_flyout_handler(menu_flyout_handler&&)                 = delete;
    menu_flyout_handler& operator=(menu_flyout_handler&&)      = delete;

    void map_items(basic_menu_flyout& f);
    void map_is_open(basic_menu_flyout& f);

    winrt::Microsoft::UI::Xaml::Controls::MenuFlyout&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::MenuFlyout& native() const noexcept { return native_; }

private:
    void apply_items(const std::vector<view*>& v);
    void apply_is_open(bool v);

    struct items_cb_t   { menu_flyout_handler<platform::windows>* self; void operator()(const std::vector<view*>& v) const { self->apply_items(v); } };
    struct is_open_cb_t { menu_flyout_handler<platform::windows>* self; void operator()(bool v)                      const { self->apply_is_open(v); } };

    winrt::Microsoft::UI::Xaml::Controls::MenuFlyout native_{nullptr};

    items_cb_t                                items_cb_{this};
    is_open_cb_t                              is_open_cb_{this};
    signal_slot<std::vector<view*> const&>    items_slot_{};
    signal_slot<const bool&>                  is_open_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_MENU_FLYOUT_HANDLER_HPP
