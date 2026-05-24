// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_tabbed_view handler.
//
// Wraps `mux::Controls::TabView` — the WinUI 3 control whose
// `TabItems` collection + `SelectedIndex` directly mirror
// `mpapp::basic_tabbed_view`'s `tab_titles` and `selected_index`. Each tab
// is a header-only `TabViewItem`; the basic_page-content slot is left empty
// here and will be wired by the upcoming `TabbedPage`/templating-engine
// follow-up.
//
// Per ADR-0013 the .cpp self-registers with `windows_dispatch`. The
// TabView is a `Control` → `FrameworkElement` → `UIElement`, so the
// registrar returns `native_` without an explicit cast.

#ifndef MPAPP_HANDLERS_WINDOWS_TABBED_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_TABBED_VIEW_HANDLER_HPP

#include <string>
#include <vector>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_tabbed_view.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp::internal {

template <>
class tabbed_view_handler<platform::windows> {
public:
    tabbed_view_handler();
    ~tabbed_view_handler();

    tabbed_view_handler(const tabbed_view_handler&)            = delete;
    tabbed_view_handler& operator=(const tabbed_view_handler&) = delete;
    tabbed_view_handler(tabbed_view_handler&&)                 = delete;
    tabbed_view_handler& operator=(tabbed_view_handler&&)      = delete;

    void map_tab_titles(basic_tabbed_view& t);
    void map_selected_index(basic_tabbed_view& t);

    // The TabView IS the native UIElement exposed to dispatch surfaces.
    winrt::Microsoft::UI::Xaml::Controls::TabView&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::TabView& native() const noexcept { return native_; }

private:
    void apply_tab_titles(const std::vector<std::string>& v);
    void apply_selected_index(int v);

    struct tab_titles_cb_t     { tabbed_view_handler<platform::windows>* self; void operator()(const std::vector<std::string>& v) const { self->apply_tab_titles(v); } };
    struct selected_index_cb_t { tabbed_view_handler<platform::windows>* self; void operator()(int v) const { self->apply_selected_index(v); } };

    winrt::Microsoft::UI::Xaml::Controls::TabView native_{nullptr};
    bool suppress_echo_ = false;

    tab_titles_cb_t                              tab_titles_cb_{this};
    selected_index_cb_t                          selected_index_cb_{this};
    signal_slot<std::vector<std::string> const&> tab_titles_slot_{};
    signal_slot<const int&>                      selected_index_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_TABBED_VIEW_HANDLER_HPP
