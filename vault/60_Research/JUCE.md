---
type: research
subject: "JUCE"
framework: "juce"
created: 2026-05-12
applicableTo:
  - "10_Architecture/Build System"
  - "10_Architecture/Handlers"
recommendation: further-study
tags:
  - type/research
  - framework/juce
---

# JUCE

> [!info] Status
> Researched — relevant lessons for [[M-01-Foundations]] and [[M-09-Tooling-DX]].

## Summary

JUCE is a single-codebase, C++ cross-platform application framework born in the audio plug-in world (it powers a large share of shipping VST/AU/AAX plug-ins). For MPAPP it is an instructive precedent: a C++ framework that has remained healthy for two decades, ships its own project generator, and delivers a single-source-of-truth UI layer across Windows, macOS, Linux, iOS, and Android. MPAPP shares JUCE's "C++-first, one codebase" stance but rejects JUCE's custom-drawing approach in favor of native widgets per [[ADR-0006-interop-parity]].

## What They Do

JUCE's `Component` is the universal UI base class: every button, slider, label, and container inherits from it and overrides `paint(Graphics&)` plus mouse/keyboard handlers. The framework owns its own rendering stack — software rasterizer with optional GPU backends (OpenGL, Metal, Direct2D more recently) — and draws every pixel itself. Styling is centralized through the Look-and-Feel system: a `LookAndFeel` subclass overrides `drawButtonBackground`, `drawSliderThumb`, etc., and the whole app gets restyled. Platform integration is confined to thin native windows ("peers") that host the JUCE-drawn surface.

Project setup historically went through **Projucer**, a GUI tool that emits per-IDE project files (Visual Studio, Xcode, Make, Android Studio) from a single `.jucer` description; modern JUCE also supports a first-class CMake flow. Modules are vendored as a tree of `juce_*` folders; the build picks them up via the project description.

For data flow JUCE relies on `ValueTree` (an observable, copy-on-write tree of typed properties with undo support) and `Value` (a single observable cell). These predate modern reactive systems but cover the same ground: typed change notifications, listener lists, and a tree shape that maps cleanly onto UI hierarchies. Threading uses a single `MessageManager` (the UI thread) plus utilities like `MessageManagerLock` and `AsyncUpdater` to marshal work — a model MPAPP can compare with [[10_Architecture/Threading and Dispatcher]].

## What Works / What Doesn't

### Strengths

- **Cross-platform fidelity**: a JUCE app feels identical across OSes precisely because nothing is delegated to native controls. Bug surface is small.
- **Build maturity**: Projucer plus the CMake support produce trustworthy artifacts; codesigning hooks are first-class.
- **Single-binary deployment**: no runtime to ship beyond the binary itself; no JIT.
- **Longevity**: the API has evolved in compatible ways for ~20 years — proof that a C++ UI framework can survive that long with discipline.
- **`ValueTree`** is a rare example of a typed observable tree with undo built in. [[10_Architecture/Observable Properties]] should at least consider its design.

### Weaknesses

- **Custom drawing is not native**: scrollbars, focus rings, IME pop-ups, accessibility tree, and platform animations all diverge from OS conventions. Accessibility in particular needs constant catch-up work.
- **No XAML/markup story**: UI is built imperatively in C++ — directly opposed to [[ADR-0003-xaml-only-no-custom-dsl]].
- **License friction**: dual-licensed under GPLv3 and a commercial license. GPL is incompatible with MPAPP's posture under [[RFC-0001-licensing-and-patent-strategy]] (Rule 9 forbids GPL runtime dependencies), and the commercial tier carries per-seat revenue terms.
- **Look-and-Feel is global, not compositional**: themes are inheritance-based; restyling one widget without affecting others requires careful subclassing.

## Applicable to MPAPP

- **Borrow**: the single-codebase project shape; a first-party project generator analogous to Projucer that emits platform IDE files (see [[10_Architecture/Build System]]); the idea of a flagship **sample gallery** app that exercises every control on every platform; the discipline of polish across all five platforms before declaring a release.
- **Borrow with adaptation**: `ValueTree`-style undo and change-notification semantics for [[10_Architecture/Observable Properties]] — MPAPP keeps strong typing via templates rather than runtime `var`.
- **Reject**: custom drawing of standard controls — MPAPP routes everything through native [[Handlers]] per [[ADR-0006-interop-parity]]. Reject the dual GPL/commercial license model; MPAPP targets Apache 2.0 + optional commercial support per [[RFC-0001-licensing-and-patent-strategy]].

> [!important] Recommendation
> `further-study` — JUCE is not a template for MPAPP's UI architecture, but its longevity, build tooling, and `ValueTree` design contain lessons MPAPP cannot afford to ignore. Track JUCE releases and revisit its accessibility and Direct2D backend work during [[M-09-Tooling-DX]].

## References

- Official: https://juce.com/
- Source: https://github.com/juce-framework/JUCE
- Tutorials: https://juce.com/learn/tutorials/
- `ValueTree` reference: https://docs.juce.com/master/classValueTree.html
- License: https://juce.com/legal/juce-8-licence/
