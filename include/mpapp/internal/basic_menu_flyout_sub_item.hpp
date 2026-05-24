// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/MenuFlyoutSubItem.md
//
// `mpapp::menu_flyout_sub_item` — a menu entry that, instead of
// invoking a command, expands into a nested submenu. Carries the same
// `text` / `is_enabled` surface as menu_flyout_item plus its own
// `items` collection (of any menu element subtype).
//
// Like menu_flyout itself, `items` is held as
// `Observable<std::vector<view*>>` of non-owning child pointers. The
// handler rebuilds the native submenu whenever the collection mutates.

#ifndef MPAPP_INTERNAL_BASIC_MENU_FLYOUT_SUB_ITEM_HPP
#define MPAPP_INTERNAL_BASIC_MENU_FLYOUT_SUB_ITEM_HPP

#include <string>
#include <vector>

#include "../observable.hpp"
#include "../platform.hpp"
#include "../view.hpp"

namespace mpapp::internal {

template <class Platform = platform::current>
class menu_flyout_sub_item_handler;

class basic_menu_flyout_sub_item : public view {
public:
    basic_menu_flyout_sub_item()                                                  = default;
    ~basic_menu_flyout_sub_item() override                                        = default;
    basic_menu_flyout_sub_item(const basic_menu_flyout_sub_item&)                       = delete;
    basic_menu_flyout_sub_item& operator=(const basic_menu_flyout_sub_item&)            = delete;
    basic_menu_flyout_sub_item(basic_menu_flyout_sub_item&&)                            = delete;
    basic_menu_flyout_sub_item& operator=(basic_menu_flyout_sub_item&&)                 = delete;

    // ----- Properties ----------------------------------------------------
    Observable<std::string>          text{""};
    Observable<std::vector<view*>>   items{};

    // ----- Handler -------------------------------------------------------
    menu_flyout_sub_item_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const menu_flyout_sub_item_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                                   has_handler() const noexcept { return handler_ != nullptr; }
    void                                                   set_handler(menu_flyout_sub_item_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    menu_flyout_sub_item_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp::internal


#endif // MPAPP_INTERNAL_BASIC_MENU_FLYOUT_SUB_ITEM_HPP
