// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Page.md
//
// `mpapp::page` — user-facing wrapper around the platform-agnostic
// `mpapp::internal::basic_page` surface. Embeds the per-platform
// handler by value and auto-binds it in the constructor, so app code
// reads as `mpapp::page x; x.<prop> = ...;` with no separate
// handler variable.
//
// Tests stay on the surface (so they don't drag in the per-platform
// handler library):
//
//     mpapp::internal::basic_page x;
//     mpapp::page_handler<mpapp::platform::mock> h;
//     h.map_title(x);

#ifndef MPAPP_PAGE_HPP
#define MPAPP_PAGE_HPP

#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "dialogs.hpp"
#include "internal/basic_page.hpp"

// Pull in the platform-current handler full definition (umbrella picks
// the right per-platform header). The handler header is allowed to see
// `basic_page` as a complete type now, which lets its inline bodies
// (mock + per-platform) access surface members.
#include "handlers/page_handler.hpp"

namespace mpapp {

class page : public internal::basic_page {
public:
    page() {
        set_handler(embedded_handler_);
        embedded_handler_.map_title(*this);
        embedded_handler_.map_content(*this);
        embedded_handler_.map_is_busy(*this);
        embedded_handler_.map_gestures(*this);
    }

    page(const page&)            = delete;
    page& operator=(const page&) = delete;
    page(page&&)                 = delete;
    page& operator=(page&&)      = delete;

    // ----- Dialog services (MAUI Page.DisplayAlert / DisplayActionSheet /
    //        DisplayPromptAsync) -------------------------------------------
    //
    // These forward to the embedded dialog service. In the mock build the
    // service records each request and replays a programmed response into
    // `on_result` synchronously (see <mpapp/dialogs.hpp>), so app logic is
    // testable without an event loop. `dialogs()` exposes the service for
    // priming responses and reading recorded requests.

    void display_alert(std::string title, std::string message, std::string cancel) {
        dialogs_.display_alert(std::move(title), std::move(message), std::move(cancel));
    }

    void display_alert(std::string title, std::string message,
                       std::string accept, std::string cancel,
                       std::function<void(bool)> on_result) {
        dialogs_.display_alert(std::move(title), std::move(message),
                               std::move(accept), std::move(cancel),
                               std::move(on_result));
    }

    void display_action_sheet(std::string title, std::string cancel,
                              std::string destruction,
                              std::vector<std::string> buttons,
                              std::function<void(std::string)> on_result) {
        dialogs_.display_action_sheet(std::move(title), std::move(cancel),
                                      std::move(destruction), std::move(buttons),
                                      std::move(on_result));
    }

    void display_prompt(std::string title, std::string message,
                        std::function<void(std::optional<std::string>)> on_result,
                        std::string accept        = "OK",
                        std::string cancel        = "Cancel",
                        std::string placeholder   = "",
                        std::string initial_value = "") {
        dialogs_.display_prompt(std::move(title), std::move(message),
                                std::move(on_result), std::move(accept),
                                std::move(cancel), std::move(placeholder),
                                std::move(initial_value));
    }

    // Access to the embedded dialog service for priming responses and
    // reading the recorded requests (`last_alert()`, `set_next_*`, …).
    mock_dialog_service&       dialogs() noexcept       { return dialogs_; }
    const mock_dialog_service& dialogs() const noexcept { return dialogs_; }

private:
    internal::page_handler<platform::current> embedded_handler_;
    mock_dialog_service                        dialogs_{};
};

// Template alias so `mpapp::page_handler<>` (host-current) and
// `mpapp::page_handler<platform::mock>` both work without naming
// `internal::`.
template <class Platform = platform::current>
using page_handler = internal::page_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_PAGE_HPP
