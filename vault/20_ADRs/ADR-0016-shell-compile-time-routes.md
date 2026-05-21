---
type: adr
id: ADR-0016
title: "Shell URI routing — compile-time route table"
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

# ADR-0016 — Shell URI routing via compile-time route table

> [!info] Status
> **proposed** — composes on top of [[ADR-0014-page-navigation-stack]]. Unblocks Shell's full surface (route templates, route guards, route-aware lifecycle).

## Context

MAUI's Shell uses **string-typed runtime routes**: apps call `Shell.RegisterRoute("//home/details", typeof(DetailsPage))` and navigate via `Shell.Current.GoToAsync("//home/details?id=42")`. The runtime parses the URI, looks up the registered handler, and instantiates the page.

This works in C# where reflection + the type system collaborate. In MPAPP we want the same surface but with **compile-time type safety**, because:

- Typos in route names fail at runtime in MAUI. C++ can catch those at compile time.
- Page-factory lookups require type erasure (`std::function<page*()>`); with templates we get the factory's exact type and skip the erasure.
- Route parameters are stringly-typed in MAUI; we want strong typing on the parameter pack.

XAML usage still needs string parity (the XAML compiler emits route strings). So the public API is strings; the C++ binding is compile-time-checked.

## Decision

We will ship a templated route registry. Apps declare routes via:

```cpp
inline constexpr auto routes = mpapp::route_table{
    mpapp::route<"home",          home_page>{},
    mpapp::route<"home/details",  details_page, mpapp::param<"id", int>>{},
    mpapp::route<"settings",      settings_page>{},
};

// Navigation is compile-time-checked:
co_await shell.go_to<"home/details">(42);     // ok
co_await shell.go_to<"home/detial">(42);      // compile error: typo
co_await shell.go_to<"home/details">("42");   // compile error: int expected
```

The XAML compiler (mpapp-xc) still emits string-based `Shell.GoToAsync("//home/details?id=42")` calls; those go through a runtime lookup against the same `route_table` instance and surface as a runtime diagnostic if the route is unknown. The compile-time path is the C++ surface; the runtime path is the XAML escape hatch.

Route templates support the standard `//`, `/`, and `?param=value` syntax for parity with MAUI. The constexpr parser at the top of the route_table extracts the route name + parameter list.

Route guards (CanActivate / CanDeactivate) and route-aware lifecycle (`OnNavigatedTo` / `OnNavigatedFrom`) are deferred to a follow-up ADR — they need the executor design (ADR-0019).

## Consequences

### Positive

- Route typos and parameter type mismatches caught at compile time when navigating from C++.
- No runtime reflection or type erasure on the C++ navigation path.
- XAML interop preserved through the string-based runtime lookup.
- The `route_table` value can be passed around as data, queried, enumerated for debug UIs, etc.

### Negative

- Compile error messages around `mpapp::route<...>` templates can be verbose. We provide a `static_assert` chain to surface clear errors ("route 'foo' not in route_table" rather than a substitution failure).
- App authors writing routes in C++ must declare them in one `route_table` literal — they cannot register routes from a plugin at runtime without falling back to the XAML/string path.

### Neutral

- Route parameters use `mpapp::param<"name", T>` template; `T` must be string-convertible (we provide `parse<T>` specializations for `int`, `long`, `double`, `bool`, `std::string`). Custom types add their own specialization.

## Alternatives Considered

- **MAUI-parity runtime strings only** — rejected; loses the type-safety story that is one of MPAPP's stated value props (Rule 7's "stricter compile-time type safety").
- **Hybrid runtime registration with compile-time validation** — folded into this proposal: the runtime lookup is the XAML path; the compile-time `go_to<"...">()` is the C++ path. Same table backs both.

## References

- [[ADR-0014-page-navigation-stack]] — provides the underlying page_stack engine Shell uses per-tab.
- [[ADR-0009-public-api-template-wrappers-only]] — template wrappers in public API.
- [[Components/Shell]]
- [[40_Roadmap/M-04c-handler-heavy-port]]
