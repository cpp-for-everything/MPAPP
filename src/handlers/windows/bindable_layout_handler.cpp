// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 bindable_layout handler implementation.

#include "mpapp/handlers/windows/bindable_layout_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

namespace mpapp {

namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

bindable_layout_handler<platform::windows>::bindable_layout_handler() {
    native_ = muxc::StackPanel{};
    native_.Orientation(muxc::Orientation::Vertical);
}

bindable_layout_handler<platform::windows>::~bindable_layout_handler() = default;

void bindable_layout_handler<platform::windows>::rebuild_children(layout& host) {
    if (native_ == nullptr) return;
    // M-04b: clear and re-append. The string-keyed items_source can't
    // be materialised yet (item_template instantiation is deferred to
    // the templating ADR). The clear half of the contract still runs
    // here so future template-driven appends start from a known state.
    try {
        native_.Children().Clear();
    } catch (...) {}
    (void)host;  // empty-view fallback handled in map_empty_view().
}

void bindable_layout_handler<platform::windows>::map_items_source(layout& host) {
    rebuild_children(host);
}

void bindable_layout_handler<platform::windows>::map_item_template(layout& /*host*/) {
    // Recorded but not yet driving instantiation — see the class-level
    // comment in the header.
}

void bindable_layout_handler<platform::windows>::map_empty_view(layout& host) {
    if (native_ == nullptr) return;
    const auto& items = bindable_layout::get_items_source(host);
    if (!items.items.empty()) return;  // only swap in the empty view when source is empty
    auto empty = bindable_layout::get_empty_view(host);
    view* raw = empty.get();
    if (raw == nullptr) return;
    if (auto el = detail::windows_dispatch::dispatch(raw); el != nullptr) {
        try {
            native_.Children().Append(el);
        } catch (...) {}
    }
}

} // namespace mpapp

// ----- ADR-0013 self-registration --------------------------------------------
//
// `bindable_layout` is an attached-property facility, not a `view`
// subclass — there is no instance to `dynamic_cast` to. The dispatcher
// below is therefore a defensive no-op: it always returns nullptr and
// the registry simply skips it on every lookup. The registrar is kept
// to satisfy the ADR-0013 self-registration contract (and to leave a
// uniform shape for future migration if BindableLayout grows a
// view-like wrapper later).

namespace {

::winrt::Microsoft::UI::Xaml::UIElement
dispatch_bindable_layout(::mpapp::view* /*v*/) {
    // No view subclass corresponds to bindable_layout — see comment above.
    return nullptr;
}

struct registrar {
    registrar() {
        ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_bindable_layout);
    }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
