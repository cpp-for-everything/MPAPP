// SPDX-License-Identifier: Apache-2.0
// WinUI 3 basic_image_cell handler implementation.

#include "mpapp/handlers/windows/image_cell_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

#include "winrt_strings.hpp"

namespace mpapp::internal {

namespace mux   = ::winrt::Microsoft::UI::Xaml;
namespace muxc  = ::winrt::Microsoft::UI::Xaml::Controls;
namespace muxmi = ::winrt::Microsoft::UI::Xaml::Media::Imaging;
namespace muxm  = ::winrt::Microsoft::UI::Xaml::Media;
namespace wf    = ::winrt::Windows::Foundation;

image_cell_handler<platform::windows>::image_cell_handler() {
    native_       = muxc::Border{};
    grid_         = muxc::Grid{};
    image_        = muxc::Image{};
    stack_        = muxc::StackPanel{};
    text_block_   = muxc::TextBlock{};
    detail_block_ = muxc::TextBlock{};

    // Two-column horizontal layout: basic_image (auto) + basic_label stack (star).
    muxc::ColumnDefinition col_img{};
    col_img.Width(mux::GridLengthHelper::FromValueAndType(1.0, mux::GridUnitType::Auto));
    muxc::ColumnDefinition col_text{};
    col_text.Width(mux::GridLengthHelper::FromValueAndType(1.0, mux::GridUnitType::Star));
    grid_.ColumnDefinitions().Append(col_img);
    grid_.ColumnDefinitions().Append(col_text);

    // Small leading icon-size basic_image; trimmed for cell rows.
    image_.Width(40.0);
    image_.Height(40.0);
    image_.Stretch(muxm::Stretch::Uniform);
    image_.VerticalAlignment(mux::VerticalAlignment::Center);
    image_.Margin({0.0, 0.0, 12.0, 0.0});
    muxc::Grid::SetColumn(image_, 0);

    stack_.Orientation(muxc::Orientation::Vertical);
    stack_.VerticalAlignment(mux::VerticalAlignment::Center);
    stack_.Children().Append(text_block_);
    stack_.Children().Append(detail_block_);
    muxc::Grid::SetColumn(stack_, 1);

    grid_.Children().Append(image_);
    grid_.Children().Append(stack_);

    native_.Padding({12.0, 6.0, 12.0, 6.0});
    native_.Child(grid_);
}

image_cell_handler<platform::windows>::~image_cell_handler() = default;

void image_cell_handler<platform::windows>::apply_text(const std::string& v) {
    if (text_block_ == nullptr) return;
    text_block_.Text(detail::to_hstring_utf8(v));
}

void image_cell_handler<platform::windows>::apply_detail(const std::string& v) {
    if (detail_block_ == nullptr) return;
    detail_block_.Text(detail::to_hstring_utf8(v));
    detail_block_.Visibility(v.empty() ? mux::Visibility::Collapsed : mux::Visibility::Visible);
}

void image_cell_handler<platform::windows>::apply_image_uri(const std::string& v) {
    if (image_ == nullptr) return;
    try {
        if (v.empty()) {
            image_.Source(nullptr);
            return;
        }
        std::string uri_str = (v.starts_with("http://") || v.starts_with("https://")
                            || v.starts_with("file:")    || v.starts_with("ms-appx:"))
            ? v
            : (std::string("file:///") + v);
        for (auto& c : uri_str) if (c == '\\') c = '/';
        wf::Uri uri{detail::to_hstring_utf8(uri_str)};
        muxmi::BitmapImage bmp{uri};
        image_.Source(bmp);
    } catch (...) {}
}

void image_cell_handler<platform::windows>::map_text(basic_image_cell& c) {
    apply_text(c.text.get());
    c.text.changed.subscribe(text_slot_, text_cb_);
}

void image_cell_handler<platform::windows>::map_detail(basic_image_cell& c) {
    apply_detail(c.detail.get());
    c.detail.changed.subscribe(detail_slot_, detail_cb_);
}

void image_cell_handler<platform::windows>::map_image_uri(basic_image_cell& c) {
    apply_image_uri(c.image_uri.get());
    c.image_uri.changed.subscribe(uri_slot_, uri_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration --------------------------------------------
namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_image_cell(::mpapp::view* v) {
    if (auto* c = dynamic_cast<::mpapp::internal::basic_image_cell*>(v); c && c->has_ic_handler()) {
        return c->ic_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_image_cell); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
