---
type: adr
id: ADR-0007
title: Every MPAPP tool runs on Windows, macOS, and Linux
status: accepted
decisionDate: 2026-05-12
deciders:
  - Alex Tsvetanov
supersedes: ""
supersededBy: ""
area: tooling
tags:
  - type/adr
  - status/accepted
  - area/tooling
  - area/build
---

# ADR-0007 — Every MPAPP tool runs on Windows, macOS, and Linux

> [!success] Status
> **accepted** on 2026-05-12.

## Context

A developer using MPAPP must not be forced onto a particular host OS to use the framework. Tools that only run on macOS, only on Windows, or only on Linux would fragment the developer experience and lock users into specific hardware.

This applies to **every tool MPAPP ships**: the `mpapp` CLI, the XAML compiler `mpapp-xc`, the Android JNI codegen `mpapp-jni-gen`, the hot-reload daemon, the LSP server, the build presets.

## Decision

Every tool MPAPP ships **must run on Windows, macOS, and Linux**. No host-OS lock-in.

Implementation guidelines:

- Tools are written in C++23 against the same MPAPP runtime (dogfood the framework where possible).
- External dependencies (libclang, LLD, libxml2) must have multi-platform builds.
- File-path handling is OS-aware (`std::filesystem`).
- No POSIX-only system calls without a Windows equivalent.
- CI validates *every* tool on Windows, macOS-when-available, and Linux runners.

**Cross-compilation** is also a goal but a separate decision — see [[RFC-0002-cross-compilation-toolchain]]. A developer on Windows must be able to produce binaries for all five targets (with the caveat that Apple platforms require macOS for signed builds — see [[10_Architecture/Build System]]).

This decision is mirrored as **CLAUDE rule 12** in [[CLAUDE]].

## Consequences

### Positive

- One developer experience regardless of host OS.
- CI matrix is simpler — same scripts on all hosts.
- Easier to find contributors — they don't need a specific machine.

### Negative

- Avoiding POSIX-only conveniences means a bit more abstraction work in tool internals.
- Windows path semantics (backslash, drive letters, case insensitivity) require care.

### Neutral

- This does not constrain *target* platforms — Linux GTK4 builds can still target Linux from a Mac. It only constrains where the tools themselves run.

## Alternatives Considered

- **Mac-only tooling** (MAUI does this for iOS builds). Rejected — locks out Windows-host developers.
- **Linux-only tooling.** Rejected — most enterprise C++ devs are on Windows or macOS.

## Implementation Notes

- [`tools/mpapp/`](../../tools/mpapp/) — the developer CLI, built per-host. Runs on Windows, Linux, macOS today.
- [`tools/mpapp-xc/`](../../tools/mpapp-xc/) — the XAML compiler, built per-host alongside `mpapp`. Same C++23 source tree everywhere; no host-OS-specific code paths in the tool itself.
- [`cmake/toolchains/`](../../cmake/toolchains/) — toolchain files that let any host produce binaries for any target (the cross-compilation half of the goal). Apple-target signing requires macOS — documented in `cmake/toolchains/README.md`.
- `mpapp-jni-gen` is not yet built; its eventual home is `tools/mpapp-jni-gen/` following the same per-host pattern.

## References

- [[10_Architecture/Build System]]
- [[RFC-0002-cross-compilation-toolchain]]
- [[CLAUDE]]
