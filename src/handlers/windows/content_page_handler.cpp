// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_content_page handler implementation.

#include "mpapp/handlers/windows/content_page_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

#include "winrt_strings.hpp"

#include "mpapp/internal/basic_content_page.hpp"
#include "mpapp/view.hpp"

namespace mpapp::internal {

namespace mux  = ::winrt::Microsoft::UI::Xaml;
namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

content_page_handler<platform::windows>::content_page_handler() {
    // `native_` is a Border (which has Padding) wrapping a 2-row Grid:
    // row 0 (Auto) = title TextBlock, row 1 (*) = content ContentControl.
    // See header comment for the Page-nesting bug that drove the switch
    // away from `muxc::Page` here.
    native_       = muxc::Border{};
    grid_         = muxc::Grid{};
    title_text_   = muxc::TextBlock{};
    content_host_ = muxc::ContentControl{};

    // Row definitions: row 0 Auto (title), row 1 * (content fills).
    {
        muxc::RowDefinition r0{};
        r0.Height(mux::GridLength{0.0, mux::GridUnitType::Auto});
        grid_.RowDefinitions().Append(r0);

        muxc::RowDefinition r1{};
        r1.Height(mux::GridLength{1.0, mux::GridUnitType::Star});
        grid_.RowDefinitions().Append(r1);
    }

    // Place title in row 0, content host in row 1.
    muxc::Grid::SetRow(title_text_, 0);
    muxc::Grid::SetRow(content_host_, 1);

    grid_.Children().Append(title_text_);
    grid_.Children().Append(content_host_);

    native_.Child(grid_);
}

content_page_handler<platform::windows>::~content_page_handler() = default;

void content_page_handler<platform::windows>::apply_title(const std::string& v) {
    if (title_text_ == nullptr) return;
    title_text_.Text(detail::to_hstring_utf8(v));
}

void content_page_handler<platform::windows>::apply_content(const std::shared_ptr<view>& v) {
    if (content_host_ == nullptr) return;
    view* raw = v.get();
    if (raw == nullptr) { content_host_.Content(nullptr); return; }
    if (auto el = detail::windows_dispatch::dispatch(raw); el != nullptr) {
        content_host_.Content(el);
    }
}

void content_page_handler<platform::windows>::apply_padding(const thickness& t) {
    if (native_ == nullptr) return;
    mux::Thickness wt{t.left, t.top, t.right, t.bottom};
    native_.Padding(wt);
}

void content_page_handler<platform::windows>::map_title(basic_content_page& p) {
    apply_title(p.title.get());
    p.title.changed.subscribe(title_slot_, title_cb_);
}

void content_page_handler<platform::windows>::map_content(basic_content_page& p) {
    apply_content(p.content.get());
    p.content.changed.subscribe(content_slot_, content_cb_);
}

void content_page_handler<platform::windows>::map_padding(basic_content_page& p) {
    apply_padding(p.padding.get());
    p.padding.changed.subscribe(padding_slot_, padding_cb_);
}

void content_page_handler<platform::windows>::bind_content(basic_content_page& p, view& child) {
    p.content.set(std::shared_ptr<view>(&child, [](view*){}));
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --

namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_content_page(::mpapp::view* v) {
    if (auto* cp = dynamic_cast<::mpapp::internal::basic_content_page*>(v); cp && cp->has_handler()) {
        return cp->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_content_page); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
