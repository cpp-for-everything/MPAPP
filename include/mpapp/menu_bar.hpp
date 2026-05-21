// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/MenuBar.md
//
// `mpapp::menu_bar` — top-level, horizontal application menu attached to
// a window or page. A flat collection of [[MenuBarItem]] children (File,
// Edit, View, …); on platforms with a native window-chrome menu (Windows
// + Linux desktops) the bar maps onto the native menu surface, and on
// mobile platforms (Android) it collapses into an overflow / toolbar
// affordance.
//
// `items` carries the children as non-owning `view*` pointers — same
// shape used by stack_layout / scroll_view children and the M-04b
// dispatch-by-view-pointer pattern. The richer `observable_list` +
// granular `MenuBarHandlerUpdate(index, item)` surface MAUI exposes
// lands with the binding-layer work; a naive clear+repopulate rebuild
// is the M-04b baseline.
//
// `is_enabled` is inherited from `view`. The MenuBar component doc lists
// it as the only `BindableProperty` MAUI exposes on top of the children
// collection, so no extra Observable is needed at this layer.

#ifndef MPAPP_MENU_BAR_HPP
#define MPAPP_MENU_BAR_HPP

#include <vector>

#include "observable.hpp"
#include "platform.hpp"
#include "view.hpp"

namespace mpapp {

template <class Platform>
class menu_bar_handler;

class menu_bar : public view {
public:
    menu_bar() = default;

    menu_bar(const menu_bar&)            = delete;
    menu_bar& operator=(const menu_bar&) = delete;
    menu_bar(menu_bar&&)                 = delete;
    menu_bar& operator=(menu_bar&&)      = delete;

    // Top-level entries. Each pointer is conventionally a
    // `mpapp::menu_bar_item*` (the dispatch registry resolves the type),
    // but any view* is accepted so future menu element variants can
    // slot in without changing the public API.
    Observable<std::vector<view*>>  items{};

    menu_bar_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const menu_bar_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                       has_handler() const noexcept { return handler_ != nullptr; }
    void                                       set_handler(menu_bar_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    menu_bar_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_MENU_BAR_HPP
