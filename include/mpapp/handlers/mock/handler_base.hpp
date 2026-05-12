// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/20_ADRs/ADR-0008-mock-first-implementation.md
//
// `mock_handler_base<Derived, VirtualView>` — CRTP base shared by every
// `*_handler<platform::mock>` specialisation. Records each invocation of
// a property mapper (or command mapper) as a `call_record` so unit tests
// can assert the exact sequence and arguments the framework would have
// passed to a real native handler.
//
// The recorder is intentionally header-only and dependency-free: no
// native SDKs, no allocations beyond the `std::vector` growth path, no
// virtual dispatch in the hot path. This keeps mock tests buildable on
// every supported host, satisfying CLAUDE Rule 6 (mock before real).
//
// A `call_record` holds:
//   - `property_name`  — the cross-platform property the mapper acted on
//                        (e.g. `"text"`, `"opacity"`).
//   - `value_repr`     — `std::format("{}", value)` of the value the
//                        mapper observed at call time. For non-formattable
//                        types, handlers fall back to `"<unformattable>"`.

#ifndef MPAPP_HANDLERS_MOCK_HANDLER_BASE_HPP
#define MPAPP_HANDLERS_MOCK_HANDLER_BASE_HPP

#include <concepts>
#include <format>
#include <string>
#include <string_view>
#include <vector>

#include "../../observable.hpp"
#include "../../signal.hpp"

namespace mpapp {

struct call_record {
    std::string property_name;
    std::string value_repr;
};

namespace detail {

// True iff `std::format("{}", value)` is well-formed for T. Used by
// `format_value` to fall back gracefully for shared_ptr<view>, custom
// enums lacking a formatter, etc.
template <class T>
concept std_formattable = requires(const T& v) {
    { std::format("{}", v) } -> std::convertible_to<std::string>;
};

template <class T>
std::string format_value(const T& value) {
    if constexpr (std_formattable<T>) {
        return std::format("{}", value);
    } else {
        return "<unformattable>";
    }
}

// Per-property binding state. Holds a slot + a small callable whose
// captured `this` records into the owning handler. Embedded by value in
// the handler (one per Observable<T> property), so no heap allocation
// occurs per mapper subscription.
template <class T>
struct property_binding {
    using value_type = T;

    struct callback_t {
        void (*record_fn)(void*, std::string_view, const T&) = nullptr;
        void*            handler                              = nullptr;
        std::string_view property_name                        = {};

        void operator()(const T& value) const {
            record_fn(handler, property_name, value);
        }
    };

    callback_t       callback{};
    signal_slot<const T&> slot{};
};

} // namespace detail

template <class Derived, class VirtualView>
class mock_handler_base {
public:
    using virtual_view_type = VirtualView;

    mock_handler_base() = default;

    mock_handler_base(const mock_handler_base&)            = delete;
    mock_handler_base& operator=(const mock_handler_base&) = delete;
    mock_handler_base(mock_handler_base&&)                 = delete;
    mock_handler_base& operator=(mock_handler_base&&)      = delete;

    // Recorded mapper invocations in chronological order. Tests assert
    // against this directly.
    const std::vector<call_record>& calls() const noexcept { return calls_; }

    // Test helper — clears the recorded sequence without resetting any
    // subscriptions the derived handler may hold.
    void clear_calls() noexcept { calls_.clear(); }

protected:
    ~mock_handler_base() = default;

    // Record a single mapper invocation. `value` is stringified via
    // `std::format` when a formatter is available; otherwise a stable
    // placeholder is recorded so test assertions still have something to
    // match on.
    template <class T>
    void record(std::string_view property_name, const T& value) {
        calls_.push_back(call_record{
            std::string{property_name},
            detail::format_value(value),
        });
    }

    // Record an invocation with no value payload (e.g. a Command<> mapper).
    void record(std::string_view property_name) {
        calls_.push_back(call_record{
            std::string{property_name},
            std::string{},
        });
    }

    // Record a property's current value AND wire its `changed` signal so
    // subsequent sets are recorded automatically. `binding` is the
    // handler-owned storage (slot + callback); pass a different one per
    // Observable to avoid clobbering. Calling this twice on the same
    // binding is safe — the slot reconnects to the most recent signal.
    template <class T>
    void bind(std::string_view property_name,
              Observable<T>& observable,
              detail::property_binding<T>& binding) {
        record(property_name, observable.get());

        binding.callback = typename detail::property_binding<T>::callback_t{
            &mock_handler_base::record_via_void,
            this,
            property_name,
        };
        observable.changed.subscribe(binding.slot, binding.callback);
    }

private:
    template <class T>
    static void record_via_void(void* self, std::string_view name, const T& value) {
        static_cast<mock_handler_base*>(self)->record(name, value);
    }

    std::vector<call_record> calls_;
};

} // namespace mpapp

#endif // MPAPP_HANDLERS_MOCK_HANDLER_BASE_HPP
