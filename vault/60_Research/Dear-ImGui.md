---
type: research
subject: "Dear ImGui"
framework: "imgui"
created: 2026-05-12
applicableTo: []
recommendation: reject
tags:
  - type/research
  - framework/imgui
---

# Dear ImGui

> [!info] Status
> Rejected as a user-facing pattern. Selected internal philosophy borrowed.

## Summary

Dear ImGui is an immediate-mode GUI library written in C++. The entire UI is rebuilt every frame from a procedural call sequence — there is no retained widget tree, no scene graph, and no persistent state owned by the library. It is studied here because MPAPP needs to understand why immediate-mode is fundamentally incompatible with the [[ADR-0004-maui-xaml-superset-compat|MAUI XAML]] worldview, and because its internal performance discipline informs MPAPP's own hot-path budget.

## What They Do

ImGui exposes a flat, function-call API (`ImGui::Begin`, `ImGui::Button`, `ImGui::SliderFloat`). Each frame, host code re-issues every widget call; the library hashes call sites to associate input state across frames. Rendering is delegated to a backend (OpenGL, Direct3D, Vulkan, Metal) — ImGui produces a vertex buffer, the host draws it. There is no [[Handler]] layer, no [[Native-View|native view]] mapping, no [[Observable-Property|observable property]] system. The whole library is a few `.cpp` files with zero dependencies on the standard library beyond the C runtime.

## Strengths

- **Tiny core.** No STL, no exceptions, no RTTI, and zero heap allocations in the steady-state render path.
- **Trivial integration.** Drop the sources into a project; pick a rendering backend; ship.
- **Excellent for tools.** Game-engine editors, debug overlays, profilers, and shader playgrounds use it because per-frame redraws map naturally to game loops.
- **Predictable performance.** No layout-invalidation graphs, no diff trees — just a single pass per frame.
- **Self-contained styling.** A small theme struct controls colors and spacing; no CSS, no XAML.

## Weaknesses

- **Immediate mode is the antithesis of retained-mode MVVM.** MPAPP is built on [[Observable-Property|observable properties]], [[Binding-Path|binding paths]], and a retained widget tree that survives across frames. There is no place in ImGui to anchor `Observable<T>` or `Computed<...>` — values are passed by reference each call and forgotten.
- **No native look-and-feel.** ImGui draws everything itself with a single texture atlas. It never produces a real `UIButton`, `android.widget.Button`, `NSButton`, or Win32 `BUTTON` control. This violates [[ADR-0006-interop-parity]] which mandates equivalent native behavior on every platform.
- **Accessibility is effectively absent.** Screen readers (UIA, AT-SPI, NSAccessibility, TalkBack) cannot introspect a draw-list. ImGui has experimental keyboard-nav and gamepad support, but no a11y tree.
- **No declarative markup.** XAML, the [[XAML-Compiler|XAML compiler]], and [[Markup]] hot-reload have no analog. Designers cannot author UI; only programmers writing C++ can.
- **Frame-coupled redraws waste energy.** Mobile and laptop battery life suffers when every frame redraws. A retained tree updates only when invalidated.
- **Layout is primitive.** Tables and columns exist, but flex/grid systems comparable to MAUI's `Grid`, `FlexLayout`, or `CollectionView` are missing or hand-rolled.
- **Theming cannot match platform conventions.** Dark/light mode, dynamic type, high-contrast settings — none flow through automatically.

## Applicable to MPAPP

- **Reject ImGui as a user-facing pattern.** MPAPP's surface is a retained widget tree authored in [[XAML Compatibility|XAML]] and driven by [[Observable-Property|observable properties]]. Immediate mode is structurally incompatible with both.
- **Reject any "ImGui-like" mode in the public API.** No `mpapp::Im::Button(...)` shortcut. Per [[ADR-0009-public-api-template-wrappers-only]], the public surface is the template-wrapper retained model only.
- **Adopt ImGui's tiny-core mindset internally.** MPAPP's hot paths (property-change notification, dispatcher tick, binding evaluation) should aim for zero heap allocations once warm. Internal implementation files may avoid STL where measurement justifies it; the public API still uses standard types per [[ADR-0001-cpp-standard-baseline]].

> [!important] Recommendation
> `reject` as a user-facing pattern. Borrow the no-allocation, tiny-core philosophy for internal hot paths only.

## References

- Official: https://github.com/ocornut/imgui
- [[ADR-0006-interop-parity]]
- [[ADR-0009-public-api-template-wrappers-only]]
- [[Observable Properties]]
- [[Handlers]]
