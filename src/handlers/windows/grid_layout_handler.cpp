// SPDX-License-Identifier: Apache-2.0
// WinUI 3 basic_grid_layout handler implementation.

#include "mpapp/handlers/windows/grid_layout_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

namespace mpapp::internal {

namespace mux  = ::winrt::Microsoft::UI::Xaml;
namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

namespace {

mux::GridLength to_grid_length(const track_def& td) {
    switch (td.k) {
        case track_def::kind::fixed: return mux::GridLength{td.value, mux::GridUnitType::Pixel};
        case track_def::kind::star:  return mux::GridLength{td.value, mux::GridUnitType::Star};
        case track_def::kind::auto_:
        default:                     return mux::GridLength{0.0, mux::GridUnitType::Auto};
    }
}

} // namespace

grid_layout_handler<platform::windows>::grid_layout_handler() {
    native_ = muxc::Grid{};
}

grid_layout_handler<platform::windows>::~grid_layout_handler() = default;

void grid_layout_handler<platform::windows>::rebuild_rows(const std::vector<track_def>& v) {
    if (native_ == nullptr) return;
    native_.RowDefinitions().Clear();
    for (const auto& td : v) {
        muxc::RowDefinition r{};
        r.Height(to_grid_length(td));
        native_.RowDefinitions().Append(r);
    }
}

void grid_layout_handler<platform::windows>::rebuild_columns(const std::vector<track_def>& v) {
    if (native_ == nullptr) return;
    native_.ColumnDefinitions().Clear();
    for (const auto& td : v) {
        muxc::ColumnDefinition c{};
        c.Width(to_grid_length(td));
        native_.ColumnDefinitions().Append(c);
    }
}

void grid_layout_handler<platform::windows>::apply_row_spacing(double s) {
    if (native_ == nullptr) return;
    native_.RowSpacing(s);
}

void grid_layout_handler<platform::windows>::apply_column_spacing(double s) {
    if (native_ == nullptr) return;
    native_.ColumnSpacing(s);
}

void grid_layout_handler<platform::windows>::add_child(basic_grid_layout& g, view& child) {
    if (native_ == nullptr) return;
    auto el = detail::windows_dispatch::dispatch(&child);
    if (el == nullptr) return;

    const auto p = g.get_placement(child);
    // Cast to FrameworkElement to read the attached-property setters
    // — the WinUI Grid.SetRow/SetColumn are static helpers on the Grid
    // class and accept any UIElement.
    muxc::Grid::SetRow(el.as<mux::FrameworkElement>(),    p.row);
    muxc::Grid::SetColumn(el.as<mux::FrameworkElement>(), p.column);
    if (p.row_span    > 1) muxc::Grid::SetRowSpan   (el.as<mux::FrameworkElement>(), p.row_span);
    if (p.column_span > 1) muxc::Grid::SetColumnSpan(el.as<mux::FrameworkElement>(), p.column_span);
    native_.Children().Append(el);
}

void grid_layout_handler<platform::windows>::map_row_definitions(basic_grid_layout& g) {
    rebuild_rows(g.row_definitions.get());
    g.row_definitions.changed.subscribe(rows_slot_, rows_cb_);
}

void grid_layout_handler<platform::windows>::map_column_definitions(basic_grid_layout& g) {
    rebuild_columns(g.column_definitions.get());
    g.column_definitions.changed.subscribe(cols_slot_, cols_cb_);
}

void grid_layout_handler<platform::windows>::map_row_spacing(basic_grid_layout& g) {
    apply_row_spacing(g.row_spacing.get());
    g.row_spacing.changed.subscribe(rsp_slot_, rsp_cb_);
}

void grid_layout_handler<platform::windows>::map_column_spacing(basic_grid_layout& g) {
    apply_column_spacing(g.column_spacing.get());
    g.column_spacing.changed.subscribe(csp_slot_, csp_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration --------------------------------------------
namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_grid_layout(::mpapp::view* v) {
    if (auto* g = dynamic_cast<::mpapp::internal::basic_grid_layout*>(v); g && g->has_handler()) {
        return g->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_grid_layout); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
