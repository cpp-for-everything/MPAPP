// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0010-animations.md
//
// `mpapp::animation_extensions` — a fluent builder + cancel registry that
// mirrors MAUI's `AnimationExtensions` (`this.Animate(...)` /
// `this.CancelAnimations()`). The builder queues multiple tick-driven
// steps and runs/seeks them as one composite (built on
// `composite_animation`): each step is appended over an evenly-distributed
// sub-range of the parent 0..1 timeline unless an explicit range is given.
//
// `cancel_animations(handle)` cancels a single queued/running handle, and a
// named-animation registry keyed by an owner pointer lets callers
// register/cancel animations by `(owner, name)`, matching MAUI's per-element
// animation bookkeeping. Operates generically over tick functions
// (`std::function<void(double)>`) and Observables; it does NOT require new
// `view.hpp` members. Platform-neutral, header-only, no macros (ADR-0002),
// Rule-of-Zero / =default.

#ifndef MPAPP_ANIMATION_ANIMATION_EXTENSIONS_HPP
#define MPAPP_ANIMATION_ANIMATION_EXTENSIONS_HPP

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "composite_animation.hpp"
#include "easing.hpp"

namespace mpapp {

// One queued step of a fluent animation. `tick` receives the eased local
// progress for the step (0..1). `begin_at`/`finish_at` describe the step's
// sub-range of the parent timeline; when left as the sentinel (< 0) the
// builder distributes the step evenly across the timeline at build time.
struct animation_step {
    composite_animation::child_tick_fn tick;
    double                             begin_at = -1.0;
    double                             finish_at = -1.0;
    easing_kind                        easing = easing_kind::linear;
};

// The lifecycle state of a fluent animation handle.
enum class animation_status : std::uint8_t {
    pending   = 0,  // queued, not yet driven to completion
    running   = 1,  // seek/advance in progress (>0, <1)
    completed = 2,  // driven to >= 1.0
    cancelled = 3,  // cancel() / cancel_animations() called
};

// A runnable fluent animation: a frozen `composite_animation` plus its
// status. Seek drives every queued step; once cancelled it no longer ticks.
class animation_handle {
public:
    explicit animation_handle(composite_animation composite)
        : composite_{ std::move(composite) } {}

    // Drive the whole composite to `global_progress` (clamped 0..1). A
    // cancelled handle is inert. Updates status to running/completed.
    void seek(double global_progress) {
        if (status_ == animation_status::cancelled) {
            return;
        }
        const double g = ease_clamp01(global_progress);
        composite_.seek(g);
        status_ = g >= 1.0 ? animation_status::completed
                           : animation_status::running;
    }

    // Convenience: drive straight to the end (the common MAUI "run it").
    void run() { seek(1.0); }

    // Cancel: stop ticking. Idempotent. Returns false if already terminal.
    bool cancel() noexcept {
        if (status_ == animation_status::cancelled
            || status_ == animation_status::completed) {
            return false;
        }
        status_ = animation_status::cancelled;
        return true;
    }

    [[nodiscard]] animation_status status() const noexcept { return status_; }
    [[nodiscard]] bool             cancelled() const noexcept {
        return status_ == animation_status::cancelled;
    }
    [[nodiscard]] bool finished() const noexcept {
        return status_ == animation_status::completed;
    }
    [[nodiscard]] std::size_t step_count() const noexcept {
        return composite_.child_count();
    }

private:
    composite_animation composite_;
    animation_status    status_ = animation_status::pending;
};

// Fluent builder. Queue steps with `add(...)`, then `build()` to freeze them
// into an `animation_handle`. Steps without an explicit range are spread
// evenly across the parent 0..1 timeline (step i of n owns [i/n, (i+1)/n]).
class animation_builder {
public:
    animation_builder() = default;

    // Append a step over an auto-distributed sub-range.
    animation_builder& add(composite_animation::child_tick_fn tick,
                           easing_kind easing = easing_kind::linear) {
        steps_.push_back(animation_step{ std::move(tick), -1.0, -1.0, easing });
        return *this;
    }

    // Append a step over an explicit [begin_at, finish_at] sub-range.
    animation_builder& add_at(double begin_at, double finish_at,
                              composite_animation::child_tick_fn tick,
                              easing_kind easing = easing_kind::linear) {
        steps_.push_back(animation_step{ std::move(tick), begin_at, finish_at,
                                         easing });
        return *this;
    }

    [[nodiscard]] std::size_t step_count() const noexcept {
        return steps_.size();
    }

