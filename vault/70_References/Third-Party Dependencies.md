---
type: reference
subject: Third-Party Dependencies Registry
tags:
  - type/reference
  - area/legal
---

# Third-Party Dependencies

Per [[CLAUDE]] rule 9 and [[RFC-0001-licensing-and-patent-strategy]], every third-party dependency MPAPP uses must appear in this table with: license, version, linking model, and a posture note.

> [!warning] License vigilance
> Adding a dependency without a row here is a CI failure (planned). Forgetting is the most common way license trouble starts.

## Current dependencies

| Dependency | Version | License | Linking | Posture | Used for |
|---|---|---|---|---|---|
| Catch2 (or GoogleTest) | TBD | BSL-1.0 / BSD-3 | static | ✅ test only | unit tests |
| fbjni | TBD | Apache 2.0 | static | ✅ permissive | Android JNI wrapper |
| C++/WinRT | TBD | MIT | header-only | ✅ permissive | Windows WinUI 3 interop |
| GTK4 | 4.x | LGPL-2.1+ | **dynamic only** | ⚠️ acceptable with rebuild path | Linux UI |
| libclang | LLVM toolchain | Apache 2.0 with LLVM exceptions | static | ✅ permissive | (only if Option B were chosen — currently unused per ADR-0009) |
| libxml2 | TBD | MIT | static | ✅ permissive | XAML parsing in `mpapp-xc` |
| Zig toolchain | TBD | MIT | external tool | ✅ permissive | cross-compilation (per RFC-0002) |
| Android NDK | TBD | Apache 2.0 | external tool | ✅ permissive | Android targets |

(Rows above are *planned* — added when actually integrated.)

## License categories

| Category | Examples | MPAPP policy |
|---|---|---|
| Permissive (Apache 2.0, MIT, BSD, ISC, BSL) | Catch2, fbjni, C++/WinRT, libxml2, LLVM | ✅ Use freely |
| Weak copyleft (LGPL, MPL) | GTK4 | ⚠️ Dynamic linking only. Publish rebuild instructions per LGPL §4. |
| Strong copyleft (GPL, AGPL) | — | ❌ Forbidden as runtime dependency |
| Source-available / commercial | Qt commercial, Live++ | ❌ Forbidden unless re-licensed or replaced |
| Public domain / CC0 | Dear ImGui (MIT in practice) | ✅ Use freely (but verify CLA-compatible if vendored) |

## When you add a new dependency

1. Add a row to this table.
2. Cite the dependency's license URL in the PR.
3. Specify linking model — static, dynamic, header-only, external tool.
4. Note any **patent grant** the license includes (Apache 2.0 has explicit; MIT does not).
5. If the license is weak copyleft, ensure dynamic linking and document the rebuild path.

## See also

- [[CLAUDE]] rule 9
- [[RFC-0001-licensing-and-patent-strategy]]
