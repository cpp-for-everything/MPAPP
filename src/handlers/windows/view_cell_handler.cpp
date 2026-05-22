// SPDX-License-Identifier: Apache-2.0
// WinUI 3 view_cell handler implementation.

#include "mpapp/handlers/windows/view_cell_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

namespace mpapp {

namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

view_cell_handler<platform::windows>::view_cell_handler() {
    native_ = muxc::Border{};
    native_.Padding({12.0, 6.0, 12.0, 6.0});
}

view_cell_handler<platform::windows>::~view_cell_handler() = default;

void view_cell_handler<platform::windows>::apply_content(view* v) {
    if (native_ == nullptr) return;
    if (v == nullptr) { native_.Child(nullptr); return; }
    if (auto el = detail::windows_dispatch::dispatch(v); el != nullptr) {
        native_.Child(el);
    } else {
        native_.Child(nullptr);
    }
}

void view_cell_handler<platform::windows>::map_content(view_cell& c) {
    apply_content(c.content.get());
    c.content.changed.subscribe(content_slot_, content_cb_);
}

} // namespace mpapp

// ---------- Self-registration --------------------------------------------
namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_view_cell(::mpapp::view* v) {
    if (auto* c = dynamic_cast<::mpapp::view_cell*>(v); c && c->has_vc_handler()) {
        return c->vc_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_view_cell); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
