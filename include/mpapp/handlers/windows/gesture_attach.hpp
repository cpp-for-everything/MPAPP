// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 wiring for the RFC-0003 gesture-recognizer family.
//
// Per-component Windows handlers call `attach(element, view)` after creating
// their native `UIElement` so every recognizer in `view.gesture_recognizers`
// gets the matching WinUI input event wired onto the element. Mirrors the
// GTK4 `linux_gestures::attach` contract.
//
// Lifetime: WinUI event tokens registered here live for the element's
// lifetime; recognizers live on `view::gesture_recognizers`, which outlives
// the platform handler (and therefore the element), so the handler lambdas
// can hold a raw pointer to the recognizer safely.

#ifndef MPAPP_HANDLERS_WINDOWS_GESTURE_ATTACH_HPP
#define MPAPP_HANDLERS_WINDOWS_GESTURE_ATTACH_HPP

#include "../../platform.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.h>

namespace mpapp {
class view;
} // namespace mpapp

namespace mpapp::internal::windows_gestures {

// Walk `v.gesture_recognizers` and wire each one onto `element` via the
// matching WinUI routed input event (Tapped / DoubleTapped / pointer +
// manipulation events). Invoke once per element setup.
void attach(winrt::Microsoft::UI::Xaml::UIElement const& element, view& v);

} // namespace mpapp::internal::windows_gestures

#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_GESTURE_ATTACH_HPP
