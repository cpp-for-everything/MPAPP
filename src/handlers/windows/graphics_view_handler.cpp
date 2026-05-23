// SPDX-License-Identifier: Apache-2.0
// WinUI 3 graphics_view handler implementation.

#include "mpapp/handlers/windows/graphics_view_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

namespace mpapp {

namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

graphics_view_handler<platform::windows>::graphics_view_handler() {
    native_ = muxc::Canvas{};
}

graphics_view_handler<platform::windows>::~graphics_view_handler() {
    if (native_ != nullptr && size_changed_token_.value != 0) {
        try { native_.SizeChanged(size_changed_token_); } catch (...) {}
        size_changed_token_ = {};
    }
}

void graphics_view_handler<platform::windows>::apply_width(int w) {
    if (native_ == nullptr) return;
    native_.Width(static_cast<double>(w));
}

void graphics_view_handler<platform::windows>::apply_height(int h) {
    if (native_ == nullptr) return;
    native_.Height(static_cast<double>(h));
}

void graphics_view_handler<platform::windows>::map_size(graphics_view& gv) {
    apply_width(gv.width.get());
    apply_height(gv.height.get());
    gv.width.changed.subscribe(w_slot_, w_cb_);
    gv.height.changed.subscribe(h_slot_, h_cb_);
}

void graphics_view_handler<platform::windows>::map_draw_count(graphics_view& /*gv*/) {
    // No-op for v1: the user-facing canvas draw API is gated on
    // ADR-0015. invalidate() in the surface still bumps draw_count
    // and emits draw_requested, which apps can observe to schedule
    // their own future work.
}

void graphics_view_handler<platform::windows>::map_drawable(graphics_view& /*gv*/) {
    // Stub for v1 of the canvas-facade migration. The Linux handler
    // wires this up against Cairo + GtkDrawingArea (off-screen render
    // → BGRA32 pixel-blit). The Windows blit path needs a WinUI 3
    // SoftwareBitmapSource (or D2D ID2D1Bitmap) wrapped over the
    // canvas's pixel_data() pointer; deferred to a follow-up so the
    // Linux migration can validate the abstract API first.
}

} // namespace mpapp

// ---------- Self-registration --------------------------------------------
namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_graphics_view(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::graphics_view*>(v); w && w->has_gv_handler()) {
        return w->gv_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_graphics_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
