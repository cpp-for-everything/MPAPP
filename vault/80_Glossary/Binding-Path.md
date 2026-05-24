---
type: glossary
term: "Binding Path"
tags:
  - type/glossary
---

# Binding Path

A compile-time-resolved chain of member accesses connecting a source observable to a target property. Type-checked at compile time. See [[Type System]].

## See in code

- [`include/mpapp/computed.hpp`](../../include/mpapp/computed.hpp) — the `Computed<&VM::a, &VM::b>` tag that carries the member-pointer pack used to encode a binding path.
- [`include/mpapp/observable.hpp`](../../include/mpapp/observable.hpp) — `Observable<T>::changed.subscribe(...)` is the receiving end; bindings install a subscription per source observable.
- [`tests/template_type_spike/test.cpp`](../../tests/template_type_spike/test.cpp) — exercises the type-system invariants the binding-path machinery rests on.
