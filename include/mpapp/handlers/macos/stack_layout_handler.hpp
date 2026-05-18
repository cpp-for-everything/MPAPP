// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — AppKit stack_layout handler. Wraps NSStackView.

#ifndef MPAPP_HANDLERS_MACOS_STACK_LAYOUT_HANDLER_HPP
#define MPAPP_HANDLERS_MACOS_STACK_LAYOUT_HANDLER_HPP

#include "../../layout.hpp"
#include "../../layout_types.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../stack_layout.hpp"

#if defined(__APPLE__)
#  include <TargetConditionals.h>
#  if !TARGET_OS_IPHONE

namespace mpapp {

template <>
class stack_layout_handler<platform::macos> {
public:
    stack_layout_handler();
    ~stack_layout_handler();

    stack_layout_handler(const stack_layout_handler&)            = delete;
    stack_layout_handler& operator=(const stack_layout_handler&) = delete;
    stack_layout_handler(stack_layout_handler&&)                 = delete;
    stack_layout_handler& operator=(stack_layout_handler&&)      = delete;

    void bind(stack_layout& s);
    void add_child(view& child);

    // NSStackView*, retained, type-erased.
    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void apply_orientation(orientation o);
    void apply_spacing(double s);
    void apply_padding(thickness t);
    void apply_horizontal_alignment(h_align a);
    void apply_vertical_alignment(v_align a);

    struct orient_cb_t   { stack_layout_handler<platform::macos>* self; void operator()(const orientation& v) const { self->apply_orientation(v); } };
    struct spacing_cb_t  { stack_layout_handler<platform::macos>* self; void operator()(const double& v) const { self->apply_spacing(v); } };
    struct padding_cb_t  { stack_layout_handler<platform::macos>* self; void operator()(const thickness& v) const { self->apply_padding(v); } };
    struct h_align_cb_t  { stack_layout_handler<platform::macos>* self; void operator()(const h_align& v) const { self->apply_horizontal_alignment(v); } };
    struct v_align_cb_t  { stack_layout_handler<platform::macos>* self; void operator()(const v_align& v) const { self->apply_vertical_alignment(v); } };

    void*         native_ = nullptr;  // retained NSStackView*
    stack_layout* bound_  = nullptr;

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

} // namespace mpapp

#  endif // !TARGET_OS_IPHONE
#endif // __APPLE__
#endif // MPAPP_HANDLERS_MACOS_STACK_LAYOUT_HANDLER_HPP
