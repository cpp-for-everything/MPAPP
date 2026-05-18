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

## 2026-W19 — ADRs accepted

Both proposed ADRs from earlier in the week were promoted to `accepted` on 2026-05-12, and their source RFCs flipped from `draft` to `accepted`:

- [[ADR-0010-licensing-and-patent-strategy]] (accepted) — Apache 2.0 + commercial dual license, Apache-style CLA enforced via cla-assistant.io, copyright held by Alex Tsvetanov with reserved LLC transfer, LGPL-only-via-dynamic-linking dependency posture, patent filing deferred until P3 with mandatory prior-art audit. Closes [[RFC-0001-licensing-and-patent-strategy]].
- [[ADR-0011-cross-compilation-toolchain]] (accepted) — Zig (`zig cc`) is the official cross-compilation toolchain; pinned via `cmake/toolchains/zig.cmake`, auto-installed by the `mpapp` CLI to `~/.mpapp/toolchains/zig-<version>/`. Closes [[RFC-0002-cross-compilation-toolchain]].

## 2026-W20 — app-shell abstraction proposed → accepted same day

- [[ADR-0012-application-window-handler-abstraction]] (proposed → **accepted** 2026-05-18) — Application / Window / Page / StackLayout / Grid extend the existing partial-specialisation widget-handler pattern; `mpapp::application` is single-instance per process on every platform; entry point is `mpapp::run<App>(argc, argv)` (no public-API macro); framework-owned `orientation` / `thickness` / `h_align` / `v_align` types. Filed in response to a user observation that `examples/windows_button_spike/main.cpp` carried ~25 raw `winrt::`/`mux::`/`muxc::`/`Mdd*` tokens in user-facing code, violating [[ADR-0006-interop-parity]] above the widget layer. Spike rewrite under [[T-0011-app-shell-abstraction]] proves the abstraction: mock surface + WinUI 3 real handlers (Application / Window / StackLayout) ship with the rewritten spike showing **zero WinRT tokens in user-facing code**. 126/126 tests pass.
