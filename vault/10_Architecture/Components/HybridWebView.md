---
type: component
mauiHandler: "HybridWebView"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/hybridwebview"
mpappStatus: android-real
platformWindows: true
platformAndroid: true
platformLinux: true
platformMacos: false
platformIos: false
tags:
  - type/component
  - status/android-real
---

# HybridWebView

> [!info] Status
> **android-real, typed-bridge v2** — Raw + typed bridge surfaces stacked on the same native messaging pipe across all 3 platforms. Each handler injects a `window.mpapp` shim into the loaded document. The bridge has two layers:
>
> **Raw bridge** (always available):
> - `send_to_js(payload)` / `message_sent` signal on the C++ side
> - `window.mpapp.send(p)` / `window.mpapp.on(fn)` on the JS side
> - JS → C++ routes through `CoreWebView2.WebMessageReceived` (Win), `WebKitUserContentManager.script-message-received::mpapp_send` (Linux), `WebView.addJavascriptInterface("mpapp_native", MppJsBridge)` (Android).
>
> **Typed JSON-RPC bridge** (opt-in, per [[ADR-0018-hybrid-webview-typed-bridge]]):
> - C++ side: `wv->set_bridge<MyBridge>()` with `register_method` (sync) + `register_async_method<T>` (async) registration. Outbound via `invoke_js` / `invoke_js_cb<T>(...)` / `invoke_js_async<T>(...)` (callback + coroutine APIs).
> - JS side: `window.mpapp.register(name, fn)` for typed JS-side methods; `window.mpapp.call(name, args...)` for outbound. The shim's `_receive` auto-dispatches by method name and posts result/error envelopes.
> - Wire format: `{"id":N,"method":"name","args":[...]}` calls; `{"id":N,"result":...}` / `{"id":N,"error":"..."}` responses.
> - Symmetric typed round-trips end-to-end. Async bridge methods take a trailing `std::function<void(T)> respond` callback they invoke when ready (sync or deferred); `process_inbound` routes through `dispatch_async(payload, on_response)` so sync methods still fire inline but async methods can defer the response until their `respond()` resolves. The `async_invoker_builder<T, Method>` partial specialization works around the C++ "pack in non-trailing position is non-deducible" rule.
>
> Implementation lives in `include/mpapp/{hybrid_bridge.hpp,hybrid_web_view.hpp,detail/json.hpp}` (~1400 LOC total) + the 3 platform handlers + the JS shim in each handler's `kBridgeShim`.

## Overview

`HybridWebView` extends [[WebView]] with a two-way JavaScript↔native messaging bridge: HTML/JS bundled inside the app (under `Resources/Raw/wwwroot` by default) can call C++ methods, and C++ can invoke JavaScript with strongly-typed JSON marshalling. It is intended for "hybrid" apps where the UI is a web SPA but business logic lives in C++ — comparable to Electron's IPC, Tauri's `invoke()`, or Capacitor's plugin bridge. MAUI ships a small JS shim (`HybridWebView.js`) that MPAPP reuses byte-for-byte for parity.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\HybridWebView\HybridWebViewHandler.cs`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\HybridWebView\` (lives next to `WebView.cs`; type is `Microsoft.Maui.Controls.HybridWebView`)
- **Docs:** [Microsoft .NET MAUI — HybridWebView](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/hybridwebview)

## MPAPP C++ API

```cpp
namespace mpapp {

class hybrid_webview : public view<hybrid_webview> {
public:
    // Where the web assets live and which file to load by default.
    Observable<std::string> hybrid_root;    // default: "wwwroot"
    Observable<std::string> default_file;   // default: "index.html"

    // Register the object whose public methods are callable from JS as
    //   window.HybridWebView.InvokeDotNet("method_name", [args...])
    template<typename T>
    void set_invoke_javascript_target(std::shared_ptr<T> target);

    // Raw string-message channel — both directions.
    event<std::string>               raw_message_received;
    void send_raw_message(std::string_view raw);

    // Strongly-typed JS invocation. Marshalling uses mpapp::json
    // (the marshaller will be selected by a future RFC).
    template<typename TReturn, typename... TArgs>
    std::future<TReturn> invoke_javascript_async(
        std::string_view method_name, TArgs&&... args);

    std::future<std::string> evaluate_javascript_async(std::string_view script);

    // Standard navigation surface inherited from webview.
    Command<> reload_command;
    void reload();
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<HybridWebView HybridRoot="wwwroot"
               DefaultFile="index.html"
               x:Name="bridge"/>
```

