// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0010-animations.md
//
// `mpapp::composite_animation` — a parent timeline that schedules child
// animations across normalized sub-ranges of itself. Counterpart to
// MAUI's `Animation.Add(beginAt, finishAt, childAnimation)`: each child
// owns a [begin_at, finish_at] slice of the parent's 0..1 progress.
//
// `seek(global_progress)` maps the parent's global 0..1 onto every
// child's local 0..1:
//   * before begin_at      -> the child receives 0 (not yet started)
//   * after  finish_at     -> the child receives 1 (already finished)
//   * within [begin_at, finish_at] -> remapped to local 0..1, then eased,
//     and the child's tick is invoked with the eased local progress.
//
// The mapping is platform-NEUTRAL + deterministic, so a staggered
// timeline is fully verifiable without a live event loop. No macros
// (ADR-0002); header-only; Rule-of-Zero.

#ifndef MPAPP_ANIMATION_COMPOSITE_ANIMATION_HPP
#define MPAPP_ANIMATION_COMPOSITE_ANIMATION_HPP

#include <functional>
#include <utility>
#include <vector>

#include "easing.hpp"

namespace mpapp {

class composite_animation {
public:
    using child_tick_fn = std::function<void(double)>;  // receives eased local progress

    composite_animation() = default;

    // Schedule `child_tick` over the parent sub-range [begin_at, finish_at]
    // (both in 0..1). The local progress fed to the child is eased with
    // `easing`. `begin_at`/`finish_at` are clamped to [0,1]; a degenerate
    // range (finish_at <= begin_at) makes the child a step that snaps to 1
    // for any global progress at/after begin_at.
    void add(double begin_at, double finish_at, child_tick_fn child_tick,
             easing_kind easing = easing_kind::linear) {
        children_.push_back(child{ ease_clamp01(begin_at),
                                   ease_clamp01(finish_at),
                                   std::move(child_tick), easing });
    }

    // Map the parent's global progress (clamped to 0..1) onto every child
    // and invoke each child's tick with its eased local progress.
    void seek(double global_progress) {
        const double g = ease_clamp01(global_progress);
        for (const auto& c : children_) {
            const double local = local_progress(c, g);
            if (c.tick) {
                c.tick(ease(c.easing, local));
            }
        }
    }

    [[nodiscard]] std::size_t child_count() const noexcept { return children_.size(); }

    void clear() noexcept { children_.clear(); }

private:
    struct child {
        double        begin_at;
        double        finish_at;
        child_tick_fn tick;
        easing_kind   easing;
    };

    // Raw (un-eased) local progress for a child at global progress `g`.
    [[nodiscard]] static double local_progress(const child& c, double g) noexcept {
        if (g <= c.begin_at) {
            return 0.0;
        }
        if (g >= c.finish_at) {
            return 1.0;
        }
        const double span = c.finish_at - c.begin_at;
        // span > 0 here: g is strictly between begin_at and finish_at.
        return (g - c.begin_at) / span;
    }

    std::vector<child> children_{};
};

} // namespace mpapp

#endif // MPAPP_ANIMATION_COMPOSITE_ANIMATION_HPP
