---
type: task
id: T-0011
title: App-shell abstraction (Application / Window / Page / layout primitives)
status: in-progress
milestone: M-03
owner: ""
area: handlers
blockedBy: []
coveragePercent: 100
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/in-progress
  - area/handlers
  - phase/p2
---

## Status — 2026-05-18

Same-day execution after filing. Landed:

* Public headers: `application`, `window`, `page`, `stack_layout`,
  `grid_layout`, `layout_types`, `run`, `native_handlers`.
* Hierarchy unification: `control<Derived>` now inherits from `view`,
  so every widget (button, label, …) is a `view*` and can be
  composed into layouts or assigned as `window.content` /
  `page.content`. `control::derived()` was previously unused.
* Mock handlers for all five new types + 17 new unit tests
  (`application_test`, `window_test`, `page_test`, `stack_layout_test`,
  `grid_layout_test`). Total mock-surface test count 126/126 (was 109).
* WinUI 3 real handlers for `application`, `window`, `stack_layout`
  with implementations in `src/handlers/windows/`. `MddBootstrap*`,
  `winrt::init_apartment`, `Application::Start`, the `mux::ApplicationT`
  subclass — all hidden inside the `application_handler<platform::windows>`
  TU. (`page` + `grid` mock-only; real handlers tracked for M-04.)
* `examples/windows_button_spike/main.cpp` rewritten — **zero
  `winrt::`/`mux::`/`muxc::`/`Mdd*` tokens** in user-facing code (verified
  via grep: only matches are in the file's documentation comment that
  names what was eliminated). Compiles and links into a 2.5 MB exe.
* Per-component docs updated: `Application.md`, `Window.md`, `Page.md`
  promoted to mock/windows-real as appropriate; new `StackLayout.md`
  and `Grid.md` created. `Controls Inventory` updated with the new
  statuses.
* [[ADR-0012-application-window-handler-abstraction]] promoted to
  `accepted`.
* Side-quest fix: `cmake/WindowsAppSDK.cmake` updated for the
  `Microsoft.Windows.AI.MachineLearning` → `Microsoft.WindowsAppSDK.ML`
  package rename in WindowsAppSDK 1.8.260416003, unblocking the spike
  build entirely.

Remaining for full task closure (the `hasScreenshots` / `hasRecordings`
gates per Rule 11):

1. Interactive screenshot of the rewritten spike running on the user's
   Windows desktop. (The exe builds + links; runtime `MddBootstrapInitialize2`
   exits with `ERROR_NOT_LOGGED_ON` in the headless tool session here —
   that's the same environment constraint that affected the T-0003 spike
   when run outside an interactive session.)
2. Screen recording of the dev loop (edit → rebuild → run) for the
   rewritten spike.

Status remains `in-progress` until those two visual-proof artefacts
land; the code-side acceptance criteria are otherwise satisfied.

# T-0011 — App-shell abstraction

## Goal

Close the largest remaining cross-platform-abstraction gap: the **app shell**.
Today every `examples/windows_button_spike/main.cpp`-style program carries
~25 raw WinRT/WinAppSDK tokens (apartment init, `MddBootstrap*`,
`mux::ApplicationT<>`, `mux::Window`, `muxc::StackPanel`,
`mux::Thickness`, `mux::HorizontalAlignment`, …). The widget surface is
abstract (`mpapp::button`, `mpapp::Observable`); the **app surface** is
not. T-0011 ports that next layer.

The gap is structural, not cosmetic: per [[ADR-0006-interop-parity]] a
user-facing program must compile *unmodified* against any platform's
handler set. As of the post-batch3 main, that is true for the
`view_model` portion of the spike (Observable + signal) but false for
everything between `wWinMain` and the first `b.text = "Click me"`.

## Acceptance Criteria

- [x] `mpapp::application` class: handler-backed, selected per platform
      via `application_handler<platform::current>`.
- [x] `mpapp::window` class: top-level chrome (title, content,
      activate/close, size).
- [x] `mpapp::page` class: navigable content host (matches MAUI's
      `Page` / `ContentPage`).
