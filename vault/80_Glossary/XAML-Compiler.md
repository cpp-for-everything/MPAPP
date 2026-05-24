---
type: glossary
term: "XAML Compiler"
tags:
  - type/glossary
---

# XAML Compiler

`mpapp-xc` — the tool that translates XAML into `consteval` C++ struct trees. The only codegen step in the framework. See [[Markup]].

## See in code

- [`tools/mpapp-xc/`](../../tools/mpapp-xc/) — the compiler binary's CMakeLists + include tree.
- [`tools/mpapp/`](../../tools/mpapp/) — the `mpapp` CLI which wraps `mpapp-xc` invocation alongside other developer commands.
- [`include/mpapp/`](../../include/mpapp/) — the component headers `mpapp-xc` emits calls against; the emitted tree only uses the same public C++ API a hand-written UI would.
