// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0008-triggers.md
//
// Property / data triggers + multi-trigger. Counterpart to MAUI's
// `Trigger`, `DataTrigger`, and `MultiTrigger`. A trigger watches one
// or more `Observable<T>` conditions and, when the condition(s) hold,
// runs its `enter_setters` against a target `view`; when they stop
// holding it runs `exit_setters`. Setters are the same
// `function<void(view&)>` shape as `style` (RFC-0005) /
// `visual_state` (RFC-0006); like those, auto-value-capture/restore is
// deferred (the app provides complete enter/exit pairs).
//
// Composes with the reactive primitives + binding layer: triggers
// subscribe to `Observable::changed` via owned `signal_slot`s, no
// macros (ADR-0009), platform-neutral (not a wrapper-component).

#ifndef MPAPP_TRIGGERS_TRIGGER_HPP
#define MPAPP_TRIGGERS_TRIGGER_HPP

#include <cstddef>
#include <exception>
#include <functional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>

#include "../observable.hpp"
#include "../signal.hpp"

namespace mpapp {

class view;  // setters take view&; full type not needed here

// Shared setter bundle + enter/exit transition logic.
class trigger_base {
public:
    using setter_map = std::unordered_map<std::string, std::function<void(view&)>>;

    setter_map enter_setters{};
    setter_map exit_setters{};

    [[nodiscard]] bool is_active() const noexcept { return active_; }

protected:
    trigger_base() = default;
    ~trigger_base() = default;
    trigger_base(const trigger_base&)            = delete;
    trigger_base& operator=(const trigger_base&) = delete;
    trigger_base(trigger_base&&)                 = delete;
    trigger_base& operator=(trigger_base&&)      = delete;

    // Transition to `now_active`; on a real edge run the matching bundle.
    void transition(view& target, bool now_active) {
        if (now_active == active_) {
            return;
        }
        active_ = now_active;
        run(active_ ? enter_setters : exit_setters, target);
    }

    static void run(const setter_map& setters, view& v) {
        for (const auto& [_, s] : setters) {
            if (!s) {
                continue;
            }
            try {
                s(v);
            } catch (const std::exception&) {
                // Swallowed per RFC-0008 (mirrors style/VSM setter handling).
            } catch (...) {
            }
        }
    }

private:
    bool active_ = false;
};

// Active while `source == value`. MAUI property Trigger (point `source`
// at the target's own Observable) + DataTrigger (point it at a
// view-model Observable, typically via the BindingContext).
template <class T>
class trigger : public trigger_base {
public:
    trigger(Observable<T>& source, T value, view& target)
        : source_{ &source }, value_{ std::move(value) }, target_{ &target } {
        evaluate();
        source_->changed.subscribe(slot_, cb_);
    }

private:
    void evaluate() { transition(*target_, source_->get() == value_); }

    struct cb_t {
        trigger* self;
        void operator()(const T&) const { self->evaluate(); }
    };

    Observable<T>*        source_;
    T                     value_;
    view*                 target_;
    cb_t                  cb_{ this };
    signal_slot<const T&> slot_{};
};

// One condition for a multi_trigger. CTAD: `when{flag, true}`.
template <class T>
struct when {
    Observable<T>* source;
    T              value;
    when(Observable<T>& s, T v) : source{ &s }, value{ std::move(v) } {}
};
template <class T>
when(Observable<T>&, T) -> when<T>;

// Active only while ALL conditions match (MAUI MultiTrigger).
template <class... Ts>
class multi_trigger : public trigger_base {
public:
    explicit multi_trigger(view& target, when<Ts>... conditions)
        : target_{ &target }
        , conds_{ conditions... }
        , cbs_{ cb_t<Ts>{ this }... } {
        evaluate();
        subscribe_all(std::index_sequence_for<Ts...>{});
    }

private:
    template <class U>
    struct cb_t {
        multi_trigger* self;
        void operator()(const U&) const { self->evaluate(); }
    };

    void evaluate() {
        transition(*target_, all_match(std::index_sequence_for<Ts...>{}));
    }

    template <std::size_t... I>
    [[nodiscard]] bool all_match(std::index_sequence<I...>) const {
        return (... && (std::get<I>(conds_).source->get() == std::get<I>(conds_).value));
    }

    template <std::size_t... I>
    void subscribe_all(std::index_sequence<I...>) {
        (subscribe_one<I>(), ...);
    }

    template <std::size_t I>
    void subscribe_one() {
        std::get<I>(conds_).source->changed.subscribe(std::get<I>(slots_),
                                                      std::get<I>(cbs_));
    }

    view*                                 target_;
    std::tuple<when<Ts>...>               conds_;
    std::tuple<cb_t<Ts>...>               cbs_;
    std::tuple<signal_slot<const Ts&>...> slots_{};
};

} // namespace mpapp

#endif // MPAPP_TRIGGERS_TRIGGER_HPP
