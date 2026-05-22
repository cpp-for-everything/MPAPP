// SPDX-License-Identifier: Apache-2.0
// WinUI 3 hybrid_web_view handler implementation.

#include "mpapp/handlers/windows/hybrid_web_view_handler.hpp"

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
namespace wv2c = ::winrt::Microsoft::Web::WebView2::Core;
namespace wf   = ::winrt::Windows::Foundation;

namespace {

// JS shim — exposes window.mpapp = { send(p), _receive(p) }. `send`
// posts up to the host, `_receive` is invoked by C++ via eval.
constexpr const char* kBridgeShim =
    "(function(){"
    "  if (window.mpapp && window.mpapp.__mpapp) return;"
    "  var listeners = [];"
    "  window.mpapp = {"
    "    __mpapp: true,"
    "    send: function(p) { window.chrome.webview.postMessage(String(p)); },"
    "    on:   function(fn) { listeners.push(fn); },"
    "    _receive: function(p) { for (var i=0; i<listeners.length; ++i) try { listeners[i](p); } catch(e) {} }"
    "  };"
    "})();";

} // namespace

hybrid_web_view_handler<platform::windows>::hybrid_web_view_handler() {
    native_ = muxc::WebView2{};
}

hybrid_web_view_handler<platform::windows>::~hybrid_web_view_handler() {
    if (native_ != nullptr) {
        if (core_ready_token_.value != 0) {
            try { native_.CoreWebView2Initialized(core_ready_token_); } catch (...) {}
            core_ready_token_ = {};
        }
        // The WebMessageReceived event lives on the CoreWebView2; the
        // CoreWebView2 may be gone by the time the destructor runs, so
        // best-effort detach.
        if (web_message_token_.value != 0) {
            try {
                if (auto core = native_.CoreWebView2()) {
                    core.WebMessageReceived(web_message_token_);
                }
            } catch (...) {}
            web_message_token_ = {};
        }
    }
}

void hybrid_web_view_handler<platform::windows>::wire_bridge() {
    if (wired_ || native_ == nullptr) return;
    auto core = native_.CoreWebView2();
    if (core == nullptr) return;

    // Inject the JS shim once per document.
    try {
        core.AddScriptToExecuteOnDocumentCreatedAsync(detail::to_hstring_utf8(kBridgeShim));
    } catch (...) {}

    if (web_message_token_.value != 0) {
        try { core.WebMessageReceived(web_message_token_); } catch (...) {}
        web_message_token_ = {};
    }
    hybrid_web_view* target = bound_;
    web_message_token_ = core.WebMessageReceived(
        [target](wv2c::CoreWebView2 const&,
                 wv2c::CoreWebView2WebMessageReceivedEventArgs const& args) {
            if (target == nullptr) return;
            try {
                const std::wstring wide{args.TryGetWebMessageAsString()};
                const std::string utf8 = detail::wstring_to_utf8(wide);
                target->last_message_in.set(utf8);
                target->message_received.emit(utf8);
            } catch (...) {}
        });
    wired_ = true;
}

void hybrid_web_view_handler<platform::windows>::send_outbound(const std::string& payload) {
    if (native_ == nullptr) return;
    try {
        auto core = native_.CoreWebView2();
        if (core == nullptr) return;
        core.PostWebMessageAsString(detail::to_hstring_utf8(payload));
    } catch (...) {}
}

void hybrid_web_view_handler<platform::windows>::map_messages(hybrid_web_view& h) {
    bound_ = &h;
    h.message_sent.subscribe(sent_slot_, sent_cb_);

    // The CoreWebView2 may not exist yet — wait for CoreWebView2Initialized.
    if (native_ == nullptr) return;
    auto* self = this;
    if (native_.CoreWebView2() != nullptr) {
        wire_bridge();
    } else {
        if (core_ready_token_.value != 0) {
            try { native_.CoreWebView2Initialized(core_ready_token_); } catch (...) {}
            core_ready_token_ = {};
        }
        core_ready_token_ = native_.CoreWebView2Initialized(
            [self](muxc::WebView2 const&, muxc::CoreWebView2InitializedEventArgs const&) {
                self->wire_bridge();
            });
    }
}

} // namespace mpapp

// ---------- Self-registration --------------------------------------------
namespace {

::winrt::Microsoft::UI::Xaml::UIElement dispatch_hybrid_web_view(::mpapp::view* v) {
    if (auto* w = dynamic_cast<::mpapp::hybrid_web_view*>(v); w && w->has_hwv_handler()) {
        return w->hwv_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::windows_dispatch::register_dispatcher(dispatch_hybrid_web_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // _WIN32
