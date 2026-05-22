// SPDX-License-Identifier: Apache-2.0
// GTK4 web_view handler — wraps WebKitGTK 6.x's WebKitWebView.
// url -> webkit_web_view_load_uri; html_source -> webkit_web_view_load_html.
// "load-changed" signal drives is_loading + can_go_back/forward +
// emits navigating / navigated. Dynamically linked (LGPL per Rule 9).

#ifndef MPAPP_HANDLERS_LINUX_WEB_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_WEB_VIEW_HANDLER_HPP

#include <string>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../web_view.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp {

template <>
class web_view_handler<platform::linux_> {
public:
    web_view_handler();
    ~web_view_handler();

    web_view_handler(const web_view_handler&)            = delete;
    web_view_handler& operator=(const web_view_handler&) = delete;
    web_view_handler(web_view_handler&&)                 = delete;
    web_view_handler& operator=(web_view_handler&&)      = delete;

    void map_url(web_view& wv);
    void map_html(web_view& wv);

    void*       native() noexcept       { return native_; }   // WebKitWebView*
    const void* native() const noexcept { return native_; }

private:
    void apply_url(const std::string& v);
    void apply_html(const std::string& v);

    struct url_cb_t {
        web_view_handler<platform::linux_>* self;
        void operator()(const std::string& v) const { self->apply_url(v); }
    };
    struct html_cb_t {
        web_view_handler<platform::linux_>* self;
        void operator()(const std::string& v) const { self->apply_html(v); }
    };

    void*         native_              = nullptr;  // WebKitWebView*
    unsigned long load_changed_handler_ = 0;
    web_view*     bound_               = nullptr;
    bool          suppress_echo_       = false;

    url_cb_t                        url_cb_{this};
    html_cb_t                       html_cb_{this};
    signal_slot<const std::string&> url_slot_{};
    signal_slot<const std::string&> html_slot_{};
};

} // namespace mpapp

#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_WEB_VIEW_HANDLER_HPP
