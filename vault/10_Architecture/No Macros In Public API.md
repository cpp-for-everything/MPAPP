---
type: moc
area: type-system
tags:
  - area/type-system
---

# No Macros In Public API

This note is the **canonical statement** of CLAUDE rule 1. It is referenced by [[ADR-0002-no-macros-in-public-api]], [[ADR-0009-public-api-template-wrappers-only]], and [[CLAUDE]].

## The rule

> No public-facing MPAPP API may require users to write `MPAPP_*(...)` macros. User-visible reflection / observable / computed / command markers use **template wrapper types** ([[ADR-0009-public-api-template-wrappers-only]]).
>
> Internal-only conveniences (`#if MPAPP_ANDROID`, build-time preprocessor guards, debug-only `#define`s in `.cpp` files) are exempt — they are not public API.

## What counts as "public API"?

Any symbol that appears in a header file the user `#include`s. Specifically:

- Class names, member functions, member variables.
- Free functions in `mpapp::` namespaces.
- Template wrappers (`Observable<T>`, `Computed<...>`, `Command<>`).
- XAML markup extensions and their C++ counterparts.

Specifically *not* public API:

- Internal `.cpp` files.
- Build-system macros (`MPAPP_PLATFORM_WINDOWS`, `MPAPP_DEBUG`).
- Tests and examples (though we recommend examples avoid macros for consistency).

## Examples

### ✅ Allowed

```cpp
class todo_vm : mpapp::view_model {
    mpapp::Observable<int> count{0};
    void increment(mpapp::Command<> = {}) { /* ... */ }
};
```

```cpp
// Internal preprocessor guard inside an MPAPP source file:
#if MPAPP_PLATFORM_ANDROID
    // Android-only handler code
#endif
```

### ❌ Forbidden

```cpp
class todo_vm {
    MPAPP_OBSERVABLE(int, count);             // ❌ public-API macro
    MPAPP_COMMAND(void, increment, ());       // ❌ public-API macro
};
```

```cpp
[[mpapp::observable]] int count = 0;          // ❌ attribute approach, rejected by ADR-0009
```

## Why

- Better IDE experience: IntelliSense, refactoring, and "go to definition" work over types and members, not over macro expansion.
- Cleaner errors: compiler errors point at C++ code, not preprocessor output.
- Forward-compatible with [P2996 static reflection](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2996r1.html).
- One less learning curve for developers coming from Qt (`Q_OBJECT`, `moc`).

## Enforcement

- **PR review.** Any header that introduces a `MPAPP_*` macro must include a justification linking back to this note showing why it's internal-only.
- **CI lint** (planned T-task): a tool scans public headers (`include/mpapp/**/*.hpp`) for `#define MPAPP_*` and fails the build.
- **Documentation:** every component note (`10_Architecture/Components/*.md`) shows the C++ API in macro-free form.

## See in code

- The full [`include/mpapp/`](../../include/mpapp/) tree — every public header here is macro-free by design. Sample:
  - [`include/mpapp/observable.hpp`](../../include/mpapp/observable.hpp) — `Observable<T>` is the template wrapper that does what `MPAPP_OBSERVABLE(int, count)` would do via macro.
  - [`include/mpapp/computed.hpp`](../../include/mpapp/computed.hpp) — `Computed<&Ptrs...>` sentinel.
  - [`include/mpapp/command.hpp`](../../include/mpapp/command.hpp) — `Command<Args...>` sentinel.
- Internal preprocessor guards (allowed): [`src/hot_reload/windows.cpp`](../../src/hot_reload/windows.cpp) gates `#if MPAPP_WINDOWS`-style blocks. They never leak into public headers.
- Build-system controls (allowed): `MPAPP_GRAPHICS_BACKEND={cairo,skia,stub}` selects a graphics backend at configure time — see [`CMakeLists.txt`](../../CMakeLists.txt) and the resulting `MPAPP_GRAPHICS_HAS_*` defines surfaced to consumers via the `unofficial::skia::skia` imported target's `INTERFACE_COMPILE_DEFINITIONS`.

## Related

- [[ADR-0002-no-macros-in-public-api]]
- [[ADR-0009-public-api-template-wrappers-only]]
- [[Type System]]
- [[CLAUDE]] rule 1
