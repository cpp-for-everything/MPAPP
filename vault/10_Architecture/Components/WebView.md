---
type: component
mauiHandler: "WebView"
mauiDocUrl: "https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/webview"
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

# WebView

> [!info] Status
> **android-real** — Win `muxc::WebView2` (Chromium/Edge) wired through `NavigationStarting/Completed`; Linux `WebKitGTK 6.x` (LGPL dynamic-link per Rule 9) wired through `load-changed`; Android `android.webkit.WebView` + custom `MppWebViewClient` routing `onPageStarted/onPageFinished`. `url` / `html_source` two-way through native loader; `is_loading` + `can_go_back/forward` + `navigating(url)` + `navigated(url, success)` propagated from each engine. JavaScript enabled by default on Android; INTERNET permission added to example manifest. Linux build conditionally compiles to a no-op stub when WebKitGTK is missing at configure time.

## Overview

`WebView` embeds a full web browser engine inside the app to render HTML content from a URL, an HTML string, or local files. It exposes navigation (`go_back`, `go_forward`, `reload`), JavaScript evaluation, cookie sync, and lifecycle events for navigation start/complete/failure. MPAPP wraps each platform's stock engine (WebView2 on Windows, system `WebView` on Android, WebKitGtk on Linux, `WKWebView` on macOS and iOS) — there is no shared engine.

## MAUI Reference

- **Handler:** `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\WebView\WebViewHandler.cs`
- **Control:** `D:\GitHub\MPAPP\references\maui\src\Controls\src\Core\WebView\` (type lives next to its source providers — `WebView.cs`, `UrlWebViewSource.cs`, `HtmlWebViewSource.cs`)
- **Docs:** [Microsoft .NET MAUI — WebView](https://learn.microsoft.com/en-us/dotnet/maui/user-interface/controls/webview)

## MPAPP C++ API

```cpp
namespace mpapp {

struct web_navigation_args {
    std::string url;
    web_navigation_event evt;     // back, forward, new_page, refresh
    web_navigation_result result; // success, cancelled, failure, timeout
};

class web_view_source { /* tag base */ };
class url_web_view_source  : public web_view_source { Observable<std::string> url; };
class html_web_view_source : public web_view_source { Observable<std::string> html; Observable<std::string> base_url; };

class webview : public view<webview> {
public:
    Observable<std::shared_ptr<web_view_source>> source;
    Observable<std::string>                      user_agent;
    Observable<bool>                             can_go_back;
    Observable<bool>                             can_go_forward;
    Observable<cookie_container>                 cookies;

    // Navigation.
    Command<>                            go_back_command;
    Command<>                            go_forward_command;
    Command<>                            reload_command;
    void go_back();
    void go_forward();
    void reload();

    // JavaScript.
    void eval(std::string_view script);
    std::future<std::string> evaluate_javascript_async(std::string_view script);

    // Events.
    event<web_navigation_args>           navigating;   // cancellable
    event<web_navigation_args>           navigated;
    event<web_process_terminated_args>   process_terminated;
};

} // namespace mpapp
```

## XAML Usage

```xml
<!-- Must match MAUI XAML per ADR-0004. -->
<WebView Source="https://example.com"
         UserAgent="MPAPP/1.0"/>

<!-- Or an HTML string -->
<WebView>
    <WebView.Source>
        <HtmlWebViewSource Html="&lt;h1&gt;Hello&lt;/h1&gt;"/>
    </WebView.Source>
</WebView>
```

## Platform Notes

| Platform | Native control | Header / source | Notes |
|---|---|---|---|
| Windows | `Microsoft.UI.Xaml.Controls.WebView2` (Chromium / Edge) | C++/WinRT | Requires the WebView2 runtime; cookies via `CoreWebView2CookieManager`. |
| Android | `android.webkit.WebView` | fbjni / JNI | Uses the system WebView (Chromium-based on modern devices); `WebSettings.setJavaScriptEnabled(true)` set by default. |
| Linux | `WebKitGTK` (`WebKitWebView`) | GTK4 | Requires `libwebkit2gtk-4.1`; listed as an LGPL runtime dep in [[70_References/Third-Party Dependencies]]. |
| macOS | `WKWebView` (WebKit) | AppKit | JS eval via `evaluateJavaScript:completionHandler:`. |
| iOS | `WKWebView` (WebKit) | UIKit | Same APIs as macOS; cookie store via `WKHTTPCookieStore`. |

## Side-by-side Examples

### MAUI

```xml
<WebView Source="https://learn.microsoft.com"
         Navigated="OnNavigated"/>
```

### MPAPP (XAML)

```xml
<WebView Source="https://learn.microsoft.com"
         Navigated="OnNavigated"/>
```

### MPAPP (C++)

```cpp
auto wv = std::make_shared<mpapp::webview>();
auto src = std::make_shared<mpapp::url_web_view_source>();
src->url = "https://learn.microsoft.com";
wv->source = src;

wv->navigated.connect([](auto& args) {
    if (args.result != mpapp::web_navigation_result::success)
        mpapp::log::warn("Navigation failed: {}", args.url);
});

// Run some JS once loaded.
auto title = wv->evaluate_javascript_async("document.title").get();
```

## Tests

Links to per-platform handler test files. Tracked in [[Test Harness]].

- Mock tests: `tests/components/webview/mock_test.cpp` (planned)
- Windows handler: `tests/components/webview/windows_test.cpp` (planned)
- Android handler: `tests/components/webview/android_test.cpp` (planned)
- Linux handler: `tests/components/webview/linux_test.cpp` (planned)
- macOS handler: `tests/components/webview/macos_test.cpp` (planned)
- iOS handler: `tests/components/webview/ios_test.cpp` (planned)

## Known Differences

Documented divergences from MAUI behavior. Each row is a candidate for an RFC if elimination is feasible.

| Aspect | MAUI behavior | MPAPP behavior | Reason | Resolved by |
|---|---|---|---|---|
| JS eval API | `Eval(string)` (fire and forget) + `EvaluateJavaScriptAsync` (returns Task<string>) | `eval()` (fire-and-forget) + `evaluate_javascript_async()` returning `std::future<std::string>` | Idiomatic C++ async surface | — |
| Cookie sync | `System.Net.CookieContainer` | `mpapp::cookie_container` value type with platform sync at apply-time | No .NET BCL dependency | — |
| Linux engine | Not supported (no MAUI Linux head) | `WebKitGTK` | Closest engine match available on GTK | — |
| `ProcessTerminated` | Default-implemented as no-op on netstandard2.0 | Always implemented; non-WebKit2-based hosts simulate by raising on crash | Cross-platform parity | — |

## See also

- [[Controls Inventory]]
- [[Handlers]]
- [[Markup]]
- [[Interop Parity]]
- [[HybridWebView]]
