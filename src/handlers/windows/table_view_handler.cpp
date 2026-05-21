// SPDX-License-Identifier: Apache-2.0
// WinUI 3 table_view handler implementation.

#include "mpapp/handlers/windows/table_view_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"
#include "winrt_strings.hpp"

namespace mpapp {

namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

table_view_handler<platform::windows>::table_view_handler() {
    native_ = muxc::ListView{};
    native_.SelectionMode(muxc::ListViewSelectionMode::Single);
}

table_view_handler<platform::windows>::~table_view_handler() = default;

void table_view_handler<platform::windows>::rebuild_items(const std::vector<table_section_data>& sections) {
    if (native_ == nullptr) return;
    native_.Items().Clear();
    for (const auto& sec : sections) {
        // Section header — non-selectable in v1, styled by a leading "▾ " marker.
        std::string header = "▾ " + sec.title;
        native_.Items().Append(winrt::box_value(detail::to_hstring_utf8(header)));
        for (const auto& row : sec.rows) {
            native_.Items().Append(winrt::box_value(detail::to_hstring_utf8(row)));
        }
    }
}

void table_view_handler<platform::windows>::apply_row_height(int /*h*/) {
    // row_height honoring is tied to ItemContainerStyle customization;
    // not wired in v1.
}

void table_view_handler<platform::windows>::map_sections(table_view& tv) {
    rebuild_items(tv.sections.get());
    tv.sections.changed.subscribe(sec_slot_, sec_cb_);
}

void table_view_handler<platform::windows>::map_row_height(table_view& tv) {
    apply_row_height(tv.row_height.get());
    tv.row_height.changed.subscribe(rh_slot_, rh_cb_);
}

} // namespace mpapp

// ---------- Self-registration --------------------------------------------
namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_table_view(::mpapp::view* v) {
    if (auto* t = dynamic_cast<::mpapp::table_view*>(v); t && t->has_tv_handler()) {
        return t->tv_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_table_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
