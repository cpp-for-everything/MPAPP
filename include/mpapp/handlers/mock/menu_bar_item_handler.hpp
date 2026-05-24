// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock basic_menu_bar_item handler — records `title` via the
// standard `bind()` plumbing and `items.count` for the sub-item
// collection (same pattern used by basic_menu_bar / basic_toolbar / basic_picker).

#ifndef MPAPP_HANDLERS_MOCK_MENU_BAR_ITEM_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_MENU_BAR_ITEM_HANDLER_HPP

#include <cstddef>
#include <string>
#include <vector>

#include "../../internal/basic_menu_bar_item.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../view.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class menu_bar_item_handler<platform::mock>
    : public mock_handler_base {
public:
    menu_bar_item_handler() = default;

    void map_title(basic_menu_bar_item& m) {
        bind("title", m.title, binding_title_);
    }

    void map_items(basic_menu_bar_item& m) {
        record("items.count", m.items.get().size());
        m.items.changed.subscribe(items_slot_, items_cb_);
    }

private:
    struct items_cb_t {
        menu_bar_item_handler<platform::mock>* self;
        void operator()(const std::vector<view*>& v) const {
            self->record("items.count", v.size());
        }
    };

    detail::property_binding<std::string>     binding_title_{};
    items_cb_t                                items_cb_{this};
    signal_slot<std::vector<view*> const&>    items_slot_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_MENU_BAR_ITEM_HANDLER_HPP
