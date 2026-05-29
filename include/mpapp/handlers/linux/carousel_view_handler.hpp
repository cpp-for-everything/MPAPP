// SPDX-License-Identifier: Apache-2.0
// GTK4 basic_carousel_view handler.
//
// Native widget: a GtkStack (one named page "0".."n-1" per item) with a
// slide-left/right transition. GtkStack has no built-in swipe, so a
// GtkGestureSwipe controller on the stack detects horizontal flings and
// advances/retreats the page through `basic_carousel_view::scroll_to`
// (which applies loop/clamp and emits position_changed). Programmatic
// `position` changes drive `gtk_stack_set_visible_child_name`. peek_count
// is a v1 no-op on GTK (GtkStack shows exactly one page); the real
// peek-area inset would need Adw.Carousel (libadwaita) — a follow-up.

#ifndef MPAPP_HANDLERS_LINUX_CAROUSEL_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_CAROUSEL_VIEW_HANDLER_HPP

#include <string>
#include <vector>

#include "../../internal/basic_carousel_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class carousel_view_handler<platform::linux_> {
public:
    carousel_view_handler();
    ~carousel_view_handler();

    carousel_view_handler(const carousel_view_handler&)            = delete;
    carousel_view_handler& operator=(const carousel_view_handler&) = delete;
    carousel_view_handler(carousel_view_handler&&)                 = delete;
    carousel_view_handler& operator=(carousel_view_handler&&)      = delete;

    void map_items_source(basic_carousel_view& c);
    void map_position(basic_carousel_view& c);
    void map_loop(basic_carousel_view& c);
    void map_is_swipe_enabled(basic_carousel_view& c);
    void map_peek_count(basic_carousel_view& c);
    void map_gestures(basic_carousel_view& c);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void rebuild_pages(const std::vector<std::string>& v);
    void apply_position(int idx);
    void apply_swipe_enabled(bool on);

    struct items_cb_t {
        carousel_view_handler<platform::linux_>* self;
        void operator()(const std::vector<std::string>&) const { self->rebuild_after_items(); }
    };
    struct pos_cb_t {
        carousel_view_handler<platform::linux_>* self;
        void operator()(int v) const { self->apply_position(v); }
    };
    struct swipe_cb_t {
        carousel_view_handler<platform::linux_>* self;
        void operator()(bool v) const { self->apply_swipe_enabled(v); }
    };
    void rebuild_after_items();

    void*  native_      = nullptr;  // GtkStack*
    void*  swipe_ctrl_  = nullptr;  // GtkGestureSwipe* (owned by the widget)
    basic_carousel_view* bound_ = nullptr;

    items_cb_t  items_cb_{this};
    pos_cb_t    pos_cb_{this};
    swipe_cb_t  swipe_cb_{this};
    signal_slot<const std::vector<std::string>&> items_slot_{};
    signal_slot<const int&>                       pos_slot_{};
    signal_slot<const bool&>                       swipe_slot_{};
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_CAROUSEL_VIEW_HANDLER_HPP
