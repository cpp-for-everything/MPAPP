// SPDX-License-Identifier: Apache-2.0
// WinUI 3 content_view handler — wraps `mux::Controls::ContentControl`.

#ifndef MPAPP_HANDLERS_WINDOWS_CONTENT_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_CONTENT_VIEW_HANDLER_HPP

#include <memory>

#include "../../content_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp {

template <>
class content_view_handler<platform::windows> {
public:
    content_view_handler();
    ~content_view_handler();
    content_view_handler(const content_view_handler&)            = delete;
    content_view_handler& operator=(const content_view_handler&) = delete;

    void map_content(content_view& c);
    void bind_content(content_view& c, view& child);

    winrt::Microsoft::UI::Xaml::Controls::ContentControl&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::ContentControl& native() const noexcept { return native_; }

private:
    void apply_content(const std::shared_ptr<view>& v);

    struct content_cb_t { content_view_handler<platform::windows>* self; void operator()(const std::shared_ptr<view>& v) const { self->apply_content(v); } };

    winrt::Microsoft::UI::Xaml::Controls::ContentControl native_{nullptr};

    content_cb_t                              content_cb_{this};
    signal_slot<std::shared_ptr<view> const&> content_slot_{};
};

} // namespace mpapp

#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_CONTENT_VIEW_HANDLER_HPP
