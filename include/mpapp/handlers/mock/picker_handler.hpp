// SPDX-License-Identifier: Apache-2.0
// Mock picker handler. The `items` Observable holds a vector<string>
// (which std::format can't format), so we record the count instead of
// the contents — same pattern bindable_layout uses for its child list.

#ifndef MPAPP_HANDLERS_MOCK_PICKER_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_PICKER_HANDLER_HPP

#include <cstddef>
#include <string>
#include <vector>

#include "../../picker.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "handler_base.hpp"

namespace mpapp {

template <>
class picker_handler<platform::mock>
    : public mock_handler_base {
public:
    picker_handler() = default;

    void map_items(picker& p) {
        record("items.count", p.items.get().size());
        p.items.changed.subscribe(items_slot_, items_cb_);
    }
    void map_selected_index(picker& p) { bind("selected_index", p.selected_index, binding_selected_); }
    void map_title(picker& p)          { bind("title",          p.title,          binding_title_); }

private:
    struct items_cb_t {
        picker_handler<platform::mock>* self;
        void operator()(const std::vector<std::string>& v) const {
            self->record("items.count", v.size());
        }
    };

    items_cb_t                                          items_cb_{this};
    signal_slot<std::vector<std::string> const&>        items_slot_{};
    detail::property_binding<int>                       binding_selected_{};
    detail::property_binding<std::string>               binding_title_{};
};

} // namespace mpapp

#endif // MPAPP_HANDLERS_MOCK_PICKER_HANDLER_HPP
