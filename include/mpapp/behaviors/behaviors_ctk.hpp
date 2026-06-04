// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0009-behaviors-and-effects.md
//
// CommunityToolkit-style neutral behaviors (surface-neutral; operate on
// signals and strings, not on native widgets):
//
//   mpapp::event_to_command_behavior<Args...>
//     Bridges an mpapp::signal<Args...> to a command_base. When the
//     signal fires, the behavior optionally maps the signal arguments
//     to a parameterless execute() call, respecting can_execute().
//     Counterpart to MAUI CommunityToolkit EventToCommandBehavior.
//
//   mpapp::text_validation_behavior
//     Validates a std::string value against configurable min/max length
//     and a required (non-empty) flag. Exposes an is_valid() getter and
//     a validity_changed signal that fires each time the valid/invalid
//     status flips. Counterpart to MAUI CommunityToolkit
//     TextValidationBehavior.
//
// No macros in the public API (ADR-0002). Header-only. No wrapper
// components (ADR-0024).

#ifndef MPAPP_BEHAVIORS_BEHAVIORS_CTK_HPP
#define MPAPP_BEHAVIORS_BEHAVIORS_CTK_HPP

#include <cstddef>
#include <functional>
#include <string>
#include <utility>

#include "../binding/relay_command.hpp"
#include "../signal.hpp"

namespace mpapp {

// ---------------------------------------------------------------------------
// event_to_command_behavior<Args...>
//
// Subscribes an mpapp::signal<Args...> to a command_base. When the signal
// fires the behavior calls command_.execute() — provided can_execute() is
// true. An optional mapper function translates the signal arguments before
// the execute call (use it to, e.g., inspect an event value and conditionally
// forward). Passing a null mapper skips the extra call and goes straight to
// execute().
//
// Usage:
//   mpapp::signal<int> value_changed;
//   mpapp::relay_command cmd{ [](){ do_something(); } };
//
//   mpapp::event_to_command_behavior<int> b{ value_changed, cmd };
//   // ...or with a mapper:
//   mpapp::event_to_command_behavior<int> b2{
//       value_changed, cmd,
//       [](int n) { return n > 0; }   // execute only when n > 0
//   };
// ---------------------------------------------------------------------------
template <class... Args>
class event_to_command_behavior {
public:
    // Mapper: given the signal args, returns true to allow execution.
    // If empty (default) execution is always allowed (subject to can_execute).
    using mapper_type = std::function<bool(Args...)>;

    // Construct and immediately subscribe to `source`.
    // Both `source` and `cmd` must outlive this behavior.
    explicit event_to_command_behavior(signal<Args...>& source,
                                       command_base&   cmd,
                                       mapper_type     mapper = {})
        : command_{ cmd }, mapper_{ std::move(mapper) }
    {
        source.subscribe(slot_, handler_);
    }

    event_to_command_behavior(const event_to_command_behavior&)            = delete;
    event_to_command_behavior& operator=(const event_to_command_behavior&) = delete;
    event_to_command_behavior(event_to_command_behavior&&)                 = delete;
    event_to_command_behavior& operator=(event_to_command_behavior&&)      = delete;

    ~event_to_command_behavior() = default;  // slot_ auto-disconnects (RAII)

private:
    // The thunk used as the signal callback. Captures `this` via handler_.
    void on_signal(Args... args) {
        if (!command_.can_execute()) {
            return;
        }
        if (mapper_) {
            if (!mapper_(static_cast<Args>(args)...)) {
                return;
            }
        }
        command_.execute();
    }

    command_base&               command_;
    mapper_type                 mapper_;
    // The callable stored by the signal must outlive the slot — keep it here.
    std::function<void(Args...)> handler_{ [this](Args... args) {
        on_signal(static_cast<Args>(args)...);
    } };
    signal_slot<Args...>        slot_{};
};

// ---------------------------------------------------------------------------
// text_validation_behavior
//
// Validates a std::string against:
//   - required_: the value must be non-empty.
//   - min_length_: value.size() must be >= min_length_ (when > 0).
//   - max_length_: value.size() must be <= max_length_ (when > 0; 0 = no cap).
//
// Call validate(text) to update the internal state. is_valid() reflects the
// current state. validity_changed fires each time the valid/invalid status
// flips (not on every call to validate()).
// ---------------------------------------------------------------------------
class text_validation_behavior {
public:
    explicit text_validation_behavior(bool        required   = false,
                                      std::size_t min_length = 0,
                                      std::size_t max_length = 0)
        : required_{ required }
        , min_length_{ min_length }
        , max_length_{ max_length }
    {}

    text_validation_behavior(const text_validation_behavior&)            = delete;
    text_validation_behavior& operator=(const text_validation_behavior&) = delete;
    text_validation_behavior(text_validation_behavior&&)                 = delete;
    text_validation_behavior& operator=(text_validation_behavior&&)      = delete;

    ~text_validation_behavior() = default;

    // Validate `text` against the current rules and update is_valid_.
    // Fires validity_changed iff the valid/invalid state actually flips.
    void validate(const std::string& text) {
        const bool next = compute_valid(text);
        if (next != is_valid_) {
            is_valid_ = next;
            validity_changed.emit(is_valid_);
        }
    }

    [[nodiscard]] bool is_valid()     const noexcept { return is_valid_;     }
    [[nodiscard]] bool required()     const noexcept { return required_;     }
    [[nodiscard]] std::size_t min_length() const noexcept { return min_length_; }
    [[nodiscard]] std::size_t max_length() const noexcept { return max_length_; }

    // Fired with the new validity state each time it flips.
    mpapp::signal<bool> validity_changed{};

private:
    [[nodiscard]] bool compute_valid(const std::string& text) const {
        if (required_ && text.empty()) {
            return false;
        }
        if (min_length_ > 0 && text.size() < min_length_) {
            return false;
        }
        if (max_length_ > 0 && text.size() > max_length_) {
            return false;
        }
        return true;
    }

    bool        required_   = false;
    std::size_t min_length_ = 0;
    std::size_t max_length_ = 0;
    bool        is_valid_   = false;
};

} // namespace mpapp

#endif // MPAPP_BEHAVIORS_BEHAVIORS_CTK_HPP
