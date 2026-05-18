---
type: adr
id: ADR-0012
title: Application / Window / Page / Layout handlers extend the widget-handler pattern
status: accepted
decisionDate: 2026-05-18
deciders:
  - alex
supersedes: []
supersededBy: []
area: handlers
---

# ADR-0012 — Application / Window / Page / Layout handlers extend the widget-handler pattern

## Status

**accepted** — [[T-0011-app-shell-abstraction]] landed the mock surface for all five new types (Application, Window, Page, StackLayout, Grid), the WinUI 3 real handlers for Application / Window / StackLayout, and the rewritten `examples/windows_button_spike/main.cpp` with zero `winrt::`/`mux::`/`muxc::`/`Mdd*` tokens in user-facing code. 126/126 tests green (109 previous + 17 new for the app-shell surface).

## Context

The post-batch3 codebase has a clean cross-platform abstraction at the
**widget** level: `mpapp::button`, `mpapp::label`, etc. each forward
property writes through `<widget>_handler<platform::current>` to a
native object (WinUI `Button`, GTK4 `GtkButton`, …).

It has **no abstraction at the app-shell level**. The T-0003 spike
(`examples/windows_button_spike/main.cpp`) carries ~25 raw
`winrt::`/`mux::`/`muxc::`/`Mdd*` tokens in user-facing code,
violating [[ADR-0006-interop-parity]] for everything above the widget
layer. The widgets themselves abstract correctly; the bootstrap, the
top-level window, the layout primitives, and the entry point do not.

This ADR pins the *pattern* MPAPP uses for that next layer.

## Decision

The app-shell layer reuses the existing handler pattern, with two small
extensions:

1. **New types follow the same partial-specialisation discipline.**
   `mpapp::application`, `mpapp::window`, `mpapp::page`,
   `mpapp::stack_layout`, `mpapp::grid_layout` are control-like classes
   whose platform handlers live at
   `mpapp/handlers/<platform>/<name>_handler.hpp` and are partial
   specialisations of `<name>_handler<Platform>`. No new mechanism.

2. **`mpapp::application` is single-instance per process on every
   platform.** Windows (WinUI), Linux (GTK4 conventionally),
   macOS (`NSApplication.shared`), iOS (`UIApplication.shared`), and
   Android (`Application`-singleton-per-JVM) all enforce or convention
   single-instance. MPAPP enforces it via an assertion in the
   handler's constructor. Multi-instance is a future ADR if and when a
   platform requires it.

3. **`mpapp::run<App>(int argc, char** argv) -> int`** is the
   entry-point helper. It constructs the platform handler, runs the
   event loop, returns the exit code. No public-API macro
   (per [[ADR-0002-no-macros-in-public-api]] and
   [[ADR-0009-public-api-template-wrappers-only]]).

4. **Layout primitives carry framework-owned types** —
   `mpapp::orientation`, `mpapp::h_align`, `mpapp::v_align`,
   `mpapp::thickness`. User code never names `muxc::Orientation`,
   `mux::Thickness`, GTK `GtkOrientation`, etc.

5. **`window.content = view_ref;`** uses non-owning references.
   The user owns the view-derived widgets; the window handler watches
   the `content` Observable and rebinds the platform window's content
   slot on change. Lifetime is guaranteed by destruction order:
   `application_handler` tears down platform state *before* user-app
   fields run their destructors.

## Consequences

**Positive:**

- The rewritten `windows_button_spike/main.cpp` has zero `winrt::`,
  `mux::`, `muxc::`, `Mdd*` tokens in user-facing code. Parity rule
  satisfied above the widget layer.
- The XAML compiler (`mpapp-xc`) lowers `<Application>`, `<Window>`,
  `<StackLayout>`, `<Grid>` to the same C++ surface the user could
  write by hand — no special-case path.
- New widgets/types added in future milestones (e.g. CollectionView)
  use exactly the pattern already documented.

**Negative / costs:**

- Five new control surfaces × five real platforms = 25 real handler
  implementations to write before parity completes. Sized into M-03
  through M-07 milestones.
- `application_handler<platform::windows>` must hide the
  *outermost-call-on-main-thread* requirement of
  `mux::Application::Start`. This forces `mpapp::run<App>` to be a
  blocking call that owns `main()`. Acceptable — every cross-platform
  UI framework imposes the same constraint.
- The framework-owned `orientation`/`thickness` types add a thin
  translation layer per handler. The translation is mechanical and
  zero-cost (compile-time `if constexpr` per platform).

**Rejected alternatives:**

- **Expose `handler.native()` as the user-facing way to attach widgets
  to layouts** (status-quo of the T-0003 spike). Rejected: it leaks
  WinRT into user code and is not portable.
- **Provide platform-specific `mpapp::win::window`, `mpapp::gtk::window`
  variants.** Rejected: contradicts [[ADR-0006-interop-parity]].
- **Use C++ modules to hide platform headers.** Rejected for now:
  MPAPP's current cross-compilation toolchain (Zig 0.13) does not bundle
  `clang-scan-deps` (see [[T-0009-cross-compilation-matrix]]), so named
  modules are blocked at the build-system layer. Revisit when the Zig
  toolchain ships scan-deps.

## Links

- Implements: [[T-0011-app-shell-abstraction]]
- Required by: [[ADR-0006-interop-parity]]
- Extends: [[Handlers]] (the handler taxonomy)
- Doesn't conflict with: [[ADR-0003-xaml-only-no-custom-dsl]] (XAML
  lowers to the same surface; no DSL is introduced)
- MAUI reference: `references/maui/src/Controls/src/Core/Application/Application.cs`,
  `references/maui/src/Controls/src/Core/Window.cs`,
  `references/maui/src/Controls/src/Core/Layout/StackLayout.cs`
