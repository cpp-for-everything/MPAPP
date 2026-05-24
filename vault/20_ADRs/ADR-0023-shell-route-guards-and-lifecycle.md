---
type: adr
id: ADR-0023
title: "Shell route guards + page lifecycle hooks"
status: accepted
decisionDate: 2026-05-23
deciders:
  - alex
supersedes: ""
supersededBy: ""
area: handlers
tags:
  - type/adr
  - status/accepted
  - area/handlers
---

# ADR-0023 — Shell route guards + page lifecycle hooks

> [!info] Status
> **accepted** (2026-05-23) — composes on top of [[ADR-0016-shell-compile-time-routes]] (which deferred this to "a follow-up ADR") and [[ADR-0014-page-navigation-stack]] (which owns the navigation engine). Together they form the full Shell-navigation contract.
>
> Implementation shipped and Rule-11-closed by [[_Archive/T-0017-typed-routing-demo|T-0017]]: `shell.can_activate` + `can_deactivate` + `navigation_blocked` signal + `page::navigated_to` / `navigated_from` lifecycle signals; live demos on Windows (WinUI 3) + Linux (GTK4) + Android NDK (JNI smoke), all three with screenshots / logcat artifacts in the archive. Tests in `tests/mock_handlers/shell_test.cpp` cover the full guard + lifecycle matrix.

## Context

[[ADR-0016-shell-compile-time-routes]] §Decision deferred route guards (`CanActivate` / `CanDeactivate`) and route-aware lifecycle (`OnNavigatedTo` / `OnNavigatedFrom`) to a follow-up ADR — explicitly:

> Route guards (CanActivate / CanDeactivate) and route-aware lifecycle (`OnNavigatedTo` / `OnNavigatedFrom`) are deferred to a follow-up ADR — they need the executor design (ADR-0019).

With [[ADR-0019-async-executor-native-dispatcher]] now shipped (executor + `task<T>` + `ui_task<T>`), the deferred work is unblocked. The compile-time route table is in place. The remaining open API surface is:

- A way for apps to **block** a navigation attempt with user-defined logic ("you have unsaved changes — abandon them?", "this route requires authentication", "this content needs to be saved first").
- A way for **pages** to react to becoming current / ceasing to be current — refresh query-string-dependent state on entry, persist on exit.

MAUI ships these as:

- `IQueryAttributable.ApplyQueryAttributes(IDictionary<string, object>)` — page receives route params on entry.
- `Page.OnNavigatedTo(NavigatedToEventArgs)` / `Page.OnNavigatedFrom(NavigatedFromEventArgs)` — virtuals on Page.
- No built-in `CanActivate` / `CanDeactivate` — apps roll their own via the navigation events. (Some frameworks like Prism layer this on top.)

We can do better than that — first-class guards are a small primitive and they're high-user-value.

## Decision

Ship guards + lifecycle as **observable-and-signal** primitives on `shell` and `page`, not as override-able virtuals (matches the rest of the MPAPP surface).

### Two-phase route guards on `shell`

Two `Observable<std::function<...>>` fields on `shell`:

```cpp
using nav_activate_guard_t   = std::function<bool(std::string_view target)>;
using nav_deactivate_guard_t = std::function<bool(std::string_view current,
                                                  std::string_view target)>;

Observable<nav_activate_guard_t>   can_activate{};
Observable<nav_deactivate_guard_t> can_deactivate{};
```

The shell's `go_to(uri)` runs the chain:

1. If `can_deactivate` is set and returns false → block (emit `navigation_blocked(target)`, no state change, no signals).
2. If `can_activate` is set and returns false → block (same).
3. Fire `navigated_from(current_uri)` on the outgoing `current_content` page.
4. Update `current_route` + `current_tab_index`, emit `navigated`.
5. Fire `navigated_to(new_uri)` on whatever `current_content` is at this point.

Guards default to empty `std::function<...>`. Empty guard = allow. Apps opt in by assigning.

The deactivate guard receives both the current URI and the target so apps can scope decisions ("warn only if leaving the editor page"). The activate guard receives only the target since the current URI is observable via `shell.current_route` anyway.

### Page lifecycle signals on `page`

Two signals on `page`:

```cpp
signal<const std::string& /*uri*/>          navigated_to{};
signal<const std::string& /*previous_uri*/> navigated_from{};
```

The shell fires them as described above. Pages subscribe in their constructor:

```cpp
class details_page : public page {
public:
    details_page() {
        navigated_to.subscribe(slot_, [this](const std::string& uri) {
            // refresh based on the new URI (parse query string, etc.)
            auto id = parse_id_from(uri);
            load_detail(id);
        });
    }
private:
    signal_slot<const std::string&> slot_{};
};
```

