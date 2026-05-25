// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/20_ADRs/ADR-0008-mock-first-implementation.md
//
// `view_handler<platform::mock>` — records every property-mapper
// invocation on the cross-cutting `view` surface. Used by P2 mock tests
// to assert the framework calls mappers in the right order, with the
// right values, exactly once per real change.
//
// Each `map_<prop>(view&)` is the entry point a host calls when wiring
// properties — it both records the current value and subscribes a slot
// so future changes are recorded as well.

#ifndef MPAPP_HANDLERS_MOCK_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_VIEW_HANDLER_HPP

#include "../../gestures/tap_gesture_recognizer.hpp"
#include "../../internal/basic_gesture_recognizer.hpp"
#include "../../platform.hpp"
#include "../../view.hpp"
#include "handler_base.hpp"

namespace mpapp {

template <>
class view_handler<platform::mock>
    : public mock_handler_base {
public:
    view_handler() = default;

    void map_automation_id(view& v)   { bind("automation_id",   v.automation_id,   binding_automation_id_); }
    void map_visibility(view& v)      { bind("visibility",      v.visibility_state, binding_visibility_); }
    void map_is_enabled(view& v)      { bind("is_enabled",      v.is_enabled,      binding_is_enabled_); }
    void map_opacity(view& v)         { bind("opacity",         v.opacity,         binding_opacity_); }
    void map_width(view& v)           { bind("width",           v.width,           binding_width_); }
    void map_height(view& v)          { bind("height",          v.height,          binding_height_); }
    void map_flow_direction(view& v)  { bind("flow_direction",  v.flow,            binding_flow_); }
    void map_z_index(view& v)         { bind("z_index",         v.z_index,         binding_z_index_); }
    void map_input_transparent(view& v) { bind("input_transparent", v.input_transparent, binding_input_transparent_); }

    // ----- Gesture recognizers (per RFC-0003) ---------------------------
    // Record one entry per attached recognizer, tagged with its `kind()`,
    // so tests can assert that the right recognizer was wired up. Real
    // handlers replace these records with native listener installs
    // (UITapGestureRecognizer, GtkGestureClick, …).
    void map_gestures(view& v) {
        for (const auto& r : v.gesture_recognizers) {
            switch (r->kind()) {
                case internal::gesture_kind::tap:
                    record_event("gesture.tap_attached");      break;
                case internal::gesture_kind::pan:
                    record_event("gesture.pan_attached");      break;
                case internal::gesture_kind::pinch:
                    record_event("gesture.pinch_attached");    break;
                case internal::gesture_kind::swipe:
                    record_event("gesture.swipe_attached");    break;
                case internal::gesture_kind::pointer:
                    record_event("gesture.pointer_attached");  break;
            }
        }
    }

    // Test helper: dispatch a synthetic tap to every tap recognizer
    // currently attached to `v`. Mirrors what a real platform's native
    // tap event would do — invoke the recognizer's `tapped` signal so
    // subscribers fire. Also records one log entry so tests can assert
    // the simulation happened.
    void simulate_tap(view& v,
                      double x = 0.0, double y = 0.0,
                      button_mask b = button_mask::primary) {
        for (const auto& r : v.gesture_recognizers) {
            if (r->kind() == internal::gesture_kind::tap) {
                auto& tap = static_cast<tap_gesture_recognizer&>(*r);
                tap.tapped.emit(tapped_event_args{x, y, b});
            }
        }
        record_event("gesture.tap_simulated");
    }

private:
    detail::property_binding<std::string>     binding_automation_id_{};
    detail::property_binding<visibility>      binding_visibility_{};
    detail::property_binding<bool>            binding_is_enabled_{};
    detail::property_binding<double>          binding_opacity_{};
    detail::property_binding<double>          binding_width_{};
    detail::property_binding<double>          binding_height_{};
    detail::property_binding<flow_direction>  binding_flow_{};
    detail::property_binding<int>             binding_z_index_{};
    detail::property_binding<bool>            binding_input_transparent_{};
};

} // namespace mpapp

#endif // MPAPP_HANDLERS_MOCK_VIEW_HANDLER_HPP
