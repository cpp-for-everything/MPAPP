// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/FlyoutView.md
//
// `mpapp::flyout_view` — two-pane master/detail container. The
// `flyout` pane is the slide-out side panel; `detail` is the main
// content area. `is_presented` toggles the drawer open/closed and is
// expected to be two-way bindable by `FlyoutPage`/`Shell` consumers.
//
// This is the M-04b "real handlers on three platforms" landing of the
// widget. The mock surface is intentionally narrow — just the three
// observable properties (`flyout`, `detail`, `is_presented`) that
// every host platform can implement uniformly. The richer
// `flyout_behavior`/`flyout_width`/`is_gesture_enabled` surface
// described in the component doc lands in a follow-up alongside the
// Shell/FlyoutPage page-level wiring.

#ifndef MPAPP_FLYOUT_VIEW_HPP
#define MPAPP_FLYOUT_VIEW_HPP

#include <memory>

#include "observable.hpp"
#include "platform.hpp"
#include "view.hpp"

namespace mpapp {

template <class Platform>
class flyout_view_handler;

class flyout_view : public view {
public:
    flyout_view() = default;

    // The drawer pane. On Windows this becomes the NavigationView's
    // PaneContent; on Linux it is the start child of a horizontal
    // GtkPaned; on Android it is added to the DrawerLayout with
    // Gravity.START.
    Observable<std::shared_ptr<view>>  flyout{};

    // The main content pane. On Windows: NavigationView.Content; on
    // Linux: end child of the GtkPaned; on Android: the first
    // DrawerLayout child (the non-drawer one).
    Observable<std::shared_ptr<view>>  detail{};

    // True when the drawer is open. Matches MAUI's `IsPresented`.
    Observable<bool>                   is_presented{false};

    flyout_view_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const flyout_view_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                          has_handler() const noexcept { return handler_ != nullptr; }
    void                                          set_handler(flyout_view_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    flyout_view_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_FLYOUT_VIEW_HPP
