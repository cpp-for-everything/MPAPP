// SPDX-License-Identifier: Apache-2.0
// WinUI 3 web_view handler implementation.

#include "mpapp/handlers/windows/web_view_handler.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.Web.WebView2.Core.h>

#include "mpapp/handlers/windows/widget_dispatch.hpp"

#include "winrt_strings.hpp"

namespace mpapp {

namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;
namespace wf   = ::winrt::Windows::Foundation;
namespace wv2c = ::winrt::Microsoft::Web::WebView2::Core;

web_view_handler<platform::windows>::web_view_handler() {
    native_ = muxc::WebView2{};
}

web_view_handler<platform::windows>::~web_view_handler() {
    if (native_ != nullptr) {
        if (nav_starting_token_.value != 0) {
            try { native_.NavigationStarting(nav_starting_token_); } catch (...) {}
            nav_starting_token_ = {};
        }
        if (nav_completed_token_.value != 0) {
            try { native_.NavigationCompleted(nav_completed_token_); } catch (...) {}
            nav_completed_token_ = {};
        }
    }
}

void web_view_handler<platform::windows>::apply_url(const std::string& v) {
    if (native_ == nullptr) return;
    if (v.empty()) return;
    try {
        suppress_echo_ = true;
        wf::Uri uri{detail::to_hstring_utf8(v)};
        native_.Source(uri);
        suppress_echo_ = false;
    } catch (...) {
        suppress_echo_ = false;
    }
}

void web_view_handler<platform::windows>::apply_html(const std::string& v) {
    if (native_ == nullptr) return;
    if (v.empty()) return;
    try {
        native_.NavigateToString(detail::to_hstring_utf8(v));
    } catch (...) {}
}

void web_view_handler<platform::windows>::map_url(web_view& wv) {
    bound_ = &wv;
    apply_url(wv.url.get());
    wv.url.changed.subscribe(url_slot_, url_cb_);

    if (native_ == nullptr) return;
    web_view* target = &wv;
    if (nav_starting_token_.value != 0) {
        native_.NavigationStarting(nav_starting_token_);
        nav_starting_token_ = {};
    }
    nav_starting_token_ = native_.NavigationStarting(
        [target](muxc::WebView2 const&, wv2c::CoreWebView2NavigationStartingEventArgs const& args) {
            try {
                target->is_loading.set(true);
                const std::wstring wide{args.Uri()};
                const std::string utf8 = detail::wstring_to_utf8(wide);
                target->navigating.emit(utf8);
            } catch (...) {}
        });

    if (nav_completed_token_.value != 0) {
        native_.NavigationCompleted(nav_completed_token_);
        nav_completed_token_ = {};
    }
    nav_completed_token_ = native_.NavigationCompleted(
        [target](muxc::WebView2 const& sender, wv2c::CoreWebView2NavigationCompletedEventArgs const& args) {
            try {
                target->is_loading.set(false);
                target->can_go_back.set(sender.CanGoBack());
                target->can_go_forward.set(sender.CanGoForward());
                const auto src = sender.Source();
                const std::wstring wide = (src != nullptr) ? std::wstring{src.ToString()} : std::wstring{};
                const std::string utf8 = detail::wstring_to_utf8(wide);
                target->navigated.emit(utf8, args.IsSuccess());
            } catch (...) {}
        });
}

void web_view_handler<platform::windows>::map_html(web_view& wv) {
    apply_html(wv.html_source.get());
    wv.html_source.changed.subscribe(html_slot_, html_cb_);
}

} // namespace mpapp

// ---------- Self-registration --------------------------------------------
namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_web_view(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::web_view*>(v); w && w->has_wv_handler()) {
        return w->wv_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_web_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
