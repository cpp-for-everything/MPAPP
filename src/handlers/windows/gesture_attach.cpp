// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 implementation of RFC-0003 gesture recognizers.
//
// Wires the cross-platform recognizers onto a native UIElement via WinUI's
// routed input events. Tap (single + double) and Pointer (enter/exit/
// press/release/move) are wired here; pan/pinch/swipe via ManipulationDelta
// are a follow-up (they need ManipulationMode opt-in per element).
//
// The handlers use generic lambdas so the WinUI delegate argument types
// don't have to be spelled out (and their projection headers pulled in) —
// C++/WinRT instantiates each lambda with the event's concrete arg types.

#include "mpapp/handlers/windows/gesture_attach.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Input.h>   // Tapped/Pointer event-handler delegates

#include "mpapp/gestures/pointer_gesture_recognizer.hpp"
#include "mpapp/gestures/tap_gesture_recognizer.hpp"
#include "mpapp/view.hpp"

namespace mux = winrt::Microsoft::UI::Xaml;

namespace mpapp::internal::windows_gestures {

namespace {

void install_tap(mux::UIElement const& el, tap_gesture_recognizer& tap) {
    auto* t = &tap;
    el.Tapped([t](auto&&, auto&&) {
        if (t->number_of_taps_required.get() <= 1) {
            t->tapped.emit(tapped_event_args{0.0, 0.0, button_mask::primary});
        }
    });
    el.DoubleTapped([t](auto&&, auto&&) {
        if (t->number_of_taps_required.get() == 2) {
            t->tapped.emit(tapped_event_args{0.0, 0.0, button_mask::primary});
        }
    });
}

void install_pointer(mux::UIElement const& el, pointer_gesture_recognizer& p) {
    auto* ptr = &p;
    // Position is reported as (0,0) for now — WinUI's PointerRoutedEventArgs
    // GetCurrentPoint readback lands with the pan/pinch follow-up. The
    // entered/exited/pressed/released transitions (what VSM input-routing
    // needs) are exact.
    el.PointerEntered ([ptr](auto&&, auto&&) { ptr->pointer_entered.emit (pointer_event_args{0.0, 0.0, button_mask::none}); });
    el.PointerExited  ([ptr](auto&&, auto&&) { ptr->pointer_exited.emit  (pointer_event_args{0.0, 0.0, button_mask::none}); });
    el.PointerPressed ([ptr](auto&&, auto&&) { ptr->pointer_pressed.emit (pointer_event_args{0.0, 0.0, button_mask::primary}); });
    el.PointerReleased([ptr](auto&&, auto&&) { ptr->pointer_released.emit(pointer_event_args{0.0, 0.0, button_mask::primary}); });
    el.PointerMoved   ([ptr](auto&&, auto&&) { ptr->pointer_moved.emit   (pointer_event_args{0.0, 0.0, button_mask::none}); });
}

} // namespace

void attach(mux::UIElement const& element, view& v) {
    if (element == nullptr) return;
    for (const auto& r : v.gesture_recognizers) {
        switch (r->kind()) {
            case gesture_kind::tap:
                install_tap(element, static_cast<tap_gesture_recognizer&>(*r));
                break;
            case gesture_kind::pointer:
                install_pointer(element, static_cast<pointer_gesture_recognizer&>(*r));
                break;
            case gesture_kind::pan:
            case gesture_kind::pinch:
            case gesture_kind::swipe:
                // Follow-up: ManipulationMode + ManipulationDelta wiring.
                break;
        }
    }
}

} // namespace mpapp::internal::windows_gestures

#endif // _WIN32
