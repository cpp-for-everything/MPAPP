// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/20_ADRs/ADR-0009-public-api-template-wrappers-only.md
//
// Observable<T> — a thin wrapper around a value that fires its embedded
// intrusive `changed` signal when set() actually mutates the underlying
// value (compared via operator==). Get/set are noexcept-friendly, no
// heap allocations on the hot path.

#ifndef MPAPP_OBSERVABLE_HPP
#define MPAPP_OBSERVABLE_HPP

#include <concepts>
#include <type_traits>
#include <utility>

#include "signal.hpp"

namespace mpapp {

template <class T>
class Observable {
public:
    using value_type = T;

    Observable() = default;

    explicit Observable(T value) noexcept(std::is_nothrow_move_constructible_v<T>)
        : value_(std::move(value)) {}

    Observable(const Observable&)            = delete;
    Observable& operator=(const Observable&) = delete;
    Observable(Observable&&)                 = delete;
    Observable& operator=(Observable&&)      = delete;

    const T& get() const noexcept { return value_; }

    // Implicit conversion so an Observable<T> reads as a const T& in
    // expression contexts: `std::string s = obs;`, `int n = count;`.
    operator const T&() const noexcept { return value_; }

    // set() compares with == and only fires `changed` on a real change.
    // For move-only T, operator== may not exist — in that case the
    // change-on-real-change semantics degrade to always-change.
    void set(T value) {
        if constexpr (requires(const T& a, const T& b) {
                          { a == b } -> std::convertible_to<bool>;
                      }) {
            if (value == value_) {
                return;
            }
        }
        value_ = std::move(value);
        changed.emit(value_);
    }

    Observable& operator=(T value) {
        set(std::move(value));
        return *this;
    }

    // Intrusive signal — caller passes a `mpapp::signal_slot<const T&>`
    // they own.
    mutable mpapp::signal<const T&> changed;

private:
    T value_{};
};

} // namespace mpapp

#endif // MPAPP_OBSERVABLE_HPP
