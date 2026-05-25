// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0003-gesture-recognizers.md
//
// `mpapp::pointer_gesture_recognizer` — counterpart to MAUI's
// `PointerGestureRecognizer`. Surfaces the five low-level pointer
// events (enter / exit / move / press / release) for hover-driven
// UI like tooltips, custom hit testing, and drag-and-drop preview.
//
// Touch-only platforms (Android, iOS without a pointer) report the
// `pressed` / `released` / `moved` triplet during a touch; `entered`
// and `exited` are no-ops there per MAUI's behaviour.
//
// XAML (1:1 with MAUI per ADR-0004):
//
//     <Image>
//         <Image.GestureRecognizers>
//             <PointerGestureRecognizer
//                 PointerEntered="OnHoverIn"
//                 PointerExited="OnHoverOut"
//                 PointerMoved="OnHoverMove"/>
//         </Image.GestureRecognizers>
//     </Image>

#ifndef MPAPP_GESTURES_POINTER_GESTURE_RECOGNIZER_HPP
#define MPAPP_GESTURES_POINTER_GESTURE_RECOGNIZER_HPP

#include "../internal/basic_gesture_recognizer.hpp"
#include "../observable.hpp"
#include "../signal.hpp"
#include "tap_gesture_recognizer.hpp"   // for button_mask

namespace mpapp {

// Event payload for any pointer transition. Position is in view-local
// pixels (origin top-left). `button` is the *single* button that
// triggered the event for press / release; for move / enter / exit it
// reports whichever button(s) are currently held (or `none`).
struct pointer_event_args {
    double      x      = 0.0;
    double      y      = 0.0;
    button_mask button = button_mask::none;
};

class pointer_gesture_recognizer : public internal::basic_gesture_recognizer {
public:
    pointer_gesture_recognizer() = default;

    // ----- Bindable configuration ----------------------------------------
    // Which buttons fire the press / release pair. Matches MAUI's
    // `ButtonsMask` default of `primary`.
    Observable<button_mask> buttons{button_mask::primary};

    // ----- Events --------------------------------------------------------
    mpapp::signal<const pointer_event_args&> pointer_entered{};
    mpapp::signal<const pointer_event_args&> pointer_exited{};
    mpapp::signal<const pointer_event_args&> pointer_moved{};
    mpapp::signal<const pointer_event_args&> pointer_pressed{};
    mpapp::signal<const pointer_event_args&> pointer_released{};

    // ----- Polymorphic identity ------------------------------------------
    [[nodiscard]] internal::gesture_kind kind() const noexcept override {
        return internal::gesture_kind::pointer;
    }
};

} // namespace mpapp

#endif // MPAPP_GESTURES_POINTER_GESTURE_RECOGNIZER_HPP
