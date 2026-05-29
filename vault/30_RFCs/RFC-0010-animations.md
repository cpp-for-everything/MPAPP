---
type: rfc
id: RFC-0010
title: Animations — tweened values, easing curves, animation manager + ViewExtensions
status: accepted
author: Alex Tsvetanov
created: 2026-05-29
area: widgets
relatedADRs:
  - ADR-0008
  - ADR-0009
  - ADR-0019
tags:
  - type/rfc
  - status/accepted
  - area/widgets
  - area/threading
---

# RFC-0010 — Animations

> [!info] Status
> **accepted** — mock surface shipped under [[T-0054-animations]]. The platform-neutral interpolation engine is complete + tested; the native per-vsync frame ticker that drives it is the Phase-6 dispatcher work ([[ADR-0019-async-executor-native-dispatcher]]).

## Problem

MPAPP had static transform Observables (`opacity`, `scale`, `rotation`, `translation_x/y`) but nothing to drive them over time — no easing, no `FadeTo`/`TranslateTo`, no animation manager. Animations are table-stakes for any modern UI.

## Proposal

A platform-neutral, deterministic interpolation engine in `include/mpapp/animation/`, driven by an explicit time step so it's fully testable without a live event loop. No macros (ADR-0009).

1. **`easing_kind` + `ease(kind, t)`** (`easing.hpp`) — MAUI's `Easing`: linear, sin/quad/cubic in-out variants, bounce_out, spring_out (overshoot). Pure functions; t clamped to [0,1].
2. **`animation`** (`animation.hpp`) — one tweened `double` from→to over a duration with an easing curve. `advance(dt)` steps it (calls an `on_tick(value)` + an `on_finished()` once); `seek(progress)` scrubs. Deterministic — no internal clock.
3. **`animation_manager`** (`animation_manager.hpp`) — owns running animations; `tick(dt)` advances all + drops finished. MAUI's `AnimationManager`.
4. **`ViewExtensions`** (`view_animations.hpp`) — `fade_to` / `scale_to` / `rotate_to` / `translate_to(x,y)` build an `animation` driving the matching `view` Observable from its current value.

The only per-platform piece is the **frame ticker** — who calls `animation_manager::tick(16ms)` each vsync. That is the native dispatcher (Phase 6 / ADR-0019). Until then (and in all tests) the manager is ticked explicitly, so the interpolation is real + verified on every platform; only the *cadence source* is native. The engine is "real on Win/Linux/Android" by construction once the ticker is wired.

## Detailed design

`animation` stores `from/to/duration/easing` + an accumulated `elapsed_`; each `advance(dt)` recomputes `progress = elapsed/duration`, applies `ease`, sets `current = from + (to-from)*eased`, emits, and flags `finished` (firing `on_finished` once) at progress ≥ 1. `translate_to` runs an eased 0→1 progress and interpolates both axes in its tick. `animation_manager::tick` advances each animation and `erase`s the finished ones.

### Tests (mock-first)

`tests/mock_handlers/animation_test.cpp` — 7 cases / 38 assertions: easing anchor points + clamping, from→to interpolation + finish-once + post-finish no-op, mid-flight easing (cubic_in at 0.5 = 0.125), manager tick/drop, and the four `ViewExtensions` helpers driving real `view` Observables.

## Alternatives

- **Coroutine animations** (`co_await fade_to(...)`). Deferred — `async_sleep` exists, but coroutine-driven frames are harder to test deterministically and need the real dispatcher. The advance-by-dt engine is testable now and the coroutine sugar can layer on top later.
- **A global animation clock inside `animation`.** Rejected — an internal clock makes tests non-deterministic + couples the engine to a time source; explicit `advance(dt)` keeps it pure.

## Open Questions

> [!todo] Open
> - [ ] Native frame ticker wiring (`animation_manager::tick` from each platform's vsync) — Phase 6 / ADR-0019.
> - [ ] `co_await`-able `fade_to(...)` coroutine sugar returning a `task<void>`.
> - [ ] Keyframe / parallel / sequential animation composition (MAUI's `Animation` child-add API).
> - [ ] `<animation>` / Storyboard XAML lowering — mpapp-xc (M-09).

## Migration / Compatibility

Pure addition; no existing surface modified. `view_animations` read/write existing `view` transform Observables.

## References

- [[ADR-0019-async-executor-native-dispatcher]] (the frame ticker home), [[RFC-0007-data-binding]] (sibling Observable-driven layer).
- `references/maui/src/Controls/src/Core/Animation.cs`, `AnimationExtensions.cs`, `ViewExtensions.cs`, `Easing.cs`.
