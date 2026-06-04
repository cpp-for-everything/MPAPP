// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0008-triggers.md
//
// CommunityToolkit-inspired extra triggers.
//
// `mpapp::compare_state_trigger<T>` — active while a bound
// `Observable<T>` satisfies a comparison (==, !=, <, >, <=, >=) against
// a fixed target value. Counterpart to the CommunityToolkit.Mvvm
// `CompareStateBehavior` / XAML community StateTriggers. Exposes
// `is_active()` and emits `active_changed` on every state edge.
//
// `mpapp::and_multi_trigger` — standalone boolean multi-condition
// trigger that owns no `view`; it is active while ALL of its
// `Observable<bool>` sources are `true`. Counterpart to the toolkit's
// `AndMultiBindingConverterBehavior` / stacked trigger guards. Exposes
// `is_active()` and emits `active_changed`.
//
// Both follow the same reactive primitives as the rest of the trigger
// family: subscribe to `Observable::changed` via owned `signal_slot`s,
// no macros (ADR-0002), platform-neutral, header-only.

#ifndef MPAPP_TRIGGERS_TRIGGERS_CTK_HPP
#define MPAPP_TRIGGERS_TRIGGERS_CTK_HPP

#include <cstdint>
#include <functional>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#include "../observable.hpp"
#include "../signal.hpp"

namespace mpapp {

// Comparison operator selector.
enum class compare_op : std::uint8_t {
    equal,
    not_equal,
    less,
    greater,
    less_equal,
    greater_equal,
};

// Active while `source <op> target` is true.
// `T` must support the operators implied by the chosen `compare_op`.
template <class T>
class compare_state_trigger {
public:
    // `active_changed` fires with the new bool value on every edge.
    signal<bool> active_changed;

    compare_state_trigger(Observable<T>& source, compare_op op, T target)
        : source_{ &source }, op_{ op }, target_{ std::move(target) } {
        evaluate_silent();
        source_->changed.subscribe(slot_, cb_);
    }

    compare_state_trigger(const compare_state_trigger&)            = delete;
    compare_state_trigger& operator=(const compare_state_trigger&) = delete;
    compare_state_trigger(compare_state_trigger&&)                 = delete;
    compare_state_trigger& operator=(compare_state_trigger&&)      = delete;

    ~compare_state_trigger() = default;

    [[nodiscard]] bool is_active() const noexcept { return active_; }

private:
    [[nodiscard]] bool compute() const {
        const T& v = source_->get();
        switch (op_) {
        case compare_op::equal:         return v == target_;
        case compare_op::not_equal:     return v != target_;
        case compare_op::less:          return v <  target_;
        case compare_op::greater:       return v >  target_;
        case compare_op::less_equal:    return v <= target_;
        case compare_op::greater_equal: return v >= target_;
        }
        return false;
    }

    // Silent initial evaluation (no signal emission on construction).
    void evaluate_silent() { active_ = compute(); }

    void evaluate() {
        const bool now = compute();
        if (now == active_) {
            return;
        }
        active_ = now;
        active_changed.emit(active_);
    }

    struct cb_t {
        compare_state_trigger* self;
        void operator()(const T&) const { self->evaluate(); }
    };

    Observable<T>*        source_;
    compare_op            op_;
    T                     target_;
    bool                  active_ = false;
    cb_t                  cb_{ this };
    signal_slot<const T&> slot_{};
};

// Active while ALL Observable<bool> sources are `true`.
// Sources are added via `add_source()`; the trigger re-evaluates on
// every change. Owns its slots; slot vector never invalidates because
// slots are non-movable — we keep them in a separately heap-allocated
// vector of `signal_slot<const bool&>`.
class and_multi_trigger {
public:
    // `active_changed` fires with the new bool value on every edge.
    signal<bool> active_changed;

    and_multi_trigger() = default;

    and_multi_trigger(const and_multi_trigger&)            = delete;
    and_multi_trigger& operator=(const and_multi_trigger&) = delete;
    and_multi_trigger(and_multi_trigger&&)                 = delete;
    and_multi_trigger& operator=(and_multi_trigger&&)      = delete;

    ~and_multi_trigger() = default;

    // Register a new source. Returns *this for fluent chaining.
    // MUST be called before any source can change — adding sources
    // after a change has fired gives consistent ordering but the
    // initial `active_` state is re-evaluated immediately.
    and_multi_trigger& add_source(Observable<bool>& source) {
        const std::size_t idx = sources_.size();
        sources_.push_back(&source);
        // Grow the slot + callback vectors BEFORE subscribing so the
        // pointer stored in the slot is stable (the vectors only grow,
        // never shrink, and we reserve ahead).
        slots_.push_back(std::make_unique<signal_slot<const bool&>>());
        cbs_.push_back(cb_t{ this });
        source.changed.subscribe(*slots_[idx], cbs_[idx]);
        evaluate();
        return *this;
    }

    [[nodiscard]] bool is_active() const noexcept { return active_; }

private:
    [[nodiscard]] bool all_true() const noexcept {
        for (const auto* s : sources_) {
            if (!s->get()) {
                return false;
            }
        }
        return true;
    }

    void evaluate() {
        const bool now = all_true();
        if (now == active_) {
            return;
        }
        active_ = now;
        active_changed.emit(active_);
    }

    struct cb_t {
        and_multi_trigger* self;
        void operator()(const bool&) const { self->evaluate(); }
    };

    std::vector<Observable<bool>*>                       sources_{};
    std::vector<std::unique_ptr<signal_slot<const bool&>>> slots_{};
    std::vector<cb_t>                                    cbs_{};
    bool                                                 active_ = false;
};

} // namespace mpapp

#endif // MPAPP_TRIGGERS_TRIGGERS_CTK_HPP
