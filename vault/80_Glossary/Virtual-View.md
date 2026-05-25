---
type: glossary
term: "Virtual View"
tags:
  - type/glossary
---

# Virtual View

The cross-platform MPAPP control the user programs against. Opposite of [[Native View]]. See [[Handlers]].

Since [[ADR-0024-wrapper-component-pattern]] the virtual view exists in two layers:

- The [[Basic-Surface]] (`mpapp::internal::basic_<name>` in `include/mpapp/internal/basic_<name>.hpp`) — pure-C++ public surface with `Observable<T>` properties + `signal<...>` events, handler held by pointer. The platform-agnostic representation tests + handler call sites work against.
- The [[Wrapper-Component]] (`mpapp::<name>` in `include/mpapp/<name>.hpp`) — inherits the surface publicly and embeds the platform-current handler by value; its constructor auto-binds the handler. The class app code instantiates directly.

Both layers participate in the *virtual view* concept — they are the cross-platform half of the handler split.

## See in code

- [`include/mpapp/internal/basic_button.hpp`](../../include/mpapp/internal/basic_button.hpp) — `class basic_button : public control<basic_button>` is the surface: `Observable<std::string> text{}` + `signal<> clicked`. Knows nothing about platforms; pairs with a handler via `set_handler(...)`.
- [`include/mpapp/button.hpp`](../../include/mpapp/button.hpp) — `class button : public internal::basic_button` is the wrapper: same observables/signals via inheritance, plus an embedded `internal::button_handler<platform::current>` member auto-bound in the constructor.
- [`include/mpapp/`](../../include/mpapp/) — one wrapper header per component; [`include/mpapp/internal/`](../../include/mpapp/internal/) — one surface header per migrated component. The user instantiates the wrapper from their app code.
- Example: [`examples/gtk4_hello/main.cpp`](../../examples/gtk4_hello/main.cpp) constructs an `mpapp::button` + an `mpapp::label` and uses them directly — no `set_handler` / `map_<property>` calls; the same view source compiles unchanged on Windows + Android because `platform::current` resolves per build target.
