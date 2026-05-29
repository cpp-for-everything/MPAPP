// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0007-data-binding.md
//
// `mpapp::binding<S, T>` — a live link between a source `Observable<S>`
// and a target `Observable<T>`. Counterpart to MAUI's `Binding`. It
// composes with the existing reactive primitives (it subscribes to the
// Observables' intrusive `changed` signals via owned `signal_slot`
// members) rather than replacing them, and is expressed entirely in
// template-wrapper types — no macros (Rule 1 / ADR-0009).
//
// Binding is platform-NEUTRAL infrastructure: it drives `Observable::set`,
// which fires the same per-property mapper a real platform handler
// installed. So a bound property update flows through the identical
// `Observable -> handler -> native widget` pipeline that already works on
// Windows / Linux / Android — binding needs no per-platform code.
//
// Like resources / VSM / gestures, binding is NOT a wrapper-component
// (ADR-0024): it owns no native widget.

#ifndef MPAPP_BINDING_BINDING_HPP
#define MPAPP_BINDING_BINDING_HPP

#include <cstdint>
#include <functional>
#include <type_traits>
#include <utility>

#include "../observable.hpp"
#include "../signal.hpp"

namespace mpapp {

// MAUI's BindingMode 1:1.
//   one_way            source -> target (default)
//   two_way            source <-> target
//   one_time           source -> target once, at construction, no live link
//   one_way_to_source  target -> source
enum class binding_mode : std::uint8_t {
    one_way           = 0,
    two_way           = 1,
    one_time          = 2,
    one_way_to_source = 3,
};

// Abstract value converter (MAUI's IValueConverter). Optional sugar —
// `binding` accepts plain `std::function` converters too; this base is
// for users who prefer a reusable converter object. Wrap one into a
// binding via `[c](const S& s){ return c->convert(s); }`.
template <class S, class T>
struct value_converter {
    virtual ~value_converter() = default;
    [[nodiscard]] virtual T convert(const S& source) const = 0;
    [[nodiscard]] virtual S convert_back(const T& target) const = 0;
};

template <class S, class T = S>
class binding {
public:
    using to_target_fn = std::function<T(const S&)>;  // source -> target
    using to_source_fn = std::function<S(const T&)>;   // target -> source

    // Construct + immediately sync. For one_way/two_way/one_time the
    // target is seeded from the source; for one_way_to_source the source
    // is seeded from the target. Live subscriptions are then wired per
    // mode (none for one_time).
    //
    // Converters are optional. When omitted and `S`/`T` are
    // implicitly convertible, the identity/static_cast conversion is
    // used; otherwise the matching converter is required (a null one
    // throws std::bad_function_call when first exercised).
    explicit binding(Observable<S>& source, Observable<T>& target,
                     binding_mode mode = binding_mode::one_way,
                     to_target_fn to_target = {}, to_source_fn to_source = {})
        : source_{ &source }
        , target_{ &target }
        , mode_{ mode }
        , to_target_{ std::move(to_target) }
        , to_source_{ std::move(to_source) } {
        // Initial sync.
        if (mode_ == binding_mode::one_way_to_source) {
            write_source(target_->get());
        } else {
            write_target(source_->get());
        }
        if (mode_ == binding_mode::one_time) {
            return;  // snapshot only — no live link
        }
        // Live links.
        if (mode_ != binding_mode::one_way_to_source) {
            source_->changed.subscribe(source_slot_, source_cb_);
        }
        if (mode_ == binding_mode::two_way ||
            mode_ == binding_mode::one_way_to_source) {
            target_->changed.subscribe(target_slot_, target_cb_);
        }
    }

    binding(const binding&)            = delete;
    binding& operator=(const binding&) = delete;
    binding(binding&&)                 = delete;  // owns intrusive slots
    binding& operator=(binding&&)      = delete;

    ~binding() = default;

    [[nodiscard]] binding_mode mode() const noexcept { return mode_; }

private:
    void write_target(const S& s) {
        if (updating_) {
            return;  // re-entrancy guard for the two-way echo
        }
        updating_ = true;
        if constexpr (std::is_convertible_v<S, T>) {
            target_->set(to_target_ ? to_target_(s) : static_cast<T>(s));
        } else {
            target_->set(to_target_(s));  // converter mandatory for non-convertible
        }
        updating_ = false;
    }

    void write_source(const T& t) {
        if (updating_) {
            return;
        }
        updating_ = true;
        if constexpr (std::is_convertible_v<T, S>) {
            source_->set(to_source_ ? to_source_(t) : static_cast<S>(t));
        } else {
            source_->set(to_source_(t));
        }
        updating_ = false;
    }

    struct source_cb_t {
        binding* self;
        void operator()(const S& s) const { self->write_target(s); }
    };
    struct target_cb_t {
        binding* self;
        void operator()(const T& t) const { self->write_source(t); }
    };

    Observable<S>* source_;
    Observable<T>* target_;
    binding_mode   mode_;
    to_target_fn   to_target_;
    to_source_fn   to_source_;
    bool           updating_ = false;

    // Member callbacks (stable address for signal::subscribe, mirroring
    // the mock-handler recorder pattern) + the owned slots.
    source_cb_t           source_cb_{ this };
    target_cb_t           target_cb_{ this };
    signal_slot<const S&> source_slot_{};
    signal_slot<const T&> target_slot_{};
};

} // namespace mpapp

#endif // MPAPP_BINDING_BINDING_HPP
