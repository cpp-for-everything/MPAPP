// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0010-animations.md
//
// `ViewExtensions`-style animation helpers: `fade_to`, `scale_to`,
// `rotate_to`, `translate_to`. Counterpart to MAUI's `ViewExtensions`.
// Each builds an `animation` driving the matching `view` Observable
// (opacity / scale / rotation / translation_x+y) from its current value
// to the target. Register the result with an `animation_manager` (or
// advance it directly) to run it. No macros; platform-neutral.

#ifndef MPAPP_ANIMATION_VIEW_ANIMATIONS_HPP
#define MPAPP_ANIMATION_VIEW_ANIMATIONS_HPP

#include <chrono>
#include <memory>

#include "../view.hpp"
#include "animation.hpp"
#include "easing.hpp"

namespace mpapp {

[[nodiscard]] inline std::shared_ptr<animation>
fade_to(view& v, double opacity, std::chrono::milliseconds duration,
        easing_kind easing = easing_kind::linear) {
    const double from = v.opacity.get();
    return std::make_shared<animation>(
        [&v](double val) { v.opacity.set(val); },
        from, opacity, duration, easing);
}

[[nodiscard]] inline std::shared_ptr<animation>
scale_to(view& v, double scale, std::chrono::milliseconds duration,
         easing_kind easing = easing_kind::linear) {
    const double from = v.scale.get();
    return std::make_shared<animation>(
        [&v](double val) { v.scale.set(val); },
        from, scale, duration, easing);
}

[[nodiscard]] inline std::shared_ptr<animation>
rotate_to(view& v, double rotation, std::chrono::milliseconds duration,
          easing_kind easing = easing_kind::linear) {
    const double from = v.rotation.get();
    return std::make_shared<animation>(
        [&v](double val) { v.rotation.set(val); },
        from, rotation, duration, easing);
}

// Animates translation_x + translation_y together: the animation runs an
// eased 0->1 progress and the tick interpolates both axes.
[[nodiscard]] inline std::shared_ptr<animation>
translate_to(view& v, double x, double y, std::chrono::milliseconds duration,
             easing_kind easing = easing_kind::linear) {
    const double fx = v.translation_x.get();
    const double fy = v.translation_y.get();
    return std::make_shared<animation>(
        [&v, fx, fy, x, y](double t) {
            v.translation_x.set(fx + (x - fx) * t);
            v.translation_y.set(fy + (y - fy) * t);
        },
        0.0, 1.0, duration, easing);
}

} // namespace mpapp

#endif // MPAPP_ANIMATION_VIEW_ANIMATIONS_HPP
