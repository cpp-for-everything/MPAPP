// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/SwipeItemView.md
//
// `mpapp::swipe_item_view` — a custom-content swipe action. Lives inside
// a `swipe_view`'s `left_items` / `right_items` collection. Hosts an
// arbitrary `view*` child so callers can build fully bespoke action pills
// (tinted panel + progress ring, multi-button clusters, etc.) instead of
// the fixed icon+text shape of `swipe_item_menu_item`.
//
// Mock surface (M-04b): the single `content` slot. The richer
// `command` / `command_parameter` / `automation_id` / `invoked` surface
// described in the component doc lands alongside the gesture-event
// plumbing in a follow-up batch.

#ifndef MPAPP_INTERNAL_BASIC_SWIPE_ITEM_VIEW_HPP
#define MPAPP_INTERNAL_BASIC_SWIPE_ITEM_VIEW_HPP

#include "../observable.hpp"
#include "../platform.hpp"
#include "../view.hpp"

namespace mpapp::internal {

template <class Platform = platform::current>
class swipe_item_view_handler;

class basic_swipe_item_view : public view {
public:
    basic_swipe_item_view() = default;
    ~basic_swipe_item_view() override = default;

    basic_swipe_item_view(const basic_swipe_item_view&)            = delete;
    basic_swipe_item_view& operator=(const basic_swipe_item_view&) = delete;
    basic_swipe_item_view(basic_swipe_item_view&&)                 = delete;
    basic_swipe_item_view& operator=(basic_swipe_item_view&&)      = delete;

    // The custom content hosted inside the action pill. Non-owning,
    // matching `swipe_view::content`.
    Observable<view*>  content{nullptr};

    swipe_item_view_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const swipe_item_view_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                              has_handler() const noexcept { return handler_ != nullptr; }
    void                                              set_handler(swipe_item_view_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    swipe_item_view_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp::internal


#endif // MPAPP_INTERNAL_BASIC_SWIPE_ITEM_VIEW_HPP
