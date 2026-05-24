---
type: glossary
term: "Hot Reload"
tags:
  - type/glossary
---

# Hot Reload

Mechanism for swapping running code or markup without restarting the app. LLVM-based for C++; `consteval`-tree swap for XAML. See [[Hot Reload]].

## See in code

- [`include/mpapp/hot_reload.hpp`](../../include/mpapp/hot_reload.hpp) — public surface for the hot-reload daemon connection (file watch + swap callbacks).
- [`src/hot_reload/windows.cpp`](../../src/hot_reload/windows.cpp) — Windows-side host implementation; Linux + macOS implementations land per the M-09 tooling milestone.
- [`vault/50_Tasks/_Archive/T-0010-hot-reload-spike/`](../50_Tasks/_Archive/T-0010-hot-reload-spike/) — the original feasibility spike (LLVM live-edit + XAML re-emit cycle).
