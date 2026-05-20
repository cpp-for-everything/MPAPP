// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/RefreshView.md
//
// `mpapp::refresh_view` — single-child container wrapping a scrollable
// child with a pull-to-refresh affordance. The spinner is visible (and
// animating) iff `is_refreshing == true`. `refresh_color` carries the
// symbolic tint; real handlers parse it into the platform-native color
// type.
//
// This is the M-04b "real handlers on three platforms" landing of the
// widget. The mock surface is intentionally narrow — just the three
// observable properties that the platform handlers consume. The richer
// `refresh_command` / `is_refresh_enabled` / `refreshing` event
// surface described in the component doc lands in a follow-up alongside
// gesture-event plumbing.

#ifndef MPAPP_REFRESH_VIEW_HPP
#define MPAPP_REFRESH_VIEW_HPP

#include <memory>

#include "observable.hpp"
#include "platform.hpp"
#include "view.hpp"

namespace mpapp {

template <class Platform>
class refresh_view_handler;

class refresh_view : public view {
public:
    refresh_view() = default;

    // The wrapped scrollable child. Same Observable<shared_ptr<view>>
    // shape used by scroll_view, content_view, and border.
    Observable<std::shared_ptr<view>>  content{};

    // True while a refresh is in flight; flips the spinner on. Consumers
    // are responsible for setting it back to false when their fetch
    // completes (matches MAUI's RefreshView contract).
    Observable<bool>                   is_refreshing{false};

    // Spinner tint, symbolic — handlers resolve to native color types.
    Observable<brush_ref>              refresh_color{};

    refresh_view_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const refresh_view_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                           has_handler() const noexcept { return handler_ != nullptr; }
    void                                           set_handler(refresh_view_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    refresh_view_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_REFRESH_VIEW_HPP
