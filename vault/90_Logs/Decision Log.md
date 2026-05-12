---
type: log
tags:
  - type/log
---

# Decision Log

Chronological pointer list of all accepted ADRs. The authoritative live view is [[_Bases/ADRs.base]] — this file is a human-readable supplement organized by date.

## 2026-05-12 — Day 1 (vault creation)

Nine ADRs accepted in a single session as MPAPP was kicked off:

- [[ADR-0001-cpp-standard-baseline]] — C++23 baseline + C++26 reflection opt-in.
- [[ADR-0002-no-macros-in-public-api]] — Forbidden in user-facing surface.
- [[ADR-0003-xaml-only-no-custom-dsl]] — XAML as the only markup language.
- [[ADR-0004-maui-xaml-superset-compat]] — Full MAUI XAML compat + platform supersets.
- [[ADR-0005-ios-macos-separate-interop]] — UIKit + AppKit, no Catalyst.
- [[ADR-0006-interop-parity]] — Every public feature on every platform.
- [[ADR-0007-cross-platform-tooling]] — All MPAPP tools run on Windows + macOS + Linux.
- [[ADR-0008-mock-first-implementation]] — Full API surface as mocks before any platform code.
- [[ADR-0009-public-api-template-wrappers-only]] — Template wrapper types as the sole public-API mechanism.

Two RFCs opened the same day:

- [[RFC-0001-licensing-and-patent-strategy]] — dual license + CLA + patent prior-art audit (draft).
- [[RFC-0002-cross-compilation-toolchain]] — Zig vs LLVM + per-platform sysroots (draft).

## 2026-W19 — ADRs proposed (awaiting acceptance)

- [[ADR-0010-licensing-and-patent-strategy]] (proposed) — promotes [[RFC-0001-licensing-and-patent-strategy]]'s recommendations: Apache 2.0 + commercial dual license, Apache-style CLA via cla-assistant.io, individual copyright assignee, LGPL-dynamic-only dependency posture, deferred patent filing after a prior-art audit.
- [[ADR-0011-cross-compilation-toolchain]] (proposed) — Zig (`zig cc`) locked in as the cross-compilation toolchain, closing [[RFC-0002-cross-compilation-toolchain]].
