---
type: adr
id: ADR-0022
title: "Android event routing — kind-discriminated listener family"
status: accepted
decisionDate: 2026-05-22
deciders:
  - alex
supersedes: ""
supersededBy: ""
area: handlers
tags:
  - type/adr
  - status/accepted
  - area/handlers
  - platform/android
---

# ADR-0022 — Android event routing: kind-discriminated listener family

> [!info] Status
> **proposed** — retroactively codifying a pattern that emerged organically across [[ADR-0014-page-navigation-stack]]'s NavigationPage back button, [[ADR-0020-virtualized-item-host-wrap-platform]]'s ListView/CollectionView/TableView row taps, [[ADR-0021-tableview-cell-types]]'s switch_cell/entry_cell two-way bindings, and [[ADR-0018-hybrid-webview-typed-bridge]]'s JS↔C++ bridge.

## Context

Android handlers wire native widget events back into MPAPP's cross-platform [[signal]] / [[Observable]] surfaces through JNI. Each native event type (click, item-click, checked-changed, text-changed, page-load, IME-action, JS-interface) needs:

1. A **Java listener class** that implements the relevant Android interface (`View.OnClickListener`, `AdapterView.OnItemClickListener`, `CompoundButton.OnCheckedChangeListener`, `android.text.TextWatcher`, `android.webkit.WebViewClient`, `android.widget.TextView.OnEditorActionListener`, `@JavascriptInterface`).
2. A **native trampoline** that JNI-dispatches the event to the right C++ handler.

The naive approach is one Java class + one trampoline per widget that wants to listen. That scales linearly with widgets and forces multiple Java classes for fundamentally identical interfaces (e.g., switch_, check_box, and radio_button all want `OnCheckedChangeListener`).

## Decision

We will share **one Java listener class per Android event interface**, parametrized by a `kind` discriminator constant the C++ side allocates. Each listener instance carries:

```java
public final class MppXxxListener implements <android-interface> {
    private final long ownerPtr;   // reinterpret_cast<jlong>(C++ handler / view)
    private final int  kind;       // discriminator
    public MppXxxListener(long ownerPtr, int kind) { ... }
    @Override public void onXxx(...) { nativeDispatchXxx(ownerPtr, kind, ...); }
    private static native void nativeDispatchXxx(long ownerPtr, int kind, ...);
}
```

The native trampoline (`src/handlers/android/xxx_dispatch.cpp`) switches on `kind` to forward to the right widget-specific function.

### Listener classes shipped today

| Java class | Android interface | C++ trampoline | Used by (kind → widget) |
|---|---|---|---|
| `MppActionRouter` | `View.OnClickListener` | `action_router.cpp` | 0=NavigationPage back · 1=Shell tab · 2=TabbedPage tab |
| `MppItemClickRouter` | `AdapterView.OnItemClickListener` | `item_click_router.cpp` | 0=ListView · 1=CollectionView · 2=TableView |
| `MppCheckedChangeListener` | `CompoundButton.OnCheckedChangeListener` | `compound_button_dispatch.cpp` | 1=switch_ · 2=check_box · 3=radio_button · 4=switch_cell |
| `MppTextWatcher` | `android.text.TextWatcher` | `text_watcher_dispatch.cpp` | 1=entry · 2=editor · 3=entry_cell |
| `MppClickRouter` | `View.OnClickListener` | (legacy — pre-ADR-0013 button-specific) | n/a |
| `MppEditorActionListener` | `TextView.OnEditorActionListener` | `editor_action_dispatch.cpp` | 1=entry_cell |
| `MppWebViewClient` | `WebViewClient` (subclass) | `web_view_client_dispatch.cpp` | n/a (one owner per WebView) |
| `MppJsBridge` | `@JavascriptInterface` host | `js_bridge_dispatch.cpp` | n/a (one bridge per HybridWebView) |
| `MppNumberPickerListener` | `NumberPicker.OnValueChangeListener` | (number_picker handler) | n/a |
| `MppSeekBarChangeListener` | `SeekBar.OnSeekBarChangeListener` | (slider handler) | n/a |

