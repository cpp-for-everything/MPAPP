---
type: rfc
id: RFC-0008
title: Triggers — property/data/multi/event/state triggers over the binding + VSM layers
status: accepted
author: Alex Tsvetanov
created: 2026-05-29
area: properties
relatedADRs:
  - ADR-0008
  - ADR-0009
tags:
  - type/rfc
  - status/accepted
  - area/properties
  - area/markup
---

# RFC-0008 — Triggers

> [!info] Status
> **accepted** — mock surface shipped under [[T-0052-triggers]]. Builds directly on RFC-0007 (binding) + RFC-0006 (VSM).

## Problem

MAUI's trigger family — `Trigger`, `DataTrigger`, `MultiTrigger`, `EventTrigger`, `StateTrigger` — is how declarative UI reacts to state without imperative event glue: "when `IsPressed` is true, set the background"; "when the VM's `HasError` is true, go to the Invalid visual state". MPAPP had none of it. With the binding engine (RFC-0007) + VSM (RFC-0006) now in place, triggers are a thin, high-value layer on top.

## Proposal

`include/mpapp/triggers/`, all composing with `Observable::changed` via owned `signal_slot`s, reusing the `function<void(view&)>` setter shape from `style`/`visual_state`. No macros (ADR-0009); platform-neutral (drives the existing Observable→handler pipeline, like binding).

1. **`trigger<T>`** (`trigger.hpp`) — active while `source == value`; runs `enter_setters` on activate, `exit_setters` on deactivate. Covers MAUI's property `Trigger` (point `source` at a target Observable) and `DataTrigger` (point it at a view-model Observable, typically via the BindingContext).
2. **`multi_trigger<Ts…>`** — active only while ALL `when<T>{source, value}` conditions match.
3. **`event_trigger<Args…>`** (`event_trigger.hpp`) — runs an action callable when a source `signal<Args…>` fires (EventTrigger + TriggerAction).
4. **`state_trigger`** (`state_trigger.hpp`) — bridges to the VSM: a boolean condition drives `visual_state_manager::go_to_state(target, active_state | inactive_state)`.

Auto value-capture/restore is deferred (same as `style`/VSM — needs the BindableProperty specificity layer); apps provide complete enter/exit setter pairs.

## Detailed design

`trigger_base` carries `enter_setters` / `exit_setters` + the `transition(target, now_active)` edge logic (runs the matching bundle only on a real edge; swallows setter exceptions). `trigger<T>` evaluates `source == value` on every change. `multi_trigger` stores a tuple of `when<T>` conditions + per-condition slots/callbacks and re-evaluates the AND on any change. `event_trigger` owns one slot on the source signal. `state_trigger` owns a slot on an `Observable<bool>` and calls into the VSM.

### Tests (mock-first)

`tests/mock_handlers/trigger_test.cpp` — 5 cases / 26 assertions: enter/exit edges, same-value no-op + re-match, multi-trigger AND semantics, event-trigger action dispatch with args, and state-trigger → VSM transitions (Valid ↔ Invalid).

## Alternatives

- **A single `Trigger` type with a runtime "kind" enum.** Rejected — the typed `trigger<T>` / `multi_trigger<Ts…>` / `event_trigger<Args…>` keep value types static (no `std::any` round-trips) and read naturally.
- **Auto value-capture for revert.** Deferred to the binding/specificity follow-up; the complete-enter/exit-pair convention matches `style`/VSM and is honest about the mock surface.

## Open Questions

> [!todo] Open
> - [ ] XAML lowering of `<Trigger>` / `<DataTrigger>` / `<MultiTrigger>` / `<EventTrigger>` / `<StateTrigger>` — mpapp-xc (M-09).
> - [ ] Auto value-capture/restore once a BindableProperty specificity layer exists.
> - [ ] `EnterActions` / `ExitActions` (TriggerAction lists) beyond setters.

## Migration / Compatibility

Pure addition; no existing surface modified; no per-platform handler code.

## References

- [[RFC-0007-data-binding]] (the condition sources), [[RFC-0006-visual-state-manager]] (StateTrigger target), [[RFC-0005-resource-dictionaries-and-styling]] (setter shape).
- `references/maui/src/Controls/src/Core/Trigger.cs`, `DataTrigger.cs`, `MultiTrigger.cs`, `EventTrigger.cs`, `StateTrigger.cs`.
