// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0010-animations.md
//
// `mpapp::easing_kind` + `ease()` — easing curves for animations.
// Counterpart to MAUI's `Easing`. Each curve maps a normalized time
// t in [0,1] to an eased progress (most stay in [0,1]; spring/bounce
// may overshoot, matching MAUI). Pure functions, no state, no macros.

#ifndef MPAPP_ANIMATION_EASING_HPP
#define MPAPP_ANIMATION_EASING_HPP

#include <cmath>
#include <cstdint>

namespace mpapp {

enum class easing_kind : std::uint8_t {
    linear      = 0,
    sin_in      = 1,
    sin_out     = 2,
    sin_in_out  = 3,
    quad_in     = 4,
    quad_out    = 5,
    quad_in_out = 6,
    cubic_in    = 7,
    cubic_out   = 8,
    cubic_in_out= 9,
    bounce_out  = 10,
    spring_out  = 11,
};

// Clamp helper (std::clamp needs <algorithm>; keep this header light).
[[nodiscard]] inline double ease_clamp01(double t) noexcept {
    return t < 0.0 ? 0.0 : (t > 1.0 ? 1.0 : t);
}

[[nodiscard]] inline double ease(easing_kind kind, double t) noexcept {
    constexpr double pi = 3.14159265358979323846;
    t = ease_clamp01(t);
    switch (kind) {
        case easing_kind::linear:
            return t;
        case easing_kind::sin_in:
            return 1.0 - std::cos(t * pi / 2.0);
        case easing_kind::sin_out:
            return std::sin(t * pi / 2.0);
        case easing_kind::sin_in_out:
            return -(std::cos(pi * t) - 1.0) / 2.0;
        case easing_kind::quad_in:
            return t * t;
        case easing_kind::quad_out:
            return 1.0 - (1.0 - t) * (1.0 - t);
        case easing_kind::quad_in_out:
            return t < 0.5 ? 2.0 * t * t
                           : 1.0 - std::pow(-2.0 * t + 2.0, 2.0) / 2.0;
        case easing_kind::cubic_in:
            return t * t * t;
        case easing_kind::cubic_out:
            return 1.0 - std::pow(1.0 - t, 3.0);
        case easing_kind::cubic_in_out:
            return t < 0.5 ? 4.0 * t * t * t
                           : 1.0 - std::pow(-2.0 * t + 2.0, 3.0) / 2.0;
        case easing_kind::bounce_out: {
            constexpr double n1 = 7.5625;
            constexpr double d1 = 2.75;
            if (t < 1.0 / d1)        return n1 * t * t;
            if (t < 2.0 / d1)        { t -= 1.5 / d1;  return n1 * t * t + 0.75; }
            if (t < 2.5 / d1)        { t -= 2.25 / d1; return n1 * t * t + 0.9375; }
            t -= 2.625 / d1;         return n1 * t * t + 0.984375;
        }
        case easing_kind::spring_out: {
            // "Ease-out-back": overshoots above 1, then settles to exactly
            // 1 at t==1 (MAUI's SpringOut-style overshoot).
            constexpr double c1 = 1.70158;
            constexpr double c3 = c1 + 1.0;
            const double u = t - 1.0;
            return 1.0 + c3 * u * u * u + c1 * u * u;
        }
    }
    return t;
}

} // namespace mpapp

#endif // MPAPP_ANIMATION_EASING_HPP
