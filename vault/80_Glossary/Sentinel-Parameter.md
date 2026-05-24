---
type: glossary
term: "Sentinel Parameter"
tags:
  - type/glossary
---

# Sentinel Parameter

A defaulted, unnamed parameter (e.g. `Computed<...> = {}`) used to attach type-level metadata to a member function without changing its callable signature. See [[Type System]].

## See in code

- [`include/mpapp/computed.hpp`](../../include/mpapp/computed.hpp) — `Computed<&VM::a, &VM::b>` is the canonical sentinel: a defaulted unnamed parameter that carries the dependency-list pack into the function signature so the binding system can deduce it at the call site.
- Trait helpers `is_computed_tag_v<T>` / `Computed<...>` keep the type-system bookkeeping out of the user-visible function body.
- The same pattern is reused for `Command<>` tags in the command surface (single `inline constexpr` sentinel instance).
