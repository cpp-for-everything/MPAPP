---
type: milestone
id: M-09
title: Tooling and developer experience polish
phase: P8
status: planned
deliverables:
  - VS Code extension stabilized
  - Visual Studio MSBuild integration for parity with MAUI users
  - XAML hot-reload UX polish across all platforms
  - LSP for XAML files surfacing mpapp-xc diagnostics
exitCriteria:
  - "VS Code extension installable from the marketplace"
  - "Visual Studio project template provided"
  - "XAML hot-reload roundtrip < 500ms on desktop"
tags:
  - type/milestone
  - phase/p8
  - status/planned
  - area/tooling
---

# M-09 — Tooling & Developer Experience

> [!info] Status
> **planned**.

## Scope

Polish the developer experience now that all five platforms ship. VS Code first per [[ADR-0007-cross-platform-tooling]]; Visual Studio for MAUI-user familiarity. CLion is explicitly **out of scope**.

## Exit Criteria

- [ ] VS Code extension on the marketplace: XAML IntelliSense, hot-reload trigger, build / debug integration.
- [ ] Visual Studio project template + MSBuild integration.
- [ ] XAML hot-reload UX polish: save → swap in < 500ms on desktop.
- [ ] LSP server for `.xaml` files (clangd-style) surfacing `mpapp-xc` diagnostics with source spans.

## Risks

> [!warning]
> - VS Code extension API surface evolves quickly. Pin and version.
> - XAML LSP is its own bus factor — invest in test coverage.

## Tasks

Linked via [[_Bases/Tasks.base]].

## See in code

- Developer CLI (existing skeleton): [`tools/mpapp/`](../../tools/mpapp/) — extension surface for `new` / `build` / `run` / `xc` subcommands.
- XAML compiler (existing skeleton): [`tools/mpapp-xc/`](../../tools/mpapp-xc/) — the polish target for IntelliSense / source-mapped diagnostics / hot-reload trigger.
- Hot-reload daemon (existing seed): [`include/mpapp/hot_reload.hpp`](../../include/mpapp/hot_reload.hpp) + [`src/hot_reload/windows.cpp`](../../src/hot_reload/windows.cpp) (Windows-only today; Linux + macOS bodies are M-09 work).
- VS Code extension + Visual Studio template do not exist yet — both are M-09 deliverables. Their eventual homes are `tools/vscode-extension/` and `tools/visual-studio-template/`.

## Related

- [[Hot Reload]]
- [[Markup]]
- [[ADR-0007-cross-platform-tooling]]
