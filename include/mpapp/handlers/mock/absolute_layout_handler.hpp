// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/AbsoluteLayout.md
//
// `absolute_layout_handler<platform::mock>` — records the two attached
// property mappers (layout_bounds, layout_flags) for a given child, and
// inherits the layout child-list command mappers (map_add, map_insert,
// map_remove, map_clear, map_update_z_index) from layout_handler<mock>.
//
// Unlike grid_layout's row/column Observables, AbsoluteLayout's bounds and
// flags are attached properties stored per-child on the layout. The mock
// mappers therefore take the layout + child and record the current
// attached value, so tests can assert what the real handler would push to
// the native placement API.

#ifndef MPAPP_HANDLERS_MOCK_ABSOLUTE_LAYOUT_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_ABSOLUTE_LAYOUT_HANDLER_HPP

#include <string>

#include "../../internal/basic_absolute_layout.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"
#include "layout_handler.hpp"

namespace mpapp::internal {

template <>
class absolute_layout_handler<platform::mock> : public layout_handler<platform::mock> {
public:
    absolute_layout_handler() = default;
    ~absolute_layout_handler() = default;

    absolute_layout_handler(const absolute_layout_handler&)            = delete;
    absolute_layout_handler& operator=(const absolute_layout_handler&) = delete;
    absolute_layout_handler(absolute_layout_handler&&)                 = delete;
    absolute_layout_handler& operator=(absolute_layout_handler&&)      = delete;

    // Attached-property mappers. Record the child's current attached value
    // — the real handler would translate this into the native placement
    // call (Canvas.SetLeft/SetTop, gtk_fixed_put, LayoutParams, …).
    void map_layout_bounds(basic_absolute_layout& a, const view& child) {
        record("layout_bounds", format_rect(a.get_layout_bounds(child)));
    }

    void map_layout_flags(basic_absolute_layout& a, const view& child) {
        record("layout_flags", to_string(a.get_layout_flags(child)));
    }

// RFC-0003 stub: per-platform real gesture wire-up is
// pending the platform's real-handler task. No-op today
// so the wrapper ctor's unconditional
// `embedded_handler_.map_gestures(*this);` links.
void map_gestures(basic_absolute_layout& /*x*/) noexcept {}

private:
    static std::string format_rect(const rect& r) {
        return "rect(" + fmt(r.x) + "," + fmt(r.y) + ","
                       + fmt(r.width) + "," + fmt(r.height) + ")";
    }

    static std::string fmt(double v) {
        std::string s = std::to_string(v);
        // Trim trailing zeros so "10.000000" reads as "10", matching the
        // std::format integral-double rendering used by record(prop, T).
        auto dot = s.find('.');
        if (dot != std::string::npos) {
            std::size_t last = s.find_last_not_of('0');
            if (last == dot) last -= 1;  // drop a lone trailing dot too
            s.erase(last + 1);
        }
        return s;
    }
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_ABSOLUTE_LAYOUT_HANDLER_HPP
