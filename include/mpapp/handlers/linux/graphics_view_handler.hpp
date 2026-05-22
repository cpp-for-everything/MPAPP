// SPDX-License-Identifier: Apache-2.0
// GTK4 graphics_view handler — wraps a GtkDrawingArea. v1 only
// establishes the surface and propagates resize via "resize" signal.
// User-facing cairo draw API is gated on ADR-0015.

#ifndef MPAPP_HANDLERS_LINUX_GRAPHICS_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_GRAPHICS_VIEW_HANDLER_HPP

#include "../../graphics_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp {

template <>
class graphics_view_handler<platform::linux_> {
public:
    graphics_view_handler();
    ~graphics_view_handler();

    graphics_view_handler(const graphics_view_handler&)            = delete;
    graphics_view_handler& operator=(const graphics_view_handler&) = delete;
    graphics_view_handler(graphics_view_handler&&)                 = delete;
    graphics_view_handler& operator=(graphics_view_handler&&)      = delete;

    void map_size(graphics_view& gv);
    void map_draw_count(graphics_view& gv);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void apply_width(int w);
    void apply_height(int h);

    struct w_cb_t {
        graphics_view_handler<platform::linux_>* self;
        void operator()(int v) const { self->apply_width(v); }
    };
    struct h_cb_t {
        graphics_view_handler<platform::linux_>* self;
        void operator()(int v) const { self->apply_height(v); }
    };

    void* native_ = nullptr;  // GtkDrawingArea*

    w_cb_t                  w_cb_{this};
    h_cb_t                  h_cb_{this};
    signal_slot<const int&> w_slot_{};
    signal_slot<const int&> h_slot_{};
};

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_GRAPHICS_VIEW_HANDLER_HPP
