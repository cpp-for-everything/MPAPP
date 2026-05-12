---
type: moc
tags:
  - type/moc
---

# Decisions MOC

Index of all ADRs by area. The authoritative live view is [[_Bases/ADRs.base]].

## Accepted (9)

### Type system

- [[ADR-0001-cpp-standard-baseline]] — C++23 baseline + C++26 reflection opt-in
- [[ADR-0002-no-macros-in-public-api]] — Public API: no macros
- [[ADR-0009-public-api-template-wrappers-only]] — Template wrapper types only

### Markup

- [[ADR-0003-xaml-only-no-custom-dsl]] — XAML as the only markup
- [[ADR-0004-maui-xaml-superset-compat]] — Full MAUI XAML compat + supersets

### Platform interop & handlers

- [[ADR-0005-ios-macos-separate-interop]] — UIKit + AppKit (no Catalyst)
- [[ADR-0006-interop-parity]] — Every feature on every platform

### Tooling & process

- [[ADR-0007-cross-platform-tooling]] — All tools run on Windows + macOS + Linux
- [[ADR-0008-mock-first-implementation]] — Full API surface as mocks first

## Proposed (2)

### Tooling & process

- [[ADR-0010-licensing-and-patent-strategy]] — Apache 2.0 + commercial dual license, Apache-style CLA, LGPL-dynamic-only deps, deferred patent filing (closes [[RFC-0001-licensing-and-patent-strategy]])

### Build & tooling

- [[ADR-0011-cross-compilation-toolchain]] — Zig (`zig cc`) as cross-compilation toolchain (closes [[RFC-0002-cross-compilation-toolchain]])

## Open RFCs (2)

- [[RFC-0001-licensing-and-patent-strategy]] (draft) — Dual license + CLA + patent strategy
- [[RFC-0002-cross-compilation-toolchain]] (draft) — Zig vs LLVM+sysroots

## See also

- [[Decision Log]] — chronological view
- [[_Bases/ADRs.base]] — live filtered view
- [[_Bases/RFCs.base]] — open RFCs
