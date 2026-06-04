# MPAPP

A multi-year project to build a **C++ cross-platform UI framework** analogous to .NET MAUI — with stricter compile-time type safety, native components per platform, optional XAML markup, and a roadmap toward eventual commercialization.

## Where to start

**This repository's authoritative documentation lives in the [`vault/`](vault/) folder** — an Obsidian vault containing every decision, architectural note, roadmap milestone, and component specification.

- **Human contributors:** open [`vault/README.md`](vault/README.md) and [`vault/00_Index/Home.md`](vault/00_Index/Home.md) in Obsidian.
- **AI agents:** read [`vault/CLAUDE.md`](vault/CLAUDE.md) first — it is the project rule book.

## Setup

1. Install [Obsidian](https://obsidian.md).
2. Open `D:\GitHub\MPAPP\vault\` as a vault (or the equivalent path on your machine).
3. On first open, Obsidian will prompt to install the three community plugins enabled in this vault: **Templater**, **Tasks**, **Excalidraw**. Accept.
4. Read `vault/CLAUDE.md`.

## Repository layout

| Path | Contents |
|---|---|
| `src/` | Portable `mpapp-core` library + per-platform native handlers (`src/handlers/<platform>/`) |
| `include/` | Public headers (`mpapp/...`) |
| `examples/` | Platform spike apps — WinUI 3 (`windows_*`), GTK4 (`gtk4_*`), the УИСС reference app, headless demos |
| `tests/` | Catch2 unit tests (run via CTest) |
| `tools/` | Host developer tools — `mpapp` CLI, `mpapp-xc` XAML compiler |
| `cmake/` | All build logic: superbuild orchestrator, target helpers, Zig toolchains |
| `vault/` | Obsidian knowledge base — all design, decisions, roadmap, components, tasks |
| `references/` | Submodules (research-only, not build deps): [dotnet/maui](https://github.com/dotnet/maui), [syncfusion/maui-toolkit](https://github.com/syncfusion/maui-toolkit), [CommunityToolkit/Maui](https://github.com/CommunityToolkit/Maui), [CommunityToolkit/dotnet](https://github.com/CommunityToolkit/dotnet) |

## Building

The build is a CMake **superbuild**: one configure builds every target the host
supports (portable core for each platform via [Zig](https://ziglang.org), plus
the native example apps with the host SDK). No `.bat` scripts, no WSL — it runs
the same on Linux, macOS, and Windows.

```sh
# Build every host-supported platform into build/<platform>/
cmake --preset all
cmake --build build

# Or just one platform's real native build (core + tests + tools + examples):
cmake --preset host
cmake --build build/host
```

Pass `--preset windows-only` / `linux-only` to restrict the cross matrix. Under
the hood each platform is a *child* build (`-DMPAPP_SUPERBUILD=OFF`); invoke that
directly when you want a single tree. See
[`vault/10_Architecture/Build System.md`](vault/10_Architecture/Build%20System.md)
and [`cmake/toolchains/README.md`](cmake/toolchains/README.md).

### Quality gates

Opt-in presets run sanitizers, coverage, and clang-tidy over the portable core
+ tests (native compiler — Clang/GCC). Off in normal builds.

```sh
cmake --preset ubsan && cmake --build build/ubsan && ctest --preset ubsan
cmake --preset asan  && cmake --build build/asan  && ctest --preset asan
cmake --preset tsan  && cmake --build build/tsan  && ctest --preset tsan
cmake --preset coverage && cmake --build build/coverage && ctest --preset coverage   # gcov
cmake --preset tidy  && cmake --build build/tidy        # clang-tidy (report-only)
```

`asan`/`ubsan`/`tidy` also run in CI (`linux-quality` job). The gate machinery
lives in [`cmake/MpappHardening.cmake`](cmake/MpappHardening.cmake); tidy checks
in [`.clang-tidy`](.clang-tidy).

## Status

See [`vault/00_Index/Current Focus.md`](vault/00_Index/Current%20Focus.md) for live status.

## License

To be decided (see [`vault/30_RFCs/RFC-0001-licensing-and-patent-strategy.md`](vault/30_RFCs/RFC-0001-licensing-and-patent-strategy.md)). Likely **Apache 2.0 + commercial dual license** with a contributor agreement.
