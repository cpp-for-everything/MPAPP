// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0003-gesture-recognizers.md
//
// `mpapp::tap_gesture_recognizer` — counterpart to MAUI's
// `TapGestureRecognizer`. Attaches to any `mpapp::view` via
// `view::add_gesture<tap_gesture_recognizer>()` (or by emplacing into
// `view::gesture_recognizers` directly). Carries the same bindable
// configuration as MAUI:
//
//   * `number_of_taps_required`  — 1 = single tap, 2 = double tap, ...
//   * `buttons`                  — mask of mouse / pen buttons that
//                                  qualify (`primary`, `secondary`, ...).
//
// Fires `tapped(const tapped_event_args&)` when the platform handler's
// native listener decides a qualifying tap occurred. The position in
// `tapped_event_args` is in view-local pixel coordinates.
//
// XAML (must compile 1:1 with MAUI per ADR-0004):
//
//     <Button Text="Tap me">
//         <Button.GestureRecognizers>
//             <TapGestureRecognizer Tapped="OnTapped"
//                                   NumberOfTapsRequired="2"
//                                   Buttons="Primary,Secondary"/>
//         </Button.GestureRecognizers>
//     </Button>

#ifndef MPAPP_GESTURES_TAP_GESTURE_RECOGNIZER_HPP
#define MPAPP_GESTURES_TAP_GESTURE_RECOGNIZER_HPP

#include <cstdint>

#include "../internal/basic_gesture_recognizer.hpp"
#include "../observable.hpp"
#include "../signal.hpp"

namespace mpapp {

// Closed-set bitmask of pointer buttons a recognizer matches against.
// Mirrors MAUI's `ButtonsMask` ([Primary, Secondary]); other bits land
// as the platform gesture-manager grows (middle button, X1/X2, pen
// barrel, ...). The default `primary` means "left mouse button, single
// touch, or pen tip" on every platform.
enum class button_mask : std::uint8_t {
    none      = 0,
    primary   = 1 << 0,   // left mouse / single touch / pen tip
    secondary = 1 << 1,   // right mouse / pen barrel
};

constexpr button_mask operator|(button_mask a, button_mask b) noexcept {
    return static_cast<button_mask>(
        static_cast<std::uint8_t>(a) | static_cast<std::uint8_t>(b));
}
constexpr button_mask operator&(button_mask a, button_mask b) noexcept {
    return static_cast<button_mask>(
        static_cast<std::uint8_t>(a) & static_cast<std::uint8_t>(b));
}
constexpr bool any(button_mask m, button_mask probe) noexcept {
    return static_cast<std::uint8_t>(m & probe) != 0;
}

// Event payload for a satisfied tap. Position is in the attached
// view's local pixel coordinate system (top-left origin). `buttons`
// reports which buttons triggered the tap — useful for distinguishing
// left vs. right click bindings on the same recognizer.
struct tapped_event_args {
    double      x       = 0.0;
    double      y       = 0.0;
    button_mask buttons = button_mask::primary;
};

class tap_gesture_recognizer : public internal::basic_gesture_recognizer {
public:
    tap_gesture_recognizer() = default;

    // ----- Bindable configuration ----------------------------------------

    Observable<int>         number_of_taps_required{1};
    Observable<button_mask> buttons{button_mask::primary};

    // ----- Event ---------------------------------------------------------

    mpapp::signal<const tapped_event_args&> tapped{};

    // ----- Polymorphic identity ------------------------------------------

    [[nodiscard]] internal::gesture_kind kind() const noexcept override {
        return internal::gesture_kind::tap;
    }
};

} // namespace mpapp

#endif // MPAPP_GESTURES_TAP_GESTURE_RECOGNIZER_HPP
