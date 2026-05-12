---
type: adr
id: ADR-0003
title: Markup language is XAML only; no custom DSL
status: accepted
decisionDate: 2026-05-12
deciders:
  - Alex Tsvetanov
supersedes: ""
supersededBy: ""
area: markup
tags:
  - type/adr
  - status/accepted
  - area/markup
---

# ADR-0003 — Markup language is XAML only; no custom DSL

> [!success] Status
> **accepted** on 2026-05-12.

## Context

Some cross-platform UI frameworks (Slint, Flutter, SwiftUI) define their own DSLs for UI markup. Others (.NET MAUI, WPF, Avalonia) use XAML. MPAPP must choose.

The user's directive is explicit: the C++ API is the canonical form of every UI; markup is sugar over it. MAUI works the same way — every XAML construct has a C# equivalent. There is no Slint-style "the DSL is the source of truth" arrangement.

## Decision

We will support **XAML as the only markup language**. There will be no MPAPP-specific DSL. The C++ API must be rich enough that any UI buildable in XAML is also buildable in pure C++ — that is, the C++ surface is *complete* with respect to the markup.

The XAML compiler `mpapp-xc` parses `.xaml` files and emits `consteval` C++ struct trees that call the same public API a hand-written C++ UI would call.

## Consequences

### Positive

- One language for both modes — the developer learns the C++ API and gets XAML support "for free."
- Migration story for MAUI/WPF users is direct: XAML transfers; only the code-behind language changes.
- The framework can be used without ever touching XAML — pure C++ usage is first-class.

### Negative

- We do not get the design freedom a fresh DSL would give us (e.g., we cannot fix XAML's verbosity).
- XAML's quirks (markup extensions, attached properties) must be faithfully reproduced.

### Neutral

- XAML compatibility scope is set by [[ADR-0004-maui-xaml-superset-compat]].

## Alternatives Considered

- **MPAPP-DSL (Slint-inspired).** Rejected — would split the developer audience and make the C++ API a second-class citizen.
- **No markup, C++ only.** Rejected — XAML is widely used in the MAUI / WPF ecosystem; supporting it is a force multiplier for adoption.

## References

- [[10_Architecture/Markup]]
- [[ADR-0004-maui-xaml-superset-compat]]
- [[60_Research/dotnet-maui-deep-dive]]
