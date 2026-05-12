---
type: research
subject: "Slint Architecture"
framework: "slint"
created: 2026-05-12
applicableTo:
  - dsl-codegen
  - compile-time-bindings
  - source-mapped-diagnostics
recommendation: adopt
tags:
  - type/research
  - framework/slint
  - area/markup
  - area/type-system
---

# Slint Architecture

> [!info] Status
> Fleshed out during [[M-01-Foundations]] for comparative analysis. The **DSL-to-typed-code codegen** pattern and **compile-time binding diagnostics** are adopted; the **custom renderer** is rejected.

## Summary

Slint is a declarative UI toolkit (Rust core, C++ and JS bindings) whose defining trait is a `.slint` DSL that the build step compiles into strongly-typed Rust or C++ source. Bindings, animations, and layout are type-checked at compile time and surfaced through source-mapped diagnostics. MPAPP adopts the codegen pattern almost verbatim for its [[XAML-Compiler|mpapp-xc]] but rejects Slint's custom renderer in favour of native platform widgets per [[10_Architecture/Platform Interop]].

## What They Do

A Slint application is split into `.slint` files (declarative UI), one or more host-language source files (logic), and a build-time compiler. The `.slint` DSL is component-oriented: each file declares one or more `component` blocks with typed properties, callbacks, child elements, expressions, and `animate` clauses. The compiler — `slint-compiler` — parses these files, type-checks every expression and binding, resolves property references across components, evaluates constant subtrees, and emits a generated source file in the host language. The generated file contains concrete structs, accessor methods, and a fully wired binding graph. Re-evaluation of bindings at runtime is driven by a dirty-flag propagator, not by a runtime path resolver — every dependency edge is known at compile time.

The runtime is a small portable core that owns an item-tree (a tree of typed scene nodes — `Rectangle`, `Text`, `TouchArea`, etc.) and a render backend. Slint ships its own renderers: a software rasteriser, a Skia-based GPU path, and a femtoVG path; on microcontrollers it runs against a framebuffer. Crucially, **Slint does not delegate to native widgets**. A `Button` in Slint is a styled rectangle plus a touch area drawn by Slint's renderer, regardless of host OS. Styles (`fluent`, `material`, `cupertino`, `cosmic`) emulate native look-and-feel rather than wrap native controls. The LSP server (`slint-lsp`) consumes the same compiler front-end and provides live diagnostics, completion, and a visual preview keyed on source positions.

## What Works / What Doesn't

### Strengths

- **Compile-time binding type-checking.** A misspelled property name or a type mismatch in a binding expression is a build error with a precise file/line/column, not a silent runtime no-op. This is exactly the diagnostic experience MPAPP targets for [[Binding-Path]].
- **Source maps from DSL to generated code.** Compiler errors point into the `.slint` file, not the generated C++/Rust — a model worth copying directly.
- **One DSL compiler reused everywhere.** The same `slint-compiler` library backs the build step, the LSP, the live preview, and the figma-import tool. Tooling cost is paid once.
- **Statically-resolved binding graph.** No runtime path parser, no string-keyed lookup, predictable performance.
- **Strong story for resource-constrained targets** (MCUs, no-std). Demonstrates that a typed codegen can ship without a sprawling runtime.
- **Royalty-free GPL + commercial dual licence** with a clear escape hatch — useful prior art for [[RFC-0001-licensing-and-patent-strategy]].

### Weaknesses

- **No native widgets.** Every control is custom-drawn; accessibility, IME, screen-reader integration, and platform conventions (right-click menus, drag thresholds, scrollbar behaviour) must be reimplemented per style and are routinely incomplete. This is the deal-breaker for MPAPP.
- **Visual non-conformance.** "Looks like" native is not native; users notice. Platform updates (Windows 11 Mica, macOS Tahoe, Android Material You) require Slint releases to catch up.
- **Custom DSL fragmentation.** `.slint` is yet another language — IDE support beyond the official LSP is thin; XAML/QML/JSX skills don't transfer cleanly.
- **Rust-first ergonomics.** The C++ binding is good but secondary; build-system integration, error messages, and examples favour Cargo.
- **Style system is closed.** Adding a new native-style theme means forking the renderer, not plugging in a new handler.
- **No accessibility tree to native AT.** A11y is improving but still flows through Slint's own model rather than UIA / AT-SPI / NSAccessibility directly.

## Applicable to MPAPP

| Item | Verdict | Where it lives in MPAPP |
|---|---|---|
| **DSL-to-typed-code codegen** | **adopt** | [[XAML-Compiler\|mpapp-xc]] does this for XAML; see [[10_Architecture/Markup]] and [[ADR-0003-xaml-only-no-custom-dsl]]. |
| **Compile-time binding type-checking** | **adopt** | Implemented via [[Binding-Path]] and [[consteval-Tree]]; see [[10_Architecture/Observable Properties]] §static-paths. |
| **Source-mapped diagnostics** (DSL → generated code → C++ error) | **adopt** | mpapp-xc emits `#line` directives back to the `.xaml` source. Tracked in [[10_Architecture/XAML Compatibility]]. |
| **Shared compiler core across build + LSP + preview** | **adopt** | One `mpapp-xc` library powers the CLI compiler, the LSP, and the [[Hot-Reload]] daemon; see [[ADR-0007-cross-platform-tooling]]. |
| **Dirty-flag propagation graph** | **adapt** | [[Observable-Property]] uses a similar invalidation model, but rooted in C++ template wrappers rather than DSL-emitted structs. |
| **Custom renderer** | **reject** | Direct conflict with [[10_Architecture/Platform Interop]] and [[ADR-0006-interop-parity]] — MPAPP wires to platform [[Handler]]s and [[Native-View]]s. |
| **Custom theme system replacing native look** | **reject** | MPAPP defers to native styling; [[Platform-Specific Views]] is the escape hatch when a platform diverges. |
| **DSL syntax** (`.slint`) | **reject** | XAML is mandated by [[ADR-0003-xaml-only-no-custom-dsl]] and [[ADR-0004-maui-xaml-superset-compat]]. |
| **Item-tree runtime** owning render nodes | **reject** | MPAPP owns a logical [[Virtual-View]] tree only; rendering belongs to the OS. |

> [!important] Recommendation
> `adopt` — adopt the **DSL-to-typed-code codegen pattern**, the **compile-time binding diagnostics with source maps**, and the **single-compiler-core powering build + LSP + preview**. **Reject** the custom renderer, the custom theme system, and the `.slint` DSL itself. MPAPP's [[XAML-Compiler]] applies Slint's codegen lessons to XAML input and emits calls into platform [[Handler]] code, preserving native look-and-feel and accessibility per [[ADR-0006-interop-parity]].

## References

- Slint homepage: https://slint.dev/
- Slint architecture overview: https://docs.slint.dev/latest/docs/slint/src/advanced/architecture
- The `.slint` language reference: https://docs.slint.dev/latest/docs/slint/src/language/slint-files
- Slint compiler source: https://github.com/slint-ui/slint/tree/master/internal/compiler
- "Why we built Slint" (SixtyFPS blog): https://slint.dev/blog/announcing-slint-1.0.html
- Slint vs Qt comparison (official): https://slint.dev/blog/porting-from-qt.html
- LSP implementation: https://github.com/slint-ui/slint/tree/master/tools/lsp
