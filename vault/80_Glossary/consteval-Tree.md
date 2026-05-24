---
type: glossary
term: "consteval Tree"
tags:
  - type/glossary
---

# consteval Tree

The compile-time-evaluated visual tree that `mpapp-xc` emits for a XAML file. Calls the same public API a hand-written C++ UI would. See [[Markup]].

## See in code

- [`tools/mpapp-xc/`](../../tools/mpapp-xc/) — the compiler whose output IS the consteval tree (one emitted `.hpp` per `.xaml` input).
- [`include/mpapp/`](../../include/mpapp/) — the public C++ surface the emitted tree targets; the emitter uses no codegen-private headers, so a developer reading the output sees the same `mpapp::button` / `mpapp::stack_layout` types they'd write by hand.
- See [[Markup]] for the design rationale (XAML stays the source of truth, no DSL invention; per [[ADR-0003-xaml-only-no-custom-dsl]]).
