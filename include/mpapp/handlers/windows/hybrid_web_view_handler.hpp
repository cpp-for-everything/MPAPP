// SPDX-License-Identifier: Apache-2.0
// WinUI 3 hybrid_web_view handler — extends the WebView2 surface with
// a C++ <-> JS bridge. Inbound messages arrive via
// CoreWebView2.WebMessageReceived; outbound via CoreWebView2.
// PostWebMessageAsString. A small script is injected at document
// creation that exposes `window.mpapp = { send: ..., on: ... }`.

#ifndef MPAPP_HANDLERS_WINDOWS_HYBRID_WEB_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_HYBRID_WEB_VIEW_HANDLER_HPP

#include <string>

#include "../../hybrid_web_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/base.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp {

template <>
class hybrid_web_view_handler<platform::windows> {
public:
    hybrid_web_view_handler();
    ~hybrid_web_view_handler();

    hybrid_web_view_handler(const hybrid_web_view_handler&)            = delete;
    hybrid_web_view_handler& operator=(const hybrid_web_view_handler&) = delete;
    hybrid_web_view_handler(hybrid_web_view_handler&&)                 = delete;
    hybrid_web_view_handler& operator=(hybrid_web_view_handler&&)      = delete;

    void map_messages(hybrid_web_view& h);
    void map_html_source(hybrid_web_view& h);

    winrt::Microsoft::UI::Xaml::Controls::WebView2&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::WebView2& native() const noexcept { return native_; }

private:
    void wire_bridge();
    void send_outbound(const std::string& payload);
    void apply_html(const std::string& html);
    // Coroutine driving the full async init chain on Loaded:
    // EnsureCoreWebView2Async → AddScript shim → WebMessageReceived
    // subscription → NavigateToString(pending_html_).
    ::winrt::fire_and_forget async_init();

    struct sent_cb_t {
        hybrid_web_view_handler<platform::windows>* self;
        void operator()(const std::string& v) const { self->send_outbound(v); }
    };
    struct html_cb_t {
        hybrid_web_view_handler<platform::windows>* self;
        void operator()(const std::string& v) const { self->apply_html(v); }
    };

    winrt::Microsoft::UI::Xaml::Controls::WebView2 native_{nullptr};
    winrt::event_token web_message_token_{};
    hybrid_web_view*   bound_       = nullptr;
    bool               wired_       = false;
    // True once AddScriptToExecuteOnDocumentCreatedAsync has resolved.
    // Until then any html_source assignment is buffered in pending_html_
    // so NavigateToString can't race ahead of the JS shim install.
    bool               shim_added_  = false;
    // True once the async init coroutine has been kicked off (the
    // WebView2 Loaded event is the trigger). Subsequent Loaded fires
    // are no-ops.
    bool               init_kicked_ = false;
    std::string        pending_html_{};

    sent_cb_t                       sent_cb_{this};
    html_cb_t                       html_cb_{this};
    signal_slot<const std::string&> sent_slot_{};
    signal_slot<const std::string&> html_slot_{};
};

} // namespace mpapp

#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_HYBRID_WEB_VIEW_HANDLER_HPP
