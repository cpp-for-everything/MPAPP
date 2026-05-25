// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock handler for `mpapp::basic_list_view`.

#ifndef MPAPP_HANDLERS_MOCK_LIST_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_LIST_VIEW_HANDLER_HPP

#include <cstddef>
#include <string>
#include <vector>

#include "../../internal/basic_list_view.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class list_view_handler<platform::mock> : public mock_handler_base {
public:
    list_view_handler() = default;
    ~list_view_handler() = default;

    list_view_handler(const list_view_handler&)            = delete;
    list_view_handler& operator=(const list_view_handler&) = delete;
    list_view_handler(list_view_handler&&)                 = delete;
    list_view_handler& operator=(list_view_handler&&)      = delete;

    void map_items_source(basic_list_view& lv) {
        record_change("items_source.count", lv.items_source.get().size());
        lv.items_source.changed.subscribe(slot_items_, items_cb_);
    }

    void map_selected_index(basic_list_view& lv) {
        record_change("selected_index", lv.selected_index.get());
        lv.selected_index.changed.subscribe(slot_sel_, sel_cb_);
    }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_list_view& /*x*/) noexcept {}


private:
    using self_t = list_view_handler<platform::mock>;

    struct items_recorder {
        self_t* self = nullptr;
        void operator()(const std::vector<std::string>& v) const {
            self->record_change("items_source.count", v.size());
        }
    };
    struct sel_recorder {
        self_t* self = nullptr;
        void operator()(int v) const { self->record_change("selected_index", v); }
    };

    items_recorder items_cb_{this};
    sel_recorder   sel_cb_{this};

    signal_slot<const std::vector<std::string>&> slot_items_{};
    signal_slot<const int&>                      slot_sel_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_LIST_VIEW_HANDLER_HPP
