// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 swipe_view handler.
//
// Wraps `muxc::SwipeControl` — the WinUI 3 native that exposes swipe
// gestures over a single child. SwipeControl's `LeftItems` / `RightItems`
// take a `muxc::SwipeItems` collection of `muxc::SwipeItem` instances; we
// build that collection from the `left_items` / `right_items` vector
// when each entry is a `swipe_item_menu_item` (the only action shape that
// maps 1:1 onto SwipeItem's icon+text contract). Other entry types are
// silently skipped — they hang off the dispatch registry and can be
// targeted by a richer composition in a follow-up batch.
//
// The host SwipeControl IS the UIElement exposed to dispatch surfaces.

#ifndef MPAPP_HANDLERS_WINDOWS_SWIPE_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_SWIPE_VIEW_HANDLER_HPP

#include <vector>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../swipe_view.hpp"
#include "../../view.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp {

template <>
class swipe_view_handler<platform::windows> {
public:
    swipe_view_handler();
    ~swipe_view_handler();

    swipe_view_handler(const swipe_view_handler&)            = delete;
    swipe_view_handler& operator=(const swipe_view_handler&) = delete;
    swipe_view_handler(swipe_view_handler&&)                 = delete;
    swipe_view_handler& operator=(swipe_view_handler&&)      = delete;

    void map_content(swipe_view& sv);
    void map_left_items(swipe_view& sv);
    void map_right_items(swipe_view& sv);

    // The host SwipeControl IS the native UIElement exposed to dispatch surfaces.
    winrt::Microsoft::UI::Xaml::Controls::SwipeControl&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::SwipeControl& native() const noexcept { return native_; }

private:
    void apply_content(view* v);
    void apply_left_items(const std::vector<view*>& items);
    void apply_right_items(const std::vector<view*>& items);

    struct content_cb_t {
        swipe_view_handler<platform::windows>* self;
        void operator()(view* v) const { self->apply_content(v); }
    };
    struct left_cb_t {
        swipe_view_handler<platform::windows>* self;
        void operator()(const std::vector<view*>& v) const { self->apply_left_items(v); }
    };
    struct right_cb_t {
        swipe_view_handler<platform::windows>* self;
        void operator()(const std::vector<view*>& v) const { self->apply_right_items(v); }
    };

    winrt::Microsoft::UI::Xaml::Controls::SwipeControl native_{nullptr};

    content_cb_t                                content_cb_{this};
    left_cb_t                                   left_cb_{this};
    right_cb_t                                  right_cb_{this};
    signal_slot<view* const&>                   content_slot_{};
    signal_slot<const std::vector<view*>&>      left_slot_{};
    signal_slot<const std::vector<view*>&>      right_slot_{};
};

} // namespace mpapp

#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_SWIPE_VIEW_HANDLER_HPP
