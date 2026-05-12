// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Layout.md
//
// `layout_handler<platform::mock>` — records property mappers for the
// abstract `layout` container, plus the child-list command mappers
// (`add`, `insert`, `remove`, `clear`, `update_z_index`). Subclass
// handlers (grid, stack, …) reuse the same recording.

#ifndef MPAPP_HANDLERS_MOCK_LAYOUT_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_LAYOUT_HANDLER_HPP

#include "../../layout.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp {

template <>
class layout_handler<platform::mock>
    : public mock_handler_base<layout_handler<platform::mock>, layout> {
public:
    layout_handler() = default;

    void map_padding(layout& l)                   { bind("padding",                   l.padding,                   binding_padding_); }
    void map_is_clipped_to_bounds(layout& l)      { bind("is_clipped_to_bounds",      l.is_clipped_to_bounds,      binding_clipped_); }
    void map_cascade_input_transparent(layout& l) { bind("cascade_input_transparent", l.cascade_input_transparent, binding_cascade_); }

    // ----- Command mappers (called by `layout` mutators in real handlers). --
    // For the mock surface, tests drive these explicitly to assert the
    // command sequence — `layout::add(...)` etc. only mutate the storage.
    void map_add(layout& /*l*/, const view& /*child*/)              { record("add"); }
    void map_insert(layout& /*l*/, std::size_t i, const view& /*c*/) { record("insert", i); }
    void map_remove(layout& /*l*/, const view& /*child*/)           { record("remove"); }
    void map_clear(layout& /*l*/)                                   { record("clear"); }
    void map_update_z_index(layout& /*l*/, const view& /*c*/, int z) { record("update_z_index", z); }

private:
    detail::property_binding<thickness>  binding_padding_{};
    detail::property_binding<bool>       binding_clipped_{};
    detail::property_binding<bool>       binding_cascade_{};
};

} // namespace mpapp

#endif // MPAPP_HANDLERS_MOCK_LAYOUT_HANDLER_HPP
