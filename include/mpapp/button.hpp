// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Button.md
//
// `mpapp::button` — cross-platform button widget. Mock-status surface:
// today only `text` and `clicked` are wired against the Windows handler.
// The full surface (text_color, font, padding, …) lands in M-03.
//
// The handler is selected via `platform::current` so user code never
// names a platform tag explicitly:
//
//   mpapp::button b;
//   b.text = "Save";
//   b.clicked.subscribe(slot, [](){ … });
//   // On Windows, b.handler().native() is a winrt Button.

#ifndef MPAPP_BUTTON_HPP
#define MPAPP_BUTTON_HPP

#include <string>

#include "control.hpp"
#include "observable.hpp"
#include "platform.hpp"
#include "signal.hpp"

namespace mpapp {

// Primary template — concrete specialisations live in
// `mpapp/handlers/<platform>/button_handler.hpp`. Forward-declared here so
// `button` can name it as a member without forcing a circular include.
template <class Platform = platform::current>
class button_handler;

class button : public control<button> {
public:
    button() = default;

    button(const button&)            = delete;
    button& operator=(const button&) = delete;
    button(button&&)                 = delete;
    button& operator=(button&&)      = delete;

    // ----- Properties -----------------------------------------------------
    Observable<std::string> text{""};

    // ----- Events ---------------------------------------------------------
    // Subscribe with a `signal_slot<>` + callable, just like Observable's
    // `changed`. The Command<> tag-typed wrapper (XAML `Command="..."`
    // binding) will be added with the full Button surface in M-03.
    mpapp::signal<>         clicked;

    // ----- Handler --------------------------------------------------------
    // Lazily constructed by the host on attach. For the spike, the host
    // creates it directly via `make_handler()`.
    button_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const button_handler<platform::current>& handler() const noexcept { return *handler_; }

    bool has_handler() const noexcept { return handler_ != nullptr; }

    // Inject a handler. The host (see `examples/windows_button_spike/`)
    // owns the handler's lifetime; `button` only holds a non-owning ref.
    void set_handler(button_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    button_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_BUTTON_HPP
