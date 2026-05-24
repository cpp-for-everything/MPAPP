// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/MenuFlyoutItem.md
//
// `mpapp::menu_flyout_item` — invokable leaf inside a menu_flyout or
// menu_flyout_sub_item. Carries a label, an enabled flag, and a
// `clicked` signal fired when the platform invokes the item. The
// richer MAUI surface (icon / Command / CommandParameter / keyboard
// accelerators) lands alongside the M-05 input plumbing.
//
// The class derives from `view` so it can be dispatched through the
// ADR-0013 per-platform registry. The handler maps to the native menu
// item type on each platform (MenuFlyoutItem on Windows, MenuItem on
// Android, GtkButton inside a popover on Linux).

#ifndef MPAPP_MENU_FLYOUT_ITEM_HPP
#define MPAPP_MENU_FLYOUT_ITEM_HPP

#include <string>

#include "observable.hpp"
#include "platform.hpp"
#include "signal.hpp"
#include "view.hpp"

namespace mpapp {

template <class Platform = platform::current>
class menu_flyout_item_handler;

class menu_flyout_item : public view {
public:
    menu_flyout_item()                                          = default;
    ~menu_flyout_item() override                                = default;
    menu_flyout_item(const menu_flyout_item&)                   = delete;
    menu_flyout_item& operator=(const menu_flyout_item&)        = delete;
    menu_flyout_item(menu_flyout_item&&)                        = delete;
    menu_flyout_item& operator=(menu_flyout_item&&)             = delete;

    // ----- Properties ----------------------------------------------------
    Observable<std::string>  text{""};
    Observable<bool>         is_enabled{true};

    // ----- Events --------------------------------------------------------
    // Fired by the platform handler when the user activates the item.
    // Tests / the M-05 command plumbing subscribe to this signal.
    mutable mpapp::signal<>  clicked;

    // ----- Handler -------------------------------------------------------
    menu_flyout_item_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const menu_flyout_item_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                               has_handler() const noexcept { return handler_ != nullptr; }
    void                                               set_handler(menu_flyout_item_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    menu_flyout_item_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_MENU_FLYOUT_ITEM_HPP
