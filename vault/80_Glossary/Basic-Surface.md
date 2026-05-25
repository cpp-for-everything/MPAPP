---
type: glossary
term: "Basic Surface"
tags:
  - type/glossary
---

# Basic Surface

The platform-agnostic class for each MPAPP component, named `mpapp::internal::basic_<name>` and declared in `include/mpapp/internal/basic_<name>.hpp`. Owns the component's `Observable<T>` properties + `mpapp::signal<...>` events, inherits the CRTP base (`control<basic_<name>>` or `view`), and holds the platform handler **by pointer** so the surface's translation unit has no ODR-use of platform handler symbols.

Tests construct the surface directly and attach a handler externally; they do not link the per-platform handler library:

```cpp
mpapp::internal::basic_button b;
mpapp::button_handler<mpapp::platform::mock> h;
h.map_text(b);
b.text = "hello";
REQUIRE(h.calls_as_strings() == std::vector<std::string>{"text=", "text=hello"});
```

The pointer-typed handler is what preserves the [[ADR-0008-mock-first-implementation]] link-isolation contract: `mock_handlers_test` builds against `libmpapp-core.a` only, with no `libmpapp-handlers-linux.a` (or Windows/Android cousin) on its link line.

Public nested types (`color`, `corner_radius`, `shape_kind`, `scroll_orientation`, …) stay in `mpapp::` namespace — only the class itself moves to `mpapp::internal::`. The split is surgical so `std::formatter<mpapp::color>` specialisations and external user code referencing the public types continue to compile.

Introduced by [[ADR-0024-wrapper-component-pattern]] as the half of the wrapper/surface split that preserves mock-first testability. Every migrated leaf component has a surface; the [[Wrapper-Component]] layer above it adds the auto-binding ergonomics.

## See in code

- Canonical surface: [`include/mpapp/internal/basic_button.hpp`](../../include/mpapp/internal/basic_button.hpp) — `class basic_button : public control<basic_button>` with `Observable<std::string> text{}`, `signal<> clicked`, `button_handler<platform::current>* handler_ = nullptr`, and the `handler() / has_handler() / set_handler()` accessor trio.
- Directory: [`include/mpapp/internal/`](../../include/mpapp/internal/) — every migrated component's surface header lives here. 58 files as of [[ADR-0024-wrapper-component-pattern]].
- Per-platform handler include path: handlers `#include "../../internal/basic_<name>.hpp"` (not the wrapper) so the surface-only include chain stays acyclic.

## See also

- [[Wrapper-Component]] — the user-facing class that inherits the surface and embeds the handler.
- [[Component]] — the two-layer split as a whole.
- [[Handler]] — what the surface's pointer points at.
- [[Mock-Implementation]] — the testing pattern that exploits the pointer-typed handler.
- [[ADR-0024-wrapper-component-pattern]] — the design decision.
- [[ADR-0008-mock-first-implementation]] — the link-isolation contract.
