---
type: adr
id: ADR-0017
title: "Grid track definitions — value-type with string parser"
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
  - area/markup
---

# ADR-0017 — Grid track definitions

> [!info] Status
> **proposed** — unblocks Grid as a real layout engine.

## Context

`Grid` in MAUI has `RowDefinitions` / `ColumnDefinitions` whose track values come in three flavors:

- **Fixed** — pixel size: `200`
- **Auto** — sized to content
- **Star** — proportional weight: `*`, `2*`

XAML expresses these as comma-separated strings: `RowDefinitions="Auto, *, 200, 2*"`. C# also exposes typed `RowDefinition`/`GridLength` constructors. MPAPP needs both — XAML for parity, typed C++ for stronger checking.

## Decision

The internal model is a **value-type vector of `mpapp::track_def`** strongly-typed entries. A **string parser** populates the vector from a MAUI-compatible source string. Both forms compile in C++; XAML always goes through the string parser.

```cpp
struct track_def {
    enum class kind : std::uint8_t { fixed, auto_, star };
    kind   k     = kind::auto_;
    double value = 0.0;   // pixels for fixed, weight for star (default 1.0)

    static track_def Auto() noexcept;
    static track_def Star(double weight = 1.0) noexcept;
    static track_def Fixed(double px) noexcept;

    // Parser: "Auto, *, 200, 2*" -> vector<track_def>
    static std::vector<track_def> parse(std::string_view spec);
};

class grid : public layout {
public:
    Observable<std::vector<track_def>> row_definitions{};
    Observable<std::vector<track_def>> column_definitions{};

    // C++ users can pass either form:
    void set_rows_from_spec(std::string_view spec) {
        row_definitions.set(track_def::parse(spec));
    }
};
```

XAML emits `grid.set_rows_from_spec("Auto, *, 200, 2*")`. C++ users can pass values directly or use the spec helper. The internal vector is always values — handlers see one shape.

The parser is a tiny constexpr-friendly tokenizer (comma split, trim whitespace, parse `Auto` / `*` / `N*` / number). It lives in `include/mpapp/grid.hpp`. No reflection, no JSON, no external dependency.

## Consequences

### Positive

- One internal model — handlers don't branch on representation.
- C++ users get strong typing if they want it; XAML users get string parity.
- The parser is easy to test in isolation (pure-function transformation).
- Future track kinds (e.g. `Min`, `Max`, fr-units) extend the enum without breaking the API shape.

### Negative

- Parse errors surface at runtime, not compile time. A `constexpr` parser could move that earlier, but at the cost of restricting users to compile-time-known strings. Mitigation: ship a clear diagnostic and a `try_parse` non-throwing variant for tooling.
- Two ways to do the same thing (string + values) doubles the documentation footprint slightly.

### Neutral

- The layout engine consuming `row_definitions` is independent of how the values got there. The grid layout algorithm is its own design problem; this ADR scopes only the track-definition representation.

## Alternatives Considered

- **String DSL only** — rejected; loses type checking on the C++ side.
- **Value-type only** — rejected; XAML still needs a parser, so we'd ship one anyway.
- **Builder fluent API only** — rejected; doesn't compose with collection mutation idioms (e.g. inserting a row at position 2 is awkward).

## References

- [[ADR-0004-maui-xaml-superset-compat]] — XAML attribute string parity.
- [[Components/Grid]]
- [[40_Roadmap/M-04c-handler-heavy-port]]