- [x] `mpapp::stack_layout` + `mpapp::grid_layout`: cross-platform
      layout primitives so the spike doesn't have to name
      `muxc::StackPanel` / `muxc::Grid`.
- [x] Single entry-point function `int mpapp::run<App>(int argc, char** argv)`
      hiding `MddBootstrap*`, `winrt::init_apartment`,
      `Application::Start` on Windows. No public-API macro (Rule 1).
- [x] WinUI 3 handler implementations for `application`, `window`,
      `stack_layout` — `windows_button_spike/main.cpp` rewritten with
      **zero `winrt::`, `mux::`, `muxc::`, `Mdd*` tokens** in the
      user-facing code (only matches are in the documentation comment).
      `page` + `grid` WinUI 3 real handlers deferred to M-04.
- [x] Mock handlers for each of the five new control types per
      [[ADR-0008-mock-first-implementation]], with 17 unit tests on
      property-mapper + lifecycle contracts. 126/126 tests pass.
- [x] Per-component notes: `Application.md`, `Window.md`, `Page.md`
      updated; `StackLayout.md` + `Grid.md` created. Each cross-links
      to its MAUI counterpart.
- [x] [[10_Architecture/Controls Inventory.md]] rows promoted —
      Application + Window + StackLayout → `windows-real`; Page +
      Grid → `mock`.
- [ ] Linux (GTK4), macOS (AppKit), iOS (UIKit), Android (fbjni) handler
      stubs. **Tracked but not in T-0011** — `native_handlers.hpp` emits
      explicit `#error` per platform with a TODO so misconfigured builds
      fail loudly. Real implementations land in M-04 / M-05 / M-06 / M-07.
- [x] [[10_Architecture/Handlers.md]] retained — the pattern was
      already documented and the new types fit it without changes.
- [ ] Screenshot of the rewritten spike running on Windows.
      (Code complete; runtime invocation in this tool session exits
      with `ERROR_NOT_LOGGED_ON` from `MddBootstrapInitialize2` — same
      headless-bootstrap constraint that affected T-0003. Needs an
      interactive desktop run.)
- [ ] Screen recording of the dev loop (edit, rebuild, run).

## Design sketch

See [[notes/api-sketch]] for the concrete proposed headers and the
rewritten `windows_button_spike/main.cpp`. See [[notes/handler-pattern]]
for how `application_handler<platform::windows>` extends the existing
handler taxonomy (and why `application` is a singleton in MAUI but does
not need to be in MPAPP).

The decision record for the app-shell layer will be
**ADR-0012-application-window-handler-abstraction** — currently `draft`,
to be promoted to `accepted` once T-0011's spike build proves the API.

## Notes

This is the **largest single architectural piece** between the current
M-02 state and shipping the first real cross-platform sample app. The
work is sized for its own batch (M-03 batch 1):

  * App-shell headers + mock handlers
  * WinUI 3 real handlers + spike rewrite
  * GTK4 real handlers (gated on T-0007 unblocking)
  * Mock-handler tests
  * Per-component docs + Inventory updates

The XAML compiler (`mpapp-xc`) is unaffected — `<Application>`,
`<Window>`, `<StackLayout>` etc. lower to the same C++ surface a user
could write by hand, which is what T-0011 builds.

## Links

- Trigger: user observation 2026-05-18 — "examples directly tied to winrt
  and `::winrt::Microsoft::UI::Xaml` without any MAUI-like abstraction
  layer"
- Sibling components: [[Components/Application]], [[Components/Window]],
  [[Components/Page]], [[Components/StackLayout]], [[Components/Grid]]
- Related: [[Handlers]], [[Platform Interop]], [[ADR-0006-interop-parity]],
  [[ADR-0008-mock-first-implementation]],
  [[ADR-0009-public-api-template-wrappers-only]]
- MAUI references: `references/maui/src/Core/src/MauiApp.cs`,
  `references/maui/src/Controls/src/Core/Application/Application.cs`,
  `references/maui/src/Controls/src/Core/Window.cs`
