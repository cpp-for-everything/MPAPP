// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0010-animations.md
//
// `mpapp::animation_manager` — owns running animations and advances
// them in lock-step. Counterpart to MAUI's `AnimationManager`. A
// per-platform frame ticker (Phase 6 dispatcher) calls `tick(dt)` once
// per vsync; finished animations are dropped automatically. The mock /
// tests call `tick` directly. No macros; platform-neutral.

#ifndef MPAPP_ANIMATION_ANIMATION_MANAGER_HPP
#define MPAPP_ANIMATION_ANIMATION_MANAGER_HPP

#include <chrono>
#include <cstddef>
#include <memory>
#include <vector>

#include "animation.hpp"

namespace mpapp {

class animation_manager {
public:
    animation_manager() = default;

    animation_manager(const animation_manager&)            = delete;
    animation_manager& operator=(const animation_manager&) = delete;
    animation_manager(animation_manager&&)                 = delete;
    animation_manager& operator=(animation_manager&&)      = delete;

    ~animation_manager() = default;

    // Register an already-built animation. Returns it for chaining.
    std::shared_ptr<animation> start(std::shared_ptr<animation> a) {
        if (a && !a->finished()) {
            active_.push_back(a);
        }
        return a;
    }

    // Advance every active animation by `dt`; drop any that finished.
    void tick(std::chrono::milliseconds dt) {
        for (auto it = active_.begin(); it != active_.end();) {
            if ((*it)->advance(dt)) {
                it = active_.erase(it);
            } else {
                ++it;
            }
        }
    }

    [[nodiscard]] std::size_t active_count() const noexcept { return active_.size(); }

    void clear() noexcept { active_.clear(); }

private:
    std::vector<std::shared_ptr<animation>> active_{};
};

} // namespace mpapp

#endif // MPAPP_ANIMATION_ANIMATION_MANAGER_HPP
