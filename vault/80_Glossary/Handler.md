---
type: glossary
term: "Handler"
tags:
  - type/glossary
---

# Handler

The bridge between an MPAPP component's [[Basic-Surface|surface]] and a native platform widget. CRTP-typed, partial-specialised on a platform tag. Each handler owns a single native widget, translates property changes from `Observable<T>` mutations to native setters via `map_<property>` methods, and forwards native events back to the surface's `signal<...>` members. See [[Handlers]].

Two attachment modes coexist after [[ADR-0024-wrapper-component-pattern]]:

* **Embedded (default for app code).** The [[Wrapper-Component]] (`mpapp::<name>`) holds a `internal::<name>_handler<platform::current>` member by value and auto-binds it in the wrapper's constructor. App code never names the handler.
* **External (mock tests + advanced flows).** The surface (`mpapp::internal::basic_<name>`) holds the handler by pointer. Tests construct the surface, instantiate `mpapp::<name>_handler<platform::mock>` separately, and call `h.map_<property>(surface)` themselves. This is what keeps mock-handler tests link-isolated from the per-platform handler library.

The handler class itself is identical in both modes — only the lifetime + attachment differ.

Handlers live in `mpapp::internal::` so the wrapper layer's `template <class P = platform::current> using <name>_handler = internal::<name>_handler<P>;` alias keeps the public spelling stable.

## See in code

- [`include/mpapp/handlers/mock/button_handler.hpp`](../../include/mpapp/handlers/mock/button_handler.hpp) — canonical mock handler (`internal::button_handler<platform::mock>`); records `map_text(basic_button&)` invocations + `clicked` emissions into a `calls()` vector, no native widget.
- [`include/mpapp/handlers/windows/button_handler.hpp`](../../include/mpapp/handlers/windows/button_handler.hpp) + [`src/handlers/windows/button_handler.cpp`](../../src/handlers/windows/button_handler.cpp) — real Windows handler wrapping `muxc::Button`. Mirrored by [`src/handlers/linux/button_handler.cpp`](../../src/handlers/linux/button_handler.cpp) (GTK4) and [`src/handlers/android/button_handler.cpp`](../../src/handlers/android/button_handler.cpp) (JNI).
- Public alias: [`include/mpapp/button.hpp`](../../include/mpapp/button.hpp) — `template <class P = platform::current> using button_handler = internal::button_handler<P>;` keeps `mpapp::button_handler<>` and `mpapp::button_handler<platform::mock>` valid spellings.
- Every component's handler set lives at `include/mpapp/handlers/<platform>/<component>_handler.hpp` + `src/handlers/<platform>/<component>_handler.cpp` — see [[Controls Inventory]] for the full matrix.

## See also

- [[Basic-Surface]] — what the handler's `map_<property>(basic_<name>&)` parameters bind to.
- [[Wrapper-Component]] — the layer that embeds the handler by value.
- [[Mock-Implementation]] — the platform::mock specialisation.
- [[Handlers]] — the architecture doc.
- [[ADR-0024-wrapper-component-pattern]] — the wrapper + surface split.
