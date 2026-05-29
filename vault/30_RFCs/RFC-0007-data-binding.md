---
type: rfc
id: RFC-0007
title: Data binding — binding<S,T> modes + converters + multi-binding + BindingContext + RelativeSource
status: accepted
author: Alex Tsvetanov
created: 2026-05-29
area: properties
relatedADRs:
  - ADR-0008
  - ADR-0009
  - ADR-0024
tags:
  - type/rfc
  - status/accepted
  - area/properties
  - area/markup
---

# RFC-0007 — Data Binding

> [!info] Status
> **accepted** — mock surface shipped under [[T-0051-data-binding-engine]]. The keystone subsystem: triggers (RFC-0008), value-converter-driven UI, and the XAML compiler's `{Binding}` lowering all build on this.

## Problem

MPAPP had the reactive *primitives* — `Observable<T>`, the intrusive `signal`/`signal_slot`, the `Computed<&Member…>` / `Command<…>` compile-time tags (ADR-0009) — but **no binding engine**. There was no way to say "keep this view property in sync with that view-model property," no `BindingContext`, no value converters, no `MultiBinding`, no `RelativeSource`. Every example wired changes by hand with bespoke `signal_slot`s. This is the single biggest gap vs MAUI and the prerequisite for triggers, `DataTemplate` data-flow, and the whole `{Binding …}` XAML story.

## Proposal

A binding layer that **composes with** the existing primitives (subscribes to `Observable::changed` via owned `signal_slot`s) rather than replacing them, expressed entirely in template-wrapper types — **no macros** (Rule 1 / ADR-0009).

1. **`mpapp::binding<S, T>`** — a live link between a source `Observable<S>` and a target `Observable<T>`, with the four MAUI modes (`one_way` / `two_way` / `one_time` / `one_way_to_source`) + optional `to_target` / `to_source` converters. `include/mpapp/binding/binding.hpp`.
2. **`mpapp::value_converter<S,T>`** — abstract IValueConverter analog (optional sugar; bindings also accept plain `std::function`).
3. **`mpapp::multi_binding<T, Ss…>`** — combine N source Observables into one target via a combiner callable; recomputes on any source change. `multi_binding.hpp`.
4. **`mpapp::binding_context`** — type-erased data context (MAUI's `BindingContext`), stored by value on every `view`, with inheritance down the visual tree resolved by `effective_binding_context(view)`. `binding_context.hpp` + `relative_source.hpp`.
5. **`mpapp::find_ancestor<C>(view)` / `relative_source`** — `{RelativeSource Self / AncestorType}`. `relative_source.hpp`.

Per [[ADR-0008-mock-first-implementation]] this is the C++ surface + tests. Crucially, **binding needs no per-platform handler code**: it drives `Observable::set`, which fires the same property mapper a real platform handler installed — so a bound update flows through the identical `Observable → handler → native widget` pipeline already real on Windows / Linux / Android. Binding is "real on all platforms" by construction (proven by a test that binds through to a mock handler and asserts the recorded mapper calls).

Per [[ADR-0024-wrapper-component-pattern]], binding is NOT a wrapper-component — pure cross-platform infrastructure.

## Detailed design

### `binding<S, T>`

Subscribes per mode and seeds the initial value at construction. A `bool updating_` re-entrancy guard (plus `Observable::set`'s compare-on-write) prevents the two-way echo loop. Converters are `std::function` (set once per change — not hot-path; the `signal` layer stays thunk-based). When `S`/`T` are implicitly convertible and no converter is given, the identity/`static_cast` conversion is used; otherwise the matching converter is required. Owns its `signal_slot`s + member-callback objects (stable address, the same pattern the mock handlers use); non-copyable / non-movable.

### `multi_binding<T, Ss…>`

Holds a `std::tuple` of source pointers, per-source member callbacks, and per-source slots. `recompute()` = `target.set(combine(sources.get()…))`; wired so any source change recomputes. One-way (sources → target).

### `binding_context` + inheritance

`binding_context` holds a `shared_ptr<void>` + the stored `type_info`, so `get<C>()` recovers the original pointer with exact-type safety. Stored by value on `view` (`local_binding_context()` / `set_binding_context<C>()`). `effective_binding_context(view)` walks `view::parent()` (the link RFC-0005 added) returning the nearest non-empty context — MAUI's inheritance. `find_ancestor<C>(view, include_self)` backs `{RelativeSource AncestorType}`.

### Tests (mock-first)

`tests/mock_handlers/binding_test.cpp` — 13 cases / 39 assertions: each of the four modes, one-way + two-way converters, two flavours of `multi_binding`, `binding_context` typed get + clear, `effective_binding_context` inheritance + shadowing + empty, `find_ancestor` (nearest typed / self-inclusion / miss), `RelativeSource Self`, and the end-to-end "binding drives a bound property through the mock handler" integration check.

## Alternatives

- **String-path binding** (`{Binding Path=Foo.Bar}` resolved by reflection). Rejected for the C++ surface — there is no reflection without macros (Rule 1). The XAML compiler (M-09) will lower a string path to a member-pointer / lambda accessor against the typed `BindingContext`; the runtime engine here is the typed target it lowers onto.
- **A central binding registry** keyed by (object, property). Rejected — the per-instance `binding<S,T>` owning its slots matches the framework's RAII/intrusive-slot style and needs no global state.
- **`std::function`-free converters** (template the converter type into `binding`). Rejected — converters aren't hot-path, and type-erasure keeps `binding<S,T>` instantiation count bounded + matches IValueConverter.

## Open Questions

> [!todo] Open
> - [ ] `DataTemplate` / `DataTemplateSelector` generalization (today `CollectionView::item_template` + `bindable_layout`'s `data_template` are bespoke). Fold into a shared `data_template` in a follow-up.
> - [ ] `{Binding}` / `{Binding Path=…}` XAML lowering — mpapp-xc (M-09), lowers a string path onto a typed accessor against the `BindingContext`.
> - [ ] `RelativeSource TemplatedParent` — needs the control-template machinery.
> - [ ] String-formatted bindings (`StringFormat`) — a converter shorthand; sugar over `to_target`.

## Migration / Compatibility

- Pure addition. The only `view` change is a new `binding_context binding_ctx_` member (one `shared_ptr` + a `type_info*`) + accessors — every existing component inherits it for free.
- No existing surface modified; no per-platform handler changes.

## References

- [[ADR-0009-public-api-template-wrappers-only]] — the no-macros / template-wrapper constraint binding composes within.
- [[ADR-0008-mock-first-implementation]], [[ADR-0024-wrapper-component-pattern]].
- [[RFC-0005-resource-dictionaries-and-styling]] — the `view::parent()` link reused for context + ancestor walks.
- `references/maui/src/Controls/src/Core/Binding.cs`, `BindingBase.cs`, `MultiBinding.cs`, `RelativeSource.cs`, `BindableObject.cs`.
