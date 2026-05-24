---
type: glossary
term: "Cross-Compilation"
tags:
  - type/glossary
---

# Cross-Compilation

Producing a binary for one target platform from a different host platform. Required from day 1 (ADR-0007). See [[Build System]].

## See in code

- [`cmake/toolchains/`](../../cmake/toolchains/) — one CMake toolchain file per target triple (`android-arm64.cmake`, `linux-arm64.cmake`, `macos-arm64.cmake`, `windows-x64.cmake`, `ios-arm64.cmake`, plus `zig.cmake` for the cross-compiler frontend).
- [`tools/mpapp/`](../../tools/mpapp/) — the `mpapp` CLI is itself cross-platform (Rule 12); the same binary on Win/macOS/Linux drives the cross-target build invocations.
- [`vault/50_Tasks/T-0009-cross-compilation-matrix/`](../50_Tasks/T-0009-cross-compilation-matrix/) — empirical matrix validation: Windows host compiles for four of six target triples via Zig 0.13 alone (still `in-progress`, gated on Apple-host availability for the macOS + iOS rows).
