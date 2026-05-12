---
type: research
subject: "wxWidgets"
framework: "wxwidgets"
created: 2026-05-12
applicableTo: []
recommendation: reject
tags:
  - type/research
  - framework/wxwidgets
---

# wxWidgets

> [!info] Status
> Reject **the framework**; affirm **the native-widget principle** it pioneered.

## Summary

wxWidgets is a long-running (since 1992) cross-platform C++ UI library whose central premise is the same as MPAPP's: **wrap the native platform widget on each operating system rather than draw your own**. On Windows, a `wxButton` is a real `BUTTON`; on macOS, an `NSButton`; on Linux, a `GtkButton`. The architectural ambition matches MPAPP's [[Handlers|handler]] model almost exactly. The implementation, however, is steeped in 1990s C++ idioms that MPAPP cannot inherit. We study wxWidgets to validate the **idea** and reject the **codebase**.

## What They Do

wxWidgets exposes a class hierarchy (`wxWindow`, `wxFrame`, `wxButton`, `wxTextCtrl`, `wxGrid`, ...) whose virtual methods are implemented per platform by a thin shim over `Win32`, `Cocoa`, `GTK+`, `Qt` (experimental), and a handful of others. Event delivery uses a `wxEvtHandler` chain. Layout uses `wxSizer` hierarchies. There is no XAML; UI is built either in code or in `XRC` (an XML format). Many bindings exist (Python via wxPython, Lua, Perl), but the canonical user is a C++ developer.

## Strengths

- **Genuinely native widgets.** Same ambition as MPAPP. A button looks like the platform's button, has the platform's a11y semantics, and respects system theming for free.
- **Massive control surface.** Tables (`wxGrid`, `wxListCtrl`, `wxDataViewCtrl`), trees, ribbon, aui-docking, rich text, HTML viewer, OpenGL canvas, property grid, MIME-aware file dialogs, printing — far broader than [[Controls Inventory|MPAPP's current inventory]].
- **Mature.** Three decades of bug fixes, edge-case handling, and platform-quirk workarounds.
- **Broad platform reach.** Windows, macOS, Linux/GTK, BSD, and historically embedded targets.
- **Stable license.** wxWindows Library Licence (LGPL-like with a static-linking exception) — friendlier than plain LGPL for the [[Build System|build system]] story per [[RFC-0001-licensing-and-patent-strategy]].

## Weaknesses

- **Macro-heavy 1990s ergonomics.** Event tables use `wxDECLARE_EVENT_TABLE`, `wxBEGIN_EVENT_TABLE(...)`, `EVT_BUTTON(...)`, `wxEND_EVENT_TABLE()`. Class declarations sprinkle `wxIMPLEMENT_DYNAMIC_CLASS`. This is precisely the world Rule 1 and [[ADR-0002-no-macros-in-public-api]] forbid.
- **C++03 idioms throughout.** Manual `new`/`delete` parent-owned widget trees, raw pointers in public API, `wxString` instead of `std::string`, hand-rolled containers (`wxArrayString`), no `constexpr`, no concepts, no ranges, no coroutines.
- **Verbose, ceremonial API.** Window construction takes many lines of `Create`, `SetSizer`, `Layout`, `Show`. No declarative markup comparable to [[XAML Compatibility|XAML]]. XRC is XML but is data-bound by hand, not by a compiler.
- **No observable / binding system.** No `Observable<T>`, no `Computed<...>`, no [[Binding-Path|binding paths]]. Event-driven only; MVVM is bolt-on.
- **No mobile story.** iOS and Android are not production targets. MPAPP requires first-class parity across all five per [[ADR-0006-interop-parity]].
- **Threading model dates from the single-core era.** `wxThread`, manual `wxMutex`, no native `co_await` integration — MPAPP's [[Threading and Dispatcher|dispatcher]] has no clean wx analog.
- **Custom string and container types.** Forces users into a wx-flavored universe rather than the standard library.
- **Hot reload is structurally impossible.** XRC is loadable at runtime but event-table wiring is compiled in. No equivalent to MPAPP's [[Hot-Reload|XAML hot reload]].

## Applicable to MPAPP

- **Adopt the native-widget principle as validation.** wxWidgets proves a single C++ API can drive truly native controls across desktop platforms. This is the exact bet MPAPP makes via the [[Handlers|handler architecture]] (now extended to mobile).
- **Adopt wxWidgets as a coverage benchmark.** Its control surface — `wxDataViewCtrl`, `wxAuiManager`, `wxPropertyGrid`, `wxRichTextCtrl` — is a useful "what real desktop apps need" yardstick when prioritizing the [[Controls Inventory|controls inventory]].
- **Reject macro-driven event tables.** MPAPP wires events through template-typed [[Observable-Property|observable]] members and [[Handlers|handler]] property mappers — never via `BEGIN_EVENT_TABLE`-style macros (Rule 1, [[ADR-0002-no-macros-in-public-api]], [[ADR-0009-public-api-template-wrappers-only]]).
- **Reject raw-pointer ownership.** MPAPP uses standard smart-pointer ownership and value semantics where possible per [[ADR-0001-cpp-standard-baseline]].
- **Reject wx-flavored types.** No `MpString`, no `MpArray`. Standard library types only at the boundary.

> [!important] Recommendation
> `reject` the wxWidgets framework as a foundation. Learn from its 30-year proof that the native-widget bet works, and use its control surface as a coverage benchmark.

## References

- Official: https://www.wxwidgets.org/
- [[ADR-0002-no-macros-in-public-api]]
- [[ADR-0006-interop-parity]]
- [[ADR-0009-public-api-template-wrappers-only]]
- [[Handlers]]
- [[Controls Inventory]]
