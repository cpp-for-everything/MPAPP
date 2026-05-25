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

#include "../../gestures/pan_gesture_recognizer.hpp"
#include "../../gestures/pinch_gesture_recognizer.hpp"
#include "../../gestures/pointer_gesture_recognizer.hpp"
#include "../../gestures/swipe_gesture_recognizer.hpp"
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

    // Synthetic pan tick. `status` controls the lifecycle phase
    // (started → running → completed/canceled); the platform handler
    // would emit started once + running per frame + completed/canceled
    // once. Mock callers drive each phase explicitly.
    void simulate_pan(view& v,
                      internal::gesture_status status,
                      int gesture_id,
                      double total_x, double total_y) {
        for (const auto& r : v.gesture_recognizers) {
            if (r->kind() == internal::gesture_kind::pan) {
                auto& pan = static_cast<pan_gesture_recognizer&>(*r);
                pan.pan_updated.emit(
                    pan_updated_event_args{status, gesture_id, total_x, total_y});
            }
        }
        record_event("gesture.pan_simulated");
    }

    // Synthetic pinch tick. `scale` is incremental (real handlers
    // compute it per tick by dividing the current two-finger spread
    // by the previous spread).
    void simulate_pinch(view& v,
                        internal::gesture_status status,
                        double scale,
                        double origin_x = 0.5, double origin_y = 0.5) {
        for (const auto& r : v.gesture_recognizers) {
            if (r->kind() == internal::gesture_kind::pinch) {
                auto& pinch = static_cast<pinch_gesture_recognizer&>(*r);
                pinch.pinch_updated.emit(
                    pinch_updated_event_args{status, scale, origin_x, origin_y});
            }
        }
        record_event("gesture.pinch_simulated");
    }

    // Synthetic swipe. Fires only on recognizers whose `direction`
    // bitmask contains the simulated `direction` — matches MAUI's
    // `SwipeGestureRecognizer.SendSwiped`'s direction filter.
    void simulate_swipe(view& v, swipe_direction direction) {
        for (const auto& r : v.gesture_recognizers) {
            if (r->kind() == internal::gesture_kind::swipe) {
                auto& sw = static_cast<swipe_gesture_recognizer&>(*r);
                if (any(sw.direction.get(), direction)) {
                    sw.swiped.emit(swiped_event_args{direction});
                }
            }
        }
        record_event("gesture.swipe_simulated");
    }

    // Synthetic pointer transitions — one helper per signal so tests
    // can drive each phase independently. Each fans out to every
    // pointer recognizer attached to `v`.
    void simulate_pointer_entered(view& v,
                                  double x = 0.0, double y = 0.0,
                                  button_mask b = button_mask::none) {
        fan_out_pointer(v, &pointer_gesture_recognizer::pointer_entered, x, y, b);
        record_event("gesture.pointer_entered_simulated");
    }
    void simulate_pointer_exited(view& v,
                                 double x = 0.0, double y = 0.0,
                                 button_mask b = button_mask::none) {
        fan_out_pointer(v, &pointer_gesture_recognizer::pointer_exited, x, y, b);
        record_event("gesture.pointer_exited_simulated");
    }
    void simulate_pointer_moved(view& v,
                                double x = 0.0, double y = 0.0,
                                button_mask b = button_mask::none) {
        fan_out_pointer(v, &pointer_gesture_recognizer::pointer_moved, x, y, b);
        record_event("gesture.pointer_moved_simulated");
    }
    void simulate_pointer_pressed(view& v,
                                  double x = 0.0, double y = 0.0,
                                  button_mask b = button_mask::primary) {
        fan_out_pointer(v, &pointer_gesture_recognizer::pointer_pressed, x, y, b);
        record_event("gesture.pointer_pressed_simulated");
    }
    void simulate_pointer_released(view& v,
                                   double x = 0.0, double y = 0.0,
                                   button_mask b = button_mask::primary) {
        fan_out_pointer(v, &pointer_gesture_recognizer::pointer_released, x, y, b);
        record_event("gesture.pointer_released_simulated");
    }

private:
    // Shared body for the five simulate_pointer_* helpers above.
    using pointer_signal = mpapp::signal<const pointer_event_args&>;
    void fan_out_pointer(view& v,
                         pointer_signal pointer_gesture_recognizer::* slot,
                         double x, double y, button_mask b) {
        for (const auto& r : v.gesture_recognizers) {
            if (r->kind() == internal::gesture_kind::pointer) {
                auto& p = static_cast<pointer_gesture_recognizer&>(*r);
                (p.*slot).emit(pointer_event_args{x, y, b});
            }
        }
    }

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
