---
type: research
subject: "Avalonia"
framework: "avalonia"
created: 2026-05-12
applicableTo:
  - "ADR-0003-xaml-only-no-custom-dsl"
  - "ADR-0004-maui-xaml-superset-compat"
  - "10_Architecture/Hot Reload"
  - "10_Architecture/XAML Compatibility"
recommendation: further-study
tags:
  - type/research
  - framework/avalonia
---

# Avalonia

> [!info] Status
> Researched — direct comparison point for [[ADR-0003-xaml-only-no-custom-dsl]] and [[10_Architecture/XAML Compatibility]].

## Summary

Avalonia is an open-source .NET cross-platform UI framework that descends from WPF and competes directly with .NET MAUI. It targets Windows, macOS, Linux, iOS, Android, and the browser (via WebAssembly), with a single XAML markup and a Skia backend. For MPAPP, Avalonia is the most architecturally similar contemporary project: it doubled down on XAML, ships a strong hot-reload story, and chose custom rendering over native widgets — the axis where MPAPP diverges.

## What They Do

Avalonia mirrors WPF's object model: `AvaloniaObject` carries `StyledProperty<T>` instances (the analog of WPF's `DependencyProperty`), bindings flow through a property system with priorities and value precedence, and templates plus styles compose visuals. The markup is a XAML dialect parsed by the **XamlIl** compiler, which emits IL directly (no runtime reflection cost) and supports compiled bindings via `x:CompileBindings`. Layout uses a classic measure/arrange pass; the visual tree separates `Visual` (rendering) from `Control` (logic) much like WPF.

Rendering goes through **Skia** by default (CPU fallback; experimental Direct2D). Every control is drawn by Avalonia, so Linux and Windows builds are pixel-identical. Platform integration is a `Window` per OS hosting the Skia surface, plus shims for IME, clipboard, drag-and-drop, and accessibility (UIA, AT-SPI, NSAccessibility).

Hot reload uses .NET **Edit-and-Continue** plus Avalonia's `HotReload` integration: XAML edits patch the parsed tree at runtime, code-behind edits patch the assembly. Tooling lives in Rider and Visual Studio, with an in-IDE XAML previewer.

Data flow leans on the .NET ecosystem — `INotifyPropertyChanged`, `ReactiveUI`, CommunityToolkit MVVM source generators. Compiled bindings (`x:DataType`) give compile-time checks comparable to MPAPP's goal in [[10_Architecture/Type System]].

## What Works / What Doesn't

### Strengths

- **XAML maturity**: closest living relative to WPF/MAUI XAML; compiled bindings, styles, control templates, and resource dictionaries all work. Strong precedent for [[ADR-0004-maui-xaml-superset-compat]].
- **Hot reload UX**: fast, reliable XAML hot reload; useful previewer. Best-in-class .NET hot-reload story today.
- **Pixel-perfect cross-platform consistency**: identical look on every OS.
- **License**: MIT — clean for downstream commercial use; aligns with [[RFC-0001-licensing-and-patent-strategy]].
- **Active community + commercial sponsor**: Avalonia OÜ funds development via paid offerings (XPF, Accelerate) without imposing GPL.

### Weaknesses

- **.NET runtime dependency**: requires CoreCLR — a foundational choice MPAPP cannot follow.
- **Skia rendering**: same critique as [[60_Research/Flutter-Engine]] — accessibility, IME, theming, and platform animation always trail the OS.
- **Property system overhead**: `StyledProperty<T>` plus the priority stack has runtime cost that template-based C++ can avoid.
- **XAML dialect drift**: close to WPF/MAUI but not identical (`Selector`/`Classes` style syntax). MPAPP's superset goal must navigate similar questions.

## Applicable to MPAPP

- **Borrow**: compiled-XAML approach (XamlIl is the closest in spirit to `mpapp-xc`); compiled bindings with `x:DataType` — direct inspiration for typed bindings under [[ADR-0003-xaml-only-no-custom-dsl]]; hot-reload UX target for [[10_Architecture/Hot Reload]]; in-IDE previewer pattern; MIT-style permissive licensing per [[RFC-0001-licensing-and-patent-strategy]].
- **Borrow with adaptation**: styles + control templates as XAML primitives, reconciled with native-widget back-ends (a templated `Button` cannot redraw a native NSButton).
- **Reject**: Skia/custom rendering — MPAPP uses native [[Handlers]] per [[ADR-0006-interop-parity]]; the .NET runtime dependency; the `AvaloniaObject` + reflection-tinged dependency-property model — MPAPP prefers template wrappers per [[ADR-0009-public-api-template-wrappers-only]].

> [!important] Recommendation
> `further-study` — Avalonia is the single most useful comparison point for MPAPP's XAML and hot-reload work. Track its XamlIl compiler, hot-reload integration, and previewer architecture closely through [[M-02-Infrastructure]] and [[M-09-Tooling-DX]]. Do **not** adopt its rendering strategy.

## References

- Official: https://avaloniaui.net/
- Source: https://github.com/AvaloniaUI/Avalonia
- XamlIl compiler: https://github.com/kekekeks/XamlIl
- Compiled bindings docs: https://docs.avaloniaui.net/docs/basics/data/bindings/compiled-bindings
- Hot reload: https://docs.avaloniaui.net/docs/guides/developer-tools/hot-reload
