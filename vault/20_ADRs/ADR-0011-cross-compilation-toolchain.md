---
type: adr
id: ADR-0011
title: Cross-compilation toolchain is Zig (zig cc)
status: proposed
decisionDate: 2026-05-12
deciders:
  - Alex Tsvetanov
supersedes: ""
supersededBy: ""
area: build
tags:
  - type/adr
  - status/proposed
  - area/build
---

# ADR-0011 — Cross-compilation toolchain is Zig (zig cc)

> [!info] Status
> **proposed** on 2026-05-12 — awaiting acceptance.

## Context

[[ADR-0007-cross-platform-tooling]] requires every developer host to produce binaries for every target. Concretely: a Windows developer must build Windows + Linux + Android; a Linux developer must build the same set; a macOS developer must additionally build signed Apple targets. The user directive is uncompromising — no host OS should lock a contributor out of producing artifacts for any platform the framework supports.

[[RFC-0002-cross-compilation-toolchain]] evaluated two approaches in detail. **Option A** was Zig (`zig cc`), a single drop-in cross-compiler bundling Clang, LLD, and libc headers/sysroots for the major targets. **Option B** was direct LLVM/Clang plus per-target sysroots — MSVC's sysroot for native Windows, the Android NDK for Android, MinGW-w64 for GNU Windows from Linux, and osxcross for unsigned Apple cross-builds. Option B is the conventional approach in the C++ ecosystem; Option A trades a small dependency (Zig itself, around 50 MB) for the elimination of nearly every per-target setup step.

The RFC recommended Option A. This ADR formalises that recommendation as the project's official toolchain choice so downstream work — toolchain files, the `mpapp` CLI's auto-install logic, CI cache keys, the third-party-dependency table — can converge on a single source of truth.

## Decision

We will use **Zig (`zig cc`) as the primary cross-compilation toolchain.** Zig wraps Clang + LLD with bundled libc headers and sysroots for many targets; one install gives us Windows / Linux / Android targets from any host. Apple targets cross-compile to unsigned binaries via osxcross when needed; signed Apple builds still require macOS (see [[ADR-0005-ios-macos-separate-interop]] and the signing caveat in [[Build System]]).

**Vendoring strategy.** Zig is **not bundled** in the repo. The `mpapp` CLI (see [[Build System]]) auto-installs Zig on first use via official binary downloads to `~/.mpapp/toolchains/zig-<version>/`. The version is pinned in `cmake/toolchains/zig.cmake`. This keeps the repo lean, keeps Zig's upgrade story explicit (bumping the pin is a reviewable change), and avoids licensing entanglement with the Zig redistribution policy.

**Toolchain file layout.** `cmake/toolchains/{windows-x64,linux-x64,linux-arm64,android-arm64,macos-arm64,ios-arm64}.cmake` each set `CMAKE_C_COMPILER`/`CMAKE_CXX_COMPILER` to the Zig wrapper with the right `--target` triple (e.g., `aarch64-linux-android` for Android, `x86_64-windows-gnu` for cross-GNU Windows). Each toolchain file also sets `CMAKE_SYSTEM_NAME`, `CMAKE_SYSTEM_PROCESSOR`, and the appropriate sysroot/feature flags so CMake's standard cross-compilation conventions remain intact.

## Consequences

### Positive

- One toolchain to install; uniform Clang diagnostics across every target, which simplifies error-message documentation and contributor onboarding.
- The Android NDK is no longer a separate hard dependency for cross-compilation — Zig bundles the equivalent libc/headers for Android targets.
- Fewer per-platform sysroot tarballs to maintain, mirror, or re-host; CI caches shrink accordingly.
- Existing `ccache` workflows continue to apply since Zig still invokes Clang under the hood.

### Negative

- Adds Zig (≈50 MB) as a hard build dependency for any contributor doing cross-compilation. Host-native-only contributors are unaffected.
- Zig's release cadence is independent of LLVM — there will be occasional lag on bleeding-edge C++23/26 features. [[ADR-0001-cpp-standard-baseline]] keeps the baseline at C++23 with C++26 reflection opt-in, so this is bounded.
- Apple targets still need macOS for signing. Zig does not change the Apple SDK / Gatekeeper constraint.

### Neutral

- A developer who only ever builds for the host platform can use their existing toolchain (MSVC on Windows, system Clang on Linux, Xcode Clang on macOS) and never touch Zig. The pin in `cmake/toolchains/zig.cmake` is only consulted on cross-target builds.

## Alternatives Considered

- **LLVM/Clang + manually-maintained sysroots** (RFC-0002 Option B) — rejected; the per-target sysroot maintenance burden, plus Android NDK version churn and osxcross update cadence, dwarfs the Zig install cost.
- **MSVC + WSL for cross-compilation** — partial solution; covers Windows-to-Linux but doesn't cover Android and adds a WSL prerequisite for every Windows contributor.
- **Bazel as cross-compilation system** — rejected; would replace CMake which is the substrate per [[Build System]], and migration cost is unjustified for a single decision area.

## References

- [[RFC-0002-cross-compilation-toolchain]] — closed by this ADR
- [[ADR-0007-cross-platform-tooling]]
- [[Build System]]
- [[CI Strategy]]
- [[70_References/Zig]]
- [Zig as a C compiler](https://ziglang.org/learn/overview/#zig-is-also-a-c-compiler)
