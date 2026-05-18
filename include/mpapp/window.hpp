// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Window.md
//
// `mpapp::window` — top-level chrome. Owns a title, a single content
// view (non-owning reference), and a few sizing properties. The platform
// handler mirrors writes into a native window (Windows: `mux::Window`,
// Linux: `GtkWindow`, macOS: `NSWindow`, iOS: `UIWindow`, Android:
// `Activity` content view).
//
// `content` is a non-owning `view*`. The user owns the view's lifetime
// (typically as a sibling field of the `mpapp::application` subclass).
// When the value of `content` changes the handler rebinds the native
// window's content slot.

#ifndef MPAPP_WINDOW_HPP
#define MPAPP_WINDOW_HPP

#include <string>

#include "control.hpp"
#include "observable.hpp"
#include "platform.hpp"
#include "signal.hpp"

namespace mpapp {

class view;

template <class Platform>
class window_handler;

class window : public control<window> {
public:
    window() = default;
    ~window() = default;

    window(const window&)            = delete;
    window& operator=(const window&) = delete;
    window(window&&)                 = delete;
    window& operator=(window&&)      = delete;

    // ----- Properties ----------------------------------------------------
    Observable<std::string> title{""};
    Observable<view*>       content{nullptr};
    // Logical (DIP) size hint. Zero means "let the platform pick".
    Observable<int>         width{0};
    Observable<int>         height{0};
    Observable<bool>        is_visible{false};

    // ----- Events --------------------------------------------------------
    // Fires after the platform window becomes visible (`Activate` /
    // `present` / `makeKeyAndVisible`).
    mpapp::signal<>         activated;
    // Fires after the user (or programmatic close()) dismisses the window.
    mpapp::signal<>         closed;

    // ----- Imperative commands ------------------------------------------
    // Handler forwards to the native `Activate`/`makeKeyAndVisible`/
    // `gtk_window_present`. The activation also sets `is_visible = true`
    // which lets a binding observe the state change.
    void show();
    void close();

    // ----- Handler -------------------------------------------------------
    window_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const window_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                     has_handler() const noexcept { return handler_ != nullptr; }
    void                                     set_handler(window_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    window_handler<platform::current>* handler_ = nullptr;
};

// Inline implementations — the handler chooses to interpret these as
// "set is_visible, signal the handler" without re-entering the Observable
// machinery (it's just a flag flip).

inline void window::show() {
    is_visible.set(true);
}

inline void window::close() {
    is_visible.set(false);
    closed.emit();
}

} // namespace mpapp

#endif // MPAPP_WINDOW_HPP
