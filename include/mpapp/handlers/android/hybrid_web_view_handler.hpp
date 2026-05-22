// SPDX-License-Identifier: Apache-2.0
// Android hybrid_web_view handler — extends the WebView surface with
// a JS <-> C++ bridge. JS calls window.mpapp.send(payload) which
// invokes MppJsBridge.send() (a @JavascriptInterface) which routes
// into the native handler via js_bridge_dispatch.cpp. C++ -> JS via
// WebView.evaluateJavascript("window.mpapp._receive(...)").

#ifndef MPAPP_HANDLERS_ANDROID_HYBRID_WEB_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_HYBRID_WEB_VIEW_HANDLER_HPP

#include <string>

#include "../../hybrid_web_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp {

template <>
class hybrid_web_view_handler<platform::android> {
public:
    hybrid_web_view_handler();
    ~hybrid_web_view_handler();

    hybrid_web_view_handler(const hybrid_web_view_handler&)            = delete;
    hybrid_web_view_handler& operator=(const hybrid_web_view_handler&) = delete;
    hybrid_web_view_handler(hybrid_web_view_handler&&)                 = delete;
    hybrid_web_view_handler& operator=(hybrid_web_view_handler&&)      = delete;

    void map_messages(hybrid_web_view& h);

    jobject native() const noexcept { return native_; }

    void on_native_inbound(const std::string& payload);

private:
    void send_outbound(const std::string& payload);

    struct sent_cb_t {
        hybrid_web_view_handler<platform::android>* self;
        void operator()(const std::string& v) const { self->send_outbound(v); }
    };

    jobject          native_     = nullptr;  // WebView (global ref)
    jobject          bridge_     = nullptr;  // MppJsBridge (global ref)
    hybrid_web_view* bound_      = nullptr;
    bool             wired_      = false;

    sent_cb_t                       sent_cb_{this};
    signal_slot<const std::string&> sent_slot_{};
};

void android_hybrid_web_view_dispatch_inbound(hybrid_web_view_handler<platform::android>* h,
                                              const std::string& payload);

} // namespace mpapp

#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_HYBRID_WEB_VIEW_HANDLER_HPP
