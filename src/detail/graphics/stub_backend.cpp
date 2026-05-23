// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Stub backend factory for the graphics facade per
// [[ADR-0015-graphics-backend-dual]].
//
// Selected by CMake's MPAPP_GRAPHICS_BACKEND option when it is set to
// "stub" (the default) or as a fallback for unimplemented backends
// (currently "cairo" and "skia"). The stub backend lives in the
// header so tests can instantiate it directly — this .cpp only
// supplies the `make_canvas` factory body so the framework's
// "default" canvas creator can be linked from a single TU.

#include <mpapp/detail/graphics/canvas.hpp>
#include <mpapp/detail/graphics/stub_canvas.hpp>

#include <memory>

namespace mpapp::detail::graphics {

std::unique_ptr<canvas> make_canvas(int width_px, int height_px) {
    return std::make_unique<stub_canvas>(width_px, height_px);
}

} // namespace mpapp::detail::graphics
