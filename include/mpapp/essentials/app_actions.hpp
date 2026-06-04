// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::app_actions` — home-screen / launcher quick-action shortcuts.
// Counterpart to MAUI Essentials `AppActions`. Abstract interface + an
// in-memory mock implementation that is fully test-drivable. Real per-
// platform backends (Windows JumpList, Android App Shortcuts, iOS Home
// Screen Quick Actions) implement the same interface and are injected via
// the DI container (RFC-0011). No macros; header-only interface.

#ifndef MPAPP_ESSENTIALS_APP_ACTIONS_HPP
#define MPAPP_ESSENTIALS_APP_ACTIONS_HPP

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

#include "../signal.hpp"

namespace mpapp {

// Represents a single quick-action shortcut. Mirrors MAUI's AppAction.
struct app_action {
    std::string id{};
    std::string title{};
    std::string subtitle{};
    std::string icon{};

    bool operator==(const app_action&) const = default;
};

class app_actions {
public:
    virtual ~app_actions() = default;

    // Returns true when the platform supports app-action shortcuts.
    [[nodiscard]] virtual bool is_supported() const = 0;

    // Returns the currently registered set of app actions.
    [[nodiscard]] virtual std::vector<app_action> get() const = 0;

    // Replaces the registered set with the supplied actions.
    // Ignored if !is_supported().
    virtual void set(const std::vector<app_action>& actions) = 0;

    // Fires when the user activates one of the registered actions.
    // Subscribers receive a copy of the activated app_action.
    mpapp::signal<app_action> app_action_activated{};
};

// Default + mock implementation: process-memory backed.
// - `set_supported(bool)` controls what is_supported() returns.
// - `set(...)` replaces the internal list when is_supported() is true;
//   the call is recorded regardless (last_set_ tracks the argument).
// - `trigger(id)` finds the action with the given id and emits
//   app_action_activated with that action.
class mock_app_actions final : public app_actions {
public:
    explicit mock_app_actions(bool supported = true)
        : supported_{ supported } {}

    // ---- Interface implementation ----------------------------------------

    [[nodiscard]] bool is_supported() const override { return supported_; }

    [[nodiscard]] std::vector<app_action> get() const override {
        return actions_;
    }

    void set(const std::vector<app_action>& actions) override {
        last_set_ = actions;
        if (!supported_) {
            return;
        }
        actions_ = actions;
    }

    // ---- Test-driving helpers --------------------------------------------

    // Change the supported flag after construction.
    void set_supported(bool value) { supported_ = value; }

    // Simulate the user activating the action identified by `id`.
    // If no action with that id is registered, this is a no-op.
    void trigger(const std::string& id) {
        auto it = std::find_if(actions_.begin(), actions_.end(),
                               [&](const app_action& a) { return a.id == id; });
        if (it == actions_.end()) {
            return;
        }
        app_action_activated.emit(*it);
    }

    // Returns the argument of the most recent set() call (std::nullopt
    // if set() has never been called).
    [[nodiscard]] std::optional<std::vector<app_action>> last_set() const {
        return last_set_;
    }

private:
    bool                                      supported_;
    std::vector<app_action>                   actions_{};
    std::optional<std::vector<app_action>>    last_set_{};
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_APP_ACTIONS_HPP