    // Freeze the queued steps into a runnable handle. Auto-ranged steps are
    // distributed evenly across the timeline in queue order.
    [[nodiscard]] std::shared_ptr<animation_handle> build() const {
        composite_animation composite;
        const std::size_t n = steps_.size();
        std::size_t auto_index = 0;
        for (const auto& s : steps_) {
            double begin_at = s.begin_at;
            double finish_at = s.finish_at;
            if (begin_at < 0.0 || finish_at < 0.0) {
                const double slice = n == 0 ? 1.0
                                            : 1.0 / static_cast<double>(n);
                begin_at = static_cast<double>(auto_index) * slice;
                finish_at = static_cast<double>(auto_index + 1) * slice;
                ++auto_index;
            }
            composite.add(begin_at, finish_at, s.tick, s.easing);
        }
        return std::make_shared<animation_handle>(std::move(composite));
    }

private:
    std::vector<animation_step> steps_{};
};

// Cancel a single handle (MAUI's per-animation cancel). Null-safe; returns
// whether the handle transitioned to cancelled.
inline bool cancel_animations(const std::shared_ptr<animation_handle>& handle)
    noexcept {
    return handle ? handle->cancel() : false;
}

// Named-animation registry keyed by an opaque owner pointer, mirroring
// MAUI's per-element animation table (`AnimationExtensions` stores running
// animations on the animatable). `register_animation(owner, name, handle)`
// records a handle; `cancel(owner, name)` cancels + removes one;
// `cancel(owner)` cancels + removes every animation for that owner. The
// owner pointer is treated as an opaque key — the registry never
// dereferences it, so any owner type is supported.
class animation_registry {
public:
    animation_registry() = default;

    animation_registry(const animation_registry&)            = delete;
    animation_registry& operator=(const animation_registry&) = delete;
    animation_registry(animation_registry&&)                 = delete;
    animation_registry& operator=(animation_registry&&)      = delete;

    ~animation_registry() = default;

    // Register (or replace) the animation named `name` for `owner`. A
    // replaced animation is cancelled first. Null owner/handle are ignored.
    void register_animation(const void* owner, std::string name,
                            std::shared_ptr<animation_handle> handle) {
        if (owner == nullptr || handle == nullptr) {
            return;
        }
        auto& named = owners_[owner];
        if (auto it = named.find(name); it != named.end()) {
            if (it->second) {
                it->second->cancel();
            }
            it->second = std::move(handle);
            return;
        }
        named.emplace(std::move(name), std::move(handle));
    }

    // Cancel + remove the single named animation. Returns true if one was
    // found (regardless of whether it was already terminal).
    bool cancel(const void* owner, const std::string& name) {
        auto owner_it = owners_.find(owner);
        if (owner_it == owners_.end()) {
            return false;
        }
        auto& named = owner_it->second;
        auto name_it = named.find(name);
        if (name_it == named.end()) {
            return false;
        }
        if (name_it->second) {
            name_it->second->cancel();
        }
        named.erase(name_it);
        if (named.empty()) {
            owners_.erase(owner_it);
        }
        return true;
    }

    // Cancel + remove every animation for `owner`. Returns the number of
    // animations that were removed.
    std::size_t cancel(const void* owner) {
        auto owner_it = owners_.find(owner);
        if (owner_it == owners_.end()) {
            return 0;
        }
        std::size_t removed = 0;
        for (auto& [name, handle] : owner_it->second) {
            if (handle) {
                handle->cancel();
            }
            ++removed;
        }
        owners_.erase(owner_it);
        return removed;
    }

    // Look up a registered animation; nullptr if absent.
    [[nodiscard]] std::shared_ptr<animation_handle>
    find(const void* owner, const std::string& name) const {
        auto owner_it = owners_.find(owner);
        if (owner_it == owners_.end()) {
            return nullptr;
        }
        auto name_it = owner_it->second.find(name);
        return name_it == owner_it->second.end() ? nullptr : name_it->second;
    }

    [[nodiscard]] bool has(const void* owner, const std::string& name) const {
        return find(owner, name) != nullptr;
    }

    // Number of named animations registered for `owner`.
    [[nodiscard]] std::size_t count(const void* owner) const {
        auto owner_it = owners_.find(owner);
        return owner_it == owners_.end() ? 0 : owner_it->second.size();
    }

    // Total number of owners with at least one registered animation.
    [[nodiscard]] std::size_t owner_count() const noexcept {
        return owners_.size();
    }

    void clear() noexcept { owners_.clear(); }

private:
    using named_map =
        std::unordered_map<std::string, std::shared_ptr<animation_handle>>;
    std::unordered_map<const void*, named_map> owners_{};
};

} // namespace mpapp

#endif // MPAPP_ANIMATION_ANIMATION_EXTENSIONS_HPP
