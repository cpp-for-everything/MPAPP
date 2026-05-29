---
type: rfc
id: RFC-0014
title: Commanding — relay_command / ICommand runtime command objects
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
---

# RFC-0014 — Commanding

> [!info] Status
> **accepted** — shipped under [[T-0059-commanding]]. Completes the MVVM trio (binding RFC-0007 + converters T-0058 + commanding) needed to bind a control to a view-model action.

## Problem

MPAPP had the compile-time `Command<>` *tag* (`command.hpp`, ADR-0009) that marks view-model methods for the XAML compiler, but **no runtime command object** — nothing a control's `command` property can bind to, no `CanExecute` gating, no `CanExecuteChanged`. In MVVM you bind `Button.Command` to a view-model `ICommand`; the button invokes it on click and disables itself when `CanExecute` is false. That object was missing.

## Proposal

`include/mpapp/binding/relay_command.hpp`, header-only, no macros:

1. **`command_base`** — the ICommand interface: `can_execute()`, `execute()`, `can_execute_changed` signal.
2. **`relay_command`** — parameterless: wraps an execute action + optional `can_execute` predicate (default always-true) + `raise_can_execute_changed()`.
3. **`relay_command_of<T>`** — parameterized (MAUI's CommandParameter / toolkit `RelayCommand<T>`): the parameter flows to `execute(param)` / `can_execute(param)`.

`execute()` is gated — it no-ops when `can_execute()` is false (MAUI semantics). Distinct from the `Command<>` tag: that's compile-time metadata; this is the runtime object a control binds to.

## Detailed design

Both commands store `std::function`s for execute + can_execute. `relay_command` derives `command_base` (so controls hold a `command_base*`/`shared_ptr<command_base>` and invoke polymorphically). `can_execute_changed` is the intrusive `signal<>`; a view-model calls `raise_can_execute_changed()` after state that affects the guard changes, and the bound control re-queries. Non-movable (owns the signal).

### Tests (mock-first)

`tests/mock_handlers/relay_command_test.cpp` — 6 cases / 20 assertions: execute runs the action; can_execute gates execute; can_execute_changed fires; parameterized pass-through + gating; polymorphic invoke via `command_base&`.

## Alternatives

- **A single `Command<Args...>` runtime template.** Rejected — the no-arg `relay_command` is the common case (it's what `command_base` exposes for polymorphic control binding); `relay_command_of<T>` covers parameters without forcing every control to know the parameter type.
- **Reusing the `Command<>` tag at runtime.** Rejected — the tag is deliberately a compile-time marker (ADR-0009); a runtime ICommand is a different concern.

## Open Questions

> [!todo] Open
> - [ ] A `command` property on the relevant controls (button/menu items/gestures) that auto-invokes the bound `command_base` on activation + toggles `is_enabled` from `can_execute`.
> - [ ] `<Button Command="{Binding Save}">` XAML lowering — mpapp-xc (M-09).
> - [ ] An `async_relay_command` returning `task<void>` (ADR-0019) for awaitable command bodies.

## Migration / Compatibility

Pure addition; no existing surface modified; platform-neutral.

## References

- [[RFC-0007-data-binding]] (the binding layer commands are bound through), [[ADR-0009-public-api-template-wrappers-only]] (the distinct `Command<>` tag).
- `references/maui` — `Microsoft.Maui.Controls.Command`, CommunityToolkit.Mvvm `RelayCommand` / `RelayCommand<T>`, `System.Windows.Input.ICommand`.
