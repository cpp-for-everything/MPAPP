// SPDX-License-Identifier: Apache-2.0
// GTK4 basic_hybrid_web_view handler — extends the WebKitGTK surface with a
// JS <-> C++ bridge. Inbound through WebKitUserContentManager
// "script-message-received::mpapp_send"; outbound through
// webkit_web_view_evaluate_javascript. A small JS shim is injected at
// document-start exposing `basic_window.mpapp`.

#ifndef MPAPP_HANDLERS_LINUX_HYBRID_WEB_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_HYBRID_WEB_VIEW_HANDLER_HPP

#include <string>

#include "../../internal/basic_hybrid_web_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class hybrid_web_view_handler<platform::linux_> {
public:
    hybrid_web_view_handler();
    ~hybrid_web_view_handler();

    hybrid_web_view_handler(const hybrid_web_view_handler&)            = delete;
    hybrid_web_view_handler& operator=(const hybrid_web_view_handler&) = delete;
    hybrid_web_view_handler(hybrid_web_view_handler&&)                 = delete;
    hybrid_web_view_handler& operator=(hybrid_web_view_handler&&)      = delete;

    void map_messages(basic_hybrid_web_view& h);
    void map_html_source(basic_hybrid_web_view& h);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

    // Called by the script-message-received GTK callback.
    void on_native_inbound(const std::string& payload);

// RFC-0003: walks `x.gesture_recognizers` and installs

// matching GtkGesture* controllers via

// `mpapp::internal::linux_gestures::attach`.

void map_gestures(basic_hybrid_web_view& x);


private:
    void send_outbound(const std::string& payload);
    void apply_html(const std::string& html);

    struct sent_cb_t {
        hybrid_web_view_handler<platform::linux_>* self;
        void operator()(const std::string& v) const { self->send_outbound(v); }
    };
    struct html_cb_t {
        hybrid_web_view_handler<platform::linux_>* self;
        void operator()(const std::string& v) const { self->apply_html(v); }
    };

    void*            native_      = nullptr;  // WebKitWebView*
    void*            content_mgr_ = nullptr;  // WebKitUserContentManager*
    unsigned long    msg_handler_id_ = 0;
    basic_hybrid_web_view* bound_ = nullptr;
    bool             wired_ = false;

    sent_cb_t                       sent_cb_{this};
    html_cb_t                       html_cb_{this};
    signal_slot<const std::string&> sent_slot_{};
    signal_slot<const std::string&> html_slot_{};
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_HYBRID_WEB_VIEW_HANDLER_HPP
