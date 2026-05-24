---
type: adr
id: ADR-0009
title: Public-API mechanism is template wrapper types only
status: accepted
decisionDate: 2026-05-12
deciders:
  - Alex Tsvetanov
supersedes: ""
supersededBy: ""
area: type-system
tags:
  - type/adr
  - status/accepted
  - area/type-system
---

# ADR-0009 — Public-API mechanism is template wrapper types only

> [!success] Status
> **accepted** on 2026-05-12.

## Context

[[ADR-0002-no-macros-in-public-api]] forbids public-API macros but does not specify the replacement mechanism. Two candidates were considered:

**Option A — Template wrapper types.**

```cpp
class todo_view_model : mpapp::view_model {
    mpapp::Observable<int>         count{0};
    mpapp::Observable<std::string> name{"Alice"};

    auto display(mpapp::Computed<&todo_view_model::count,
                                 &todo_view_model::name> = {}) const
        { return std::format("{}: {}", name.get(), count.get()); }

    void increment(mpapp::Command<> = {})
        { count.set(count.get() + 1); }
};
```

- `Observable<T>` is a thin wrapper that exposes `get/set/value` and an embedded intrusive signal node.
- `Computed<...>` and `Command<>` are zero-cost tag types whose template arguments encode the dependency or command binding at the type level. They appear as unnamed default-valued parameters.
- Pure C++23. No tools required. Compiles on MSVC, Clang, GCC, Apple Clang.

**Option B — C++ attribute namespaces.**

```cpp
class todo_view_model {
    [[mpapp::observable]] int         count = 0;
    [[mpapp::observable]] std::string name  = "Alice";
    [[mpapp::computed(count, name)]] auto display() const { ... }
    [[mpapp::command]]               void increment() { ... }
};
```

- Visually closer to MAUI's `[ObservableProperty]`.
- Requires a libclang-based meta-compiler (`mpapp-mc`) to parse the attributes and emit descriptor sidecars.
- Adds a build dependency.

## Decision

We will use **Option A — template wrapper types — exclusively**. Option B is **rejected** and will not be added as a secondary syntax.

## Consequences

### Positive

- **No meta-compiler needed** for the property/command system. The XAML compiler (`mpapp-xc`) is the only codegen tool, and it's a separate concern.
- Standard C++ — every conforming C++23 compiler handles this without special configuration.
- IDE support is automatic (types are first-class to IntelliSense/clangd).
- Zero runtime overhead: `Observable<T>` is a wrapper, `Computed<>` / `Command<>` are empty tag types with template-only state.

### Negative

- Slightly more verbose than the MAUI-style attribute syntax.
- `Computed<...>` requires that dependencies be pointer-to-member-accessible — fine for class members, harder for free variables.

### Neutral

- Bridges to C++26 reflection ([[ADR-0001-cpp-standard-baseline]]) remain available *internally* without changing the public-API mechanism.

## Alternatives Considered

- **Option B (attributes + meta-compiler).** Rejected for the reasons above — adds tooling burden without enough ergonomic gain.
- **Macros.** Rejected by [[ADR-0002-no-macros-in-public-api]].
- **Inheritance** (`class MyVM : observable<int, "count">, observable<std::string, "name">`). Rejected — string-name template parameters are awkward and the inheritance gets unwieldy fast.

## Implementation Notes

The three template wrappers, each in one file:

- [`include/mpapp/observable.hpp`](../../include/mpapp/observable.hpp) — `Observable<T>` with `get()/set()/operator T()/operator=` + an embedded `signal<const T&> changed` handle. Short-circuits on `==` so a same-value set does not fire.
- [`include/mpapp/computed.hpp`](../../include/mpapp/computed.hpp) — `Computed<&Member, ...>` zero-cost tag type; the framework picks the dependency pack out of a function's parameter list at template-instantiation time.
- [`include/mpapp/command.hpp`](../../include/mpapp/command.hpp) — `Command<Args...>` zero-cost tag type marking a method as bindable from XAML `Command="..."` syntax.
- Supporting infrastructure: [`include/mpapp/signal.hpp`](../../include/mpapp/signal.hpp) is the intrusive `signal<Args...>` + `signal_slot` pair backing `Observable::changed` (no heap allocation on subscribe/unsubscribe). Canonical usage in [`include/mpapp/button.hpp`](../../include/mpapp/button.hpp) — `Observable<std::string> text{}`.
- [`tests/template_type_spike/test.cpp`](../../tests/template_type_spike/test.cpp) — exercises the compile-time invariants the wrappers depend on.

## References

- [[ADR-0002-no-macros-in-public-api]]
- [[10_Architecture/Type System]]
- [[10_Architecture/Observable Properties]]
