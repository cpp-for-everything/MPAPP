---
type: moc
tags:
  - type/moc
---

# Decisions MOC

Index of all ADRs by area. The authoritative live view is [[_Bases/ADRs.base]].

## Accepted (24)

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
- [[ADR-0012-application-window-handler-abstraction]] — Application / Window / Page / Layout handlers extend the widget-handler pattern
- [[ADR-0013-data-driven-widget-dispatch]] — Data-driven widget dispatch via per-platform registries
- [[ADR-0014-page-navigation-stack]] — Page navigation stack semantics
- [[ADR-0015-graphics-backend-dual]] — Dual 2D graphics backend (Cairo + Skia, compile-time selectable)
- [[ADR-0016-shell-compile-time-routes]] — Shell URI routing: compile-time route table
- [[ADR-0017-grid-track-definitions]] — Grid track definitions: value-type with string parser
- [[ADR-0018-hybrid-webview-typed-bridge]] — HybridWebView JS bridge: typed async method calls
- [[ADR-0020-virtualized-item-host-wrap-platform]] — Virtualised item host: wrap platform recyclers
- [[ADR-0021-tableview-cell-types]] — TableView cell type tree (full MAUI parity)
- [[ADR-0022-android-kind-discriminated-routers]] — Android event routing: kind-discriminated listener family
- [[ADR-0023-shell-route-guards-and-lifecycle]] — Shell route guards + page lifecycle hooks
- [[ADR-0024-wrapper-component-pattern]] — Wrapper-component pattern: auto-binding wrapper around a platform-agnostic surface

### Threading

- [[ADR-0019-async-executor-native-dispatcher]] — Async executor: native UI dispatcher + `task<T>` adapter

### Tooling & process

- [[ADR-0007-cross-platform-tooling]] — All tools run on Windows + macOS + Linux
- [[ADR-0008-mock-first-implementation]] — Full API surface as mocks first
- [[ADR-0010-licensing-and-patent-strategy]] — Apache 2.0 + commercial dual license, Apache-style CLA, LGPL-dynamic-only deps, deferred patent filing (closes [[RFC-0001-licensing-and-patent-strategy]])

### Build

- [[ADR-0011-cross-compilation-toolchain]] — Zig (`zig cc`) as cross-compilation toolchain (closes [[RFC-0002-cross-compilation-toolchain]])

## See also

- [[Decision Log]] — chronological view
- [[_Bases/ADRs.base]] — live filtered view
- [[_Bases/RFCs.base]] — open RFCs
