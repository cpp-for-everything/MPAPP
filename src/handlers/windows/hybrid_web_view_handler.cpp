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

// JS shim. Three surfaces in one object:
//   * Raw bridge: send(p) posts a string to the host; on(fn) registers
//     a listener that fires for every inbound payload not consumed by
//     the typed bridge below.
//   * Typed JS-side method registration: register(name, fn) registers
//     a JS function callable from C++ via wv->invoke_js("name", args).
//     When C++ invokes a registered method, the return value is posted
//     back as {"id":N,"result":<value>} for the C++-side response
//     router to pick up.
//   * Typed JS->C++ call: call(name, ...args) posts a JSON-RPC
//     envelope to C++. The C++ side dispatches through its attached
//     hybrid_bridge.
constexpr const char* kBridgeShim =
    "(function(){"
    "  if (window.mpapp && window.mpapp.__mpapp) return;"
    "  var listeners = [];"
    "  var methods   = {};"
    "  var nextId    = 0;"
    "  window.mpapp = {"
    "    __mpapp: true,"
    "    send: function(p) { window.chrome.webview.postMessage(String(p)); },"
    "    on:   function(fn) { listeners.push(fn); },"
    "    register: function(name, fn) { methods[name] = fn; },"
    "    call: function(name) {"
    "      var id = ++nextId;"
    "      var args = Array.prototype.slice.call(arguments, 1);"
    "      window.mpapp.send(JSON.stringify({id: id, method: name, args: args}));"
    "      return id;"
    "    },"
    "    _receive: function(p) {"
    "      var env = null;"
    "      try { env = JSON.parse(p); } catch (e) { env = null; }"
    "      if (env && typeof env === 'object' && typeof env.method === 'string'"
    "          && Object.prototype.hasOwnProperty.call(methods, env.method)) {"
    "        var ret;"
    "        try { ret = methods[env.method].apply(null, env.args || []); }"
    "        catch (e) {"
    "          window.mpapp.send(JSON.stringify({id: env.id, error: String(e)}));"
    "          return;"
    "        }"
    "        window.mpapp.send(JSON.stringify({"
    "          id: env.id,"
    "          result: ret === undefined ? null : ret"
    "        }));"
    "        return;"
    "      }"
    "      for (var i = 0; i < listeners.length; ++i)"
    "        try { listeners[i](p); } catch (e) {}"
    "    }"
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
                // Single choke point — hybrid_web_view::process_inbound
                // decides whether to route through an attached bridge
                // or fall through to the raw message_received signal.
                target->process_inbound(utf8);
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
