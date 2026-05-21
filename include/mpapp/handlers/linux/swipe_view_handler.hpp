// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 swipe_view handler.
//
// GTK4 has no native swipe-actions widget. This handler is the M-04b
// content-only baseline: a vertical `GtkBox` hosts the wrapped content
// child; `left_items` / `right_items` are tracked through the registry
// but no actual gesture is wired yet. Full `GtkGestureDrag`-driven
// reveal animation lands in a follow-up batch alongside the Android
// `ViewDragHelper` plumbing.

#ifndef MPAPP_HANDLERS_LINUX_SWIPE_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_SWIPE_VIEW_HANDLER_HPP

#include <vector>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../swipe_view.hpp"
#include "../../view.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp {

template <>
class swipe_view_handler<platform::linux_> {
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

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void apply_content(view* v);
    void apply_left_items(const std::vector<view*>& items);
    void apply_right_items(const std::vector<view*>& items);

    struct content_cb_t {
        swipe_view_handler<platform::linux_>* self;
        void operator()(view* v) const { self->apply_content(v); }
    };
    struct left_cb_t {
        swipe_view_handler<platform::linux_>* self;
        void operator()(const std::vector<view*>& v) const { self->apply_left_items(v); }
    };
    struct right_cb_t {
        swipe_view_handler<platform::linux_>* self;
        void operator()(const std::vector<view*>& v) const { self->apply_right_items(v); }
    };

    void* native_        = nullptr; // GtkBox* (vertical) — host
    void* current_child_ = nullptr; // GtkWidget* — wrapped content

    content_cb_t                                content_cb_{this};
    left_cb_t                                   left_cb_{this};
    right_cb_t                                  right_cb_{this};
    signal_slot<view* const&>                   content_slot_{};
    signal_slot<const std::vector<view*>&>      left_slot_{};
    signal_slot<const std::vector<view*>&>      right_slot_{};
};

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_SWIPE_VIEW_HANDLER_HPP
