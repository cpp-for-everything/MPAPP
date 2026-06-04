// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0014-commanding.md
//
// `mpapp::async_relay_command` / `async_relay_command_of<T>` — the async
// counterpart to `relay_command` (relay_command.hpp). Mirrors
// CommunityToolkit.Mvvm's `AsyncRelayCommand`: it wraps an asynchronous
// action and tracks an `is_running` flag for the duration of that action.
//
// The async action is modelled as a continuation-passing callable:
//
//     std::function<void(std::function<void()> done)>
//
// i.e. the action receives a `done` completion callback that it must invoke
// when the asynchronous work finishes. This keeps the type platform-neutral
// and fully host-verifiable (no real threads): a test can capture `done` and
// invoke it manually to drive completion deterministically. It composes with
// the `executor`/`task<T>` model too — a coroutine body simply calls `done()`
// at its final continuation.
//
// While the action is running, `can_execute()` returns false (so a bound
// control disables itself) unless `allow_concurrent` is set. Starting and
// completing the action each emit `can_execute_changed`. No macros; header-
// only; platform-neutral.

#ifndef MPAPP_BINDING_ASYNC_RELAY_COMMAND_HPP
#define MPAPP_BINDING_ASYNC_RELAY_COMMAND_HPP

#include <functional>
#include <utility>

#include "../signal.hpp"
#include "relay_command.hpp"

namespace mpapp {

// Parameterless async relay command. Wraps an async action expressed in
// continuation-passing style: the action is handed a `done` callback that it
// invokes when the asynchronous work has completed.
class async_relay_command : public command_base {
public:
    // `execute` is the async action: it receives a completion callback and
    // must invoke it exactly once when the work finishes. `can_execute` is an
    // optional guard (defaults to always-executable). When `allow_concurrent`
    // is false (the default), the command additionally gates re-entry while a
    // previous invocation is still running.
    explicit async_relay_command(
        std::function<void(std::function<void()>)> execute,
        std::function<bool()>                      can_execute      = {},
        bool                                       allow_concurrent = false)
        : execute_{ std::move(execute) }
        , can_execute_{ std::move(can_execute) }
        , allow_concurrent_{ allow_concurrent } {}

    async_relay_command(const async_relay_command&)            = delete;
    async_relay_command& operator=(const async_relay_command&) = delete;
    async_relay_command(async_relay_command&&)                 = delete; // owns signal
    async_relay_command& operator=(async_relay_command&&)      = delete;

    ~async_relay_command() override = default;

    // True iff the wrapped async action is currently in flight.
    [[nodiscard]] bool is_running() const noexcept { return is_running_; }

    // Whether concurrent invocations are permitted.
    [[nodiscard]] bool allow_concurrent() const noexcept {
        return allow_concurrent_;
    }

    // Executable when the user guard (if any) permits AND — unless concurrent
    // execution is allowed — no invocation is currently running.
    [[nodiscard]] bool can_execute() const override {
        if (!allow_concurrent_ && is_running_) {
            return false;
        }
        return can_execute_ ? can_execute_() : true;
    }

    // Start the async action. Sets is_running, emits can_execute_changed, then
    // invokes the action with a completion callback that clears is_running and
    // re-emits can_execute_changed. Does nothing if not currently executable.
    void execute() override {
        if (!execute_ || !can_execute()) {
            return;
        }
        is_running_ = true;
        can_execute_changed.emit();
        execute_([this] { complete(); });
    }

    // Notify bound controls to re-query can_execute().
    void raise_can_execute_changed() { can_execute_changed.emit(); }

private:
    // Completion callback handed to the async action. Idempotent guard against
    // a double-completion clearing state twice / spurious extra signals.
    void complete() {
        if (!is_running_) {
            return;
        }
        is_running_ = false;
        can_execute_changed.emit();
    }

    std::function<void(std::function<void()>)> execute_;
    std::function<bool()>                      can_execute_;
    bool                                       allow_concurrent_ = false;
    bool                                       is_running_       = false;
};

// Parameterized async command (the toolkit's AsyncRelayCommand<T>). The
// parameter flows to execute/can_execute; the completion callback is appended.
template <class T>
class async_relay_command_of {
public:
    explicit async_relay_command_of(
        std::function<void(const T&, std::function<void()>)> execute,
        std::function<bool(const T&)>                        can_execute      = {},
        bool                                                 allow_concurrent = false)
        : execute_{ std::move(execute) }
        , can_execute_{ std::move(can_execute) }
        , allow_concurrent_{ allow_concurrent } {}

    async_relay_command_of(const async_relay_command_of&)            = delete;
    async_relay_command_of& operator=(const async_relay_command_of&) = delete;
    async_relay_command_of(async_relay_command_of&&)                 = delete;
    async_relay_command_of& operator=(async_relay_command_of&&)      = delete;

    ~async_relay_command_of() = default;

    [[nodiscard]] bool is_running() const noexcept { return is_running_; }

    [[nodiscard]] bool allow_concurrent() const noexcept {
        return allow_concurrent_;
    }

    [[nodiscard]] bool can_execute(const T& param) const {
        if (!allow_concurrent_ && is_running_) {
            return false;
        }
        return can_execute_ ? can_execute_(param) : true;
    }

    void execute(const T& param) {
        if (!execute_ || !can_execute(param)) {
            return;
        }
        is_running_ = true;
        can_execute_changed.emit();
        execute_(param, [this] { complete(); });
    }

    void raise_can_execute_changed() { can_execute_changed.emit(); }

    mpapp::signal<> can_execute_changed{};

private:
    void complete() {
        if (!is_running_) {
            return;
        }
        is_running_ = false;
        can_execute_changed.emit();
    }

    std::function<void(const T&, std::function<void()>)> execute_;
    std::function<bool(const T&)>                        can_execute_;
    bool                                                 allow_concurrent_ = false;
    bool                                                 is_running_       = false;
};

} // namespace mpapp

#endif // MPAPP_BINDING_ASYNC_RELAY_COMMAND_HPP
