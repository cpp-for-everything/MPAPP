// SPDX-License-Identifier: Apache-2.0
// WinUI 3 basic_text_cell handler implementation.

#include "mpapp/handlers/windows/text_cell_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

#include "winrt_strings.hpp"

namespace mpapp::internal {

namespace mux  = ::winrt::Microsoft::UI::Xaml;
namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

text_cell_handler<platform::windows>::text_cell_handler() {
    native_       = muxc::Border{};
    stack_        = muxc::StackPanel{};
    text_block_   = muxc::TextBlock{};
    detail_block_ = muxc::TextBlock{};

    stack_.Orientation(muxc::Orientation::Vertical);
    stack_.Children().Append(text_block_);
    stack_.Children().Append(detail_block_);

    // Padding inside the cell to match native row styling.
    native_.Padding({12.0, 6.0, 12.0, 6.0});
    native_.Child(stack_);
}

text_cell_handler<platform::windows>::~text_cell_handler() = default;

void text_cell_handler<platform::windows>::apply_text(const std::string& v) {
    if (text_block_ == nullptr) return;
    text_block_.Text(detail::to_hstring_utf8(v));
}

void text_cell_handler<platform::windows>::apply_detail(const std::string& v) {
    if (detail_block_ == nullptr) return;
    detail_block_.Text(detail::to_hstring_utf8(v));
    // Hide the detail row when empty so single-line text_cells don't
    // leave a phantom blank line.
    detail_block_.Visibility(v.empty() ? mux::Visibility::Collapsed : mux::Visibility::Visible);
}

void text_cell_handler<platform::windows>::map_text(basic_text_cell& c) {
    apply_text(c.text.get());
    c.text.changed.subscribe(text_slot_, text_cb_);
}

void text_cell_handler<platform::windows>::map_detail(basic_text_cell& c) {
    apply_detail(c.detail.get());
    c.detail.changed.subscribe(detail_slot_, detail_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration --------------------------------------------
namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_text_cell(::mpapp::view* v) {
    if (auto* c = dynamic_cast<::mpapp::internal::basic_text_cell*>(v); c && c->has_tc_handler()) {
        return c->tc_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_text_cell); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
