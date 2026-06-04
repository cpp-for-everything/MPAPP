// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/Application.md
//
// `mpapp::application` — root of every MPAPP program. User subclasses
// override `on_launch()` (required) and the lifecycle hooks
// (`on_suspend`, `on_resume`, `on_terminate`) and runs the program via
// `mpapp::run<MyApp>(argc, argv)` (see <mpapp/run.hpp>).
//
// Single-instance per process on every platform (ADR-0012). The
// handler enforces this in its constructor.
//
// Lifetime: the user's `App` instance is constructed inside
// `mpapp::run<App>` and lives until the handler's event loop exits.
// All MPAPP widgets the user owns (button, label, window, layout, …)
// are typically members of the App, so their destruction order is
// guaranteed: the handler runs its own destructor *before* the App's
// fields run theirs, so platform-side teardown happens with the user
// widgets still alive.

#ifndef MPAPP_APPLICATION_HPP
#define MPAPP_APPLICATION_HPP

#include <cstddef>

#include "detail/modal_stack.hpp"
#include "essentials/app_info.hpp"   // for mpapp::app_theme
#include "platform.hpp"
#include "signal.hpp"
#include "view.hpp"

namespace mpapp {

// Primary template. Real specialisations live under
// `mpapp/handlers/<platform>/application_handler.hpp`.
template <class Platform = platform::current>
class application_handler;

class application {
public:
    application() = default;
    virtual ~application() = default;

    application(const application&)            = delete;
    application& operator=(const application&) = delete;
    application(application&&)                 = delete;
    application& operator=(application&&)      = delete;

    // ----- User lifecycle hooks -----------------------------------------
    // `on_launch` is the only required override — it's called on the UI
    // thread once the platform's init is complete (Windows: after
    // `Application::Start` invokes the App's `OnLaunched`; GTK: after
    // `g_application_run` enters; AppKit: after `applicationDidFinishLaunching`;
    // UIKit: after `application(_:didFinishLaunchingWithOptions:)`;
    // Android: after `onCreate` on the main looper).
    virtual void on_launch() = 0;
    virtual void on_suspend()   {}
    virtual void on_resume()    {}
    virtual void on_terminate() {}

    // ----- Modal navigation (MAUI Navigation.PushModalAsync) -------------
    // A modal page sits above the regular navigation surface and is
    // dismissed LIFO. `push_modal` / `pop_modal` are the sync primitives;
    // `modal_pushed` / `modal_popped` fire around each mutation.

    void push_modal(view& page) { modal_stack_.push(&page); }

    // Returns the dismissed modal (nullptr if no modal was showing).
    view* pop_modal() { return modal_stack_.pop(); }

    [[nodiscard]] std::size_t modal_stack_depth() const noexcept {
        return modal_stack_.depth();
    }

    // The top-most modal, or nullptr when nothing is presented modally.
    [[nodiscard]] view* current_modal() const noexcept {
        return modal_stack_.current();
    }

    // Engine access for handlers / advanced callers.
    detail::modal_stack&       modal_stack() noexcept       { return modal_stack_; }
    const detail::modal_stack& modal_stack() const noexcept { return modal_stack_; }

    // Re-exposed from the modal engine so callers can subscribe without
    // reaching into the stack.
    signal<view*>& modal_pushed() noexcept { return modal_stack_.modal_pushed; }
    signal<view*>& modal_popped() noexcept { return modal_stack_.modal_popped; }

    // ----- Requested theme (MAUI Application.RequestedTheme) -------------
    // The app-level requested theme. Setting a new value emits
    // `requested_theme_changed`; setting the same value is a no-op.

    [[nodiscard]] mpapp::app_theme requested_theme() const noexcept {
        return requested_theme_;
    }

    void set_requested_theme(mpapp::app_theme theme) {
        if (theme == requested_theme_) return;
        requested_theme_ = theme;
        requested_theme_changed.emit(theme);
    }

    signal<mpapp::app_theme> requested_theme_changed{};

    // ----- Handler access ------------------------------------------------
    // The framework wires this up from `mpapp::run<App>` before invoking
    // `on_launch`; user code may read it (e.g. to call `request_exit()`)
    // but generally has no reason to.
    application_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const application_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                          has_handler() const noexcept { return handler_ != nullptr; }
    void                                          set_handler(application_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    application_handler<platform::current>* handler_ = nullptr;
    detail::modal_stack                     modal_stack_{};
    mpapp::app_theme                        requested_theme_ = mpapp::app_theme::unspecified;
};

} // namespace mpapp

#endif // MPAPP_APPLICATION_HPP
