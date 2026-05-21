// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/GraphicsView.md
//
// `mpapp::graphics_view` — Skia-style canvas surface. The user
// subscribes to `draw_requested` and issues a stream of low-level
// draw commands against a backend-specific drawing context. Mock
// surface exposes the redraw trigger + a `draw_count` counter for
// tests; the real handlers bind to Skia / Cairo / Quartz / GDI+.

#ifndef MPAPP_GRAPHICS_VIEW_HPP
#define MPAPP_GRAPHICS_VIEW_HPP

#include <cstddef>

#include "observable.hpp"
#include "platform.hpp"
#include "signal.hpp"
#include "view.hpp"

namespace mpapp {

template <class Platform>
class graphics_view_handler;

class graphics_view : public view {
public:
    graphics_view() = default;
    ~graphics_view() override = default;

    graphics_view(const graphics_view&)            = delete;
    graphics_view& operator=(const graphics_view&) = delete;
    graphics_view(graphics_view&&)                 = delete;
    graphics_view& operator=(graphics_view&&)      = delete;

    // ----- Surface ------------------------------------------------------

    Observable<int> width{0};
    Observable<int> height{0};

    // Read-only counter — bumped each time invalidate() is called.
    Observable<std::size_t> draw_count{0};

    // ----- Signals ------------------------------------------------------

    // The real handler emits this on the UI thread when the platform
    // tells it to redraw (resize, expose, manual invalidate). User code
    // subscribes and issues draw commands.
    signal<> draw_requested{};

    // ----- Mutators -----------------------------------------------------

    // Schedule a redraw. Mock: bump draw_count, emit draw_requested.
    void invalidate() {
        draw_count.set(draw_count.get() + 1);
        draw_requested.emit();
    }

    // ----- Handler ------------------------------------------------------

    graphics_view_handler<platform::current>&       gv_handler() noexcept       { return *gv_handler_; }
    const graphics_view_handler<platform::current>& gv_handler() const noexcept { return *gv_handler_; }
    bool                                            has_gv_handler() const noexcept { return gv_handler_ != nullptr; }
    void                                            set_gv_handler(graphics_view_handler<platform::current>& h) noexcept { gv_handler_ = &h; }

private:
    graphics_view_handler<platform::current>* gv_handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_GRAPHICS_VIEW_HPP
