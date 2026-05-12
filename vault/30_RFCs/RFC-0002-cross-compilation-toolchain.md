---
type: rfc
id: RFC-0002
title: Cross-compilation toolchain — Zig vs LLVM/Clang + per-platform sysroots
status: draft
author: Alex Tsvetanov
created: 2026-05-12
area: build
relatedADRs:
  - ADR-0007
tags:
  - type/rfc
  - status/draft
  - area/build
---

# RFC-0002 — Cross-compilation toolchain

> [!info] Status
> **draft** — under discussion.

## Problem

[[ADR-0007-cross-platform-tooling]] and the user's directive both require that a developer on any supported host OS be able to produce binaries for every supported target. Specifically:

- A **Windows** developer must build Windows + Linux + Android binaries.
- A **macOS** developer must additionally build macOS + iOS (signed) and the Apple unsigned-cross targets.
- A **Linux** developer must build Windows + Linux + Android binaries.

This is a multi-target cross-compilation problem with five host platforms and five target platforms, with the major caveat that **producing signed Apple binaries requires macOS** (Apple SDK + signing tools).

## Proposal

Two candidate approaches, with a recommendation.

### Option A — Zig as the cross-compilation backbone (recommended)

[Zig](https://ziglang.org/)'s `zig cc` is Clang + LLD + bundled libc headers and sysroots for many targets. Single command:

```bash
zig cc --target=x86_64-windows-gnu       hello.cpp -o hello.exe
zig cc --target=aarch64-linux-gnu        hello.cpp -o hello
zig cc --target=aarch64-linux-android    hello.cpp -o libapp.so
```

**Pros:**
- One install, one CLI, one config. No managing per-platform NDK installs or sysroot tarballs.
- Same compiler (Clang) across every target — diagnostics are uniform.
- Active maintenance by the Zig project; libc bundling is solid.
- Already integrates with CMake via `CMAKE_C_COMPILER=zig cc` toolchain files.

**Cons:**
- Adds Zig as a build dependency (~50 MB install).
- Zig's release cadence is independent of LLVM; occasional lag on newest C++23/26 features.
- Apple targets (`*-macos`, `*-ios`) work for unsigned compilation only — signing still requires `codesign` on macOS.

### Option B — LLVM/Clang + per-platform sysroots (conventional)

Direct Clang + LLD plus manually-managed sysroots:

- Windows host: MSVC's `clang-cl` + MSVC sysroot for native Windows; MinGW-w64 for GNU Windows; Android NDK for Android; manual osxcross for unsigned Apple.
- Linux host: distro Clang + glibc; Android NDK; MinGW-w64; osxcross.
- macOS host: Xcode Clang for native + iOS; cross-compilation toolchain for other targets.

**Pros:**
- More conventional in the C++ ecosystem.
- Direct control over LLVM/Clang version per target.

**Cons:**
- Significant per-platform setup burden — multiple toolchains, multiple sysroots, multiple installers.
- Diagnostics may differ slightly between toolchain versions.
- More CI matrix complexity.

### Recommendation

**Option A (Zig).** The reduced setup cost makes this strictly preferable for a small team. If a Zig limitation bites us mid-project, falling back to Option B for one or two targets is straightforward — Zig is additive, not exclusive.

## Detailed Design

### CMake integration

```cmake
# cmake/toolchains/android-arm64.cmake
set(CMAKE_C_COMPILER zig cc -target aarch64-linux-android)
set(CMAKE_CXX_COMPILER zig c++ -target aarch64-linux-android)
set(CMAKE_SYSTEM_NAME Android)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
```

The `mpapp` CLI dispatches to the right toolchain file based on `--target`:

```
mpapp build --target windows-x64        → uses windows-x64.cmake toolchain
mpapp build --target linux-arm64        → uses linux-arm64.cmake toolchain
mpapp build --target android-arm64      → uses android-arm64.cmake toolchain
mpapp build --target macos-arm64        → requires macOS host (signed) or osxcross (unsigned)
mpapp build --target ios-arm64          → requires macOS host (signed) or osxcross (unsigned)
mpapp build --all                       → all targets the host supports
```

### Validation matrix

Every PR runs the cross-build matrix from the host that runs the CI job:

- `windows-latest` job: builds windows-x64, linux-x64, android-arm64.
- `ubuntu-latest` job: builds linux-x64 native, windows-x64 cross, android-arm64.
- `macos-latest` job (tagged releases only initially): builds all five targets including signed macos-arm64 and ios-arm64.
- Self-hosted macOS runner (when MacBook online): assumes the role of `macos-latest` for every PR.

## Alternatives

- **Per-platform native toolchains, no cross.** Rejected — fails user directive.
- **Bazel as build system.** Considered. Strong cross-compilation story but a much bigger learning curve for users coming from CMake. The MAUI ecosystem is CMake-friendly.

## Open Questions

> [!todo] Open
> - [ ] Validate Zig's C++23 feature coverage against what MPAPP uses (deducing `this`, `std::expected`, coroutines)
> - [ ] Benchmark Zig vs MSVC build times on Windows-native target
> - [ ] Decide vendoring strategy: bundle Zig in the repo? Pin via `mpapp` CLI's auto-install?
> - [ ] Confirm Android NDK API levels match Zig's Android target

## Migration / Compatibility

N/A — initial decision.

## References

- [Zig as a C compiler](https://ziglang.org/learn/overview/#zig-is-also-a-c-compiler)
- [osxcross](https://github.com/tpoechtrager/osxcross)
- [Android NDK](https://developer.android.com/ndk)
- [[10_Architecture/Build System]]
- [[ADR-0007-cross-platform-tooling]]