Then from the bundled JS:

```js
// Send a typed call to C++.
const sum = await window.HybridWebView.InvokeDotNet("add", [2, 3]);

// Send a raw message.
window.HybridWebView.SendRawMessage("hello from JS");
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.WebView2` | C++/WinRT | Asset serving via `CoreWebView2.SetVirtualHostNameToFolderMapping`; messages via `WebMessageReceived`. |
| Android | `android.webkit.WebView` | fbjni / JNI | Asset serving via `WebViewAssetLoader`; messages via `addJavascriptInterface` with the shim. |
| Linux | `WebKitGTK` (`WebKitWebView`) with a custom URI scheme handler | GTK4 | Messages via `webkit_web_view_send_message_to_page` + `script-message-received`. |
| macOS | `WKWebView` with `WKURLSchemeHandler` for asset serving | AppKit | Messages via `WKScriptMessageHandler`; same shim as iOS. |
| iOS | `WKWebView` with `WKURLSchemeHandler` for asset serving | UIKit | Messages via `WKScriptMessageHandler`. |

## Side-by-side Examples

### MAUI

```xml
<HybridWebView HybridRoot="wwwroot" RawMessageReceived="OnRaw"/>
```

```csharp
hybrid.SetInvokeJavaScriptTarget(new Bridge());
var name = await hybrid.InvokeJavaScriptAsync<string>("getName", null);
```

### MPAPP (XAML)

```xml
<HybridWebView HybridRoot="wwwroot" RawMessageReceived="OnRaw"/>
```

### MPAPP (C++)

```cpp
auto bridge = std::make_shared<mpapp::hybrid_webview>();
bridge->hybrid_root  = "wwwroot";
bridge->default_file = "index.html";

struct app_bridge {
    int add(int a, int b) { return a + b; }
    std::string greeting(std::string who) { return "Hello, " + who + "!"; }
};
bridge->set_invoke_javascript_target(std::make_shared<app_bridge>());

bridge->raw_message_received.connect([](const std::string& msg) {
    mpapp::log::info("JS says: {}", msg);
});

auto name = bridge->invoke_javascript_async<std::string>("getName").get();
```

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/hybridwebview/mock_test.cpp` (planned)
- Windows handler: `tests/components/hybridwebview/windows_test.cpp` (planned)
- Android handler: `tests/components/hybridwebview/android_test.cpp` (planned)
- Linux handler: `tests/components/hybridwebview/linux_test.cpp` (planned)
- macOS handler: `tests/components/hybridwebview/macos_test.cpp` (planned)
- iOS handler: `tests/components/hybridwebview/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| JS-callable target | `SetInvokeJavaScriptTarget<T>` requires `DynamicallyAccessedMembers` for AOT | Methods are registered via a small reflection helper template — no runtime member discovery required | C++ has no managed reflection; explicit registration is preferred | — |
| JSON marshalling | `JsonTypeInfo<T>` (System.Text.Json source-gen) | `mpapp::json` (TBD) — single, header-only marshaller chosen via RFC | No System.Text.Json equivalent | RFC pending |
| Default `HybridRoot` | `wwwroot` | Same | — | — |
| Linux engine | Not supported | `WebKitGTK` | Closest engine match on GTK | — |
| Async API shape | `Task<TReturn?>` (nullable boxed) | `std::future<TReturn>` (throws on JS exception) | Idiomatic C++ surface | — |

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[WebView]]
