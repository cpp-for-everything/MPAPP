---
type: adr
id: ADR-0006
title: Interop parity — every public feature works on every platform
status: accepted
decisionDate: 2026-05-12
deciders:
  - Alex Tsvetanov
supersedes: ""
supersededBy: ""
area: handlers
tags:
  - type/adr
  - status/accepted
  - area/handlers
---

# ADR-0006 — Interop parity: every public feature works on every platform

> [!success] Status
> **accepted** on 2026-05-12.

## Context

Cross-platform frameworks routinely ship features that "work on most platforms" — leaving a trail of `#if PLATFORM` branches and surprised developers. MAUI itself has documented divergences (font support, file system semantics, return-key handling).

MPAPP's design goal is stricter: the public API is the *intersection* of behavior across all five supported platforms. If a feature can't be implemented faithfully on every backend, it's not a public feature — it's a platform extension, exposed under a clearly platform-scoped namespace.

## Decision

Every member of MPAPP's public API surface — every property, method, event, layout primitive, animation operator, command, and observable signal — **must behave equivalently on all five supported platforms** (Windows, Android, Linux, macOS, iOS) at the time of GA.

Behavioral parity means: same observable inputs → same observable outputs, within documented numerical / pixel tolerances. Visual styling (colors, fonts, native chrome) may differ — that's expected for a native-look framework.

Platform-only features:

- Live under `mpapp::platform::<name>::` namespaces.
- Have explicit `requires` constraints or `#if` guards.
- Are documented in their component note's "Platform Notes" section as a divergence.

This decision is mirrored as **CLAUDE rule 2** in [[CLAUDE]].

## Consequences

### Positive

- A user can write to the public API with confidence that platform-specific bugs are framework bugs, not feature gaps.
- Clean migration story: code that uses only the public API runs on every platform with no porting.
- The framework can be audited for parity drift by running the same conformance suite on every platform.

### Negative

- Slowest-supported-feature wins: if Linux/GTK4 lacks a Windows-style "AppearanceCustomization" API, we either implement it on GTK or drop it from public.
- More CI work — every PR must run cross-platform tests, not just home-platform tests.

### Neutral

- Internal handlers are free to diverge in implementation. Parity is about *observable behavior*, not implementation strategy.

## Alternatives Considered

- **Best-effort parity** (MAUI style). Rejected — leads to fragmented apps.
- **Union of features** (any platform-specific API ships). Rejected — fails the "cross-platform code that just works" promise.

## References

- [[10_Architecture/Interop Parity]]
- [[10_Architecture/Platform-Specific Views]]
- [[CLAUDE]]
