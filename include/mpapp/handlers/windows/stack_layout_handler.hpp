// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — WinUI 3 basic_stack_layout handler.
//
// `stack_layout_handler<platform::windows>` — wraps a
// `winrt::Microsoft::UI::Xaml::Controls::StackPanel`, propagates
// orientation / spacing / padding / alignment from the cross-platform
// Observables, and dispatches `add` / `remove` / `clear` to the
// underlying `Children` collection.
//
// Children are non-owning `view*`s. The user-side ownership lives on
// the `mpapp::application` subclass; the handler only forwards.

#ifndef MPAPP_HANDLERS_WINDOWS_STACK_LAYOUT_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_STACK_LAYOUT_HANDLER_HPP

#include "../../layout.hpp"
#include "../../layout_types.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_stack_layout.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp::internal {

template <>
class stack_layout_handler<platform::windows> {
public:
    stack_layout_handler();
    ~stack_layout_handler();

    stack_layout_handler(const stack_layout_handler&)            = delete;
    stack_layout_handler& operator=(const stack_layout_handler&) = delete;
    stack_layout_handler(stack_layout_handler&&)                 = delete;
    stack_layout_handler& operator=(stack_layout_handler&&)      = delete;

    // Wires every property + the initial child list in one call.
    void bind(basic_stack_layout& s);

    // Append a child view to the native StackPanel. Dispatches on the
    // known view subclasses (basic_button / basic_label / nested basic_stack_layout) to
    // pull the right native widget. New widget types added here as the
    // surface grows.
    void add_child(view& child);

    // Native widget access — used by the window handler when this
    // basic_stack_layout is set as the window's content.
    winrt::Microsoft::UI::Xaml::Controls::StackPanel&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::StackPanel& native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_stack_layout& /*x*/) noexcept {}


private:
    void apply_orientation(orientation o);
    void apply_spacing(double s);
    void apply_padding(thickness t);
    void apply_horizontal_alignment(h_align a);
    void apply_vertical_alignment(v_align a);

    struct orient_cb_t {
        stack_layout_handler<platform::windows>* self;
        void operator()(const orientation& v) const { self->apply_orientation(v); }
    };
    struct spacing_cb_t {
        stack_layout_handler<platform::windows>* self;
        void operator()(const double& v) const { self->apply_spacing(v); }
    };
    struct padding_cb_t {
        stack_layout_handler<platform::windows>* self;
        void operator()(const thickness& v) const { self->apply_padding(v); }
    };
    struct h_align_cb_t {
        stack_layout_handler<platform::windows>* self;
        void operator()(const h_align& v) const { self->apply_horizontal_alignment(v); }
    };
    struct v_align_cb_t {
        stack_layout_handler<platform::windows>* self;
        void operator()(const v_align& v) const { self->apply_vertical_alignment(v); }
    };

    winrt::Microsoft::UI::Xaml::Controls::StackPanel native_{nullptr};
    basic_stack_layout*                                    bound_ = nullptr;

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
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_STACK_LAYOUT_HANDLER_HPP
