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
#include <functional>

#include "observable.hpp"
#include "platform.hpp"
#include "signal.hpp"
#include "view.hpp"

namespace mpapp::detail::graphics {
// Forward-declared — the canvas surface is in the detail/graphics
// header; user code that sets a drawable callback will include it
// directly. Forward-declaring here keeps graphics_view.hpp light.
class canvas;
} // namespace mpapp::detail::graphics

namespace mpapp {

template <class Platform = platform::current>
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

    // User-supplied draw callback. The real handler invokes this each
    // time it paints, passing a canvas sized to (width, height). Default
    // is a null function — handlers treat a null `drawable` as a no-op
    // paint (clear background only). Set via:
    //
    //     gv.drawable = [&](mpapp::detail::graphics::canvas& c) {
    //         c.set_fill(...); c.fill_rect({...}); ...
    //     };
    //
    // Include `<mpapp/detail/graphics/canvas.hpp>` for the full canvas
    // API. The Observable is wired to `draw_count` indirectly — setting
    // `drawable` re-triggers a paint on platforms whose handler watches
    // the .changed signal.
    using draw_callback_t = std::function<void(detail::graphics::canvas&)>;
    Observable<draw_callback_t> drawable{draw_callback_t{}};

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
