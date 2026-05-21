// SPDX-License-Identifier: Apache-2.0
// WinUI 3 tabbed_page handler. Uses `mux::Controls::Pivot` (the closest
// MAUI-style tabbed container in WinUI 3; `TabView` exists too but is
// document-style and overkill for a phone-tab metaphor).

#ifndef MPAPP_HANDLERS_WINDOWS_TABBED_PAGE_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_TABBED_PAGE_HANDLER_HPP

#include <string>
#include <vector>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../tabbed_page.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp {

template <>
class tabbed_page_handler<platform::windows> {
public:
    tabbed_page_handler();
    ~tabbed_page_handler();

    tabbed_page_handler(const tabbed_page_handler&)            = delete;
    tabbed_page_handler& operator=(const tabbed_page_handler&) = delete;
    tabbed_page_handler(tabbed_page_handler&&)                 = delete;
    tabbed_page_handler& operator=(tabbed_page_handler&&)      = delete;

    void map_children(tabbed_page& tp);
    void map_selected_index(tabbed_page& tp);

    winrt::Microsoft::UI::Xaml::Controls::Pivot&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::Pivot& native() const noexcept { return native_; }

private:
    void rebuild_children(const std::vector<page*>& kids);
    void apply_selection(int idx);

    struct children_cb_t {
        tabbed_page_handler<platform::windows>* self;
        void operator()(const std::vector<page*>& v) const { self->rebuild_children(v); }
    };
    struct selection_cb_t {
        tabbed_page_handler<platform::windows>* self;
        void operator()(int v) const { self->apply_selection(v); }
    };

    winrt::Microsoft::UI::Xaml::Controls::Pivot native_{nullptr};
    tabbed_page* bound_ = nullptr;

    children_cb_t  children_cb_{this};
    selection_cb_t selection_cb_{this};
    signal_slot<const std::vector<page*>&> children_slot_{};
    signal_slot<const int&>                selection_slot_{};
};

} // namespace mpapp

#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_TABBED_PAGE_HANDLER_HPP
