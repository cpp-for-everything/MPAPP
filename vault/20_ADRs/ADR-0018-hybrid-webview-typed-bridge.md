---
type: adr
id: ADR-0018
title: "HybridWebView JS bridge — typed async method calls"
status: proposed
decisionDate: 2026-05-21
deciders: []
supersedes: ""
supersededBy: ""
area: handlers
tags:
  - type/adr
  - status/proposed
  - area/handlers
---

# ADR-0018 — HybridWebView JS bridge: typed async method calls

> [!info] Status
> **proposed** — unblocks HybridWebView real handlers + the C++↔JS protocol they expose.

## Context

HybridWebView's value over plain WebView is the **C++ ↔ JS bridge**: the embedded page can call back into the host application, and the host can invoke JS methods on the page. The protocol could be:

- Raw string passing (app rolls its own format).
- A JSON-RPC envelope.
- A **typed surface**: app registers C++ methods; framework generates JS proxies and dispatches inbound calls.

MAUI ships HybridWebView in net8+ with a typed-ish surface (`InvokeJavaScriptAsync<T>`, `EvaluateJavaScriptAsync`). The .NET interop layer handles the serialization. In MPAPP we have the same opportunity but with stronger compile-time guarantees.

## Decision

We will ship a **typed async method bridge** with the following shape:

```cpp
class my_bridge : public mpapp::hybrid_bridge {
public:
    // Methods callable from JS.
    [[mpapp::js_method]] mpapp::task<std::string> get_user_name() {
        co_return current_user_;
    }
    [[mpapp::js_method]] mpapp::task<int> add(int a, int b) {
        co_return a + b;
    }
};

// In app code:
auto wv = std::make_shared<mpapp::hybrid_web_view>();
wv->set_bridge<my_bridge>();   // codegen happens here at compile time

// JS side automatically gets:
//   window.mpapp.get_user_name()  -> Promise<string>
//   window.mpapp.add(1, 2)        -> Promise<number>
```

Internally:

- The `[[mpapp::js_method]]` attribute is a marker the future `mpapp-jni-gen`-style tool emits a JS shim for at build time. For v1, we hand-write the shim per bridge (tool comes in M-09 tooling milestone).
- Outbound messages from C++ to JS use the same shim: `wv->invoke_js<"add">(1, 2)` returns a `task<int>` resolved by the JS side.
- Wire format is JSON: `{"id": 7, "method": "add", "args": [1, 2]}` / `{"id": 7, "result": 3}` / `{"id": 7, "error": "..."}`.
- Parameter / return serialization uses a tiny `to_json` / `from_json` machinery in `include/mpapp/detail/json.hpp`. Supports primitives (int, double, bool, string) + `std::vector<T>` + `std::optional<T>` + user-defined types via `tag_invoke`. No external JSON library.
- Promise/task plumbing reuses [[ADR-0019-async-executor]]'s `task<T>` + UI-thread dispatcher.
- Security: the JS bridge is opt-in — `hybrid_web_view` has no bridge attached by default. Origin checks live in the handler (only same-origin pages can hit the bridge by default; opt-in cross-origin).

## Consequences

### Positive

- App-side C++ and JS code see typed APIs on both sides.
- Wire protocol is standard JSON-RPC-shaped; debuggable in browser devtools.
- No runtime reflection; codegen pattern matches the rest of MPAPP's compile-time-first philosophy.
- Origin check defaults to safe; explicit opt-in for cross-origin.

### Negative

- Codegen tool (`mpapp-bridge-gen`) is real work — punted to M-09. v1 ships with hand-written shims per bridge, documented as "use the codegen tool when it lands."
- JSON dependency. Mitigated by shipping a tiny in-house tag_invoke-based serializer (~500 LOC) rather than pulling nlohmann::json.
- Backward compatibility: changing a method signature requires both ends to rebuild + the deployed JS to be re-served. Document as a known constraint of typed bridges.

### Neutral

- Apps that need raw string passing for backward compat can use the bridge's `send_to_js` / `message_received` primitives directly; the typed layer is opt-in.

## Alternatives Considered

- **Raw string passing** — too low-level; every app reinvents request/reply matching.
- **JSON-RPC envelope without codegen** — viable but loses C++/JS-side type checking. Stringly-typed method names.
- **Defer to a later ADR** — rejected; HybridWebView is in M-04c scope and users need a protocol decision to use it.

## References

- [[ADR-0019-async-executor]] — the task<T> shape this bridge resolves into.
- [[ADR-0009-public-api-template-wrappers-only]] — compile-time-typed surface.
- [[Components/HybridWebView]] · [[Components/WebView]]
