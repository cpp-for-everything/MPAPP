// SPDX-License-Identifier: Apache-2.0
// WinUI 3 basic_tabbed_page handler implementation. Wraps a Pivot and
// rebuilds its PivotItem children whenever the cross-platform
// children Observable<vector<basic_page*>> changes.

#include "mpapp/handlers/windows/tabbed_page_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"
#include "mpapp/internal/basic_page.hpp"

#include "winrt_strings.hpp"

namespace mpapp::internal {

namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

tabbed_page_handler<platform::windows>::tabbed_page_handler() {
    native_ = muxc::Pivot{};
}

tabbed_page_handler<platform::windows>::~tabbed_page_handler() = default;

void tabbed_page_handler<platform::windows>::rebuild_children(const std::vector<basic_page*>& kids) {
    if (native_ == nullptr) return;
    native_.Items().Clear();
    for (basic_page* p : kids) {
        if (p == nullptr) continue;
        muxc::PivotItem pi{};
        pi.Header(winrt::box_value(detail::to_hstring_utf8(p->title.get())));
        if (auto el = detail::windows_dispatch::dispatch(p); el != nullptr) {
            pi.Content(el);
        }
        native_.Items().Append(pi);
    }
}

void tabbed_page_handler<platform::windows>::apply_selection(int idx) {
    if (native_ == nullptr) return;
    if (idx < 0 || idx >= static_cast<int>(native_.Items().Size())) return;
    native_.SelectedIndex(idx);
}

void tabbed_page_handler<platform::windows>::map_children(basic_tabbed_page& tp) {
    bound_ = &tp;
    rebuild_children(tp.children.get());
    tp.children.changed.subscribe(children_slot_, children_cb_);
}

void tabbed_page_handler<platform::windows>::map_selected_index(basic_tabbed_page& tp) {
    apply_selection(tp.selected_index.get());
    tp.selected_index.changed.subscribe(selection_slot_, selection_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration --------------------------------------------
namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_tabbed_page(::mpapp::view* v) {
    if (auto* t = dynamic_cast<::mpapp::internal::basic_tabbed_page*>(v); t && t->has_tp_handler()) {
        return t->tp_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_tabbed_page); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
