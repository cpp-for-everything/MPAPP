// SPDX-License-Identifier: Apache-2.0
// WinUI 3 basic_list_view handler. Wraps `mux::Controls::ListView` (the
// selection-heavy variant; for raw items we'd use ItemsRepeater).
// items_source is rendered as a flat list of string items via the
// ListView's built-in Items collection.

#ifndef MPAPP_HANDLERS_WINDOWS_LIST_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_LIST_VIEW_HANDLER_HPP

#include <string>
#include <vector>

#include "../../internal/basic_list_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp::internal {

template <>
class list_view_handler<platform::windows> {
public:
    list_view_handler();
    ~list_view_handler();

    list_view_handler(const list_view_handler&)            = delete;
    list_view_handler& operator=(const list_view_handler&) = delete;
    list_view_handler(list_view_handler&&)                 = delete;
    list_view_handler& operator=(list_view_handler&&)      = delete;

    void map_items_source(basic_list_view& lv);
    void map_selected_index(basic_list_view& lv);

    winrt::Microsoft::UI::Xaml::Controls::ListView&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::ListView& native() const noexcept { return native_; }

private:
    void rebuild_items(const std::vector<std::string>& v);
    void apply_selection(int idx);

    struct items_cb_t {
        list_view_handler<platform::windows>* self;
        void operator()(const std::vector<std::string>& v) const { self->rebuild_items(v); }
    };
    struct sel_cb_t {
        list_view_handler<platform::windows>* self;
        void operator()(int v) const { self->apply_selection(v); }
    };

    winrt::Microsoft::UI::Xaml::Controls::ListView native_{nullptr};
    basic_list_view* bound_ = nullptr;
    bool       suppress_selection_event_ = false;

    items_cb_t items_cb_{this};
    sel_cb_t   sel_cb_{this};
    signal_slot<const std::vector<std::string>&> items_slot_{};
    signal_slot<const int&>                       sel_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_LIST_VIEW_HANDLER_HPP
