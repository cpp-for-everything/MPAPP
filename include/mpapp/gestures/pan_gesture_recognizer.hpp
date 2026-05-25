// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0003-gesture-recognizers.md
//
// `mpapp::pan_gesture_recognizer` — counterpart to MAUI's
// `PanGestureRecognizer`. Detects a multi-finger drag and reports the
// cumulative displacement (TotalX / TotalY) at every status tick from
// `started` → repeated `running` → `completed` (or `canceled`).
//
// XAML (1:1 with MAUI per ADR-0004):
//
//     <Image Source="map.png">
//         <Image.GestureRecognizers>
//             <PanGestureRecognizer TouchPoints="1" PanUpdated="OnPan"/>
//         </Image.GestureRecognizers>
//     </Image>

#ifndef MPAPP_GESTURES_PAN_GESTURE_RECOGNIZER_HPP
#define MPAPP_GESTURES_PAN_GESTURE_RECOGNIZER_HPP

#include "../internal/basic_gesture_recognizer.hpp"
#include "../observable.hpp"
#include "../signal.hpp"

namespace mpapp {

// Event payload for a pan tick. `gesture_id` is a per-gesture sequence
// number so a consumer can tell two simultaneous pans apart; the
// platform handler is responsible for allocating a fresh id per gesture
// start. `total_x` and `total_y` are the cumulative offset from the
// gesture's start point, in view-local pixels.
struct pan_updated_event_args {
    internal::gesture_status status      = internal::gesture_status::started;
    int                      gesture_id  = 0;
    double                   total_x     = 0.0;
    double                   total_y     = 0.0;
};

class pan_gesture_recognizer : public internal::basic_gesture_recognizer {
public:
    pan_gesture_recognizer() = default;

    // ----- Bindable configuration ----------------------------------------
    // Number of fingers that must be down for the gesture to track.
    // 1 = single-finger drag (default); 2 = two-finger drag, etc.
    Observable<int> touch_points{1};

    // ----- Event ---------------------------------------------------------
    mpapp::signal<const pan_updated_event_args&> pan_updated{};

    // ----- Polymorphic identity ------------------------------------------
    [[nodiscard]] internal::gesture_kind kind() const noexcept override {
        return internal::gesture_kind::pan;
    }
};

} // namespace mpapp

#endif // MPAPP_GESTURES_PAN_GESTURE_RECOGNIZER_HPP
