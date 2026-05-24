// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_toolbar handler implementation.

#include "mpapp/handlers/windows/toolbar_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

#include "winrt_strings.hpp"

namespace mpapp::internal {

namespace mux  = ::winrt::Microsoft::UI::Xaml;
namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

toolbar_handler<platform::windows>::toolbar_handler() {
    native_ = muxc::CommandBar{};
}

toolbar_handler<platform::windows>::~toolbar_handler() = default;

void toolbar_handler<platform::windows>::apply_items(const std::vector<toolbar_item>& v) {
    if (native_ == nullptr) return;
    try {
        auto primary = native_.PrimaryCommands();
        primary.Clear();
        for (const auto& item : v) {
            muxc::AppBarButton btn{};
            btn.Label(detail::to_hstring_utf8(item.text));
            // Stub: ignore the icon string for now if it's empty; otherwise
            // fall back to a search-style symbol icon. A real resource-URI
            // pipeline lands with the image_source binding work in M-05.
            if (!item.icon.empty()) {
                muxc::SymbolIcon icon{muxc::Symbol::Find};
                btn.Icon(icon);
            }
            primary.Append(btn);
        }
    } catch (...) {}
}

void toolbar_handler<platform::windows>::apply_title(const std::string& v) {
    if (native_ == nullptr) return;
    try {
        muxc::TextBlock title{};
        title.Text(detail::to_hstring_utf8(v));
        native_.Content(title);
    } catch (...) {}
}

void toolbar_handler<platform::windows>::map_items(basic_toolbar& t) {
    apply_items(t.items.get());
    t.items.changed.subscribe(items_slot_, items_cb_);
}
void toolbar_handler<platform::windows>::map_title(basic_toolbar& t) {
    apply_title(t.title.get());
    t.title.changed.subscribe(title_slot_, title_cb_);
}

} // namespace mpapp::internal
namespace {

// Per ADR-0013: self-register so the container dispatch surfaces can
// resolve `view*` → `UIElement` without a per-widget dynamic_cast branch
// in basic_stack_layout / window / basic_scroll_view / border / basic_content_view.
::winrt::Microsoft::UI::Xaml::UIElement dispatch_toolbar(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::internal::basic_toolbar*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() {
        ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_toolbar);
    }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
