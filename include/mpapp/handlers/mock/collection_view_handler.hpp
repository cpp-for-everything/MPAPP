// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock handler for `mpapp::basic_collection_view`.

#ifndef MPAPP_HANDLERS_MOCK_COLLECTION_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_COLLECTION_VIEW_HANDLER_HPP

#include <cstddef>
#include <string>
#include <vector>

#include "../../internal/basic_collection_view.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class collection_view_handler<platform::mock> : public mock_handler_base {
public:
    collection_view_handler() = default;
    ~collection_view_handler() = default;

    collection_view_handler(const collection_view_handler&)            = delete;
    collection_view_handler& operator=(const collection_view_handler&) = delete;
    collection_view_handler(collection_view_handler&&)                 = delete;
    collection_view_handler& operator=(collection_view_handler&&)      = delete;

    void map_items_source(basic_collection_view& cv) {
        record_change("items_source.count", cv.items_source.get().size());
        cv.items_source.changed.subscribe(slot_items_, items_cb_);
    }

    void map_selected_index(basic_collection_view& cv) {
        record_change("selected_index", cv.selected_index.get());
        cv.selected_index.changed.subscribe(slot_sel_, sel_cb_);
    }

    void map_selected_indices(basic_collection_view& cv) {
        record_change("selected_indices.count", cv.selected_indices.get().size());
        cv.selected_indices.changed.subscribe(slot_seli_, seli_cb_);
    }

    // Records the current layout enum as an integer so tests can assert
    // the full vertical_list / horizontal_list / vertical_grid /
    // horizontal_grid cycle reaches the handler.
    void map_layout(basic_collection_view& cv) {
        record_change("layout",
                      static_cast<int>(cv.layout.get()));
        cv.layout.changed.subscribe(slot_layout_, layout_cb_);
    }

    // Most-recent layout the handler observed. Defaults to vertical_list
    // (the surface default) until map_layout records the first value.
    collection_layout last_layout = collection_layout::vertical_list;

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_collection_view& /*x*/) noexcept {}


private:
    using self_t = collection_view_handler<platform::mock>;

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
    struct seli_recorder {
        self_t* self = nullptr;
        void operator()(const std::vector<int>& v) const {
            self->record_change("selected_indices.count", v.size());
        }
    };
    struct layout_recorder {
        self_t* self = nullptr;
        void operator()(collection_layout v) const {
            self->last_layout = v;
            self->record_change("layout", static_cast<int>(v));
        }
    };

    items_recorder  items_cb_{this};
    sel_recorder    sel_cb_{this};
    seli_recorder   seli_cb_{this};
    layout_recorder layout_cb_{this};

    signal_slot<const std::vector<std::string>&> slot_items_{};
    signal_slot<const int&>                      slot_sel_{};
    signal_slot<const std::vector<int>&>         slot_seli_{};
    signal_slot<const collection_layout&>        slot_layout_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_COLLECTION_VIEW_HANDLER_HPP
