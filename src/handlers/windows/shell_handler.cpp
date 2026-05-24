// SPDX-License-Identifier: Apache-2.0
// WinUI 3 basic_shell handler implementation.

#include "mpapp/handlers/windows/shell_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

#include "winrt_strings.hpp"

namespace mpapp::internal {

namespace mux  = ::winrt::Microsoft::UI::Xaml;
namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

shell_handler<platform::windows>::shell_handler() {
    native_       = muxc::SplitView{};
    main_grid_    = muxc::Grid{};
    tab_strip_    = muxc::StackPanel{};
    content_host_ = muxc::ContentControl{};

    native_.DisplayMode(muxc::SplitViewDisplayMode::Inline);
    native_.PanePlacement(muxc::SplitViewPanePlacement::Left);
    native_.IsPaneOpen(false);
    native_.OpenPaneLength(240.0);

    // main_grid: row 0 Auto (tab strip), row 1 * (content host)
    {
        muxc::RowDefinition r0{};
        r0.Height(mux::GridLength{0.0, mux::GridUnitType::Auto});
        main_grid_.RowDefinitions().Append(r0);

        muxc::RowDefinition r1{};
        r1.Height(mux::GridLength{1.0, mux::GridUnitType::Star});
        main_grid_.RowDefinitions().Append(r1);
    }

    tab_strip_.Orientation(muxc::Orientation::Horizontal);
    muxc::Grid::SetRow(tab_strip_,    0);
    muxc::Grid::SetRow(content_host_, 1);
    main_grid_.Children().Append(tab_strip_);
    main_grid_.Children().Append(content_host_);

    native_.Content(main_grid_);
}

shell_handler<platform::windows>::~shell_handler() = default;

void shell_handler<platform::windows>::rebuild_tab_strip(const std::vector<std::string>& labels) {
    if (tab_strip_ == nullptr) return;
    tab_strip_.Children().Clear();
    for (std::size_t i = 0; i < labels.size(); ++i) {
        muxc::Button btn{};
        btn.Content(winrt::box_value(detail::to_hstring_utf8(labels[i])));
        const int idx = static_cast<int>(i);
        basic_shell* s = bound_;
        btn.Click([s, idx](
            winrt::Windows::Foundation::IInspectable const&,
            winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
            if (s != nullptr) s->current_tab_index.set(idx);
        });
        tab_strip_.Children().Append(btn);
    }
}

void shell_handler<platform::windows>::apply_selection(int idx) {
    // Visual styling of the selected tab is deferred; surface change is
    // propagated via current_tab_index Observable so app code can react.
    (void)idx;
}

void shell_handler<platform::windows>::apply_is_flyout_open(bool v) {
    if (native_ == nullptr) return;
    native_.IsPaneOpen(v);
}

void shell_handler<platform::windows>::apply_flyout_content(basic_page* p) {
    if (native_ == nullptr) return;
    if (p == nullptr) { native_.Pane(nullptr); return; }
    if (auto el = detail::windows_dispatch::dispatch(p); el != nullptr) {
        native_.Pane(el);
    } else {
        native_.Pane(nullptr);
    }
}

void shell_handler<platform::windows>::apply_current_content(basic_page* p) {
    if (content_host_ == nullptr) return;
    if (p == nullptr) { content_host_.Content(nullptr); return; }
    if (auto el = detail::windows_dispatch::dispatch(p); el != nullptr) {
        content_host_.Content(el);
    } else {
        content_host_.Content(nullptr);
    }
}

void shell_handler<platform::windows>::map_tabs(basic_shell& s) {
    bound_ = &s;
    rebuild_tab_strip(s.tabs.get());
    s.tabs.changed.subscribe(tabs_slot_, tabs_cb_);
}

void shell_handler<platform::windows>::map_current_tab_index(basic_shell& s) {
    apply_selection(s.current_tab_index.get());
    s.current_tab_index.changed.subscribe(sel_slot_, sel_cb_);
}

void shell_handler<platform::windows>::map_is_flyout_open(basic_shell& s) {
    apply_is_flyout_open(s.is_flyout_open.get());
    s.is_flyout_open.changed.subscribe(flyout_open_slot_, flyout_open_cb_);
}

void shell_handler<platform::windows>::map_flyout_content(basic_shell& s) {
    apply_flyout_content(s.flyout_content.get());
    s.flyout_content.changed.subscribe(flyout_content_slot_, flyout_content_cb_);
}

void shell_handler<platform::windows>::map_current_content(basic_shell& s) {
    apply_current_content(s.current_content.get());
    s.current_content.changed.subscribe(content_slot_, content_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration --------------------------------------------
namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_shell(::mpapp::view* v) {
    if (auto* s = dynamic_cast<::mpapp::internal::basic_shell*>(v); s && s->has_shell_handler()) {
        return s->shell_handler_ref().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_shell); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
