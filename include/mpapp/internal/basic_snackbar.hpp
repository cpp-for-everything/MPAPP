// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Snackbar.md
//
// `mpapp::internal::basic_snackbar` — CommunityToolkit Snackbar surface.
// Mock-first implementation (ADR-0008). Mirrors MAUI CommunityToolkit's
// `Snackbar`: a transient notification with an optional action button.
// Default duration matches the CTK default of 3000 ms.

#ifndef MPAPP_INTERNAL_BASIC_SNACKBAR_HPP
#define MPAPP_INTERNAL_BASIC_SNACKBAR_HPP

#include <string>

#include "../observable.hpp"
#include "../platform.hpp"
#include "../signal.hpp"
#include "../view.hpp"

namespace mpapp::internal {

// Primary template — concrete specialisations live in
// `mpapp/handlers/<platform>/snackbar_handler.hpp` and
// `mpapp/handlers/mock/snackbar_handler.hpp`. Forward-declared here so
// `basic_snackbar` can name it as a pointer-typed member without forcing
// a circular include.
template <class Platform = platform::current>
class snackbar_handler;

class basic_snackbar : public view {
public:
    basic_snackbar() = default;

    basic_snackbar(const basic_snackbar&)            = delete;
    basic_snackbar& operator=(const basic_snackbar&) = delete;
    basic_snackbar(basic_snackbar&&)                 = delete;
    basic_snackbar& operator=(basic_snackbar&&)      = delete;

    // ----- Properties -------------------------------------------------------

    // Primary notification message. MAUI CTK: `Snackbar.Text`.
    Observable<std::string> text{""};

    // Label shown on the optional action button. MAUI CTK: `Snackbar.ActionButtonText`.
    Observable<std::string> action_text{""};

    // Auto-dismiss timeout in milliseconds. MAUI CTK: `Snackbar.Duration`.
    Observable<double> duration_ms{3000.0};

    // Reflects whether the snackbar is currently visible. Set by show()/dismiss().
    Observable<bool> is_shown{false};

    // ----- Commands / actions -----------------------------------------------

    // Request the snackbar to become visible. The handler records the show
    // command and transitions `is_shown` to true.
    void show() {
        is_shown = true;
        shown.emit();
    }

    // Request the snackbar to hide. The handler records the dismiss command
    // and transitions `is_shown` to false.
    void dismiss() {
        is_shown = false;
        dismissed.emit();
    }

    // ----- Events -----------------------------------------------------------

    // Fired by show() after `is_shown` transitions to true.
    mpapp::signal<> shown;

    // Fired by dismiss() after `is_shown` transitions to false.
    mpapp::signal<> dismissed;

    // Fired when the user activates the action button. The handler calls
    // `action_invoked.emit()` in response to the native tap.
    mpapp::signal<> action_invoked;

    // ----- Handler attachment (pointer-based, opt-in) -----------------------
    [[nodiscard]] snackbar_handler<platform::current>&       handler() noexcept       { return *handler_; }
    [[nodiscard]] const snackbar_handler<platform::current>& handler() const noexcept { return *handler_; }
    [[nodiscard]] bool                                        has_handler() const noexcept { return handler_ != nullptr; }
    void                                                      set_handler(snackbar_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    snackbar_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp::internal

#endif // MPAPP_INTERNAL_BASIC_SNACKBAR_HPP
