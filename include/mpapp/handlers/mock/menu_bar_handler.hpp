// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock basic_menu_bar handler — records `items.count` so the
// std::format-unfriendly `vector<view*>` payload can still be asserted
// against. Mirrors the pattern basic_toolbar / basic_picker use for their items
// collections.

#ifndef MPAPP_HANDLERS_MOCK_MENU_BAR_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_MENU_BAR_HANDLER_HPP

#include <cstddef>
#include <vector>

#include "../../internal/basic_menu_bar.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../view.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class menu_bar_handler<platform::mock>
    : public mock_handler_base {
public:
    menu_bar_handler() = default;

    void map_items(basic_menu_bar& b) {
        record("items.count", b.items.get().size());
        b.items.changed.subscribe(items_slot_, items_cb_);
    }

private:
    struct items_cb_t {
        menu_bar_handler<platform::mock>* self;
        void operator()(const std::vector<view*>& v) const {
            self->record("items.count", v.size());
        }
    };

    items_cb_t                                items_cb_{this};
    signal_slot<std::vector<view*> const&>    items_slot_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_MENU_BAR_HANDLER_HPP
