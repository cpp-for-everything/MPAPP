// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0010-animations.md
//
// Extra `ViewExtensions`-style helpers operating on standalone Observables —
// no view.hpp dependency. Counterpart to the generic `AnimateTo` overloads in
// MAUI's `ViewExtensions` / `CommunityToolkit.Mvvm`.
//
//   animate_value  — drive any `Observable<double>` from its current value to
//                    a target over a duration with an easing curve.
//   color_to       — drive an `Observable<color>` with per-channel lerp.
//   animate_to<T>  — generic overload for any T that supplies a lerp helper
//                    (provided as a template parameter or callable).
//
// No macros (ADR-0002). Header-only. Platform-neutral.

#ifndef MPAPP_ANIMATION_VIEW_ANIMATIONS_EXTRA_HPP
#define MPAPP_ANIMATION_VIEW_ANIMATIONS_EXTRA_HPP

#include <chrono>
#include <functional>
#include <memory>
#include <utility>

#include "../color.hpp"
#include "../observable.hpp"
#include "animation.hpp"
#include "easing.hpp"

namespace mpapp {

// ---------------------------------------------------------------------------
// color_lerp — interpolate each channel linearly.
// ---------------------------------------------------------------------------

[[nodiscard]] inline color color_lerp(const color& from, const color& to,
                                      double t) noexcept {
    return color{
        from.r + (to.r - from.r) * t,
        from.g + (to.g - from.g) * t,
        from.b + (to.b - from.b) * t,
        from.a + (to.a - from.a) * t,
    };
}

// ---------------------------------------------------------------------------
// animate_value — drive any Observable<double>
// ---------------------------------------------------------------------------

[[nodiscard]] inline std::shared_ptr<animation>
animate_value(Observable<double>& target, double to,
              std::chrono::milliseconds duration,
              easing_kind easing = easing_kind::linear) {
    const double from = target.get();
    return std::make_shared<animation>(
        [&target](double val) { target.set(val); },
        from, to, duration, easing);
}

// ---------------------------------------------------------------------------
// color_to — drive any Observable<color> with per-channel lerp
// ---------------------------------------------------------------------------

[[nodiscard]] inline std::shared_ptr<animation>
color_to(Observable<color>& target, color to,
         std::chrono::milliseconds duration,
         easing_kind easing = easing_kind::linear) {
    const color from = target.get();
    return std::make_shared<animation>(
        [&target, from, to](double t) {
            target.set(color_lerp(from, to, t));
        },
        0.0, 1.0, duration, easing);
}

// ---------------------------------------------------------------------------
// animate_to<T> — generic overload driven by a caller-supplied lerp callable
//
//   LerpFn must model:  T fn(const T& from, const T& to, double t)
// ---------------------------------------------------------------------------

template <class T, class LerpFn>
[[nodiscard]] std::shared_ptr<animation>
animate_to(Observable<T>& target, T to,
           std::chrono::milliseconds duration,
           LerpFn lerp_fn,
           easing_kind easing = easing_kind::linear) {
    const T from = target.get();
    return std::make_shared<animation>(
        [&target, from, to, lerp_fn = std::move(lerp_fn)](double t) {
            target.set(lerp_fn(from, to, t));
        },
        0.0, 1.0, duration, easing);
}

} // namespace mpapp

#endif // MPAPP_ANIMATION_VIEW_ANIMATIONS_EXTRA_HPP
