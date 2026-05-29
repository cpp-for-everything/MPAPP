// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0010-animations.md
//
// `mpapp::animation` — a single tweened value over a duration with an
// easing curve. Counterpart to MAUI's `Animation`. The interpolation is
// platform-NEUTRAL + deterministic; a driver advances it by elapsed
// time (`advance(dt)`). The real frame ticker that calls `advance` each
// vsync is the per-platform dispatcher (Phase 6 / ADR-0019); tests +
// the `animation_manager` mock drive it explicitly, so the curve is
// fully verifiable without a live event loop.
//
// No macros (ADR-0009); not a wrapper-component.

#ifndef MPAPP_ANIMATION_ANIMATION_HPP
#define MPAPP_ANIMATION_ANIMATION_HPP

#include <chrono>
#include <functional>
#include <utility>

#include "easing.hpp"

namespace mpapp {

class animation {
public:
    using tick_fn     = std::function<void(double)>;  // receives interpolated value
    using finished_fn = std::function<void()>;

    animation(tick_fn on_tick, double from, double to,
              std::chrono::milliseconds duration,
              easing_kind easing = easing_kind::linear,
              finished_fn on_finished = {})
        : on_tick_{ std::move(on_tick) }
        , on_finished_{ std::move(on_finished) }
        , from_{ from }
        , to_{ to }
        , duration_{ duration }
        , easing_{ easing } {
        emit(0.0);  // seed the starting value
    }

    // Advance by `dt`. Calls on_tick with the interpolated value, and
    // on_finished once when it reaches the end. Returns true once the
    // animation has finished (so a manager can drop it).
    bool advance(std::chrono::milliseconds dt) {
        if (finished_) {
            return true;
        }
        elapsed_ += dt;
        const double progress = duration_.count() <= 0
            ? 1.0
            : static_cast<double>(elapsed_.count()) /
                  static_cast<double>(duration_.count());
        if (progress >= 1.0) {
            emit(1.0);
            finished_ = true;
            if (on_finished_) {
                on_finished_();
            }
            return true;
        }
        emit(progress);
        return false;
    }

    // Jump to a normalized progress in [0,1] without affecting the
    // finished flag (useful for scrubbing + tests).
    void seek(double progress01) { emit(ease_clamp01(progress01)); }

    [[nodiscard]] bool   finished() const noexcept { return finished_; }
    [[nodiscard]] double current() const noexcept { return current_; }

private:
    void emit(double progress) {
        const double eased = ease(easing_, progress);
        current_ = from_ + (to_ - from_) * eased;
        if (on_tick_) {
            on_tick_(current_);
        }
    }

    tick_fn                   on_tick_;
    finished_fn               on_finished_;
    double                    from_;
    double                    to_;
    std::chrono::milliseconds duration_;
    easing_kind               easing_;
    std::chrono::milliseconds elapsed_{ 0 };
    double                    current_ = 0.0;
    bool                      finished_ = false;
};

} // namespace mpapp

#endif // MPAPP_ANIMATION_ANIMATION_HPP
