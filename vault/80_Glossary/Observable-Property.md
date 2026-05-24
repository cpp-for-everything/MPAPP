---
type: glossary
term: "Observable Property"
tags:
  - type/glossary
---

# Observable Property

A wrapper type `Observable<T>` that exposes get/set semantics and fires a change signal on mutation. The foundation of MPAPP's binding system. See [[Observable Properties]].

## See in code

- [`include/mpapp/observable.hpp`](../../include/mpapp/observable.hpp) — the template itself: `get()`, `set(...)`, and the `changed` signal handle. Short-circuits on `==` so a same-value set does not fire.
- [`include/mpapp/signal.hpp`](../../include/mpapp/signal.hpp) — the `signal<Args...>` + `signal_slot` pair that backs `Observable<T>::changed`.
- [`include/mpapp/button.hpp`](../../include/mpapp/button.hpp) — canonical usage: `Observable<std::string> text{}` is one of the bindable surface members; every component header follows the same pattern.
