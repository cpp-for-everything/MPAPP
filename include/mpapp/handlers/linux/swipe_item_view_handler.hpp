// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 swipe_item_view handler.
//
// A `GtkBox` (vertical) hosting the custom content child. The host
// `swipe_view` does not yet wire the reveal gesture (M-04b content-only
// baseline), so the item is rendered inline as a plain content host.

#ifndef MPAPP_HANDLERS_LINUX_SWIPE_ITEM_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_SWIPE_ITEM_VIEW_HANDLER_HPP

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../swipe_item_view.hpp"
#include "../../view.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp {

template <>
class swipe_item_view_handler<platform::linux_> {
public:
    swipe_item_view_handler();
    ~swipe_item_view_handler();

    swipe_item_view_handler(const swipe_item_view_handler&)            = delete;
    swipe_item_view_handler& operator=(const swipe_item_view_handler&) = delete;
    swipe_item_view_handler(swipe_item_view_handler&&)                 = delete;
    swipe_item_view_handler& operator=(swipe_item_view_handler&&)      = delete;

    void map_content(swipe_item_view& iv);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void apply_content(view* v);

    struct content_cb_t {
        swipe_item_view_handler<platform::linux_>* self;
        void operator()(view* v) const { self->apply_content(v); }
    };

    void* native_        = nullptr; // GtkBox* (vertical)
    void* current_child_ = nullptr; // GtkWidget* — wrapped content

    content_cb_t                content_cb_{this};
    signal_slot<view* const&>   content_slot_{};
};

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_SWIPE_ITEM_VIEW_HANDLER_HPP
