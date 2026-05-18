---
type: task
id: T-0011
title: App-shell abstraction (Application / Window / Page / layout primitives)
status: todo
milestone: M-03
owner: ""
area: handlers
blockedBy: []
coveragePercent: 0
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/todo
  - area/handlers
  - phase/p2
---

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

- [ ] `mpapp::application` class: single-instance, owns the event loop,
      exposes the main dispatcher. Selected per platform via the same
      `application_handler<platform::current>` partial-specialisation
      pattern used by widget handlers.
- [ ] `mpapp::window` class: top-level chrome (title, content,
      activate/close, size). Handler-backed.
- [ ] `mpapp::page` class: navigable content host (matches MAUI's
      `Page` / `ContentPage`). Handler-backed.
- [ ] `mpapp::stack_layout` (and at minimum `grid_layout`): cross-platform
      layout primitives so the spike doesn't have to name
      `muxc::StackPanel`. Existing `mpapp::layout` may be the parent.
- [ ] Single entry-point function `int mpapp::run<App>(int argc, char** argv)`
      that hides `MddBootstrap*`, `winrt::init_apartment`,
      `Application::Start` on Windows and the equivalent goo on every
      other platform. No public-API macro (Rule 1).
- [ ] WinUI 3 handler implementations for `application`, `window`,
      `page`, `stack_layout` — enough that `windows_button_spike/main.cpp`
      can be rewritten with **zero `winrt::`, `mux::`, `muxc::`,
      `Mdd*` tokens** in the user-facing code.
- [ ] Mock handlers for each of the four new control types (per
      [[ADR-0008-mock-first-implementation]]) with unit tests on the
      lifecycle contract.
- [ ] Per-component notes in `10_Architecture/Components/`:
      `Application.md`, `Window.md`, `Page.md`, `StackLayout.md`,
      `Grid.md` — each cross-linked to the MAUI handler it mirrors.
- [ ] [[10_Architecture/Handlers.md]] updated with the app-shell layer.
- [ ] [[10_Architecture/Controls Inventory.md]] rows promoted to `mock`
      for the four new types, and to `windows-real` once the WinUI
      handlers land.
- [ ] Linux (GTK4), macOS (AppKit), iOS (UIKit), Android (fbjni) handler
      stubs with `// TODO(T-NNNN)` markers so the surface is visible —
      real implementations follow in their own milestones.
- [ ] 100% line+branch coverage on every new mock handler + the
      `mpapp::run<App>` entry-point glue (mock path).
- [ ] Screenshot of the rewritten spike running on Windows; recording of
      the rewritten code building + launching for the **first time** on
      a fresh checkout to prove the user-facing surface has no remaining
      WinRT tokens.

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
