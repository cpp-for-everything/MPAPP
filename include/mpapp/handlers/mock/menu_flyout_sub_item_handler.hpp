// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock menu_flyout_sub_item handler.
//
// Records `text` via `bind()` and `items.count` via a custom callback
// (vector<view*> has no std::format spelling). Mirrors the menu_flyout
// shape since the two share the items-vector mechanic.

#ifndef MPAPP_HANDLERS_MOCK_MENU_FLYOUT_SUB_ITEM_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_MENU_FLYOUT_SUB_ITEM_HANDLER_HPP

#include <cstddef>
#include <string>
#include <vector>

#include "../../menu_flyout_sub_item.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../view.hpp"
#include "handler_base.hpp"

namespace mpapp {

template <>
class menu_flyout_sub_item_handler<platform::mock>
    : public mock_handler_base {
public:
    menu_flyout_sub_item_handler() = default;

    void map_text(menu_flyout_sub_item& s) {
        bind("text", s.text, binding_text_);
    }

    void map_items(menu_flyout_sub_item& s) {
        record("items.count", s.items.get().size());
        s.items.changed.subscribe(items_slot_, items_cb_);
    }

private:
    struct items_cb_t {
        menu_flyout_sub_item_handler<platform::mock>* self;
        void operator()(const std::vector<view*>& v) const {
            self->record("items.count", v.size());
        }
    };

    detail::property_binding<std::string>      binding_text_{};
    items_cb_t                                 items_cb_{this};
    signal_slot<std::vector<view*> const&>     items_slot_{};
};

} // namespace mpapp

#endif // MPAPP_HANDLERS_MOCK_MENU_FLYOUT_SUB_ITEM_HANDLER_HPP
