// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Button.md
//
// `mpapp::internal::basic_button` — the platform-agnostic button surface.
// Split out from `mpapp/button.hpp` so the per-platform handler headers
// (which need basic_button as a complete type to spell out their
// `map_text(basic_button&)` signatures) can include just THIS header
// without pulling in `mpapp::button` (the wrapper), which would create
// a circular include: `button.hpp` -> handler umbrella -> per-platform
// handler -> `button.hpp` again.
//
// The wrapper `mpapp::button` lives in `mpapp/button.hpp` and embeds the
// per-platform handler by value. The surface here holds the handler by
// POINTER, so basic_button can be instantiated (e.g. in mock-first tests)
// without linking any platform handler symbols.

#ifndef MPAPP_INTERNAL_BASIC_BUTTON_HPP
#define MPAPP_INTERNAL_BASIC_BUTTON_HPP

#include <string>

#include "../control.hpp"
#include "../observable.hpp"
#include "../platform.hpp"
#include "../signal.hpp"

namespace mpapp::internal {

// Primary template — concrete specialisations live in
// `mpapp/handlers/<platform>/button_handler.hpp` and
// `mpapp/handlers/mock/button_handler.hpp`. Forward-declared here so
// `basic_button` can name it as a pointer-typed member without forcing
// a circular include.
template <class Platform = platform::current>
class button_handler;

class basic_button : public control<basic_button> {
public:
    basic_button() = default;

    basic_button(const basic_button&)            = delete;
    basic_button& operator=(const basic_button&) = delete;
    basic_button(basic_button&&)                 = delete;
    basic_button& operator=(basic_button&&)      = delete;

    // ----- Properties ----------------------------------------------------
    Observable<std::string> text{""};

    // ----- Events --------------------------------------------------------
    mpapp::signal<>         clicked;

    // ----- Handler attachment (pointer-based, opt-in) --------------------
    button_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const button_handler<platform::current>& handler() const noexcept { return *handler_; }

    bool has_handler() const noexcept { return handler_ != nullptr; }

    void set_handler(button_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    button_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp::internal

#endif // MPAPP_INTERNAL_BASIC_BUTTON_HPP
