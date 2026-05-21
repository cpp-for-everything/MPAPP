// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 swipe_item_view handler.
//
// Wraps `muxc::ContentControl` — the simplest single-child host that
// participates in the visual tree as a `UIElement`. WinUI's native
// `muxc::SwipeItem` is icon-and-text only (not a UIElement) and cannot
// host arbitrary content, so we host the custom content tree as a
// ContentControl child of the parent `swipe_view`'s SwipeControl. The
// gesture invocation lifecycle is owned by the parent SwipeControl on
// the action collection it builds for `swipe_item_menu_item` entries;
// `swipe_item_view` is just a custom content carrier in this M-04b
// landing — full gesture-event plumbing lands in a follow-up batch.

#ifndef MPAPP_HANDLERS_WINDOWS_SWIPE_ITEM_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_SWIPE_ITEM_VIEW_HANDLER_HPP

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../swipe_item_view.hpp"
#include "../../view.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp {

template <>
class swipe_item_view_handler<platform::windows> {
public:
    swipe_item_view_handler();
    ~swipe_item_view_handler();

    swipe_item_view_handler(const swipe_item_view_handler&)            = delete;
    swipe_item_view_handler& operator=(const swipe_item_view_handler&) = delete;
    swipe_item_view_handler(swipe_item_view_handler&&)                 = delete;
    swipe_item_view_handler& operator=(swipe_item_view_handler&&)      = delete;

    void map_content(swipe_item_view& iv);

    winrt::Microsoft::UI::Xaml::Controls::ContentControl&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::ContentControl& native() const noexcept { return native_; }

private:
    void apply_content(view* v);

    struct content_cb_t {
        swipe_item_view_handler<platform::windows>* self;
        void operator()(view* v) const { self->apply_content(v); }
    };

    winrt::Microsoft::UI::Xaml::Controls::ContentControl native_{nullptr};

    content_cb_t                content_cb_{this};
    signal_slot<view* const&>   content_slot_{};
};

} // namespace mpapp

#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_SWIPE_ITEM_VIEW_HANDLER_HPP
