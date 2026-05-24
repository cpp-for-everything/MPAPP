// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/MenuFlyoutSeparator.md
//
// `mpapp::menu_flyout_separator` — pure visual divider between groups
// of menu_flyout_item entries. Carries no observable properties — the
// type itself is the marker. Each platform handler translates it into
// the native separator widget (MenuFlyoutSeparator on Windows, a
// GtkSeparator on Linux, an empty divider MenuItem on Android).

#ifndef MPAPP_INTERNAL_BASIC_MENU_FLYOUT_SEPARATOR_HPP
#define MPAPP_INTERNAL_BASIC_MENU_FLYOUT_SEPARATOR_HPP

#include "../platform.hpp"
#include "../view.hpp"

namespace mpapp::internal {

template <class Platform = platform::current>
class menu_flyout_separator_handler;

class basic_menu_flyout_separator : public view {
public:
    basic_menu_flyout_separator()                                                 = default;
    ~basic_menu_flyout_separator() override                                       = default;
    basic_menu_flyout_separator(const basic_menu_flyout_separator&)                     = delete;
    basic_menu_flyout_separator& operator=(const basic_menu_flyout_separator&)          = delete;
    basic_menu_flyout_separator(basic_menu_flyout_separator&&)                          = delete;
    basic_menu_flyout_separator& operator=(basic_menu_flyout_separator&&)               = delete;

    // No observable properties — the marker type drives the handler.

    // ----- Handler -------------------------------------------------------
    menu_flyout_separator_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const menu_flyout_separator_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                                    has_handler() const noexcept { return handler_ != nullptr; }
    void                                                    set_handler(menu_flyout_separator_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    menu_flyout_separator_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp::internal


#endif // MPAPP_INTERNAL_BASIC_MENU_FLYOUT_SEPARATOR_HPP
