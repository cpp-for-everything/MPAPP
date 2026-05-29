// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Button.md
//
// Two-layer design:
//
//   * `mpapp::internal::basic_button` — the platform-agnostic surface,
//     defined in `internal/basic_button.hpp`. Observable properties,
//     signals, and a `handler_` POINTER (not a value member) so the
//     surface doesn't pull in any platform-handler symbols. Tests
//     construct `basic_button` directly + attach a mock handler
//     externally — same as the historical pattern.
//
//   * `mpapp::button` — user-facing wrapper. Inherits `basic_button`
//     publicly (so `b.text`, `b.clicked`, etc. all work via inheritance)
//     AND embeds a platform-current handler as a value member, with the
//     constructor auto-binding handler ↔ surface. App code never spells
//     out a platform tag and never needs a separate handler variable:
//
//         mpapp::button b;          // surface + handler bound in ctor
//         b.text = "Save";
//         b.clicked.subscribe(slot, [](){ … });
//         // On Linux/GTK4, b.handler().native() is a GtkButton*.
//
// Tests stay on the surface:
//
//     mpapp::internal::basic_button b;
//     mpapp::internal::button_handler<mpapp::platform::mock> h;
//     h.map_text(b);
//     b.text = "hello";
//     REQUIRE(h.calls_as_strings() == …);
//
// This keeps `mock_handlers_test` from needing to link the per-platform
// handler libraries (`basic_button` holds a handler POINTER, not a value).
// Apps get the auto-wired ergonomics; tests keep their mock-first
// architecture intact.
//
// Lifecycle note: `mpapp::button`'s constructor runs the platform
// handler's constructor — on Linux that's `gtk_button_new_with_label("")`,
// on Windows a `winrt::Microsoft::UI::Xaml::Controls::Button{}`, etc.
// These require the UI subsystem to be initialized. `mpapp::run<App>`
// does the init before constructing the app instance, so app-shell
// members are safe. Free-floating `mpapp::button` at namespace scope
// or pre-`run<>` will trip those native APIs — use `basic_button` if
// you need a deferred-binding surface.

#ifndef MPAPP_BUTTON_HPP
#define MPAPP_BUTTON_HPP

#include "internal/basic_button.hpp"

// `basic_button` is fully declared above. Pull the platform-current
// handler full definition (umbrella picks the right per-platform
// header based on the build target). The handler header is allowed to
// see `basic_button` as a complete type now, which lets its inline
// bodies (mock + per-platform) access `b.text` etc.
//
// The umbrella → per-platform handler header chain only depends on
// `internal/basic_button.hpp`, so this include does not re-enter
// `mpapp/button.hpp` and the wrapper class below can embed the handler
// as a complete-type value member.
#include "handlers/button_handler.hpp"

namespace mpapp {

// User-facing wrapper. Auto-binds `basic_button` ↔ `button_handler`
// at construction. App code reads as `mpapp::button b; b.text = "...";`
// with no handler variable in sight.
class button : public internal::basic_button {
public:
    button() {
        set_handler(embedded_handler_);
        embedded_handler_.map_text(*this);
        embedded_handler_.map_clicked(*this);
        embedded_handler_.map_semantics(*this);
        // RFC-0003: walk basic_view::gesture_recognizers and install
        // the per-platform native listeners. No-op on platform::mock;
        // attaches GtkGesture* on Linux, UI*GestureRecognizer on
        // Apple, etc. once the per-platform map_gestures land.
        embedded_handler_.map_gestures(*this);
    }

    button(const button&)            = delete;
    button& operator=(const button&) = delete;
    button(button&&)                 = delete;
    button& operator=(button&&)      = delete;

private:
    internal::button_handler<platform::current> embedded_handler_;
};

// User-facing handler-type alias. App code rarely names this since the
// `button` wrapper owns its handler; useful in advanced flows that
// want to hold a handler reference directly, or for mock-platform tests
// that need to spell out `mpapp::button_handler<platform::mock>`.
//
// Template alias with `Platform = platform::current` default lets
// callers write `mpapp::button_handler<>` (host-current handler) or
// `mpapp::button_handler<platform::mock>` (mock handler for tests)
// without naming `internal::`.
template <class Platform = platform::current>
using button_handler = internal::button_handler<Platform>;

} // namespace mpapp

#endif // MPAPP_BUTTON_HPP
