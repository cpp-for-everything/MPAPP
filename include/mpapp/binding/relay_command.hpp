// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0014-commanding.md
//
// `mpapp::command_base` (ICommand) + `relay_command` / `relay_command_of<T>`
// — the MVVM commanding primitive. Counterpart to MAUI/.NET's `ICommand`
// + CommunityToolkit `RelayCommand`. A command bundles an action with a
// `can_execute` guard and a `can_execute_changed` signal so a bound
// control (e.g. a button) can enable/disable itself. Distinct from the
// compile-time `Command<>` *tag* (`command.hpp`, ADR-0009) which marks
// view-model methods for the XAML compiler — this is the runtime command
// OBJECT a control binds to. No macros; platform-neutral.

#ifndef MPAPP_BINDING_RELAY_COMMAND_HPP
#define MPAPP_BINDING_RELAY_COMMAND_HPP

#include <functional>
#include <utility>

#include "../signal.hpp"

namespace mpapp {

// ICommand: a parameterless command interface.
class command_base {
public:
    virtual ~command_base() = default;

    [[nodiscard]] virtual bool can_execute() const = 0;
    virtual void execute() = 0;

    // Fired when can_execute()'s result may have changed, so bound
    // controls re-query (MAUI's CanExecuteChanged).
    mpapp::signal<> can_execute_changed{};

protected:
    command_base() = default;
};

// Parameterless relay command: wraps an execute action + optional
// can_execute predicate (defaults to always-executable).
class relay_command : public command_base {
public:
    explicit relay_command(std::function<void()> execute,
                           std::function<bool()> can_execute = {})
        : execute_{ std::move(execute) }, can_execute_{ std::move(can_execute) } {}

    relay_command(const relay_command&)            = delete;
    relay_command& operator=(const relay_command&) = delete;
    relay_command(relay_command&&)                 = delete;  // owns signal
    relay_command& operator=(relay_command&&)      = delete;

    ~relay_command() override = default;

    [[nodiscard]] bool can_execute() const override {
        return can_execute_ ? can_execute_() : true;
    }

    void execute() override {
        if (execute_ && can_execute()) {
            execute_();
        }
    }

    // Notify bound controls to re-query can_execute().
    void raise_can_execute_changed() { can_execute_changed.emit(); }

private:
    std::function<void()> execute_;
    std::function<bool()> can_execute_;
};

// Parameterized command (MAUI's ICommand with a CommandParameter / the
// toolkit's RelayCommand<T>). The parameter flows to execute/can_execute.
template <class T>
class relay_command_of {
public:
    explicit relay_command_of(std::function<void(const T&)> execute,
                              std::function<bool(const T&)> can_execute = {})
        : execute_{ std::move(execute) }, can_execute_{ std::move(can_execute) } {}

    relay_command_of(const relay_command_of&)            = delete;
    relay_command_of& operator=(const relay_command_of&) = delete;
    relay_command_of(relay_command_of&&)                 = delete;
    relay_command_of& operator=(relay_command_of&&)      = delete;

    ~relay_command_of() = default;

    [[nodiscard]] bool can_execute(const T& param) const {
        return can_execute_ ? can_execute_(param) : true;
    }

    void execute(const T& param) {
        if (execute_ && can_execute(param)) {
            execute_(param);
        }
    }

    void raise_can_execute_changed() { can_execute_changed.emit(); }

    mpapp::signal<> can_execute_changed{};

private:
    std::function<void(const T&)> execute_;
    std::function<bool(const T&)> can_execute_;
};

} // namespace mpapp

#endif // MPAPP_BINDING_RELAY_COMMAND_HPP
