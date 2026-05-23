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

// Coroutine driving the full async init chain on Loaded:
//   EnsureCoreWebView2Async → AddScriptToExecuteOnDocumentCreatedAsync
//   → WebMessageReceived subscription → NavigateToString(pending_html_).
//
// Why co_await rather than chained Completed delegates: setting Completed
// on an IAsyncOperation/IAsyncAction whose only strong ref is a local
// `auto op = ...` is fragile — the runtime can drop the operation before
// the delegate fires. co_await keeps each op alive via the coroutine
// frame and naturally serializes the steps so the JS shim is in place
// before navigation begins, eliminating the original async-init race.
//
// `this` outlives the coroutine: the handler is owned by the demo app
// and lives for the app's lifetime, so suspended frames stay valid.
::winrt::fire_and_forget
hybrid_web_view_handler<platform::windows>::async_init() {
    if (native_ == nullptr) co_return;

    try {
        co_await native_.EnsureCoreWebView2Async();
    } catch (::winrt::hresult_error const&) {
        // ERROR_MOD_NOT_FOUND (0x8007007E) means WebView2Loader.dll or
        // Microsoft.Web.WebView2.Core.dll wasn't deployed next to the
        // executable. mpapp_add_winappsdk_runtime() copies both — keep
        // that wiring in place.
        co_return;
    }
    auto core = native_.CoreWebView2();
    if (core == nullptr) co_return;

    // Install the JS shim BEFORE any navigation. co_await keeps the
    // op alive via the coroutine frame, sidestepping the orphaned-
    // Completed-delegate pitfall that was the original async-init race.
    try {
        co_await core.AddScriptToExecuteOnDocumentCreatedAsync(
            detail::to_hstring_utf8(kBridgeShim));
    } catch (::winrt::hresult_error const&) {
        // Continue: the page will still load, just without the bridge.
    }
    shim_added_ = true;

    // Subscribe WebMessageReceived once.
    if (web_message_token_.value == 0) {
        hybrid_web_view* target = bound_;
        web_message_token_ = core.WebMessageReceived(
            [target](wv2c::CoreWebView2 const&,
                     wv2c::CoreWebView2WebMessageReceivedEventArgs const& args) {
                if (target == nullptr) return;
                try {
                    const std::wstring wide{args.TryGetWebMessageAsString()};
                    const std::string utf8 = detail::wstring_to_utf8(wide);
                    target->process_inbound(utf8);
                } catch (...) {}
            });
    }
    wired_ = true;

    // Flush any html that was buffered while we waited for init.
    if (!pending_html_.empty()) {
        std::string html = std::move(pending_html_);
        pending_html_.clear();
        try {
            core.NavigateToString(detail::to_hstring_utf8(html));
        } catch (...) {}
    }
}

void hybrid_web_view_handler<platform::windows>::wire_bridge() {
    // Synchronous fallback. Taken only when map_messages finds
    // CoreWebView2 already initialized (rare; a second map_messages on
    // a reused handler). The common cold-start path goes through
    // async_init().
    if (wired_ || native_ == nullptr) return;
    auto core = native_.CoreWebView2();
    if (core == nullptr) return;

    auto* self = this;
    try {
        auto op = core.AddScriptToExecuteOnDocumentCreatedAsync(
            detail::to_hstring_utf8(kBridgeShim));
        op.Completed([self](wf::IAsyncOperation<winrt::hstring> const&,
                            wf::AsyncStatus) {
            self->shim_added_ = true;
            if (!self->pending_html_.empty()) {
                std::string html = std::move(self->pending_html_);
                self->pending_html_.clear();
                self->apply_html(html);
            }
        });
    } catch (...) {
        shim_added_ = true;
    }

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

    if (native_ == nullptr) return;
    // Rare path: handler reused on a WebView2 whose CoreWebView2 is
    // already initialized. Wire synchronously.
    if (native_.CoreWebView2() != nullptr) {
        wire_bridge();
        return;
    }
    // Common path: defer the async init chain to the Loaded event so
    // the WebView2 is in the visual tree before we touch CoreWebView2.
    // EnsureCoreWebView2Async returns ERROR_MOD_NOT_FOUND when called
    // before the control has been parented, and the CoreWebView2
    // Initialized event simply never fires on its own — the WebView2
    // sits idle until something explicitly drives init.
    auto* self = this;
    native_.Loaded(
        [self](::winrt::Windows::Foundation::IInspectable const&,
               ::winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
            if (self->init_kicked_) return;
            self->init_kicked_ = true;
            self->async_init();
        });
}

void hybrid_web_view_handler<platform::windows>::apply_html(const std::string& html) {
    if (native_ == nullptr || html.empty()) return;
    // Two cases trigger buffering:
    //   * CoreWebView2 isn't ready yet — async_init() (kicked off by the
    //     WebView2 Loaded event) will flush pending_html_ once init
    //     completes.
    //   * CoreWebView2 is ready but the JS-shim AddScriptToExecuteOn-
    //     DocumentCreatedAsync hasn't resolved yet — same flush path
    //     applies; buffering avoids racing the shim.
    auto core = native_.CoreWebView2();
    if (core == nullptr) {
        pending_html_ = html;
        return;
    }
    if (!shim_added_) {
        pending_html_ = html;
        return;
    }
    try {
        core.NavigateToString(detail::to_hstring_utf8(html));
    } catch (...) {}
}

void hybrid_web_view_handler<platform::windows>::map_html_source(hybrid_web_view& h) {
    apply_html(h.html_source.get());
    h.html_source.changed.subscribe(html_slot_, html_cb_);
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
