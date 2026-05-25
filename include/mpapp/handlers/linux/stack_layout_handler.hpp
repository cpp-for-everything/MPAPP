// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — GTK4 basic_stack_layout handler.
//
// Wraps a `GtkBox`. Orientation drives `gtk_box_set_orientation`;
// spacing drives `gtk_box_set_spacing`; padding drives the four
// `gtk_widget_set_margin_*` calls; alignment drives
// `gtk_widget_set_halign` / `set_valign`.

#ifndef MPAPP_HANDLERS_LINUX_STACK_LAYOUT_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_STACK_LAYOUT_HANDLER_HPP

#include "../../layout.hpp"
#include "../../layout_types.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_stack_layout.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class stack_layout_handler<platform::linux_> {
public:
    stack_layout_handler();
    ~stack_layout_handler();

    stack_layout_handler(const stack_layout_handler&)            = delete;
    stack_layout_handler& operator=(const stack_layout_handler&) = delete;
    stack_layout_handler(stack_layout_handler&&)                 = delete;
    stack_layout_handler& operator=(stack_layout_handler&&)      = delete;

    void bind(basic_stack_layout& s);
    void add_child(view& child);

    // GtkBox*, type-erased.
    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

// RFC-0003: walks `x.gesture_recognizers` and installs

// matching GtkGesture* controllers via

// `mpapp::internal::linux_gestures::attach`.

void map_gestures(basic_stack_layout& x);


private:
    void apply_orientation(orientation o);
    void apply_spacing(double s);
    void apply_padding(thickness t);
    void apply_horizontal_alignment(h_align a);
    void apply_vertical_alignment(v_align a);

    struct orient_cb_t {
        stack_layout_handler<platform::linux_>* self;
        void operator()(const orientation& v) const { self->apply_orientation(v); }
    };
    struct spacing_cb_t {
        stack_layout_handler<platform::linux_>* self;
        void operator()(const double& v) const { self->apply_spacing(v); }
    };
    struct padding_cb_t {
        stack_layout_handler<platform::linux_>* self;
        void operator()(const thickness& v) const { self->apply_padding(v); }
    };
    struct h_align_cb_t {
        stack_layout_handler<platform::linux_>* self;
        void operator()(const h_align& v) const { self->apply_horizontal_alignment(v); }
    };
    struct v_align_cb_t {
        stack_layout_handler<platform::linux_>* self;
        void operator()(const v_align& v) const { self->apply_vertical_alignment(v); }
    };

    void*         native_ = nullptr;  // GtkWidget* (GtkBox)
    basic_stack_layout* bound_  = nullptr;

    orient_cb_t                       orient_cb_{this};
    spacing_cb_t                      spacing_cb_{this};
    padding_cb_t                      padding_cb_{this};
    h_align_cb_t                      h_align_cb_{this};
    v_align_cb_t                      v_align_cb_{this};
    signal_slot<const orientation&>   orient_slot_{};
    signal_slot<const double&>        spacing_slot_{};
    signal_slot<const thickness&>     padding_slot_{};
    signal_slot<const h_align&>       h_align_slot_{};
    signal_slot<const v_align&>       v_align_slot_{};
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_STACK_LAYOUT_HANDLER_HPP
