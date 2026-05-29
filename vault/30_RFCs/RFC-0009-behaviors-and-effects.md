---
type: rfc
id: RFC-0009
title: Behaviors + Effects — attached view extensibility points
status: accepted
author: Alex Tsvetanov
created: 2026-05-29
area: widgets
relatedADRs:
  - ADR-0008
  - ADR-0009
  - ADR-0024
tags:
  - type/rfc
  - status/accepted
  - area/widgets
---

# RFC-0009 — Behaviors + Effects

> [!info] Status
> **accepted** — mock surface shipped under [[T-0053-behaviors-and-effects]].

## Problem

MAUI offers two attached-extensibility points beyond triggers: **Behaviors** (`Behavior` / `Behavior<T>` — reusable objects that hook a view's lifecycle + events without subclassing) and **Effects** (`Effect` / `RoutedEffect` — a platform visual tweak resolved by id). MPAPP had neither. Behaviors are the idiomatic way to package reusable view logic (a numeric-entry validator, a drag handler); Effects are legacy but still appear in MAUI XAML.

## Proposal

Two small attached collections on `view`, mirroring `gesture_recognizers`:

1. **`mpapp::behavior`** (`behaviors/behavior.hpp`) — abstract base with `on_attached(view&)` / `on_detached(view&)` (default no-ops). `view::behaviors` collection + `add_behavior<B>(args…)` (constructs, attaches, returns ref) + `remove_behavior(b)` (detaches, drops).
2. **`mpapp::effect`** (`effects/effect.hpp`) — minimal base carrying a `resolution_id` (MAUI's effect-registry key) + `on_attached`/`on_detached`. `view::effects` collection + `add_effect<E>(args…)`.

Both bases forward-declare `view` (so view.hpp embeds the collections without a cycle), are header-only, use no macros (ADR-0009), and are platform-neutral.

> [!note] Effects vs Handlers
> MAUI has effectively superseded Effects with Handlers, and MPAPP's handler architecture (ADR-0024) already covers what Effects did. The `effect` attach-point exists for **surface parity** + to give legacy `<Effect>` XAML a lowering target. New code should prefer a handler.

## Detailed design

`add_behavior<B>` / `add_effect<E>` static_assert the type derives from the base, `make_shared` it (perfect-forwarding ctor args), push to the collection, call `on_attached(*this)`, and return a reference. `remove_behavior` finds by address, calls `on_detached(*this)`, erases. View destruction drops the collections without calling `on_detached` (the defaulted dtor) — callers needing detach side effects call `remove_behavior` first; documented on the member.

### Tests (mock-first)

`tests/mock_handlers/behavior_test.cpp` — 4 cases / 13 assertions: attach increments the lifecycle counter + collection size; ctor-arg forwarding (`tagged_behavior`); remove detaches; `add_effect` attaches + exposes `resolution_id`.

## Alternatives

- **Behavior as a CRTP mixin on the view.** Rejected — MAUI behaviors are runtime-attachable/removable objects, which the shared_ptr collection models; CRTP would bake them into the type.
- **Skip Effects entirely** (handlers supersede them). Kept a minimal attach-point only for XAML-parity / lowering; flagged as legacy.

## Open Questions

> [!todo] Open
> - [ ] `<Behavior>` / `<Effect>` XAML lowering — mpapp-xc (M-09).
> - [ ] Typed `behavior_of<TView>` convenience that down-casts the host in `on_attached`.
> - [ ] Per-platform effect registry (resolve `resolution_id` → native rendering) if any Effect is kept long-term.

## Migration / Compatibility

Pure addition: two new `view` collections + their `add_*`/`remove_*` members. No existing surface modified; no per-platform handler code.

## References

- [[RFC-0008-triggers]] (sibling extensibility layer), [[ADR-0024-wrapper-component-pattern]] (handlers supersede Effects).
- `references/maui/src/Controls/src/Core/Behavior.cs`, `Effect.cs`, `RoutedEffect.cs`.