The "n/a" rows have a one-instance-per-owner shape (e.g., `MppWebViewClient` carries one `handlerPtr` only; there's no need for a kind because each WebView has its own listener instance bound to a specific handler).

### Allocating a new kind

When adding a new widget that needs an existing Android-event interface:

1. Pick the next unused integer in the relevant class's `kind` namespace. The C++ trampoline's `switch` block is the source of truth; the Java class doesn't enumerate them.
2. Add the new `case` to the trampoline (`src/handlers/android/<xxx>_dispatch.cpp`). Forward to a `mpapp::android_<widget>_dispatch_<event>(handler*, payload)` free function.
3. Implement that free function in the widget handler's `.cpp`. Convention: it pulls state off the `bound_` pointer and updates Observables / emits signals.
4. Add a comment line to the Java class's header documenting the new kind. **Strongly recommended** — without it, kind allocation drifts as different contributors guess what's taken.

### Allocating a new listener class

When the existing classes don't fit (a new Android event interface is needed):

1. Create `examples/android_hello/app/src/main/java/io/mpapp/Mpp<Name>Listener.java`. Follow the existing-listener shape: final class, `ownerPtr` + `kind` + payload fields, native dispatch method.
2. Create `src/handlers/android/<name>_dispatch.cpp` with the JNI trampoline. JNI symbol name = `Java_io_mpapp_Mpp<Name>Listener_nativeDispatch<Method>`.
3. Document the new class + initial kind in the Java header comment.
4. Update this ADR's table.

## Consequences

### Positive

- **One Java class per interface, not per widget.** A new widget that needs an existing event interface ships purely C++ — no new `.java` file, no Gradle rebuild ceremony.
- **Stable JNI symbol surface.** Changing widget internals doesn't move JNI symbols. The trampoline names form a tiny, well-known set.
- **Easy to read.** The C++ trampoline's `switch` block lists every consumer of an Android event interface in one place.
- **No reflection.** The `kind` discriminator is a plain `int`; no `Class.forName` / `getMethod` on the JNI hot path.

### Negative

- **Kind allocation drift.** Without discipline, two contributors could grab the same `kind` number and the trampoline's `switch` block becomes ambiguous. Mitigation: the Java class's header comment is the canonical kind registry; the ADR table above is a backup.
- **Type erasure on `ownerPtr`.** The `jlong` collapses pointer type information, requiring the trampoline to `reinterpret_cast` to the right C++ type. A mismatch is undefined behavior. Mitigation: the trampoline's `switch` arm and the `set_handler` call site are usually in the same .cpp file, keeping the cast obviously correct.
- **One-shot listeners pay the `kind` cost.** `MppWebViewClient` and `MppJsBridge` carry a `kind` field they don't strictly need — there's only one instance per owner. The uniformity is worth the dead int.

### Neutral

- Apple platforms have analogous patterns (selectors / blocks) but their interop story is different enough that this ADR scopes to Android only.

## Alternatives Considered

- **One Java class per widget.** Rejected — multiplies Java surface area; every new widget needs a Gradle rebuild even for a trivial JNI route.
- **JNI-direct from each widget handler.** Rejected — Java still needs the listener interface implementation; the boilerplate moves but doesn't shrink.
- **fbjni-style generated bindings.** Considered but deferred to [[T-0004-jni-codegen-spike]]. The kind-discriminated family is the manual pattern fbjni codegen would target.

## References

- [[ADR-0014-page-navigation-stack]] · [[ADR-0018-hybrid-webview-typed-bridge]] · [[ADR-0020-virtualized-item-host-wrap-platform]] · [[ADR-0021-tableview-cell-types]] — ADRs whose Android handlers use this pattern.
- `examples/android_hello/app/src/main/java/io/mpapp/Mpp*Listener.java` — the listener classes themselves.
- `src/handlers/android/*_dispatch.cpp` — the native trampolines.
- [[T-0004-jni-codegen-spike]] — fbjni-based codegen that may eventually generate this pattern automatically.
