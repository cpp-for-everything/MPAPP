// SPDX-License-Identifier: Apache-2.0
// GTK4 basic_flex_layout handler. GTK has no native flexbox container, so
// this wraps a GtkBox-derived widget configured to emulate flexbox: the
// container properties (direction, wrap, justify_content, align_items,
// align_content) map to GtkOrientation + halign/valign + expand flags,
// and per-child attached props (grow/shrink) map to child hexpand/vexpand.
//
// Note: full multi-line flex_wrap support degrades to single-line on GTK;
// a future follow-up could wire a custom GtkLayoutManager for true wrap.

#ifndef MPAPP_HANDLERS_LINUX_FLEX_LAYOUT_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_FLEX_LAYOUT_HANDLER_HPP

#include "../../internal/basic_flex_layout.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class flex_layout_handler<platform::linux_> {
public:
    flex_layout_handler();
    ~flex_layout_handler();

    flex_layout_handler(const flex_layout_handler&)            = delete;
    flex_layout_handler& operator=(const flex_layout_handler&) = delete;
    flex_layout_handler(flex_layout_handler&&)                 = delete;
    flex_layout_handler& operator=(flex_layout_handler&&)      = delete;

    void map_direction(basic_flex_layout& f);
    void map_wrap(basic_flex_layout& f);
    void map_justify_content(basic_flex_layout& f);
    void map_align_items(basic_flex_layout& f);
    void map_align_content(basic_flex_layout& f);
    void map_position(basic_flex_layout& f);

    void add_child(basic_flex_layout& f, view& child);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

// RFC-0003: walks `x.gesture_recognizers` and installs

// matching GtkGesture* controllers via

// `mpapp::internal::linux_gestures::attach`.

void map_gestures(basic_flex_layout& x);


private:
    void apply_direction(flex_direction d);
    void apply_wrap(flex_wrap w);
    void apply_justify_content(flex_justify j);
    void apply_align_items(flex_align_items a);
    void apply_align_content(flex_align_content a);
    void apply_position(flex_position p);

    struct direction_cb_t {
        flex_layout_handler<platform::linux_>* self;
        void operator()(flex_direction d) const { self->apply_direction(d); }
    };
    struct wrap_cb_t {
        flex_layout_handler<platform::linux_>* self;
        void operator()(flex_wrap w) const { self->apply_wrap(w); }
    };
    struct justify_cb_t {
        flex_layout_handler<platform::linux_>* self;
        void operator()(flex_justify j) const { self->apply_justify_content(j); }
    };
    struct align_items_cb_t {
        flex_layout_handler<platform::linux_>* self;
        void operator()(flex_align_items a) const { self->apply_align_items(a); }
    };
    struct align_content_cb_t {
        flex_layout_handler<platform::linux_>* self;
        void operator()(flex_align_content a) const { self->apply_align_content(a); }
    };
    struct position_cb_t {
        flex_layout_handler<platform::linux_>* self;
        void operator()(flex_position p) const { self->apply_position(p); }
    };

    void* native_ = nullptr;  // GtkBox*

    direction_cb_t     direction_cb_{this};
    wrap_cb_t          wrap_cb_{this};
    justify_cb_t       justify_cb_{this};
    align_items_cb_t   align_items_cb_{this};
    align_content_cb_t align_content_cb_{this};
    position_cb_t      position_cb_{this};

    signal_slot<const flex_direction&>     direction_slot_{};
    signal_slot<const flex_wrap&>          wrap_slot_{};
    signal_slot<const flex_justify&>       justify_slot_{};
    signal_slot<const flex_align_items&>   align_items_slot_{};
    signal_slot<const flex_align_content&> align_content_slot_{};
    signal_slot<const flex_position&>      position_slot_{};
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_FLEX_LAYOUT_HANDLER_HPP
