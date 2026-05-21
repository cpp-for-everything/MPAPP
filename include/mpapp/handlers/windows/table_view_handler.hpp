// SPDX-License-Identifier: Apache-2.0
// WinUI 3 table_view handler. Wraps mux::Controls::ListView with sections
// flattened into title-row + data-row pairs. Cell-typed rendering (per
// ADR-0021's cell tree) is a follow-up; v1 renders all entries as plain
// string items, with section titles styled by a leading "▾ " marker so
// they read as headers.

#ifndef MPAPP_HANDLERS_WINDOWS_TABLE_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_TABLE_VIEW_HANDLER_HPP

#include <string>
#include <vector>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../table_view.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp {

template <>
class table_view_handler<platform::windows> {
public:
    table_view_handler();
    ~table_view_handler();

    table_view_handler(const table_view_handler&)            = delete;
    table_view_handler& operator=(const table_view_handler&) = delete;
    table_view_handler(table_view_handler&&)                 = delete;
    table_view_handler& operator=(table_view_handler&&)      = delete;

    void map_sections(table_view& tv);
    void map_row_height(table_view& tv);

    winrt::Microsoft::UI::Xaml::Controls::ListView&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::ListView& native() const noexcept { return native_; }

private:
    void rebuild_items(const std::vector<table_section_data>& sections);
    void apply_row_height(int h);

    struct sec_cb_t {
        table_view_handler<platform::windows>* self;
        void operator()(const std::vector<table_section_data>& v) const { self->rebuild_items(v); }
    };
    struct rh_cb_t {
        table_view_handler<platform::windows>* self;
        void operator()(int v) const { self->apply_row_height(v); }
    };

    winrt::Microsoft::UI::Xaml::Controls::ListView native_{nullptr};

    sec_cb_t sec_cb_{this};
    rh_cb_t  rh_cb_{this};
    signal_slot<const std::vector<table_section_data>&> sec_slot_{};
    signal_slot<const int&>                              rh_slot_{};
};

} // namespace mpapp

#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_TABLE_VIEW_HANDLER_HPP
