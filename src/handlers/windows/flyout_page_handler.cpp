// SPDX-License-Identifier: Apache-2.0
// WinUI 3 basic_flyout_page handler implementation.

#include "mpapp/handlers/windows/flyout_page_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

namespace mpapp::internal {

namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

flyout_page_handler<platform::windows>::flyout_page_handler() {
    native_ = muxc::SplitView{};
    native_.DisplayMode(muxc::SplitViewDisplayMode::Inline);
    native_.PanePlacement(muxc::SplitViewPanePlacement::Left);
    native_.IsPaneOpen(false);
    native_.OpenPaneLength(240.0);
}

flyout_page_handler<platform::windows>::~flyout_page_handler() = default;

void flyout_page_handler<platform::windows>::apply_flyout(basic_page* p) {
    if (native_ == nullptr) return;
    if (p == nullptr) { native_.Pane(nullptr); return; }
    if (auto el = detail::windows_dispatch::dispatch(p); el != nullptr) {
        native_.Pane(el);
    } else {
        native_.Pane(nullptr);
    }
}

void flyout_page_handler<platform::windows>::apply_detail(basic_page* p) {
    if (native_ == nullptr) return;
    if (p == nullptr) { native_.Content(nullptr); return; }
    if (auto el = detail::windows_dispatch::dispatch(p); el != nullptr) {
        native_.Content(el);
    } else {
        native_.Content(nullptr);
    }
}

void flyout_page_handler<platform::windows>::apply_is_presented(bool v) {
    if (native_ == nullptr) return;
    native_.IsPaneOpen(v);
}

void flyout_page_handler<platform::windows>::map_flyout(basic_flyout_page& fp) {
    apply_flyout(fp.flyout.get());
    fp.flyout.changed.subscribe(flyout_slot_, flyout_cb_);
}

void flyout_page_handler<platform::windows>::map_detail(basic_flyout_page& fp) {
    apply_detail(fp.detail.get());
    fp.detail.changed.subscribe(detail_slot_, detail_cb_);
}

void flyout_page_handler<platform::windows>::map_is_presented(basic_flyout_page& fp) {
    apply_is_presented(fp.is_presented.get());
    fp.is_presented.changed.subscribe(presented_slot_, presented_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration --------------------------------------------
#include "mpapp/internal/basic_flyout_page.hpp"

namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_flyout_page(::mpapp::view* v) {
    if (auto* f = dynamic_cast<::mpapp::internal::basic_flyout_page*>(v); f && f->has_fp_handler()) {
        return f->fp_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_flyout_page); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
