---
type: adr
id: ADR-0008
title: Mock-first implementation — full API surface as mocks before any platform code
status: accepted
decisionDate: 2026-05-12
deciders:
  - Alex Tsvetanov
supersedes: ""
supersededBy: ""
area: process
tags:
  - type/adr
  - status/accepted
  - area/process
---

# ADR-0008 — Mock-first implementation: full API surface as mocks before any platform code

> [!success] Status
> **accepted** on 2026-05-12.

## Context

A traditional UI framework grows platform by platform: ship a few controls on Windows, then port to Android, then macOS, and so on. The first platform's design becomes the de-facto specification and later platforms struggle to fit.

The user has directed a different strategy: stand up the **entire public API surface as mock C++ classes first** (every MAUI handler, every control, full method signatures, dummy implementations), validate the type system end-to-end with mock-based unit tests, **then** add platform implementations one platform at a time.

This is closer to a contract-first or interface-first design. It ensures the C++ API surface is locked in before any platform-specific concession can shape it.

## Decision

After the P1 infrastructure milestone:

1. **P2 — Full Mock Surface.** Every component in [[Controls Inventory]] is implemented as a C++ class with:
   - The full public API (properties, methods, events, attached properties).
   - `Observable<T>` properties wired up.
   - Dummy platform handlers that log/no-op.
   - Mock-based unit tests covering change notifications, bindings, command invocation, and layout-measurement contracts.
   - **100% line + branch coverage** of the public API (CLAUDE rule 11).
2. **P3 onwards — Real platforms.** Windows first, then Android, Linux, macOS, iOS. Each platform converts dummy handlers to real native interop, one control at a time. The public API does not change; only handlers do.

A control's porting status (`not-started → mock → <platform>-real → parity-complete`) is tracked in [[Controls Inventory]] and on each component's `mpappStatus` frontmatter.

This decision is mirrored as **CLAUDE rule 6** in [[CLAUDE]].

## Consequences

### Positive

- The C++ API is locked in *before* any platform pressures it. No "we can't do that on iOS, so the API has to change" surprises.
- Mock tests can run on any host with no native dependencies — very cheap CI.
- Onboarding a new platform is a series of well-scoped "convert this mock to real" PRs.
- We can prove API completeness early — every MAUI XAML construct lowers to *something* before any platform exists.

### Negative

- P2 takes longer than a traditional "ship one platform fast" approach.
- Mocks need maintenance until real handlers replace them.
- Risk of "mocks always pass" complacency — mitigated by P3's UI smoke tests that must pass against real platforms.

### Neutral

- The infrastructure (CMake, CI, test harness) is built in P1 *before* mocks, so mocks have somewhere to live.

## Alternatives Considered

- **Platform-by-platform incremental.** Rejected — risks first-platform bias.
- **Spec-only design without code.** Rejected — written specs drift from implementations; types-as-spec doesn't.

## References

- [[10_Architecture/Test Harness]]
- [[10_Architecture/Controls Inventory]]
- [[40_Roadmap/M-03-Mock-Surface]]
- [[CLAUDE]]
