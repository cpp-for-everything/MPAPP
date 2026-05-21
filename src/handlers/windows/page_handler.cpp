// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 page handler implementation.

#include "mpapp/handlers/windows/page_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

#include "winrt_strings.hpp"

#include "mpapp/page.hpp"
#include "mpapp/view.hpp"

namespace mpapp {

namespace mux  = ::winrt::Microsoft::UI::Xaml;
namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

page_handler<platform::windows>::page_handler() {
    // Build a Page wrapping a Grid with two rows: title + content host.
    native_       = muxc::Page{};
    grid_         = muxc::Grid{};
    title_text_   = muxc::TextBlock{};
    content_host_ = muxc::ContentControl{};
    busy_ring_    = muxc::ProgressRing{};

    // Row definitions: row 0 Auto (title), row 1 * (content fills).
    {
        muxc::RowDefinition r0{};
        r0.Height(mux::GridLength{0.0, mux::GridUnitType::Auto});
        grid_.RowDefinitions().Append(r0);

        muxc::RowDefinition r1{};
        r1.Height(mux::GridLength{1.0, mux::GridUnitType::Star});
        grid_.RowDefinitions().Append(r1);
    }

    // Place title in row 0, content host + busy ring share row 1
    // (the ring overlays the content host when IsActive=true).
    muxc::Grid::SetRow(title_text_,   0);
    muxc::Grid::SetRow(content_host_, 1);
    muxc::Grid::SetRow(busy_ring_,    1);

    busy_ring_.IsActive(false);
    busy_ring_.HorizontalAlignment(mux::HorizontalAlignment::Center);
    busy_ring_.VerticalAlignment  (mux::VerticalAlignment::Center);

    grid_.Children().Append(title_text_);
    grid_.Children().Append(content_host_);
    grid_.Children().Append(busy_ring_);

    native_.Content(grid_);
}

page_handler<platform::windows>::~page_handler() = default;

void page_handler<platform::windows>::apply_title(const std::string& v) {
    if (title_text_ == nullptr) return;
    title_text_.Text(detail::to_hstring_utf8(v));
}

void page_handler<platform::windows>::apply_content(view* v) {
    if (content_host_ == nullptr) return;
    if (v == nullptr) { content_host_.Content(nullptr); return; }
    // ADR-0013: query the registry.
    if (auto el = detail::windows_dispatch::dispatch(v); el != nullptr) {
        content_host_.Content(el);
    } else {
        content_host_.Content(nullptr);
    }
}

void page_handler<platform::windows>::apply_is_busy(bool v) {
    if (busy_ring_ == nullptr) return;
    busy_ring_.IsActive(v);
}

void page_handler<platform::windows>::map_title(page& p) {
    apply_title(p.title.get());
    p.title.changed.subscribe(title_slot_, title_cb_);
}

void page_handler<platform::windows>::map_content(page& p) {
    apply_content(p.content.get());
    p.content.changed.subscribe(content_slot_, content_cb_);
}

void page_handler<platform::windows>::map_is_busy(page& p) {
    apply_is_busy(p.is_busy.get());
    p.is_busy.changed.subscribe(busy_slot_, busy_cb_);
}

void page_handler<platform::windows>::bind_content(page& p, view& child) {
    p.content.set(&child);
}

} // namespace mpapp

// ---------- Self-registration with the per-platform dispatch registry --

namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_page(::mpapp::view* v) {
    if (auto* p = dynamic_cast<::mpapp::page*>(v); p && p->has_handler()) {
        return p->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_page); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
