// SPDX-License-Identifier: Apache-2.0
// Mock basic_toolbar handler. The `items` Observable holds a
// `vector<toolbar_item>` (which std::format can't format), so we record
// the count instead of the contents — same pattern basic_picker uses for its
// items vector.

#ifndef MPAPP_HANDLERS_MOCK_TOOLBAR_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_TOOLBAR_HANDLER_HPP

#include <cstddef>
#include <string>
#include <vector>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_toolbar.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class toolbar_handler<platform::mock>
    : public mock_handler_base {
public:
    toolbar_handler() = default;

    void map_items(basic_toolbar& t) {
        record("items.count", t.items.get().size());
        t.items.changed.subscribe(items_slot_, items_cb_);
    }
    void map_title(basic_toolbar& t) { bind("title", t.title, binding_title_); }

private:
    struct items_cb_t {
        toolbar_handler<platform::mock>* self;
        void operator()(const std::vector<toolbar_item>& v) const {
            self->record("items.count", v.size());
        }
    };

    items_cb_t                                       items_cb_{this};
    signal_slot<std::vector<toolbar_item> const&>    items_slot_{};
    detail::property_binding<std::string>            binding_title_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_TOOLBAR_HANDLER_HPP
