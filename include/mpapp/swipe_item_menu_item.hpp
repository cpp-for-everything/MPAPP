// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/SwipeItemMenuItem.md
//
// `mpapp::swipe_item_menu_item` — the standard icon+text action pill
// used inside a `swipe_view`'s `left_items` / `right_items` collection.
// "Delete", "Archive", "Favourite" — the typical inline-row affordances
// when you swipe a list item.
//
// Mock surface (M-04b): `text` + `icon_uri` plus an `invoked` signal
// (the worker prompt marks `invoked` as optional but it costs nothing
// to wire and unblocks `mpapp::xc` from emitting `Click="…"` -> invoke
// lowering for swipe items). The richer `background` paint /
// `is_destructive` / `command` / `command_parameter` / `visibility`
// surfaces described in the component doc land in a follow-up batch.

#ifndef MPAPP_SWIPE_ITEM_MENU_ITEM_HPP
#define MPAPP_SWIPE_ITEM_MENU_ITEM_HPP

#include <string>

#include "observable.hpp"
#include "platform.hpp"
#include "signal.hpp"
#include "view.hpp"

namespace mpapp {

template <class Platform>
class swipe_item_menu_item_handler;

class swipe_item_menu_item : public view {
public:
    swipe_item_menu_item() = default;
    ~swipe_item_menu_item() override = default;

    swipe_item_menu_item(const swipe_item_menu_item&)            = delete;
    swipe_item_menu_item& operator=(const swipe_item_menu_item&) = delete;
    swipe_item_menu_item(swipe_item_menu_item&&)                 = delete;
    swipe_item_menu_item& operator=(swipe_item_menu_item&&)      = delete;

    // Button-like action contract.
    Observable<std::string>  text{""};
    // URI / file-path to the action's icon. Symbolic — the real
    // `image_source` variant replaces this when image-source plumbing
    // lands. Handlers parse it as a `file:` / plain path for now.
    Observable<std::string>  icon_uri{""};

    // Fired when the action is invoked (gesture released over the item
    // OR a programmatic activation). The Windows real handler hooks
    // `mux::Controls::SwipeItem::Invoked`; Linux/Android paths emit on
    // a programmatic activation only until gesture plumbing lands.
    signal<>                 invoked{};

    swipe_item_menu_item_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const swipe_item_menu_item_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                                   has_handler() const noexcept { return handler_ != nullptr; }
    void                                                   set_handler(swipe_item_menu_item_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    swipe_item_menu_item_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_SWIPE_ITEM_MENU_ITEM_HPP
