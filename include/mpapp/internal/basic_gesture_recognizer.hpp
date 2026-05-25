// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0003-gesture-recognizers.md
//
// `mpapp::internal::basic_gesture_recognizer` — abstract polymorphic base
// for the gesture-recognizer family (tap / pan / pinch / swipe / pointer).
// Concrete recognizers live in `mpapp::` (not `mpapp::internal::`) because
// they are user-facing configuration objects, not platform-agnostic
// widget surfaces — the `internal::` namespace here is only for the
// polymorphic base that `view::gesture_recognizers` stores by
// `shared_ptr<basic_gesture_recognizer>`.
//
// Recognizers do NOT use the wrapper-component pattern from
// ADR-0024: they own no native widget and have no embedded handler.
// They are configuration + signals; the attached `view`'s handler
// wires native input events to the recognizer's signals via
// `view_handler<P>::map_gestures(view&)`.

#ifndef MPAPP_INTERNAL_BASIC_GESTURE_RECOGNIZER_HPP
#define MPAPP_INTERNAL_BASIC_GESTURE_RECOGNIZER_HPP

#include <cstdint>

namespace mpapp::internal {

// Closed-set discriminator. The platform-handler's `map_gestures` walks
// `view.gesture_recognizers` and dispatches on `kind()` to install the
// right native listener (`UITapGestureRecognizer`, `GtkGestureClick`,
// `UIElement.Tapped`, ...). Closed-set rather than open polymorphic
// dispatch because the family is bounded by MAUI's surface and adding
// a new kind is an RFC-level event, not an extension point.
enum class gesture_kind : std::uint8_t {
    tap     = 0,
    pan     = 1,
    pinch   = 2,
    swipe   = 3,
    pointer = 4,
};

// Lifecycle status of a multi-stage gesture (pan / pinch). MAUI's
// `GestureStatus` 1:1. Tap and swipe are single-shot and don't surface
// status; they fire once when the recognizer detects them.
enum class gesture_status : std::uint8_t {
    started   = 0,
    running   = 1,
    completed = 2,
    canceled  = 3,
};

class basic_gesture_recognizer {
public:
    virtual ~basic_gesture_recognizer() = default;

    basic_gesture_recognizer(const basic_gesture_recognizer&)            = delete;
    basic_gesture_recognizer& operator=(const basic_gesture_recognizer&) = delete;
    basic_gesture_recognizer(basic_gesture_recognizer&&)                 = delete;
    basic_gesture_recognizer& operator=(basic_gesture_recognizer&&)      = delete;

    [[nodiscard]] virtual gesture_kind kind() const noexcept = 0;

protected:
    basic_gesture_recognizer() = default;
};

} // namespace mpapp::internal

#endif // MPAPP_INTERNAL_BASIC_GESTURE_RECOGNIZER_HPP
