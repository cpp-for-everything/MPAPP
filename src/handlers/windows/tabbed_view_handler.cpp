// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 tabbed_view handler implementation.

#include "mpapp/handlers/windows/tabbed_view_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

#include "winrt_strings.hpp"

namespace mpapp {

namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

tabbed_view_handler<platform::windows>::tabbed_view_handler() {
    native_ = muxc::TabView{};
    // The default `TabView` chrome lets users add and close tabs
    // interactively. Our cross-platform contract is "data-driven" — the
    // tabs reflect the model's `tab_titles` — so hide the add button and
    // disable user reordering / closing to keep behaviour consistent
    // with the Linux + Android handlers (which have no such affordances).
    native_.IsAddTabButtonVisible(false);
    native_.CanReorderTabs(false);
    native_.CanDragTabs(false);
    native_.TabWidthMode(muxc::TabViewWidthMode::Equal);
}

tabbed_view_handler<platform::windows>::~tabbed_view_handler() = default;

void tabbed_view_handler<platform::windows>::apply_tab_titles(const std::vector<std::string>& v) {
    if (native_ == nullptr) return;
    suppress_echo_ = true;
    try {
        auto items = native_.TabItems();
        items.Clear();
        for (const auto& title : v) {
            muxc::TabViewItem item{};
            item.Header(::winrt::box_value(detail::to_hstring_utf8(title)));
            // We're not closing tabs programmatically through the user's
            // close-tab gesture — disable per-tab close so the user can't
            // mutate the model from the UI.
            item.IsClosable(false);
            items.Append(item);
        }
    } catch (...) {}
    suppress_echo_ = false;
}

void tabbed_view_handler<platform::windows>::apply_selected_index(int v) {
    if (native_ == nullptr) return;
    suppress_echo_ = true;
    try {
        native_.SelectedIndex(v);
    } catch (...) {}
    suppress_echo_ = false;
}

void tabbed_view_handler<platform::windows>::map_tab_titles(tabbed_view& t) {
    apply_tab_titles(t.tab_titles.get());
    t.tab_titles.changed.subscribe(tab_titles_slot_, tab_titles_cb_);
}

void tabbed_view_handler<platform::windows>::map_selected_index(tabbed_view& t) {
    apply_selected_index(t.selected_index.get());
    t.selected_index.changed.subscribe(selected_index_slot_, selected_index_cb_);
}

} // namespace mpapp

// ----- ADR-0013 self-registration --------------------------------------

namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_tabbed_view(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::tabbed_view*>(v); w && w->has_handler()) {
        return w->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() {
        ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_tabbed_view);
    }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
