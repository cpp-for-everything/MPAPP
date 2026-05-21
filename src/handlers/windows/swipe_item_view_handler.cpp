// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 swipe_item_view handler implementation.

#include "mpapp/handlers/windows/swipe_item_view_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

namespace mpapp {

namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

swipe_item_view_handler<platform::windows>::swipe_item_view_handler() {
    try {
        native_ = muxc::ContentControl{};
    } catch (...) {
        native_ = nullptr;
    }
}

swipe_item_view_handler<platform::windows>::~swipe_item_view_handler() = default;

void swipe_item_view_handler<platform::windows>::apply_content(view* v) {
    if (native_ == nullptr) return;
    if (v == nullptr) {
        try { native_.Content(nullptr); } catch (...) {}
        return;
    }
    if (auto el = detail::windows_dispatch::dispatch(v); el != nullptr) {
        try { native_.Content(el); } catch (...) {}
    } else {
        try { native_.Content(nullptr); } catch (...) {}
    }
}

void swipe_item_view_handler<platform::windows>::map_content(swipe_item_view& iv) {
    apply_content(iv.content.get());
    iv.content.changed.subscribe(content_slot_, content_cb_);
}

} // namespace mpapp

// ----- ADR-0013 self-registration --------------------------------------

namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_swipe_item_view(::mpapp::view* v) {
    if (auto* iv = dynamic_cast<::mpapp::swipe_item_view*>(v); iv && iv->has_handler()) {
        return iv->handler().native();
    }
    return nullptr;
}

struct swipe_item_view_registrar {
    swipe_item_view_registrar() {
        ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_swipe_item_view);
    }
};

[[maybe_unused]] swipe_item_view_registrar _swipe_item_view_reg;

} // namespace

#endif // _WIN32
