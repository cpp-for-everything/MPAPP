---
type: adr
id: ADR-0021
title: "TableView cell type tree — full MAUI parity"
status: proposed
decisionDate: 2026-05-21
deciders: []
supersedes: ""
supersededBy: ""
area: handlers
tags:
  - type/adr
  - status/proposed
  - area/handlers
---

# ADR-0021 — TableView cell type tree

> [!info] Status
> **proposed** — unblocks TableView's full cell-typed surface (currently mock holds `vector<{title, rows-of-string}>`).

## Context

MAUI's `TableView` rows are not arbitrary views — they're typed **Cell** instances:

- `TextCell` — text + detail text + image + a tap event
- `EntryCell` — label + an inline text entry, two-way bound to a value
- `SwitchCell` — label + a toggle switch
- `ViewCell` — wraps an arbitrary `View`
- `ImageCell` — text + detail text + image source

Each renders with native cell styling on each platform. Users compose these directly in XAML; the native handler decides cell reuse strategy.

The MAUI cell hierarchy has nice properties:

- **Native styling** for free (e.g. iOS Settings-style rows).
- **Compile-time-typed** value bindings (an EntryCell's `Text` is `string`, a SwitchCell's `On` is `bool`).
- **Source-compat** for apps porting from MAUI.

The alternatives (one generic cell wrapping a view; or no cells at all) lose the typed binding story.

## Decision

We will ship the **full MAUI cell hierarchy** as concrete C++ types:

```cpp
namespace mpapp {

class cell : public element {
public:
    Observable<bool> is_enabled{true};
    signal<>         tapped{};
    // ... base styling props
};

class text_cell : public cell {
public:
    Observable<std::string> text{""};
    Observable<std::string> detail{""};
    Observable<std::string> text_color{""};
    Observable<std::string> detail_color{""};
};

class entry_cell : public cell {
public:
    Observable<std::string> label{""};
    Observable<std::string> text{""};
    Observable<std::string> placeholder{""};
    Observable<keyboard_kind> keyboard{keyboard_kind::default_};
    signal<const std::string&> completed{};
};

class switch_cell : public cell {
public:
    Observable<std::string> text{""};
    Observable<bool>        on{false};
    signal<bool>            on_changed{};
};

class view_cell : public cell {
public:
    Observable<view*> content{nullptr};
};

class image_cell : public text_cell {
public:
    Observable<std::string> image_uri{""};
};

}
```

`table_view::sections` updates from `std::vector<{string, vector<string>}>` to `std::vector<table_section>` where each `table_section` is `{title, vector<std::unique_ptr<cell>>}`. The mock surface still works (a `text_cell` with just `text` is API-equivalent to a string row); the real cells unlock the full MAUI shape.

Each cell type gets its own handler — same widget-handler pattern as button/label/switch_/etc. Each platform's table_view handler queries the registry to resolve cells to native row views.

## Consequences

### Positive

- MAUI parity preserved — users porting code don't reshape their cell trees.
- Typed bindings on cell value props (SwitchCell's `on` is `bool`, not `string`).
- Native cell styling rendered correctly on each platform (iOS Settings rows, Android list items, etc.).
- New cell types extend the hierarchy without breaking existing apps.

### Negative

- 5 new widget surfaces + 5 mock handlers + 5×3 = 15 real handlers when this lands fully. Up-front investment is real. Mitigation: text_cell + entry_cell + switch_cell ship in M-04c; view_cell + image_cell can defer to M-04d if scope is tight.
- TableView's table_view_handler must understand cell type dispatch (the registry helps — each cell self-registers, the handler picks the right row factory).
- Cell lifetime: TableView owns the cells (via `unique_ptr`). Reuse during recycling moves the cells, not the data — same as MAUI's model.

### Neutral

- Cell mock handlers can land incrementally — text_cell first is enough to start using TableView at the mock level.

## Alternatives Considered

- **Single generic cell + arbitrary view** — rejected; loses MAUI parity and typed bindings.
- **Hybrid (text_cell + view_cell only)** — rejected; users still expect switch_cell / entry_cell for the common settings-screen pattern, and shipping a half-tree feels worse than committing.
- **Skip cells entirely** — rejected; TableView is essentially useless without them.

## References

- [[ADR-0020-virtualized-item-host-wrap-platform]] — table_view itself uses native recyclers; cells are the row factory.
- [[Components/TableView]]
- [[40_Roadmap/M-04c-handler-heavy-port]]
