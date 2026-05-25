// SPDX-License-Identifier: Apache-2.0
// WinUI 3 basic_table_view handler. Wraps mux::Controls::ListView and
// renders one of two surfaces:
//
//   1. typed_sections (preferred when non-empty) — each cell's native
//      UIElement is appended to the ListView's Items via ADR-0013
//      dispatch. Section titles still flatten in as bold-marked
//      header rows.
//   2. sections (fallback) — plain-string rendering with the leading
//      "▾ " section-title marker.

#ifndef MPAPP_HANDLERS_WINDOWS_TABLE_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_TABLE_VIEW_HANDLER_HPP

#include <string>
#include <vector>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_table_view.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp::internal {

template <>
class table_view_handler<platform::windows> {
public:
    table_view_handler();
    ~table_view_handler();

    table_view_handler(const table_view_handler&)            = delete;
    table_view_handler& operator=(const table_view_handler&) = delete;
    table_view_handler(table_view_handler&&)                 = delete;
    table_view_handler& operator=(table_view_handler&&)      = delete;

    void map_sections(basic_table_view& tv);
    void map_typed_sections(basic_table_view& tv);
    void map_row_height(basic_table_view& tv);

    winrt::Microsoft::UI::Xaml::Controls::ListView&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::ListView& native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_table_view& /*x*/) noexcept {}


private:
    void rebuild_items(const std::vector<table_section_data>& sections);
    void rebuild_typed(const std::vector<table_section_typed>& sections);
    void apply_row_height(int h);
    void rebuild_active();   // picks typed vs plain based on which is non-empty

    struct sec_cb_t {
        table_view_handler<platform::windows>* self;
        void operator()(const std::vector<table_section_data>&) const { self->rebuild_active(); }
    };
    struct typed_cb_t {
        table_view_handler<platform::windows>* self;
        void operator()(const std::vector<table_section_typed>&) const { self->rebuild_active(); }
    };
    struct rh_cb_t {
        table_view_handler<platform::windows>* self;
        void operator()(int v) const { self->apply_row_height(v); }
    };

    winrt::Microsoft::UI::Xaml::Controls::ListView native_{nullptr};
    basic_table_view* bound_ = nullptr;

    sec_cb_t   sec_cb_{this};
    typed_cb_t typed_cb_{this};
    rh_cb_t    rh_cb_{this};
    signal_slot<const std::vector<table_section_data>&>  sec_slot_{};
    signal_slot<const std::vector<table_section_typed>&> typed_slot_{};
    signal_slot<const int&>                              rh_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_TABLE_VIEW_HANDLER_HPP
