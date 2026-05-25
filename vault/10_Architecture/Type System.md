---
type: moc
area: type-system
tags:
  - area/type-system
---

# Type System

The MPAPP type system is the foundation that makes "compile-time type safety beyond MAUI" achievable. It rests on three pillars:

1. **Template wrapper types** — `Observable<T>`, `Computed<...>`, `Command<>` — encode reflection-like metadata at the type level. No macros, no codegen. See [[ADR-0009-public-api-template-wrappers-only]].
2. **CRTP control bases** — handlers are partial specializations on `platform_tag`, dispatched statically. See [[Handlers]].
3. **Compile-time binding paths** — `bind(&Person::name)` resolves to a `binding_path<Person, &Person::name>` type checked against target property types. Binding errors are compile errors.

## The three primitives

### `Observable<T>`

A thin wrapper exposing `get()/set()/value()` plus an embedded intrusive signal node. Implicit conversion to `T` lets it read as a value. Setting compares with `==` and fires the embedded signal on change.

```cpp
mpapp::Observable<int> count{0};
count.set(count.get() + 1);          // explicit
count = 42;                          // operator= sets
int n = count;                       // implicit conversion reads
count.changed.subscribe([](int n){ /* … */ });
```

### `Computed<&Member, ...>`

A zero-cost tag type that, as an unnamed default-valued parameter on a member function, declares the function as a computed property whose dependencies are the listed pointer-to-members:

```cpp
auto display(mpapp::Computed<&todo_view_model::count,
                             &todo_view_model::name> = {}) const
    { return std::format("{}: {}", name.get(), count.get()); }
```

The framework picks this up via template argument deduction: any callable with a `Computed<...>` parameter is treated as a computed property whose change-notification fires when any listed member's `Observable` signals.

### `Command<Args...>`

Same tag-type pattern, applied to commands:

```cpp
void increment(mpapp::Command<> = {})
    { count.set(count.get() + 1); }
```

The presence of `Command<>` marks the method as bindable to XAML `Command="…"` syntax or to `bind(&VM::increment)` from C++.

## What the meta-compiler does (and doesn't) handle

The XAML compiler `mpapp-xc` is the only codegen step in the framework. It does **not** read C++ headers — it reads `.xaml` and emits `consteval` C++ that calls the public API.

The property/command system needs **no** meta-compiler. Everything is template metaprogramming.

This is the key divergence from a Qt-style approach (`moc` reads headers and emits sidecar `.cpp`). MPAPP's elegance comes from avoiding that step entirely.

## C++26 reflection bridge

Per [[ADR-0001-cpp-standard-baseline]], when [P2996](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2996r1.html) ships in production toolchains, the *internal* implementation of `Observable<T>` / `Computed<...>` can use `std::meta::reflect_of<...>` to simplify dependency tracking. The public API does not change.

## See in code

The three pillars, each in one file:

- **Template wrapper types**
  - [`include/mpapp/observable.hpp`](../../include/mpapp/observable.hpp) — `Observable<T>` with `get()/set()/operator T()/operator=` + embedded `changed` signal.
  - [`include/mpapp/computed.hpp`](../../include/mpapp/computed.hpp) — `Computed<&Ptrs...>` sentinel tag for computed-property declaration.
  - [`include/mpapp/command.hpp`](../../include/mpapp/command.hpp) — `Command<Args...>` sentinel tag for bindable methods.
  - [`include/mpapp/signal.hpp`](../../include/mpapp/signal.hpp) — the intrusive `signal<Args...>` / `signal_slot<Args...>` pair backing `changed`.
- **CRTP control bases + platform-tag dispatch**
  - [`include/mpapp/platform.hpp`](../../include/mpapp/platform.hpp) — the six `platform::{windows,linux_,android,macos,ios,mock}` tag types. Note the trailing underscore on `linux_` because `linux` is a predefined toolchain macro.
  - [`include/mpapp/handlers/`](../../include/mpapp/handlers/) — handler templates partial-specialised on platform tags (under `mpapp::internal::`). The compiler picks one specialisation per component per platform; missing one is a link error, not a runtime fallback. The public `template <class P = platform::current> using <name>_handler = internal::<name>_handler<P>;` alias next to each wrapper keeps `mpapp::<name>_handler<>` and `mpapp::<name>_handler<platform::mock>` valid spellings.
  - CRTP parameterises on the **surface** type (`control<basic_<name>>`), not the wrapper, so introspection lives on the platform-agnostic layer where the handler reads it. See [[ADR-0024-wrapper-component-pattern]] for the surface/wrapper split.
- **Wrapper-component layering (ADR-0024)**
  - [`include/mpapp/internal/`](../../include/mpapp/internal/) — surface classes (`internal::basic_<name>`) hold the handler by pointer; this is the type test code constructs directly.
  - [`include/mpapp/<name>.hpp`](../../include/mpapp/) — wrapper classes inherit the surface publicly and embed the handler by value; user app code constructs these.
- **Compile-time binding paths**
  - [`tests/template_type_spike/test.cpp`](../../tests/template_type_spike/test.cpp) — exercises the type-system invariants the binding-path machinery rests on (Observable / Computed / Command compile-time properties).

## See also

- [[No Macros In Public API]] — Rule 1
- [[Observable Properties]] — detailed implementation
- [[Handlers]] — how controls use the type system
- [[ADR-0009-public-api-template-wrappers-only]]
