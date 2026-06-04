// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0010-animations.md
//
// `mpapp::repeating_animation` — wraps a tick function and drives it
// over multiple cycles, mirroring MAUI's repeat / loop / auto-reverse
// semantics. Each cycle maps elapsed time onto a normalized progress in
// [0,1]; with `auto_reverse` enabled, odd-numbered cycles play 1..0 so a
// round-trip "ping-pongs". `repeat_count` of -1 loops forever.
//
// The wrapper is platform-NEUTRAL + deterministic: a driver advances it
// by elapsed time (`advance(dt)`), exactly like `mpapp::animation`. No
// macros (ADR-0009); not a wrapper-component.

#ifndef MPAPP_ANIMATION_ANIMATION_REPEAT_HPP
#define MPAPP_ANIMATION_ANIMATION_REPEAT_HPP

#include <chrono>
#include <functional>
#include <utility>

namespace mpapp {

class repeating_animation {
public:
    using tick_fn     = std::function<void(double)>;  // receives cycle progress in [0,1]
    using finished_fn = std::function<void()>;

    // `repeat_count` is the number of cycles to play; -1 means infinite.
    // A `duration` of <= 0 collapses each cycle to a single end-frame.
    repeating_animation(tick_fn on_tick, std::chrono::milliseconds duration,
                        int repeat_count = 1, bool auto_reverse = false,
                        finished_fn on_finished = {})
        : on_tick_{ std::move(on_tick) }
        , on_finished_{ std::move(on_finished) }
        , duration_{ duration }
        , repeat_count_{ repeat_count }
        , auto_reverse_{ auto_reverse } {
        emit(0.0);  // seed the starting value of the first cycle
    }

    // Advance by `dt`. Calls on_tick with each cycle's (possibly reversed)
    // progress, fires on_finished once when all cycles complete, and
    // returns true once the animation has finished (so a manager can drop
    // it). Infinite animations (repeat_count == -1) never return true.
    bool advance(std::chrono::milliseconds dt) {
        if (finished_) {
            return true;
        }
        elapsed_ += dt;
        for (;;) {
            const double progress = cycle_progress();
            if (progress < 1.0) {
                emit(map_cycle(progress));
                return false;
            }
            // Current cycle reached its end: emit the boundary, count it.
            emit(map_cycle(1.0));
            ++completed_cycles_;
            if (is_infinite()) {
                roll_over_cycle();
                continue;
            }
            if (completed_cycles_ >= repeat_count_) {
                finished_ = true;
                if (on_finished_) {
                    on_finished_();
                }
                return true;
            }
            roll_over_cycle();
        }
    }

    // Cancel without firing on_finished; further advances are no-ops.
    void cancel() noexcept { finished_ = true; }

    [[nodiscard]] bool is_finished() const noexcept { return finished_; }
    [[nodiscard]] int  completed_cycles() const noexcept { return completed_cycles_; }
    [[nodiscard]] double current() const noexcept { return current_; }

private:
    [[nodiscard]] bool is_infinite() const noexcept { return repeat_count_ < 0; }

    // Normalized progress within the current cycle; >= 1.0 once the cycle
    // has elapsed (duration <= 0 collapses immediately to 1.0).
    [[nodiscard]] double cycle_progress() const noexcept {
        if (duration_.count() <= 0) {
            return 1.0;
        }
        return static_cast<double>(elapsed_.count()) /
               static_cast<double>(duration_.count());
    }

    // Map a forward cycle progress to the emitted value, reversing on odd
    // cycles when auto_reverse is enabled (ping-pong).
    [[nodiscard]] double map_cycle(double progress) const noexcept {
        if (auto_reverse_ && (completed_cycles_ % 2 != 0)) {
            return 1.0 - progress;
        }
        return progress;
    }

    // Subtract one cycle's worth of elapsed time so leftover time carries
    // into the next cycle, then seed the next cycle's starting frame.
    void roll_over_cycle() {
        if (duration_.count() > 0) {
            elapsed_ -= duration_;
        } else {
            elapsed_ = std::chrono::milliseconds{ 0 };
        }
        emit(map_cycle(0.0));
    }

    void emit(double value) {
        current_ = value;
        if (on_tick_) {
            on_tick_(value);
        }
    }

    tick_fn                   on_tick_;
    finished_fn               on_finished_;
    std::chrono::milliseconds duration_;
    int                       repeat_count_;
    bool                      auto_reverse_;
    std::chrono::milliseconds elapsed_{ 0 };
    int                       completed_cycles_ = 0;
    double                    current_ = 0.0;
    bool                      finished_ = false;
};

} // namespace mpapp

#endif // MPAPP_ANIMATION_ANIMATION_REPEAT_HPP
