---
type: adr
id: ADR-0002
title: No macros in the public API
status: accepted
decisionDate: 2026-05-12
deciders:
  - Alex Tsvetanov
supersedes: ""
supersededBy: ""
area: type-system
tags:
  - type/adr
  - status/accepted
  - area/type-system
---

# ADR-0002 — No macros in the public API

> [!success] Status
> **accepted** on 2026-05-12.

## Context

.NET MAUI uses runtime reflection plus source-generator attributes (`[ObservableProperty]`, `[RelayCommand]`) to reduce MVVM boilerplate. A naive C++ translation would reach for preprocessor macros like `MPAPP_OBSERVABLE_PROPERTY(int, count)`. Macros are pervasive in C++ UI frameworks (Qt's `Q_OBJECT`, wxWidgets' event tables) but they hurt readability, IDE tooling, error messages, and refactorability.

MPAPP is being designed for a developer audience that values modern C++ idioms.

## Decision

We will **not** require users to write `MPAPP_*` macros anywhere in the public API surface. Observable properties, computed values, commands, bindable controls, and every other framework-provided concept must be expressed in plain C++23 syntax using template types, function signatures, or standard language facilities.

The specific replacement mechanism is **template wrapper types** ([[ADR-0009-public-api-template-wrappers-only]]).

**Exemption:** internal preprocessor guards (`#if MPAPP_ANDROID`, `#ifdef MPAPP_DEBUG`, build-system flags) are permitted in framework *internal* code and in user code where the user explicitly opts into platform-specific code blocks. They are not part of the public API surface.

This decision is mirrored as **CLAUDE rule 1** in [[CLAUDE]].

## Consequences

### Positive

- Better IDE experience: IntelliSense and refactoring tools work over types and members, not over macro expansion.
- Cleaner error messages: compiler errors point at C++ code, not preprocessor output.
- Forward-compatible with C++26 static reflection ([[ADR-0001-cpp-standard-baseline]]).
- Less learning curve for developers coming from Qt — they get to leave `Q_OBJECT` and `moc` behind.

### Negative

- The framework has to work harder to provide the same ergonomics that source generators give MAUI. Mitigated by [[ADR-0009-public-api-template-wrappers-only]].
- Some patterns that are trivial with macros (cross-cutting registration, compile-time-string member lookup) require more thought.

### Neutral

- The XAML compiler `mpapp-xc` is an external code generator, not a macro system — it produces normal C++ that the user can read.

## Alternatives Considered

- **Macros (MAUI-style `[ObservableProperty]` analog).** Rejected — directly contrary to the design philosophy here.
- **C++ attributes (`[[mpapp::observable]]`).** Considered. Compatible with standard C++ (custom attribute namespaces are allowed since C++11). Would require a libclang-based meta-compiler to read them. **Rejected** in [[ADR-0009-public-api-template-wrappers-only]] in favor of pure template wrappers, which need no external tool.

## Implementation Notes

- [`include/mpapp/`](../../include/mpapp/) — every public header in this tree is macro-free by design. `grep -r '^#define MPAPP_' include/mpapp/` returns no public-API matches; the only `MPAPP_*` defines in headers are conditional-compilation gates (`#if defined(MPAPP_GRAPHICS_HAS_SKIA)` etc.).
- Internal preprocessor (allowed): `MPAPP_GRAPHICS_BACKEND` / `MPAPP_GRAPHICS_HAS_*` set by [`CMakeLists.txt`](../../CMakeLists.txt) and consumed inside framework `.cpp` files; `MPAPP_PLATFORM_*` per-platform guards in the per-platform `src/handlers/<plat>/` trees.
- The mechanism that replaces the macros: [`include/mpapp/observable.hpp`](../../include/mpapp/observable.hpp) / [`computed.hpp`](../../include/mpapp/computed.hpp) / [`command.hpp`](../../include/mpapp/command.hpp) — see [[ADR-0009-public-api-template-wrappers-only]] for the design.

## References

- [[10_Architecture/No Macros In Public API]]
- [[ADR-0009-public-api-template-wrappers-only]]
- [[CLAUDE]]
