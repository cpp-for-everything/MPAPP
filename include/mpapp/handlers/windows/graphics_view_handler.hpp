// SPDX-License-Identifier: Apache-2.0
// WinUI 3 graphics_view handler — wraps a muxc::Canvas. User-facing
// canvas-drawing API is gated on ADR-0015; this v1 handler establishes
// the native surface and propagates resize events into width/height.

#ifndef MPAPP_HANDLERS_WINDOWS_GRAPHICS_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_GRAPHICS_VIEW_HANDLER_HPP

#include "../../graphics_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp {

template <>
class graphics_view_handler<platform::windows> {
public:
    graphics_view_handler();
    ~graphics_view_handler();

    graphics_view_handler(const graphics_view_handler&)            = delete;
    graphics_view_handler& operator=(const graphics_view_handler&) = delete;
    graphics_view_handler(graphics_view_handler&&)                 = delete;
    graphics_view_handler& operator=(graphics_view_handler&&)      = delete;

    void map_size(graphics_view& gv);
    void map_draw_count(graphics_view& gv);

    winrt::Microsoft::UI::Xaml::Controls::Canvas&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::Canvas& native() const noexcept { return native_; }

private:
    void apply_width(int w);
    void apply_height(int h);

    struct w_cb_t {
        graphics_view_handler<platform::windows>* self;
        void operator()(int v) const { self->apply_width(v); }
    };
    struct h_cb_t {
        graphics_view_handler<platform::windows>* self;
        void operator()(int v) const { self->apply_height(v); }
    };

    winrt::Microsoft::UI::Xaml::Controls::Canvas native_{nullptr};
    winrt::event_token size_changed_token_{};

    w_cb_t                  w_cb_{this};
    h_cb_t                  h_cb_{this};
    signal_slot<const int&> w_slot_{};
    signal_slot<const int&> h_slot_{};
};

} // namespace mpapp

#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_GRAPHICS_VIEW_HANDLER_HPP
