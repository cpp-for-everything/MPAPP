// SPDX-License-Identifier: Apache-2.0
// Android web_view handler — wraps android.webkit.WebView. JavaScript
// enabled by default. url -> loadUrl; html_source -> loadDataWithBaseURL.
// MppWebViewClient (custom WebViewClient) routes onPageStarted /
// onPageFinished through the shared trampoline, driving is_loading +
// can_go_back/forward + navigating / navigated.

#ifndef MPAPP_HANDLERS_ANDROID_WEB_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_WEB_VIEW_HANDLER_HPP

#include <string>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../web_view.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp {

template <>
class web_view_handler<platform::android> {
public:
    web_view_handler();
    ~web_view_handler();

    web_view_handler(const web_view_handler&)            = delete;
    web_view_handler& operator=(const web_view_handler&) = delete;
    web_view_handler(web_view_handler&&)                 = delete;
    web_view_handler& operator=(web_view_handler&&)      = delete;

    void map_url(web_view& wv);
    void map_html(web_view& wv);

    jobject native() const noexcept { return native_; }

    // Called from web_view_client_dispatch.cpp.
    void on_native_page_started(const std::string& url);
    void on_native_page_finished(const std::string& url, bool success);

private:
    void apply_url(const std::string& v);
    void apply_html(const std::string& v);
    void refresh_can_go();

    struct url_cb_t {
        web_view_handler<platform::android>* self;
        void operator()(const std::string& v) const { self->apply_url(v); }
    };
    struct html_cb_t {
        web_view_handler<platform::android>* self;
        void operator()(const std::string& v) const { self->apply_html(v); }
    };

    jobject       native_   = nullptr;  // WebView (global ref)
    jobject       client_   = nullptr;  // MppWebViewClient (global ref)
    web_view*     bound_    = nullptr;
    bool          suppress_echo_ = false;

    url_cb_t                        url_cb_{this};
    html_cb_t                       html_cb_{this};
    signal_slot<const std::string&> url_slot_{};
    signal_slot<const std::string&> html_slot_{};
};

void android_web_view_dispatch_page_started(web_view_handler<platform::android>* h,
                                            const std::string& url);
void android_web_view_dispatch_page_finished(web_view_handler<platform::android>* h,
                                             const std::string& url,
                                             bool success);

} // namespace mpapp

#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_WEB_VIEW_HANDLER_HPP
