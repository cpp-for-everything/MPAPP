---
type: moc
tags:
  - type/moc
---

# Current Focus

> [!important] Status — 2026-W19
> **Phase P0 active. Milestone [[M-01-Foundations]] in progress.**
>
> Just-completed: vault created, 11 ADRs accepted (ADR-0010 licensing + ADR-0011 Zig cross-compilation now locked in), both opening RFCs closed, 56 components stubbed, 10 tasks created.
>
> Next: stand up the Zig toolchain files under `cmake/toolchains/`, kick off the mock surface, start the async executor work, plus the existing P0 spikes (T-0002 template-type, T-0003 WinUI button, T-0004 JNI codegen) and component-doc population (T-0005).

## This week (2026-W19)

Updates here weekly. Don't add dates further out than this week.

- [ ] Flesh out [[Components/Button]] as the reference per-component doc; use it as the template for the other 55.
- [ ] Start [[T-0002-template-type-spike]] — `Observable<T>` / `Computed<...>` / `Command<>` prototype.
- [ ] Land the Zig toolchain files (`cmake/toolchains/{windows-x64,linux-x64,linux-arm64,android-arm64,macos-arm64,ios-arm64}.cmake`) per [[ADR-0011-cross-compilation-toolchain]].
- [ ] Kick off the mock surface per [[ADR-0008-mock-first-implementation]] — first mock control + handler skeleton.
- [ ] Start the async executor spike (threading + event-loop integration substrate).
- [ ] Begin populating [[Qt-Property-System]] research note.

## Active milestone

[[M-01-Foundations]] — exit criteria, risks, and linked tasks live there.

## Recently accepted

- [[ADR-0011-cross-compilation-toolchain]] (2026-05-12) — Zig (`zig cc`) is the primary cross-compilation toolchain; closes [[RFC-0002-cross-compilation-toolchain]].
- [[ADR-0010-licensing-and-patent-strategy]] (2026-05-12) — Apache 2.0 + commercial dual license, Apache-style CLA, LGPL-dynamic-only deps, deferred patent filing; closes [[RFC-0001-licensing-and-patent-strategy]].
- [[ADR-0009-public-api-template-wrappers-only]] — template wrapper types only; option B (attributes) rejected.
- [[ADR-0008-mock-first-implementation]] — mock-first strategy.
- (See [[Decision Log]] for the full day-1 burst.)

## Pinned reading

- [[CLAUDE]] — vault rules.
- [[Type System]] — the template-wrapper-type design.
- [[Build System]] — cross-compilation matrix.
- [[dotnet-maui-deep-dive]] — the spec MPAPP mirrors.
