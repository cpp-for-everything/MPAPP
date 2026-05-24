// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/SwipeView.md
//
// `mpapp::swipe_view` — gesture-revealed action container. Wraps a single
// content child and reveals contextual action panels (`left_items` /
// `right_items`) when the user swipes horizontally on the content.
//
// This is the M-04b "real handlers on three platforms" landing of the
// SwipeView family — paired with `swipe_item_view` and `swipe_item_menu_item`.
// The mock surface is intentionally narrow: a `content` slot plus
// `left_items` / `right_items` vectors of `view*` action children. The
// richer top/bottom-items / `swipe_mode` / `is_open` / threshold surfaces
// described in the component doc land in a follow-up alongside the
// gesture-event plumbing.
//
// Degradation contract (per ADR-0006 + worker prompt): real handlers
// host the content + the action children but the GTK4 and Android paths
// do not implement actual swipe gestures yet — Windows alone uses the
// native `mux::Controls::SwipeControl`. Treat the Linux / Android
// renderings as "content-only" with the action items hidden until a
// follow-up batch wires their gesture recognisers.

#ifndef MPAPP_SWIPE_VIEW_HPP
#define MPAPP_SWIPE_VIEW_HPP

#include <vector>

#include "observable.hpp"
#include "platform.hpp"
#include "view.hpp"

namespace mpapp {

template <class Platform = platform::current>
class swipe_view_handler;

class swipe_view : public view {
public:
    swipe_view() = default;
    ~swipe_view() override = default;

    swipe_view(const swipe_view&)            = delete;
    swipe_view& operator=(const swipe_view&) = delete;
    swipe_view(swipe_view&&)                 = delete;
    swipe_view& operator=(swipe_view&&)      = delete;

    // The single wrapped content view that the user swipes over.
    // Non-owning raw pointer (caller owns the lifetime), matching `page`
    // / `window` rather than `refresh_view` / `flyout_view`. The worker
    // prompt specifies this shape.
    Observable<view*>                  content{nullptr};

    // Action collections revealed when the user swipes right / left.
    // Each entry is a non-owning `view*` (typically a
    // `swipe_item_view` or `swipe_item_menu_item`, but any `view`
    // subclass that registers with the ADR-0013 dispatch surface works).
    Observable<std::vector<view*>>     left_items{};
    Observable<std::vector<view*>>     right_items{};

    swipe_view_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const swipe_view_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                         has_handler() const noexcept { return handler_ != nullptr; }
    void                                         set_handler(swipe_view_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    swipe_view_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_SWIPE_VIEW_HPP
