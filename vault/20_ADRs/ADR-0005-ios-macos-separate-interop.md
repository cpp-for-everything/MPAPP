---
type: adr
id: ADR-0005
title: iOS and macOS are separate interop layers; no Mac Catalyst
status: accepted
decisionDate: 2026-05-12
deciders:
  - Alex Tsvetanov
supersedes: ""
supersededBy: ""
area: platform-interop
tags:
  - type/adr
  - status/accepted
  - area/handlers
  - platform/ios
  - platform/macos
---

# ADR-0005 — iOS and macOS are separate interop layers; no Mac Catalyst

> [!success] Status
> **accepted** on 2026-05-12.

## Context

.NET MAUI runs macOS apps via **Mac Catalyst** — Apple's framework for running iOS (UIKit) apps on macOS with some translation. This is convenient (one code path covers both) but produces apps that feel iOS-like on macOS: limited AppKit integration, sidebars and toolbars that don't match macOS conventions, and missing platform features.

MAUI accepts this trade-off. MPAPP will not.

UIKit is the native API on iOS. **AppKit** is the native API on macOS — not Catalyst. Treating them as separate platforms doubles the Apple surface but yields apps that feel native on both.

## Decision

`button_handler<platform::ios>` is implemented against **UIKit** in Objective-C++ `.mm` files.
`button_handler<platform::macos>` is implemented against **AppKit** in separate Objective-C++ `.mm` files.

There is no Mac Catalyst code path in MPAPP. macOS apps use AppKit, period.

The same applies to every other handler.

## Consequences

### Positive

- macOS apps feel native — proper menu bars, NSWindow integration, sidebar conventions, etc.
- iOS apps stay focused on UIKit idioms without macOS compromises.
- Future platforms (visionOS, tvOS) can be added without entangling them with the macOS path.

### Negative

- Double the Apple interop work. Mitigated because both UIKit and AppKit are well-documented and stable.
- Two CI runners (Simulator + macOS native) required, instead of one Catalyst path.

### Neutral

- Interop parity ([[ADR-0006-interop-parity]]) still applies: any public API works on both, even if the underlying handlers diverge.

## Alternatives Considered

- **Catalyst (MAUI-style).** Rejected — produces second-class macOS apps.
- **AppKit-only, drop iOS until later.** Rejected — iOS is a core target.
- **Single Objective-C++ codebase, dispatch at runtime.** Rejected — defeats the static-dispatch handler architecture and introduces ABI ambiguity.

## Implementation Notes

- [`src/handlers/macos/`](../../src/handlers/macos/) — AppKit handlers in Objective-C++ `.mm` files. App-shell seed set today (`application_handler.mm`, `button_handler.mm`, `label_handler.mm`); further components fill in once an Apple host is online (M-07).
- [`src/handlers/ios/`](../../src/handlers/ios/) — UIKit handlers in Objective-C++ `.mm` files. Same seed shape as macOS but targeting UIKit (`UIButton` etc.); fill-in pending M-08.
- Two separate directory trees with separate `.mm` files; no shared "Apple" path, no Catalyst — implementation matches the decision verbatim.

## References

- [[10_Architecture/Platform Interop]]
- [[ADR-0006-interop-parity]]
- [Apple Catalyst docs](https://developer.apple.com/mac-catalyst/)
