// SPDX-License-Identifier: Apache-2.0
// WinUI 3 basic_navigation_page handler implementation. Per ADR-0014, the
// `page_stack` engine fires page_did_appear when the top of the stack
// changes; we subscribe and swap the content host's child to the new
// top, resolved via the ADR-0013 dispatch registry.

#include "mpapp/handlers/windows/navigation_page_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

#include "winrt_strings.hpp"

#include "mpapp/internal/basic_page.hpp"

namespace mpapp::internal {

namespace mux  = ::winrt::Microsoft::UI::Xaml;
namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

navigation_page_handler<platform::windows>::navigation_page_handler() {
    // `native_` IS the Grid. Earlier versions wrapped this in a
    // `muxc::Page` but nesting Page inside another container (e.g. a
    // `mux::Window`'s Content slot OR another Page's ContentControl)
    // triggers a late layout-pass exception that crashes the WinUI 3
    // message loop a few hundred ms after Window.Activate().
    native_       = muxc::Grid{};
    bar_          = muxc::StackPanel{};
    back_button_  = muxc::Button{};
    title_text_   = muxc::TextBlock{};
    content_host_ = muxc::ContentControl{};

    // Row 0 Auto (bar), Row 1 * (content)
    {
        muxc::RowDefinition r0{};
        r0.Height(mux::GridLength{0.0, mux::GridUnitType::Auto});
        native_.RowDefinitions().Append(r0);

        muxc::RowDefinition r1{};
        r1.Height(mux::GridLength{1.0, mux::GridUnitType::Star});
        native_.RowDefinitions().Append(r1);
    }

    // Bar: horizontal stack of [back basic_button, title]
    bar_.Orientation(muxc::Orientation::Horizontal);
    back_button_.Content(winrt::box_value(detail::to_hstring_utf8(std::string{"<"})));
    back_button_.Visibility(mux::Visibility::Collapsed); // hidden when depth <= 1
    bar_.Children().Append(back_button_);
    bar_.Children().Append(title_text_);

    muxc::Grid::SetRow(bar_,          0);
    muxc::Grid::SetRow(content_host_, 1);

    native_.Children().Append(bar_);
    native_.Children().Append(content_host_);
}

navigation_page_handler<platform::windows>::~navigation_page_handler() = default;

void navigation_page_handler<platform::windows>::apply_top(view* new_top) {
    if (content_host_ == nullptr) return;
    if (new_top == nullptr) {
        content_host_.Content(nullptr);
        title_text_.Text(detail::to_hstring_utf8(std::string{}));
        return;
    }
    // ADR-0013: ask the registry for new_top's native UIElement.
    if (auto el = detail::windows_dispatch::dispatch(new_top); el != nullptr) {
        content_host_.Content(el);
    } else {
        content_host_.Content(nullptr);
    }
    // Pull title from the new top basic_page.
    if (auto* p = dynamic_cast<basic_page*>(new_top); p != nullptr) {
        apply_title(p->title.get());
    }
}

void navigation_page_handler<platform::windows>::apply_title(const std::string& v) {
    if (title_text_ == nullptr) return;
    title_text_.Text(detail::to_hstring_utf8(v));
}

void navigation_page_handler<platform::windows>::apply_back_visibility(std::size_t depth) {
    if (back_button_ == nullptr) return;
    back_button_.Visibility(depth > 1 ? mux::Visibility::Visible : mux::Visibility::Collapsed);
}

void navigation_page_handler<platform::windows>::map_stack(basic_navigation_page& np) {
    bound_ = &np;

    // Seed: render whatever is currently on top.
    apply_top(np.stack().top());
    apply_back_visibility(np.stack().depth());

    // Subscribe to lifecycle: page_did_appear is the right hook —
    // engine emits it after the stack mutation, so by then dispatch()
    // can resolve the new top.
    np.stack().page_did_appear.subscribe(did_appear_slot_, did_appear_cb_);

    // Track depth changes for the back-basic_button visibility.
    np.stack_depth.changed.subscribe(depth_slot_, depth_cb_);

    // Wire the back basic_button to pop().
    basic_navigation_page* npp = &np;
    back_button_.Click([npp](
        winrt::Windows::Foundation::IInspectable const&,
        winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
        if (npp->stack().depth() > 1) npp->pop();
    });
}

} // namespace mpapp::internal
// ---------- Self-registration with the per-platform dispatch registry --
#include "mpapp/handlers/windows/widget_dispatch.hpp"
#include "mpapp/internal/basic_navigation_page.hpp"

namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_navigation_page(::mpapp::view* v) {
    if (auto* n = dynamic_cast<::mpapp::internal::basic_navigation_page*>(v); n && n->has_np_handler()) {
        return n->np_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_navigation_page); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
