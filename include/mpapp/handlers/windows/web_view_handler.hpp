// SPDX-License-Identifier: Apache-2.0
// WinUI 3 basic_web_view handler — wraps muxc::WebView2 (Chromium / Edge).
// url -> Source(Uri); html_source -> NavigateToString.
// NavigationStarting / NavigationCompleted update is_loading +
// can_go_back/forward and emit navigating / navigated.

#ifndef MPAPP_HANDLERS_WINDOWS_WEB_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_WEB_VIEW_HANDLER_HPP

#include <string>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_web_view.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp::internal {

template <>
class web_view_handler<platform::windows> {
public:
    web_view_handler();
    ~web_view_handler();

    web_view_handler(const web_view_handler&)            = delete;
    web_view_handler& operator=(const web_view_handler&) = delete;
    web_view_handler(web_view_handler&&)                 = delete;
    web_view_handler& operator=(web_view_handler&&)      = delete;

    void map_url(basic_web_view& wv);
    void map_html(basic_web_view& wv);

    winrt::Microsoft::UI::Xaml::Controls::WebView2&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::WebView2& native() const noexcept { return native_; }

private:
    void apply_url(const std::string& v);
    void apply_html(const std::string& v);

    struct url_cb_t {
        web_view_handler<platform::windows>* self;
        void operator()(const std::string& v) const { self->apply_url(v); }
    };
    struct html_cb_t {
        web_view_handler<platform::windows>* self;
        void operator()(const std::string& v) const { self->apply_html(v); }
    };

    winrt::Microsoft::UI::Xaml::Controls::WebView2 native_{nullptr};
    winrt::event_token nav_starting_token_{};
    winrt::event_token nav_completed_token_{};
    basic_web_view*          bound_         = nullptr;
    bool               suppress_echo_ = false;

    url_cb_t                        url_cb_{this};
    html_cb_t                       html_cb_{this};
    signal_slot<const std::string&> url_slot_{};
    signal_slot<const std::string&> html_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_WEB_VIEW_HANDLER_HPP
