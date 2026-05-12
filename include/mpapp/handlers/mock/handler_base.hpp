// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/20_ADRs/ADR-0008-mock-first-implementation.md
//
// `mock_handler_base` — common scaffolding for `platform::mock` handler
// specialisations. A mock handler subscribes to every Observable on its
// owning control and records each change as a `call_record` so unit tests
// can assert the exact sequence and arguments the framework would have
// passed to a real native handler.
//
// Two equivalent recording APIs coexist:
//
//   * `record(name, value)` / `record(name)` / `bind(name, obs, binding)`
//     — used by the layout-group handlers (view, layout, scroll_view, …).
//     `bind()` records the current value AND wires the property's `changed`
//     signal so subsequent sets are recorded automatically.
//
//   * `record_change(prop, value)` / `record_event(name)` +
//     `mock_property_recorder<Owner, T>` — used by the simple-input
//     handlers (button, label, entry, …). The recorder is a stable
//     member callable so its address can be handed to `signal::subscribe`,
//     mirroring the Windows handler pattern.
//
// Both APIs share storage: a `std::vector<call_record>` where each entry
// is `{ property_name, value_repr }`. Tests that prefer the flat-string
// form ("text=hello", "clicked") use `calls_as_strings()`; tests that
// want the structured form read `calls()[i].property_name` and `.value_repr`.
//
// Design constraints:
//   - Zero heap on the value path; the call log uses a std::vector but is
//     only touched in tests, never on a UI hot path.
//   - Header-only. Mock handlers have no .cpp; the entire control surface
//     should compile with just include/mpapp/.

#ifndef MPAPP_HANDLERS_MOCK_HANDLER_BASE_HPP
#define MPAPP_HANDLERS_MOCK_HANDLER_BASE_HPP

#include <concepts>
#include <format>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
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
concept ostreamable = requires(std::ostream& os, const T& v) {
    { os << v } -> std::convertible_to<std::ostream&>;
};

template <class T>
std::string format_value(const T& value) {
    if constexpr (std_formattable<T>) {
        return std::format("{}", value);
    } else if constexpr (ostreamable<T>) {
        std::ostringstream oss;
        oss << std::boolalpha << value;
        return oss.str();
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

class mock_handler_base {
public:
    mock_handler_base()                                    = default;
    ~mock_handler_base()                                   = default;
    mock_handler_base(const mock_handler_base&)            = delete;
    mock_handler_base& operator=(const mock_handler_base&) = delete;
    mock_handler_base(mock_handler_base&&)                 = delete;
    mock_handler_base& operator=(mock_handler_base&&)      = delete;

    // Structured view of the call log. Layout-group tests assert against
    // `.property_name` and `.value_repr` directly.
    const std::vector<call_record>& calls() const noexcept { return calls_; }

    // Flat-string view of the call log. Each entry is `"<prop>=<value>"`
    // when `value_repr` is non-empty, or just `"<prop>"` for bare events.
    // Used by simple-input tests that prefer single-string assertions.
    std::vector<std::string> calls_as_strings() const {
        std::vector<std::string> out;
        out.reserve(calls_.size());
        for (const auto& c : calls_) {
            if (c.value_repr.empty()) {
                out.push_back(c.property_name);
            } else {
                std::string row;
                row.reserve(c.property_name.size() + 1 + c.value_repr.size());
                row.append(c.property_name);
                row.push_back('=');
                row.append(c.value_repr);
                out.push_back(std::move(row));
            }
        }
        return out;
    }

    // Reset the log between scenarios. Does not touch any signal
    // subscriptions a derived handler may hold.
    void clear_calls() noexcept { calls_.clear(); }

    // ------- Layout-handler API (Unit 7) -----------------------------------

    // Record a single mapper invocation. `value` is stringified via
    // `std::format` when a formatter is available; otherwise an operator<<
    // fallback is used; otherwise a stable placeholder is recorded.
    template <class T>
    void record(std::string_view property_name, const T& value) {
        calls_.push_back(call_record{
            std::string{property_name},
            detail::format_value(value),
        });
    }

    // Record an invocation with no value payload (e.g. a Command<> mapper
    // or a bare event such as `clicked`).
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
            &mock_handler_base::record_via_void<T>,
            this,
            property_name,
        };
        observable.changed.subscribe(binding.slot, binding.callback);
    }

    // ------- Input-handler API (Unit 8) ------------------------------------

    // Append a `<prop>=<value>` row to the log. Equivalent to
    // `record(prop, value)`; provided so the simple-input handlers and
    // their `mock_property_recorder` callables read naturally.
    template <class T>
    void record_change(const char* prop, const T& value) {
        record(std::string_view{prop}, value);
    }

    // Append a bare event row (no `=value`) to the log. Equivalent to
    // `record(name)`.
    void record_event(const char* name) {
        record(std::string_view{name});
    }

private:
    template <class T>
    static void record_via_void(void* self, std::string_view name, const T& value) {
        static_cast<mock_handler_base*>(self)->record(name, value);
    }

    std::vector<call_record> calls_{};
};

// Convenience callable — derived handlers embed one of these per
// Observable. Holds a pointer to its owner handler and a property name;
// invoking it logs the new value. Stored as a member so its address is
// stable for `signal::subscribe`.
template <class Owner, class T>
struct mock_property_recorder {
    Owner*      self = nullptr;
    const char* prop = "";
    void operator()(const T& v) const { self->record_change(prop, v); }
};

} // namespace mpapp

#endif // MPAPP_HANDLERS_MOCK_HANDLER_BASE_HPP
