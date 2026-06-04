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
    // --- appended: full MAUI easing set ---
    quart_in     = 12,
    quart_out    = 13,
    quart_in_out = 14,
    quint_in     = 15,
    quint_out    = 16,
    quint_in_out = 17,
    expo_in      = 18,
    expo_out     = 19,
    expo_in_out  = 20,
    circ_in      = 21,
    circ_out     = 22,
    circ_in_out  = 23,
    back_in      = 24,
    back_out     = 25,
    back_in_out  = 26,
    elastic_in   = 27,
    elastic_out  = 28,
    elastic_in_out = 29,
    bounce_in    = 30,
    bounce_in_out = 31,
    spring_in    = 32,
    spring_in_out = 33,
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

        // --- quart ---
        case easing_kind::quart_in:
            return t * t * t * t;
        case easing_kind::quart_out: {
            const double u = 1.0 - t;
            return 1.0 - u * u * u * u;
        }
        case easing_kind::quart_in_out:
            return t < 0.5 ? 8.0 * t * t * t * t
                           : 1.0 - std::pow(-2.0 * t + 2.0, 4.0) / 2.0;

        // --- quint ---
        case easing_kind::quint_in:
            return t * t * t * t * t;
        case easing_kind::quint_out: {
            const double u = 1.0 - t;
            return 1.0 - u * u * u * u * u;
        }
        case easing_kind::quint_in_out:
            return t < 0.5 ? 16.0 * t * t * t * t * t
                           : 1.0 - std::pow(-2.0 * t + 2.0, 5.0) / 2.0;

        // --- expo ---
        case easing_kind::expo_in:
            return t == 0.0 ? 0.0 : std::pow(2.0, 10.0 * t - 10.0);
        case easing_kind::expo_out:
            return t == 1.0 ? 1.0 : 1.0 - std::pow(2.0, -10.0 * t);
        case easing_kind::expo_in_out:
            if (t == 0.0) return 0.0;
            if (t == 1.0) return 1.0;
            return t < 0.5 ? std::pow(2.0, 20.0 * t - 10.0) / 2.0
                           : (2.0 - std::pow(2.0, -20.0 * t + 10.0)) / 2.0;

        // --- circ ---
        case easing_kind::circ_in:
            return 1.0 - std::sqrt(1.0 - t * t);
        case easing_kind::circ_out:
            return std::sqrt(1.0 - (t - 1.0) * (t - 1.0));
        case easing_kind::circ_in_out:
            return t < 0.5
                       ? (1.0 - std::sqrt(1.0 - 4.0 * t * t)) / 2.0
                       : (std::sqrt(1.0 - std::pow(-2.0 * t + 2.0, 2.0)) + 1.0) / 2.0;

        // --- back ---
        case easing_kind::back_in: {
            constexpr double c1 = 1.70158;
            constexpr double c3 = c1 + 1.0;
            return c3 * t * t * t - c1 * t * t;
        }
        case easing_kind::back_out: {
            constexpr double c1 = 1.70158;
            constexpr double c3 = c1 + 1.0;
            const double u = t - 1.0;
            return 1.0 + c3 * u * u * u + c1 * u * u;
        }
        case easing_kind::back_in_out: {
            constexpr double c1 = 1.70158;
            constexpr double c2 = c1 * 1.525;
            return t < 0.5
                       ? (std::pow(2.0 * t, 2.0) * ((c2 + 1.0) * 2.0 * t - c2)) / 2.0
                       : (std::pow(2.0 * t - 2.0, 2.0) * ((c2 + 1.0) * (2.0 * t - 2.0) + c2) + 2.0) / 2.0;
        }

        // --- elastic ---
        case easing_kind::elastic_in: {
            if (t == 0.0) return 0.0;
            if (t == 1.0) return 1.0;
            constexpr double c4 = (2.0 * 3.14159265358979323846) / 3.0;
            return -std::pow(2.0, 10.0 * t - 10.0) * std::sin((t * 10.0 - 10.75) * c4);
        }
        case easing_kind::elastic_out: {
            if (t == 0.0) return 0.0;
            if (t == 1.0) return 1.0;
            constexpr double c4 = (2.0 * 3.14159265358979323846) / 3.0;
            return std::pow(2.0, -10.0 * t) * std::sin((t * 10.0 - 0.75) * c4) + 1.0;
        }
        case easing_kind::elastic_in_out: {
            if (t == 0.0) return 0.0;
            if (t == 1.0) return 1.0;
            constexpr double c5 = (2.0 * 3.14159265358979323846) / 4.5;
            return t < 0.5
                       ? -(std::pow(2.0, 20.0 * t - 10.0) * std::sin((20.0 * t - 11.125) * c5)) / 2.0
                       : (std::pow(2.0, -20.0 * t + 10.0) * std::sin((20.0 * t - 11.125) * c5)) / 2.0 + 1.0;
        }

        // --- bounce ---
        case easing_kind::bounce_in:
            // bounce_in(t) = 1 - bounce_out(1-t)
            {
                double u = 1.0 - t;
                constexpr double n1 = 7.5625;
                constexpr double d1 = 2.75;
                double b = 0.0;
                if (u < 1.0 / d1)        b = n1 * u * u;
                else if (u < 2.0 / d1) { u -= 1.5 / d1;  b = n1 * u * u + 0.75; }
                else if (u < 2.5 / d1) { u -= 2.25 / d1; b = n1 * u * u + 0.9375; }
                else                   { u -= 2.625 / d1; b = n1 * u * u + 0.984375; }
                return 1.0 - b;
            }
        case easing_kind::bounce_in_out: {
            if (t < 0.5) {
                // bounce_in(2t)/2
                double u = 1.0 - 2.0 * t;
                constexpr double n1 = 7.5625;
                constexpr double d1 = 2.75;
                double b = 0.0;
                if (u < 1.0 / d1)        b = n1 * u * u;
                else if (u < 2.0 / d1) { u -= 1.5 / d1;  b = n1 * u * u + 0.75; }
                else if (u < 2.5 / d1) { u -= 2.25 / d1; b = n1 * u * u + 0.9375; }
                else                   { u -= 2.625 / d1; b = n1 * u * u + 0.984375; }
                return (1.0 - b) / 2.0;
            } else {
                // (1 + bounce_out(2t-1))/2
                double u = 2.0 * t - 1.0;
                constexpr double n1 = 7.5625;
                constexpr double d1 = 2.75;
                double b = 0.0;
                if (u < 1.0 / d1)        b = n1 * u * u;
                else if (u < 2.0 / d1) { u -= 1.5 / d1;  b = n1 * u * u + 0.75; }
                else if (u < 2.5 / d1) { u -= 2.25 / d1; b = n1 * u * u + 0.9375; }
                else                   { u -= 2.625 / d1; b = n1 * u * u + 0.984375; }
                return (1.0 + b) / 2.0;
            }
        }

        // --- spring ---
        case easing_kind::spring_in: {
            // Ease-in-back: starts by pulling back, then accelerates forward.
            constexpr double c1 = 1.70158;
            constexpr double c3 = c1 + 1.0;
            return c3 * t * t * t - c1 * t * t;
        }
        case easing_kind::spring_in_out: {
            // Ease-in-out-back: undershoot on first half, overshoot on second.
            constexpr double c1 = 1.70158;
            constexpr double c2 = c1 * 1.525;
            return t < 0.5
                       ? (std::pow(2.0 * t, 2.0) * ((c2 + 1.0) * 2.0 * t - c2)) / 2.0
                       : (std::pow(2.0 * t - 2.0, 2.0) * ((c2 + 1.0) * (2.0 * t - 2.0) + c2) + 2.0) / 2.0;
        }
    }
    return t;
}

} // namespace mpapp

#endif // MPAPP_ANIMATION_EASING_HPP
