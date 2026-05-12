---
type: adr
id: ADR-0001
title: C++23 baseline with C++26 reflection opt-in
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

# ADR-0001 — C++23 baseline with C++26 reflection opt-in

> [!success] Status
> **accepted** on 2026-05-12.

## Context

MPAPP must achieve compile-time type safety beyond what .NET MAUI gets through runtime reflection. We need to choose a C++ standard that:

- Has wide compiler support today (MSVC, Clang 17+, GCC 13+, Apple Clang).
- Supports concepts, coroutines, modules, `std::expected`, and other modern facilities the framework will rely on.
- Provides a credible upgrade path to **C++26 static reflection** ([P2996](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2996r1.html)) once it ships in production toolchains.

Three options were considered:

1. **C++20** — broadest reach; supports coroutines and concepts; loses some C++23 niceties (`std::expected`, multidimensional subscript, deducing `this`).
2. **C++23** — the recommended baseline; available today on all major compilers; deducing `this` is particularly useful for the framework's CRTP handler architecture.
3. **C++26** — aggressive; couples the framework's release to P2996 acceptance and compiler availability; multi-year wait risk.

## Decision

We will use **C++23 as the baseline standard**, with **C++26 static reflection treated as an opt-in upgrade** that the framework can take advantage of *when* it's available without making it a hard prerequisite.

## Consequences

### Positive

- Compiles today on every major toolchain.
- Deducing `this` (`auto&& self`) makes the CRTP base templates ergonomic.
- `std::expected` cleans up error paths in the handler/interop layers.
- Forward-compatible: when P2996 ships, the framework can use it additively to simplify internal reflection layers without breaking the public API.

### Negative

- Loses access to a small set of C++26 facilities until that standard ships.
- Older toolchains (pre-MSVC 17.6, pre-Clang 17, pre-GCC 13) are unsupported.

### Neutral

- The public-API mechanism (template wrapper types — [[ADR-0009-public-api-template-wrappers-only]]) is independent of the C++ standard choice.

## Alternatives Considered

- **C++20 baseline** — rejected because deducing `this` and `std::expected` materially improve the framework's internal ergonomics.
- **C++26 baseline** — rejected because production-quality P2996 support is on an uncertain schedule outside our control.

## References

- [[60_Research/dotnet-maui-deep-dive]]
- [[70_References/C++26 Reflection P2996]]
- [[10_Architecture/Type System]]
