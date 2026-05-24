---
type: glossary
term: "Virtual View"
tags:
  - type/glossary
---

# Virtual View

The cross-platform MPAPP control (e.g. `mpapp::button`) — the API users program against. Opposite of [[Native View]]. See [[Handlers]].

## See in code

- [`include/mpapp/button.hpp`](../../include/mpapp/button.hpp) — `class button` is the virtual view: pure-C++ public surface with `Observable<std::string> text{}` + `signal<> clicked`. Knows nothing about platforms; pairs with a handler via `set_handler(...)`.
- [`include/mpapp/`](../../include/mpapp/) — one virtual-view header per component; the user instantiates these from their app code.
- Example: [`examples/windows_button_spike/main.cpp`](../../examples/windows_button_spike/main.cpp) constructs an `mpapp::button` + an `mpapp::label` and binds them — the same view source compiles unchanged on Linux + Android with different handler template-args.
