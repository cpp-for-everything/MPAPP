// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0003-gesture-recognizers.md
//
// `mpapp::swipe_gesture_recognizer` — counterpart to MAUI's
// `SwipeGestureRecognizer`. Single-shot: fires `swiped` once when the
// platform detects a fling whose direction matches one of the bits in
// the recognizer's `direction` property (a flag enum; the same
// recognizer can match multiple directions by `|`-ing them together).
//
// XAML (1:1 with MAUI per ADR-0004):
//
//     <Border>
//         <Border.GestureRecognizers>
//             <SwipeGestureRecognizer
//                 Direction="Left,Right"
//                 Threshold="120"
//                 Swiped="OnSwiped"/>
//         </Border.GestureRecognizers>
//     </Border>

#ifndef MPAPP_GESTURES_SWIPE_GESTURE_RECOGNIZER_HPP
#define MPAPP_GESTURES_SWIPE_GESTURE_RECOGNIZER_HPP

#include <cstdint>

#include "../internal/basic_gesture_recognizer.hpp"
#include "../observable.hpp"
#include "../signal.hpp"

namespace mpapp {

// Flag enum matching MAUI's `SwipeDirection` bit values exactly so the
// XAML compiler can lower `Direction="Left,Right"` 1:1.
enum class swipe_direction : std::uint8_t {
    none  = 0,
    right = 1 << 0,
    left  = 1 << 1,
    up    = 1 << 2,
    down  = 1 << 3,
};

constexpr swipe_direction operator|(swipe_direction a, swipe_direction b) noexcept {
    return static_cast<swipe_direction>(
        static_cast<std::uint8_t>(a) | static_cast<std::uint8_t>(b));
}
constexpr swipe_direction operator&(swipe_direction a, swipe_direction b) noexcept {
    return static_cast<swipe_direction>(
        static_cast<std::uint8_t>(a) & static_cast<std::uint8_t>(b));
}
constexpr bool any(swipe_direction m, swipe_direction probe) noexcept {
    return static_cast<std::uint8_t>(m & probe) != 0;
}

// Event payload for a satisfied swipe. The recognizer reports the
// single direction that triggered it (not the full bitmask of
// configured directions). If multiple directions are configured and
// the platform fires for one, only that bit is set in `direction`.
struct swiped_event_args {
    swipe_direction direction = swipe_direction::none;
};

class swipe_gesture_recognizer : public internal::basic_gesture_recognizer {
public:
    swipe_gesture_recognizer() = default;

    // ----- Bindable configuration ----------------------------------------
    // Bitmask of directions that satisfy this recognizer. Default is
    // `right` to match MAUI's `default(SwipeDirection)` = 1.
    Observable<swipe_direction> direction{swipe_direction::right};

    // Minimum fling distance (in view-local pixels) required for the
    // recognizer to consider the gesture a swipe. MAUI's default is
    // 100 — matched here.
    Observable<unsigned> threshold{100};

    // ----- Event ---------------------------------------------------------
    mpapp::signal<const swiped_event_args&> swiped{};

    // ----- Polymorphic identity ------------------------------------------
    [[nodiscard]] internal::gesture_kind kind() const noexcept override {
        return internal::gesture_kind::swipe;
    }
};

} // namespace mpapp

#endif // MPAPP_GESTURES_SWIPE_GESTURE_RECOGNIZER_HPP
