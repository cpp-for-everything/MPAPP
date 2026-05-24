// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/StackLayout.md
//
// `stack_layout_handler<platform::mock>` — records the four
// stack-specific property mappers (orientation, spacing, alignment).
// Children mapping is inherited from `layout_handler<platform::mock>`
// via the layout child-mutator pattern — but for the mock surface we
// keep recording localised to this handler.

#ifndef MPAPP_HANDLERS_MOCK_STACK_LAYOUT_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_STACK_LAYOUT_HANDLER_HPP

#include "../../platform.hpp"
#include "../../internal/basic_stack_layout.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class stack_layout_handler<platform::mock> : public mock_handler_base {
public:
    stack_layout_handler() = default;
    ~stack_layout_handler() = default;

    stack_layout_handler(const stack_layout_handler&)            = delete;
    stack_layout_handler& operator=(const stack_layout_handler&) = delete;
    stack_layout_handler(stack_layout_handler&&)                 = delete;
    stack_layout_handler& operator=(stack_layout_handler&&)      = delete;

    void map_orientation(basic_stack_layout& s) {
        record_change("orientation", s.stack_orientation.get());
        s.stack_orientation.changed.subscribe(orient_slot_, orient_cb_);
    }

    void map_spacing(basic_stack_layout& s) {
        record_change("spacing", s.spacing.get());
        s.spacing.changed.subscribe(spacing_slot_, spacing_cb_);
    }

    void map_horizontal_alignment(basic_stack_layout& s) {
        record_change("horizontal_alignment", s.horizontal_alignment.get());
        s.horizontal_alignment.changed.subscribe(h_align_slot_, h_align_cb_);
    }

    void map_vertical_alignment(basic_stack_layout& s) {
        record_change("vertical_alignment", s.vertical_alignment.get());
        s.vertical_alignment.changed.subscribe(v_align_slot_, v_align_cb_);
    }

    // Child-mutation mappers — for the mock surface we record them
    // explicitly; the real WinUI handler updates `StackPanel::Children`.
    void map_add(basic_stack_layout& /*s*/, view& /*child*/) { record_event("add"); }
    void map_remove(basic_stack_layout& /*s*/, view& /*child*/) { record_event("remove"); }
    void map_clear(basic_stack_layout& /*s*/) { record_event("clear"); }

private:
    using self_t = stack_layout_handler<platform::mock>;

    mock_property_recorder<self_t, orientation> orient_cb_{this, "orientation"};
    signal_slot<const orientation&>             orient_slot_{};

    mock_property_recorder<self_t, double>      spacing_cb_{this, "spacing"};
    signal_slot<const double&>                  spacing_slot_{};

    mock_property_recorder<self_t, h_align>     h_align_cb_{this, "horizontal_alignment"};
    signal_slot<const h_align&>                 h_align_slot_{};

    mock_property_recorder<self_t, v_align>     v_align_cb_{this, "vertical_alignment"};
    signal_slot<const v_align&>                 v_align_slot_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_STACK_LAYOUT_HANDLER_HPP
