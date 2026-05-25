// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock handler for `mpapp::basic_table_view`.

#ifndef MPAPP_HANDLERS_MOCK_TABLE_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_TABLE_VIEW_HANDLER_HPP

#include <cstddef>
#include <vector>

#include "../../platform.hpp"
#include "../../internal/basic_table_view.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class table_view_handler<platform::mock> : public mock_handler_base {
public:
    table_view_handler() = default;
    ~table_view_handler() = default;

    table_view_handler(const table_view_handler&)            = delete;
    table_view_handler& operator=(const table_view_handler&) = delete;
    table_view_handler(table_view_handler&&)                 = delete;
    table_view_handler& operator=(table_view_handler&&)      = delete;

    void map_sections(basic_table_view& tv) {
        record_change("sections.count", tv.sections.get().size());
        tv.sections.changed.subscribe(slot_sec_, sec_cb_);
    }

    void map_typed_sections(basic_table_view& tv) {
        record_change("typed_sections.count", tv.typed_sections.get().size());
        tv.typed_sections.changed.subscribe(slot_typed_, typed_cb_);
    }

    void map_row_height(basic_table_view& tv) {
        record_change("row_height", tv.row_height.get());
        tv.row_height.changed.subscribe(slot_rh_, rh_cb_);
    }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_table_view& /*x*/) noexcept {}


private:
    using self_t = table_view_handler<platform::mock>;

    struct sec_recorder {
        self_t* self = nullptr;
        void operator()(const std::vector<table_section_data>& v) const {
            self->record_change("sections.count", v.size());
        }
    };
    struct typed_recorder {
        self_t* self = nullptr;
        void operator()(const std::vector<table_section_typed>& v) const {
            self->record_change("typed_sections.count", v.size());
        }
    };
    struct rh_recorder {
        self_t* self = nullptr;
        void operator()(int v) const { self->record_change("row_height", v); }
    };

    sec_recorder   sec_cb_{this};
    typed_recorder typed_cb_{this};
    rh_recorder    rh_cb_{this};

    signal_slot<const std::vector<table_section_data>&>  slot_sec_{};
    signal_slot<const std::vector<table_section_typed>&> slot_typed_{};
    signal_slot<const int&>                              slot_rh_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_TABLE_VIEW_HANDLER_HPP
