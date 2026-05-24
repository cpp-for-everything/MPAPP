// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock basic_menu_flyout handler.
//
// `items` is a `vector<view*>` (no std::format spelling), so we record
// the count instead of the contents — same pattern basic_toolbar / basic_picker
// use for their items vectors. `is_open` is recorded through the
// standard `bind()` helper.

#ifndef MPAPP_HANDLERS_MOCK_MENU_FLYOUT_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_MENU_FLYOUT_HANDLER_HPP

#include <cstddef>
#include <vector>

#include "../../internal/basic_menu_flyout.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../view.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class menu_flyout_handler<platform::mock>
    : public mock_handler_base {
public:
    menu_flyout_handler() = default;

    void map_items(basic_menu_flyout& f) {
        record("items.count", f.items.get().size());
        f.items.changed.subscribe(items_slot_, items_cb_);
    }

    void map_is_open(basic_menu_flyout& f) {
        bind("is_open", f.is_open, binding_is_open_);
    }

private:
    struct items_cb_t {
        menu_flyout_handler<platform::mock>* self;
        void operator()(const std::vector<view*>& v) const {
            self->record("items.count", v.size());
        }
    };

    items_cb_t                                items_cb_{this};
    signal_slot<std::vector<view*> const&>    items_slot_{};
    detail::property_binding<bool>            binding_is_open_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_MENU_FLYOUT_HANDLER_HPP
