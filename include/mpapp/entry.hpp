// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Entry.md
//
// `mpapp::entry` — single-line text input. Mock-status surface: the full
// public API per the Entry component note, with a `platform::mock`
// handler that records property changes for unit tests. Real platform
// handlers replace the mock as each platform lands.

#ifndef MPAPP_ENTRY_HPP
#define MPAPP_ENTRY_HPP

#include <string>

#include "command.hpp"
#include "control.hpp"
#include "observable.hpp"
#include "platform.hpp"

namespace mpapp {

template <class Platform>
class entry_handler;

class entry : public control<entry> {
public:
    entry() = default;

    entry(const entry&)            = delete;
    entry& operator=(const entry&) = delete;
    entry(entry&&)                 = delete;
    entry& operator=(entry&&)      = delete;

    // ----- Properties -----------------------------------------------------
    // The full Entry surface tracks every MAUI ITextInput slot. Strongly-
    // typed value types (color, font, keyboard, …) ship in later batches;
    // for the mock surface only primitive-typed Observables are wired so
    // every component can be exercised end-to-end on every host.
    Observable<std::string> text{""};
    Observable<std::string> placeholder{""};
    Observable<bool>        is_password{false};
    Observable<bool>        is_read_only{false};
    Observable<int>         max_length{-1};
    Observable<int>         cursor_position{0};

    // ----- Commands -------------------------------------------------------
    // Bindable command hooks per ADR-0009 — Command<...> tag types make
    // the slot discoverable by the XAML compiler without a public macro.
    void completed(Command<> = {}) {}
    void text_changed(std::string, Command<std::string> = {}) {}

    // ----- Handler --------------------------------------------------------
    entry_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const entry_handler<platform::current>& handler() const noexcept { return *handler_; }

    bool has_handler() const noexcept { return handler_ != nullptr; }
    void set_handler(entry_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    entry_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_ENTRY_HPP