### What we deliberately don't ship

- **No `IQueryAttributable` equivalent.** Route-param extraction is whatever the user does with the URI string. The `route_table::build_uri` from ADR-0016 builds outgoing URIs; a future `parse_args<Path>(uri)` symmetric helper is out of scope here.
- **No async guards.** `can_activate` / `can_deactivate` return `bool` synchronously. Apps that need an async confirm-dialog return false from the guard, show the dialog, then re-invoke `go_to(target)` after the user picks yes. This matches the typical UX where dialogs preempt navigation entirely.
- **No multi-stage guards.** One guard per slot. Apps that need to chain multiple checks compose them inside their callable. Keeps the surface flat.
- **No lifecycle hooks beyond to/from.** No `before_navigate_to`, no `navigated_to_completed`. Two signals cover the typical refresh-on-enter / persist-on-exit pattern.

## Consequences

### Positive

- High-value app pattern (block-on-dirty, auth gates) becomes one-liner instead of bespoke hooks.
- Page lifecycle hooks compose cleanly with the existing observable+signal model — no new primitive.
- The same two guards cover the string-based `go_to(uri)` and the typed `go_to<Path, &Table>(args...)` from ADR-0016, because the latter delegates to the former.
- Guards are testable in isolation — no platform handler required.
- `navigation_blocked` signal gives apps a single observation point for "user attempted to leave but couldn't" telemetry.

### Negative

- Apps that want to compose multiple guards (e.g. a global one + a page-specific one) write their own chain inside the callable. The framework doesn't help. Mitigation: document the recipe.
- Page lifecycle on a page that's used in multiple routes will fire `navigated_to` on every entry — the page can't distinguish "first entry" from "re-entry" without tracking it itself. Mitigation: same as MAUI; apps track this with a member bool if they care.

### Neutral

- Lifecycle signals fire even on `go_to(uri)` where the URI doesn't actually change `current_content` (e.g. navigating to the same route). Apps that want to dedupe check `uri == previous_uri` inside their handler.
- Order of `navigated` and `navigated_to` is documented: `navigated` first (shell-level), then per-page `navigated_to`. Apps that wire content-swap logic via `navigated` subscriber → swap → page hooks fire on the new content.

## Alternatives Considered

- **MAUI-parity virtuals on page** — rejected; goes against [[ADR-0009-public-api-template-wrappers-only]]'s "no inheritance-for-extension" stance. Signals and Observables compose better.
- **Single fused guard `can_navigate(current, target)`** — rejected; two-phase matches user mental model better. Activate-only guards (auth) shouldn't need to know about the current route.
- **Async guards returning `task<bool>`** — rejected as v1; sync covers the common cases and apps can fall back to "block, show dialog, re-invoke" for async. Reconsider if real users hit the gap.
- **First-class route-param dict on page lifecycle** — deferred to a future ADR; covered by the still-unbuilt `parse_args<Path>(uri)` helper.

## Implementation Notes

- Shell surface (guards + navigation_blocked signal): [`include/mpapp/shell.hpp`](../../include/mpapp/shell.hpp) — `Observable<nav_activate_guard_t> can_activate{}`, `Observable<nav_deactivate_guard_t> can_deactivate{}`, `signal<std::string_view> navigation_blocked`. Run order documented in `go_to(uri)` comments.
- Page lifecycle signals: [`include/mpapp/page.hpp`](../../include/mpapp/page.hpp) — `signal<const std::string&> navigated_to{}` / `navigated_from{}`. Fired by shell from its `go_to(uri)` chain after `current_content` swap completes.
- Real handlers don't have to do anything special for guards — they're pure cross-platform behavior on the surface. The handlers' platform back-button hooks ([`src/handlers/android/shell_handler.cpp`](../../src/handlers/android/shell_handler.cpp) for `KeyEvent.KEYCODE_BACK`, Windows for `SystemNavigationManager.BackRequested`) call `shell.go_back()` which routes through the same guard chain.
- Tests: [`tests/mock_handlers/shell_test.cpp`](../../tests/mock_handlers/shell_test.cpp) covers the full activate/deactivate/blocked matrix + page lifecycle ordering against the mock handler.
- Closure evidence: [[_Archive/T-0017-typed-routing-demo]] — Win/Linux/Android live demos exercising the guard chain end-to-end with screenshots + logcat artifacts.

## References

- [[ADR-0016-shell-compile-time-routes]] — the deferral source.
- [[ADR-0014-page-navigation-stack]] — owns the lower-level page_stack engine.
- [[ADR-0019-async-executor-native-dispatcher]] — unblocked this ADR by shipping the executor.
- [[Components/Shell]] · [[Components/Page]]
- [[40_Roadmap/M-04c-handler-heavy-port]]
