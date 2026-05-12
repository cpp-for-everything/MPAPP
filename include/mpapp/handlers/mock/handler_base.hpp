// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/20_ADRs/ADR-0008-mock-first-implementation.md
//
// `mock_handler_base` — common scaffolding for `platform::mock` handler
// specialisations. A mock handler subscribes to every Observable on its
// owning control and records each change as a string in a call log. Tests
// then assert against `calls()` to verify the property-mapper plumbing
// fires exactly once per real change (the no-change-no-emit contract is
// owned by Observable itself, but the handler-level wiring needs proof).
//
// Design constraints:
//   - Zero heap on the value path; the call log uses a std::vector but is
//     only touched in tests, never on a UI hot path. A real handler would
//     replace the log with native widget mutations.
//   - No std::function — each mapper is a stable member callable so its
//     address can be handed to `signal::subscribe`, mirroring the Windows
//     handler pattern (see button_handler.hpp).
//   - Header-only. Mock handlers have no .cpp; the entire control surface
//     should compile with just include/mpapp/.

#ifndef MPAPP_HANDLERS_MOCK_HANDLER_BASE_HPP
#define MPAPP_HANDLERS_MOCK_HANDLER_BASE_HPP

#include <concepts>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "../../platform.hpp"

namespace mpapp {

// Helper — stringify a value for the call log. Specialise on demand; the
// default path uses operator<< if available, else a placeholder.
namespace detail {

template <class T>
concept ostreamable = requires(std::ostream& os, const T& v) {
    { os << v } -> std::convertible_to<std::ostream&>;
};

template <class T>
inline std::string to_log_string(const T& value) {
    if constexpr (ostreamable<T>) {
        std::ostringstream oss;
        oss << std::boolalpha << value;
        return oss.str();
    } else {
        return std::string{"<value>"};
    }
}

} // namespace detail

// Base for all `platform::mock` handlers. Derived classes own the per-
// property `signal_slot<const T&>` members and per-property `record`
// callables. The base just holds the call log and exposes the
// `record_change()` / `record_event()` helpers used by `mock_property_recorder`
// and friends. Those helpers are public because the recorder callables
// invoke them from external scope (signal::subscribe's thunk).
class mock_handler_base {
public:
    mock_handler_base()                                    = default;
    ~mock_handler_base()                                   = default;
    mock_handler_base(const mock_handler_base&)            = delete;
    mock_handler_base& operator=(const mock_handler_base&) = delete;
    mock_handler_base(mock_handler_base&&)                 = delete;
    mock_handler_base& operator=(mock_handler_base&&)      = delete;

    // Read-only view of the call log. Each entry is `"<prop>=<value>"`
    // for property changes or `"<name>"` for bare events.
    const std::vector<std::string>& calls() const noexcept { return calls_; }

    // Reset the log between scenarios.
    void clear_calls() noexcept { calls_.clear(); }

    // Append a `<prop>=<value>` row to the log. Invoked by recorder
    // callables, which are not friends — keep public for the spike;
    // a tighter friend wrapper would only add boilerplate.
    template <class T>
    void record_change(const char* prop, const T& value) {
        std::string row = prop;
        row += '=';
        row += detail::to_log_string(value);
        calls_.push_back(std::move(row));
    }

    // Append a bare event row (no `=value`) to the log. Used by event
    // recorders for signals like `clicked` that have no payload.
    void record_event(const char* name) { calls_.emplace_back(name); }

private:
    std::vector<std::string> calls_{};
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
