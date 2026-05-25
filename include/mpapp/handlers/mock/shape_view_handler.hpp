// SPDX-License-Identifier: Apache-2.0
// Mock handler for `mpapp::basic_shape_view`.

#ifndef MPAPP_HANDLERS_MOCK_SHAPE_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_SHAPE_VIEW_HANDLER_HPP

#include <cstdint>
#include <string>

#include "../../platform.hpp"
#include "../../internal/basic_shape_view.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class shape_view_handler<platform::mock> : public mock_handler_base {
public:
    shape_view_handler() = default;
    ~shape_view_handler() = default;

    shape_view_handler(const shape_view_handler&)            = delete;
    shape_view_handler& operator=(const shape_view_handler&) = delete;
    shape_view_handler(shape_view_handler&&)                 = delete;
    shape_view_handler& operator=(shape_view_handler&&)      = delete;

    void map_kind(basic_shape_view& s) {
        record_change("kind", static_cast<std::uint8_t>(s.kind.get()));
        s.kind.changed.subscribe(slot_kind_, kind_cb_);
    }

    void map_data(basic_shape_view& s) {
        record_change("data", s.data.get());
        s.data.changed.subscribe(slot_data_, data_cb_);
    }

    void map_fill(basic_shape_view& s) {
        record_change("fill", s.fill.get());
        s.fill.changed.subscribe(slot_fill_, fill_cb_);
    }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_shape_view& /*x*/) noexcept {}


private:
    using self_t = shape_view_handler<platform::mock>;

    struct kind_recorder {
        self_t* self = nullptr;
        void operator()(shape_kind v) const { self->record_change("kind", static_cast<std::uint8_t>(v)); }
    };
    struct data_recorder {
        self_t* self = nullptr;
        void operator()(const std::string& v) const { self->record_change("data", v); }
    };
    struct fill_recorder {
        self_t* self = nullptr;
        void operator()(const std::string& v) const { self->record_change("fill", v); }
    };

    kind_recorder kind_cb_{this};
    data_recorder data_cb_{this};
    fill_recorder fill_cb_{this};

    signal_slot<const shape_kind&>  slot_kind_{};
    signal_slot<const std::string&> slot_data_{};
    signal_slot<const std::string&> slot_fill_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_SHAPE_VIEW_HANDLER_HPP
