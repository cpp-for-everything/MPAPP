// SPDX-License-Identifier: Apache-2.0
// WinUI 3 basic_flex_layout handler. WinUI has no built-in flexbox panel,
// so this wraps a custom mux::Controls::Panel whose measure/arrange
// implements flexbox semantics from the mapped container properties.
// Per-child layout reads the attached store on basic_flex_layout
// (order/grow/shrink/align_self/basis) in the add_child path.

#ifndef MPAPP_HANDLERS_WINDOWS_FLEX_LAYOUT_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_FLEX_LAYOUT_HANDLER_HPP

#include "../../internal/basic_flex_layout.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp::internal {

template <>
class flex_layout_handler<platform::windows> {
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

    // Add a child, reading its attached flex props (order, grow, shrink,
    // align_self, basis). The child's native UIElement is resolved via
    // the ADR-0013 dispatch registry.
    void add_child(basic_flex_layout& f, view& child);

    winrt::Microsoft::UI::Xaml::Controls::Panel&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::Panel& native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_flex_layout& /*x*/) noexcept {}


private:
    void apply_direction(flex_direction d);
    void apply_wrap(flex_wrap w);
    void apply_justify_content(flex_justify j);
    void apply_align_items(flex_align_items a);
    void apply_align_content(flex_align_content a);
    void apply_position(flex_position p);

    struct direction_cb_t {
        flex_layout_handler<platform::windows>* self;
        void operator()(flex_direction d) const { self->apply_direction(d); }
    };
    struct wrap_cb_t {
        flex_layout_handler<platform::windows>* self;
        void operator()(flex_wrap w) const { self->apply_wrap(w); }
    };
    struct justify_cb_t {
        flex_layout_handler<platform::windows>* self;
        void operator()(flex_justify j) const { self->apply_justify_content(j); }
    };
    struct align_items_cb_t {
        flex_layout_handler<platform::windows>* self;
        void operator()(flex_align_items a) const { self->apply_align_items(a); }
    };
    struct align_content_cb_t {
        flex_layout_handler<platform::windows>* self;
        void operator()(flex_align_content a) const { self->apply_align_content(a); }
    };
    struct position_cb_t {
        flex_layout_handler<platform::windows>* self;
        void operator()(flex_position p) const { self->apply_position(p); }
    };

    winrt::Microsoft::UI::Xaml::Controls::Panel native_{nullptr};

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
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_FLEX_LAYOUT_HANDLER_HPP
