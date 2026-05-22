// SPDX-License-Identifier: Apache-2.0
// WinUI 3 grid_layout handler. Wraps mux::Controls::Grid, populating
// RowDefinitions + ColumnDefinitions from the cross-platform track_def
// vectors (ADR-0017 surface). Per-child placement reads the attached
// store on grid_layout and calls Grid.SetRow / Grid.SetColumn.

#ifndef MPAPP_HANDLERS_WINDOWS_GRID_LAYOUT_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_GRID_LAYOUT_HANDLER_HPP

#include <vector>

#include "../../grid_layout.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp {

template <>
class grid_layout_handler<platform::windows> {
public:
    grid_layout_handler();
    ~grid_layout_handler();

    grid_layout_handler(const grid_layout_handler&)            = delete;
    grid_layout_handler& operator=(const grid_layout_handler&) = delete;
    grid_layout_handler(grid_layout_handler&&)                 = delete;
    grid_layout_handler& operator=(grid_layout_handler&&)      = delete;

    void map_row_definitions(grid_layout& g);
    void map_column_definitions(grid_layout& g);
    void map_row_spacing(grid_layout& g);
    void map_column_spacing(grid_layout& g);

    // Add a child at its attached (row, column, spans). The child's
    // native UIElement is resolved via the ADR-0013 dispatch registry.
    void add_child(grid_layout& g, view& child);

    winrt::Microsoft::UI::Xaml::Controls::Grid&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::Grid& native() const noexcept { return native_; }

private:
    void rebuild_rows(const std::vector<track_def>& v);
    void rebuild_columns(const std::vector<track_def>& v);
    void apply_row_spacing(double s);
    void apply_column_spacing(double s);

    struct rows_cb_t {
        grid_layout_handler<platform::windows>* self;
        void operator()(const std::vector<track_def>& v) const { self->rebuild_rows(v); }
    };
    struct cols_cb_t {
        grid_layout_handler<platform::windows>* self;
        void operator()(const std::vector<track_def>& v) const { self->rebuild_columns(v); }
    };
    struct rsp_cb_t {
        grid_layout_handler<platform::windows>* self;
        void operator()(double s) const { self->apply_row_spacing(s); }
    };
    struct csp_cb_t {
        grid_layout_handler<platform::windows>* self;
        void operator()(double s) const { self->apply_column_spacing(s); }
    };

    winrt::Microsoft::UI::Xaml::Controls::Grid native_{nullptr};

    rows_cb_t rows_cb_{this};
    cols_cb_t cols_cb_{this};
    rsp_cb_t  rsp_cb_{this};
    csp_cb_t  csp_cb_{this};
    signal_slot<const std::vector<track_def>&> rows_slot_{};
    signal_slot<const std::vector<track_def>&> cols_slot_{};
    signal_slot<const double&>                  rsp_slot_{};
    signal_slot<const double&>                  csp_slot_{};
};

} // namespace mpapp

#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_GRID_LAYOUT_HANDLER_HPP
