// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/MenuBarItem.md
//
// `mpapp::menu_bar_item` — a single top-level entry inside a [[MenuBar]]
// ("File", "Edit", "View", …). Exposes a `title` label and a child
// collection that real handlers render as a drop-down menu when the
// entry is activated. The child collection is currently typed as
// `std::vector<view*>` (the M-04b cross-platform subset) so the menu_bar
// family parents can share the dispatch-by-view-pointer pattern used by
// stack_layout / scroll_view / border. Richer item types (separator,
// sub-item) land alongside the M-04c menu_flyout family.
//
// Real handlers map `title` → the platform native string slot and
// rebuild a child list whenever `items` changes. The rebuild is naive
// (clear + re-populate) for now; granular `MenuBarItemHandlerUpdate`
// patching mirrors MAUI's surface and lands with the binding-layer work.

#ifndef MPAPP_MENU_BAR_ITEM_HPP
#define MPAPP_MENU_BAR_ITEM_HPP

#include <string>
#include <vector>

#include "observable.hpp"
#include "platform.hpp"
#include "view.hpp"

namespace mpapp {

template <class Platform = platform::current>
class menu_bar_item_handler;

class menu_bar_item : public view {
public:
    menu_bar_item() = default;

    menu_bar_item(const menu_bar_item&)            = delete;
    menu_bar_item& operator=(const menu_bar_item&) = delete;
    menu_bar_item(menu_bar_item&&)                 = delete;
    menu_bar_item& operator=(menu_bar_item&&)      = delete;

    // Label rendered as the top-level entry text ("File", "Edit", …).
    Observable<std::string>          title{};

    // Child entries that render in the drop-down. Non-owning pointers —
    // ownership stays with the caller. Same shape used by `menu_bar`'s
    // top-level `items` collection. The M-04b subset accepts any view*;
    // the menu_flyout family lands a typed `menu_element` variant later.
    Observable<std::vector<view*>>   items{};

    menu_bar_item_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const menu_bar_item_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                            has_handler() const noexcept { return handler_ != nullptr; }
    void                                            set_handler(menu_bar_item_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    menu_bar_item_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_MENU_BAR_ITEM_HPP
