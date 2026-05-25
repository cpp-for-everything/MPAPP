// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0003-gesture-recognizers.md
//
// `mpapp::pinch_gesture_recognizer` — counterpart to MAUI's
// `PinchGestureRecognizer`. Detects a two-finger pinch / spread and
// reports the *incremental* scale factor between consecutive ticks
// (matching MAUI's contract — multiply the previous scale by `scale`
// to track total zoom). Reports the centroid of the two contact points
// in view-local normalised coordinates (0.0 = top/left, 1.0 = bottom/
// right).
//
// XAML (1:1 with MAUI per ADR-0004):
//
//     <Image Source="map.png">
//         <Image.GestureRecognizers>
//             <PinchGestureRecognizer PinchUpdated="OnPinch"/>
//         </Image.GestureRecognizers>
//     </Image>

#ifndef MPAPP_GESTURES_PINCH_GESTURE_RECOGNIZER_HPP
#define MPAPP_GESTURES_PINCH_GESTURE_RECOGNIZER_HPP

#include "../internal/basic_gesture_recognizer.hpp"
#include "../signal.hpp"

namespace mpapp {

// Event payload for a pinch tick. `scale` is the incremental ratio
// (1.0 = no change, > 1.0 = spread out, < 1.0 = pinch in); multiply
// successive `scale`s to track total zoom. `origin_x` / `origin_y`
// are in view-local NORMALISED coordinates [0.0, 1.0] so consumers
// can place a zoom anchor without knowing the view's pixel size.
struct pinch_updated_event_args {
    internal::gesture_status status   = internal::gesture_status::started;
    double                   scale    = 1.0;
    double                   origin_x = 0.5;
    double                   origin_y = 0.5;
};

class pinch_gesture_recognizer : public internal::basic_gesture_recognizer {
public:
    pinch_gesture_recognizer() = default;

    // ----- Event ---------------------------------------------------------
    mpapp::signal<const pinch_updated_event_args&> pinch_updated{};

    // ----- Polymorphic identity ------------------------------------------
    [[nodiscard]] internal::gesture_kind kind() const noexcept override {
        return internal::gesture_kind::pinch;
    }
};

} // namespace mpapp

#endif // MPAPP_GESTURES_PINCH_GESTURE_RECOGNIZER_HPP
