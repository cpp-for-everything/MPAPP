# Windows WebView2 async-init race (T-0027 follow-up)

`hybrid_web_view_handler<windows>::wire_bridge` registers the JS shim
via `CoreWebView2.AddScriptToExecuteOnDocumentCreatedAsync(...)`.
The "Async" suffix matters: the method returns an `IAsyncOperation`
that resolves later, on the CoreWebView2 dispatcher thread. The
handler currently fires the call and discards the operation:

```cpp
try {
    core.AddScriptToExecuteOnDocumentCreatedAsync(
        detail::to_hstring_utf8(kBridgeShim));
} catch (...) {}
```

Immediately after, when `map_html_source` was added in this task,
the handler may call `CoreWebView2.NavigateToString(html)` to load
the page. If the AddScript op hasn't resolved by the time the
navigation reaches document-creation, the shim is not yet injected
and the page loads without `window.mpapp` — the page's `init()`
script's `setTimeout` polling never sees window.mpapp populated, so
the auto-fired `window.mpapp.call('notify', ...)` never reaches C++.

The visible symptom in the T-0027 Win screenshot:
`bridge_calls: 0   last_js_event: (none)`. The Linux side of the
same task shows `bridge_calls: 1   last_js_event: page loaded ...`
because WebKitGTK's `webkit_user_content_manager_add_script` is
synchronous and the shim is in place before `webkit_web_view_load_html`
runs.

## Fix (follow-up bug)

Two options:

1. **Await the IAsyncOperation** before calling NavigateToString.
   `winrt::Microsoft::Web::WebView2::Core::CoreWebView2` is a WinRT
   API; the IAsyncOperation can be co_await'd from a coroutine, or
   plumbed through a Completed handler.
2. **Always pre-register the shim during CoreWebView2 initialization**,
   then guarantee NavigateToString runs AFTER the script-added
   completion handler fires. This requires reordering the flush of
   `pending_html_`.

Either fix is a straightforward 15-line patch to
`src/handlers/windows/hybrid_web_view_handler.cpp`'s wire_bridge +
the CoreWebView2Initialized callback. Deferred from T-0027 to keep
the Rule 11 screenshot closure scope bounded; tracked here for the
next session.

## Workaround for users

Apps using `hybrid_web_view` on Windows that need the auto-call-on-load
pattern can defer the `html_source = ...` assignment by ~250 ms using
a `DispatcherQueue` post or a `Sleep + Activate` pattern in their
on_launch. The Linux + Android paths don't need this workaround.