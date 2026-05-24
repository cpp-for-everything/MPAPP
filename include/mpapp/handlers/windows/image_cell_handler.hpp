// SPDX-License-Identifier: Apache-2.0
// WinUI 3 basic_image_cell handler — Border wrapping a horizontal Grid:
// leading mux::Controls::Image (auto column) + vertical StackPanel of
// two TextBlocks (text + detail) in the star column. Image source is
// a BitmapImage constructed from the cell's `image_uri` (file://,
// http://, ms-appx:// all resolved by BitmapImage).

#ifndef MPAPP_HANDLERS_WINDOWS_IMAGE_CELL_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_IMAGE_CELL_HANDLER_HPP

#include <string>

#include "../../internal/basic_image_cell.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp::internal {

template <>
class image_cell_handler<platform::windows> {
public:
    image_cell_handler();
    ~image_cell_handler();

    image_cell_handler(const image_cell_handler&)            = delete;
    image_cell_handler& operator=(const image_cell_handler&) = delete;
    image_cell_handler(image_cell_handler&&)                 = delete;
    image_cell_handler& operator=(image_cell_handler&&)      = delete;

    void map_text(basic_image_cell& c);
    void map_detail(basic_image_cell& c);
    void map_image_uri(basic_image_cell& c);

    winrt::Microsoft::UI::Xaml::Controls::Border&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::Border& native() const noexcept { return native_; }

private:
    void apply_text(const std::string& v);
    void apply_detail(const std::string& v);
    void apply_image_uri(const std::string& v);

    struct text_cb_t {
        image_cell_handler<platform::windows>* self;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };
    struct detail_cb_t {
        image_cell_handler<platform::windows>* self;
        void operator()(const std::string& v) const { self->apply_detail(v); }
    };
    struct uri_cb_t {
        image_cell_handler<platform::windows>* self;
        void operator()(const std::string& v) const { self->apply_image_uri(v); }
    };

    winrt::Microsoft::UI::Xaml::Controls::Border     native_{nullptr};
    winrt::Microsoft::UI::Xaml::Controls::Grid       grid_{nullptr};
    winrt::Microsoft::UI::Xaml::Controls::Image      image_{nullptr};
    winrt::Microsoft::UI::Xaml::Controls::StackPanel stack_{nullptr};
    winrt::Microsoft::UI::Xaml::Controls::TextBlock  text_block_{nullptr};
    winrt::Microsoft::UI::Xaml::Controls::TextBlock  detail_block_{nullptr};

    text_cb_t                       text_cb_{this};
    detail_cb_t                     detail_cb_{this};
    uri_cb_t                        uri_cb_{this};
    signal_slot<const std::string&> text_slot_{};
    signal_slot<const std::string&> detail_slot_{};
    signal_slot<const std::string&> uri_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_IMAGE_CELL_HANDLER_HPP
