// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/MenuFlyoutSeparator.md
//
// `mpapp::menu_flyout_separator` — pure visual divider between groups
// of menu_flyout_item entries. Carries no observable properties — the
// type itself is the marker. Each platform handler translates it into
// the native separator widget (MenuFlyoutSeparator on Windows, a
// GtkSeparator on Linux, an empty divider MenuItem on Android).

#ifndef MPAPP_MENU_FLYOUT_SEPARATOR_HPP
#define MPAPP_MENU_FLYOUT_SEPARATOR_HPP

#include "platform.hpp"
#include "view.hpp"

namespace mpapp {

template <class Platform>
class menu_flyout_separator_handler;

class menu_flyout_separator : public view {
public:
    menu_flyout_separator()                                                 = default;
    ~menu_flyout_separator() override                                       = default;
    menu_flyout_separator(const menu_flyout_separator&)                     = delete;
    menu_flyout_separator& operator=(const menu_flyout_separator&)          = delete;
    menu_flyout_separator(menu_flyout_separator&&)                          = delete;
    menu_flyout_separator& operator=(menu_flyout_separator&&)               = delete;

    // No observable properties — the marker type drives the handler.

    // ----- Handler -------------------------------------------------------
    menu_flyout_separator_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const menu_flyout_separator_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                                    has_handler() const noexcept { return handler_ != nullptr; }
    void                                                    set_handler(menu_flyout_separator_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    menu_flyout_separator_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_MENU_FLYOUT_SEPARATOR_HPP
