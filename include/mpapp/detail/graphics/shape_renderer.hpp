// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Shared shape_view → canvas rendering helper.
//
// Each platform's shape_view real handler used to carry its own per-
// kind rendering code (Cairo on Linux, XAML Shape primitives on
// Windows, an Android custom view on Android). This header surfaces
// the rendering logic as a single function that draws a shape_view
// into a backend-agnostic ADR-0015 canvas of (w, h). Real handlers
// build a facade canvas, call this helper, then blit the resulting
// pixels into their native draw surface (same pattern graphics_view
// uses for arbitrary draw callbacks).
//
// One source of truth → all three platforms render identically once
// they've all migrated. Linux is the first cutover; Windows + Android
// follow in T-0031 phase 2.

#ifndef MPAPP_DETAIL_GRAPHICS_SHAPE_RENDERER_HPP
#define MPAPP_DETAIL_GRAPHICS_SHAPE_RENDERER_HPP

namespace mpapp {
class shape_view;
namespace detail::graphics {
class canvas;
}
} // namespace mpapp

namespace mpapp::detail::graphics {

// Render `sv` into `c` filling the (w, h) region. Honors the
// shape_view's kind / data / fill / stroke / stroke_thickness /
// opacity Observables. Fill and stroke hex colors are parsed via
// `color::from_hex`. An empty fill string skips the fill pass; same
// for stroke. The renderer never clears the canvas — the handler
// is expected to do that itself (or rely on the facade's default
// transparent allocation).
void render_shape_view(canvas&            c,
                       const shape_view&  sv,
                       int                w,
                       int                h);

} // namespace mpapp::detail::graphics

#endif // MPAPP_DETAIL_GRAPHICS_SHAPE_RENDERER_HPP
