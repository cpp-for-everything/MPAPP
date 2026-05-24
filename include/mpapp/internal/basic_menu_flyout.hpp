// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/MenuFlyout.md
//
// `mpapp::menu_flyout` — popup / context menu host. A flyout owns a flat
// list of menu elements (items / separators / sub-items) and an
// `is_open` toggle. The richer MAUI surface (anchor placement,
// `show(point)`/`hide()` commands, KeyboardAccelerators) lands
// incrementally alongside the M-05 input plumbing; the M-04b real
// surface is the items + visibility pair every host platform can
// implement uniformly.
//
// `items` is held as `Observable<std::vector<view*>>` — the menu
// flyout children (menu_flyout_item / menu_flyout_separator /
// menu_flyout_sub_item) are themselves `view` subclasses, dispatched
// to native widgets through the ADR-0013 per-platform registry.
//
// Lifetime: the `items` vector stores non-owning pointers; the user
// owns each element's lifetime. The handlers rebuild the native menu
// whenever the items vector mutates (same shape as `toolbar::items`).

#ifndef MPAPP_INTERNAL_BASIC_MENU_FLYOUT_HPP
#define MPAPP_INTERNAL_BASIC_MENU_FLYOUT_HPP

#include <vector>

#include "../observable.hpp"
#include "../platform.hpp"
#include "../view.hpp"

namespace mpapp::internal {

template <class Platform = platform::current>
class menu_flyout_handler;

class basic_menu_flyout : public view {
public:
    basic_menu_flyout()                                = default;
    ~basic_menu_flyout() override                      = default;
    basic_menu_flyout(const basic_menu_flyout&)              = delete;
    basic_menu_flyout& operator=(const basic_menu_flyout&)   = delete;
    basic_menu_flyout(basic_menu_flyout&&)                   = delete;
    basic_menu_flyout& operator=(basic_menu_flyout&&)        = delete;

    // ----- Properties ----------------------------------------------------
    // Flat list of child menu elements (menu_flyout_item /
    // menu_flyout_separator / menu_flyout_sub_item). Non-owning.
    Observable<std::vector<view*>>  items{};

    // True when the flyout is currently shown.
    Observable<bool>                is_open{false};

    // ----- Handler -------------------------------------------------------
    menu_flyout_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const menu_flyout_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                          has_handler() const noexcept { return handler_ != nullptr; }
    void                                          set_handler(menu_flyout_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    menu_flyout_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp::internal


#endif // MPAPP_INTERNAL_BASIC_MENU_FLYOUT_HPP
