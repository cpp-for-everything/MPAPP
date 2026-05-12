# MPAPP Vault — Welcome

This is the knowledge base for **MPAPP**, a multi-year project to build a C++ cross-platform UI framework analogous to .NET MAUI with stricter compile-time type safety.

**If you are an AI agent, read [[CLAUDE]] first** — it is the authoritative contract for vault conventions and project rules.

**If you are a human contributor**, this README is your quick-start.

---

## What's where

| Folder | Purpose |
|---|---|
| `00_Index/` | Start here: [[00_Index/Home\|Home]], [[00_Index/Current Focus\|Current Focus]] |
| `10_Architecture/` | Per-subsystem technical design |
| `10_Architecture/Components/` | One note per MAUI control (the porting work surface) |
| `20_ADRs/` | Architecture Decision Records (immutable once accepted) |
| `30_RFCs/` | Proposals under discussion |
| `40_Roadmap/` | Milestones organized into phases |
| `50_Tasks/` | Work items (each task is a **folder** with screenshots, recordings, logs, tests, notes) |
| `60_Research/` | Comparative analysis of other frameworks (MAUI, Qt, Slint, JUCE, …) |
| `70_References/` | External links, third-party dependency tracking |
| `80_Glossary/` | Term definitions (heavily wikilinked) |
| `90_Logs/` | Weekly notes, decision log |
| `_Bases/` | Live filtered tables (database views over the vault) |
| `_Canvases/` | Diagrams (architecture, build graph, parity matrix) |
| `_Templates/` | Note templates (use Templater) |

---

## How to use this vault

1. **Open in Obsidian.** Point Obsidian at `D:\GitHub\MPAPP\vault\` as the vault root. Plugins are pre-configured (Bases, Templater, Tasks, Excalidraw).
2. **Read [[CLAUDE]].** Even if you're a human — it's the rule book.
3. **Read [[00_Index/Home]].** It has live dashboards (embedded bases) showing what's in flight.
4. **Read [[00_Index/Current Focus]].** It's a one-paragraph "what we're working on this week."

---

## Key project decisions (so far)

Nine ADRs accepted on day 1 (2026-05-12) — see [[_Bases/ADRs.base]] for the live list. Headlines:

- C++23 baseline ([[ADR-0001-cpp-standard-baseline]])
- **No macros in public API** ([[ADR-0002-no-macros-in-public-api]])
- Markup is **XAML only**, full MAUI compatibility ([[ADR-0003-xaml-only-no-custom-dsl]], [[ADR-0004-maui-xaml-superset-compat]])
- iOS and macOS are **separate** (UIKit + AppKit, no Catalyst) ([[ADR-0005-ios-macos-separate-interop]])
- **Interop parity** — every feature works on every platform ([[ADR-0006-interop-parity]])
- All tooling is **cross-platform** ([[ADR-0007-cross-platform-tooling]])
- **Mock-first** implementation: full API surface before any real platform code ([[ADR-0008-mock-first-implementation]])
- Public-API mechanism: **template wrapper types only** ([[ADR-0009-public-api-template-wrappers-only]])

Two RFCs open: licensing + patent strategy ([[RFC-0001-licensing-and-patent-strategy]]) and cross-compilation toolchain ([[RFC-0002-cross-compilation-toolchain]]).

---

## Setting up Obsidian

Plugins to enable (already configured in `.obsidian/`):

- **Bases** (core) — live database views
- **Canvas** (core) — visual diagrams
- **Templater** (community) — dynamic templates with date/ID auto-fill
- **Tasks** (community) — inline task syntax with queries
- **Excalidraw** (community) — freeform sketches

On first open, Obsidian may prompt to install the community plugins. Accept.

---

## Conventions (TL;DR)

- **No macros in public API.**
- **No time estimates.** Order + exit criteria only.
- **Accepted ADRs are immutable.** Supersede them.
- **Tasks are folders.** Each contains screenshots, recordings, logs, tests, notes.
- **100% coverage** required to close a task.
- **MAUI source is the spec** — when in doubt, check `D:\GitHub\MPAPP\maui\src\`.

Full rule book: [[CLAUDE]].
