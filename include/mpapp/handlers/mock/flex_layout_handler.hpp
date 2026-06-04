// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/FlexLayout.md
//
// `flex_layout_handler<platform::mock>` — records the container property
// mappers (direction, wrap, justify_content, align_items, align_content,
// position), the per-child attached-property setters (order, grow,
// shrink, align_self, basis), and the child-list command mappers (add,
// remove, clear). Tests drive these explicitly to assert the exact
// sequence and arguments the framework would have passed to a real native
// handler.

#ifndef MPAPP_HANDLERS_MOCK_FLEX_LAYOUT_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_FLEX_LAYOUT_HANDLER_HPP

#include "../../internal/basic_flex_layout.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class flex_layout_handler<platform::mock> : public mock_handler_base {
public:
    flex_layout_handler() = default;
    ~flex_layout_handler() = default;

    flex_layout_handler(const flex_layout_handler&)            = delete;
    flex_layout_handler& operator=(const flex_layout_handler&) = delete;
    flex_layout_handler(flex_layout_handler&&)                 = delete;
    flex_layout_handler& operator=(flex_layout_handler&&)      = delete;

    // ----- Container property mappers ---------------------------------
    void map_direction(basic_flex_layout& f) {
        record_change("direction", f.direction.get());
        f.direction.changed.subscribe(direction_slot_, direction_cb_);
    }

    void map_wrap(basic_flex_layout& f) {
        record_change("wrap", f.wrap.get());
        f.wrap.changed.subscribe(wrap_slot_, wrap_cb_);
    }

    void map_justify_content(basic_flex_layout& f) {
        record_change("justify_content", f.justify_content.get());
        f.justify_content.changed.subscribe(justify_slot_, justify_cb_);
    }

    void map_align_items(basic_flex_layout& f) {
        record_change("align_items", f.align_items.get());
        f.align_items.changed.subscribe(align_items_slot_, align_items_cb_);
    }

    void map_align_content(basic_flex_layout& f) {
        record_change("align_content", f.align_content.get());
        f.align_content.changed.subscribe(align_content_slot_, align_content_cb_);
    }

    void map_position(basic_flex_layout& f) {
        record_change("position", f.position.get());
        f.position.changed.subscribe(position_slot_, position_cb_);
    }

    // ----- Per-child attached-property setters ------------------------
    // Real handlers read the attached store in add_child; the mock
    // records the resolved value so tests assert the native per-child API
    // would have received it.
    void set_order(basic_flex_layout& f, view& child) {
        record_change("order", f.get_child_props(child).order);
    }
    void set_grow(basic_flex_layout& f, view& child) {
        record_change("grow", f.get_child_props(child).grow);
    }
    void set_shrink(basic_flex_layout& f, view& child) {
        record_change("shrink", f.get_child_props(child).shrink);
    }
    void set_align_self(basic_flex_layout& f, view& child) {
        record_change("align_self", f.get_child_props(child).align_self);
    }
    void set_basis(basic_flex_layout& f, view& child) {
        record_change("basis", f.get_child_props(child).basis);
    }

    // ----- Child-mutation mappers -------------------------------------
    // For the mock surface we record them explicitly; the real handler
    // updates the native container's children collection.
    void map_add(basic_flex_layout& /*f*/, view& /*child*/) { record_event("add"); }
    void map_remove(basic_flex_layout& /*f*/, view& /*child*/) { record_event("remove"); }
    void map_clear(basic_flex_layout& /*f*/) { record_event("clear"); }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_flex_layout& /*x*/) noexcept {}


private:
    using self_t = flex_layout_handler<platform::mock>;

    mock_property_recorder<self_t, flex_direction>     direction_cb_{this, "direction"};
    signal_slot<const flex_direction&>                 direction_slot_{};

    mock_property_recorder<self_t, flex_wrap>          wrap_cb_{this, "wrap"};
    signal_slot<const flex_wrap&>                      wrap_slot_{};

    mock_property_recorder<self_t, flex_justify>       justify_cb_{this, "justify_content"};
    signal_slot<const flex_justify&>                   justify_slot_{};

    mock_property_recorder<self_t, flex_align_items>   align_items_cb_{this, "align_items"};
    signal_slot<const flex_align_items&>               align_items_slot_{};

    mock_property_recorder<self_t, flex_align_content> align_content_cb_{this, "align_content"};
    signal_slot<const flex_align_content&>             align_content_slot_{};

    mock_property_recorder<self_t, flex_position>      position_cb_{this, "position"};
    signal_slot<const flex_position&>                  position_slot_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_FLEX_LAYOUT_HANDLER_HPP
