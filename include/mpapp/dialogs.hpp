// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Page.md
//
// `mpapp::dialog_service` — the page-level modal dialog surface. Mirrors
// MAUI's `Page.DisplayAlert` / `DisplayActionSheet` / `DisplayPromptAsync`.
//
// MAUI returns `Task<bool>` / `Task<string>` / `Task<string?>` from these
// methods. Rather than drag an event loop into the test surface, MPAPP
// models the request as a recorded value plus a programmable response: the
// host (or a test) primes the next result via `set_next_*_result(...)`, then
// the `display_*` call records the request AND immediately invokes the
// supplied `on_result` callback with the programmed value. This keeps the
// surface deterministic and fully unit-testable without a dispatcher.
//
// Abstract interface (`dialog_service`) + a settable mock implementation
// (`mock_dialog_service`). Header-only; conforms to ADR-0002 (no macros in
// the public API).

#ifndef MPAPP_DIALOGS_HPP
#define MPAPP_DIALOGS_HPP

#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace mpapp {

// ---- Recorded-request value types ------------------------------------------

// A simple alert: title + message + a single dismiss ("cancel") button, or a
// two-button accept/cancel confirmation when `accept` is non-empty.
struct alert_request {
    std::string title{};
    std::string message{};
    std::string accept{};   // empty for a single-button alert
    std::string cancel{};

    [[nodiscard]] bool operator==(const alert_request&) const = default;
};

// An action sheet: a title, a cancel button, an optional destructive button,
// and an ordered list of plain buttons.
struct action_sheet_request {
    std::string              title{};
    std::string              cancel{};
    std::string              destruction{};   // empty when there is none
    std::vector<std::string> buttons{};

    [[nodiscard]] bool operator==(const action_sheet_request&) const = default;
};

// A text prompt: title + message, accept/cancel button labels, plus the
// placeholder and initial value of the input field.
struct prompt_request {
    std::string title{};
    std::string message{};
    std::string accept{};
    std::string cancel{};
    std::string placeholder{};
    std::string initial_value{};

    [[nodiscard]] bool operator==(const prompt_request&) const = default;
};

// ---- Abstract interface ----------------------------------------------------

class dialog_service {
public:
    virtual ~dialog_service() = default;

    // Single-button alert (no result). MAUI: DisplayAlert(title,message,cancel).
    virtual void display_alert(std::string title,
                               std::string message,
                               std::string cancel) = 0;

    // Two-button confirmation. `on_result(true)` for accept, `false` for
    // cancel. MAUI: DisplayAlert(title,message,accept,cancel) -> Task<bool>.
    virtual void display_alert(std::string title,
                               std::string message,
                               std::string accept,
                               std::string cancel,
                               std::function<void(bool)> on_result) = 0;

    // Action sheet. `on_result(choice)` receives the chosen button label
    // (which may be the cancel or destruction label).
    // MAUI: DisplayActionSheet(...) -> Task<string>.
    virtual void display_action_sheet(std::string title,
                                      std::string cancel,
                                      std::string destruction,
                                      std::vector<std::string> buttons,
                                      std::function<void(std::string)> on_result) = 0;

    // Text prompt. `on_result(nullopt)` when cancelled, otherwise the entered
    // text. MAUI: DisplayPromptAsync(...) -> Task<string?>.
    virtual void display_prompt(std::string title,
                                std::string message,
                                std::function<void(std::optional<std::string>)> on_result,
                                std::string accept        = "OK",
                                std::string cancel        = "Cancel",
                                std::string placeholder   = "",
                                std::string initial_value = "") = 0;
};

// ---- Mock / in-memory implementation ---------------------------------------

// Records the most recent request of each kind and replays a programmed
// response into the caller's `on_result` callback. Defaults: an un-primed
// confirmation alert resolves to `false`, an un-primed action sheet resolves
// to its `cancel` label, and an un-primed prompt resolves to `nullopt`.
class mock_dialog_service final : public dialog_service {
public:
    mock_dialog_service() = default;

    // --- Display methods (interface) ----------------------------------------

    void display_alert(std::string title,
                       std::string message,
                       std::string cancel) override {
        last_alert_ = alert_request{std::move(title), std::move(message),
                                    std::string{}, std::move(cancel)};
    }

    void display_alert(std::string title,
                       std::string message,
                       std::string accept,
                       std::string cancel,
                       std::function<void(bool)> on_result) override {
        last_alert_ = alert_request{std::move(title), std::move(message),
                                    std::move(accept), std::move(cancel)};
        const bool result = next_alert_result_.value_or(false);
        next_alert_result_.reset();
        if (on_result) on_result(result);
    }

    void display_action_sheet(std::string title,
                              std::string cancel,
                              std::string destruction,
                              std::vector<std::string> buttons,
                              std::function<void(std::string)> on_result) override {
        action_sheet_request req{std::move(title), cancel,
                                 std::move(destruction), std::move(buttons)};
        last_action_sheet_ = req;
        std::string result = next_action_sheet_result_.value_or(req.cancel);
        next_action_sheet_result_.reset();
        if (on_result) on_result(std::move(result));
    }

    void display_prompt(std::string title,
                        std::string message,
                        std::function<void(std::optional<std::string>)> on_result,
                        std::string accept        = "OK",
                        std::string cancel        = "Cancel",
                        std::string placeholder   = "",
                        std::string initial_value = "") override {
        last_prompt_ = prompt_request{std::move(title), std::move(message),
                                      std::move(accept), std::move(cancel),
                                      std::move(placeholder),
                                      std::move(initial_value)};
        std::optional<std::string> result = next_prompt_primed_
                                                ? next_prompt_result_
                                                : std::nullopt;
        next_prompt_primed_ = false;
        next_prompt_result_.reset();
        if (on_result) on_result(std::move(result));
    }

    // --- Programmable responders (mock-specific) ----------------------------

    void set_next_alert_result(bool accepted) { next_alert_result_ = accepted; }

    void set_next_action_sheet_result(std::string choice) {
        next_action_sheet_result_ = std::move(choice);
    }

    void set_next_prompt_result(std::optional<std::string> text) {
        next_prompt_result_ = std::move(text);
        next_prompt_primed_ = true;
    }

    // --- Recorded-request getters -------------------------------------------

    [[nodiscard]] const std::optional<alert_request>& last_alert() const noexcept {
        return last_alert_;
    }
    [[nodiscard]] const std::optional<action_sheet_request>& last_action_sheet() const noexcept {
        return last_action_sheet_;
    }
    [[nodiscard]] const std::optional<prompt_request>& last_prompt() const noexcept {
        return last_prompt_;
    }

    // Clears all recorded requests (useful between test assertions).
    void clear() noexcept {
        last_alert_.reset();
        last_action_sheet_.reset();
        last_prompt_.reset();
    }

private:
    std::optional<alert_request>        last_alert_{};
    std::optional<action_sheet_request> last_action_sheet_{};
    std::optional<prompt_request>       last_prompt_{};

    std::optional<bool>        next_alert_result_{};
    std::optional<std::string> next_action_sheet_result_{};
    std::optional<std::string> next_prompt_result_{};
    bool                       next_prompt_primed_ = false;
};

} // namespace mpapp

#endif // MPAPP_DIALOGS_HPP
