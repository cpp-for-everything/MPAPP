# CLAUDE.md — MPAPP Vault Agent Contract

This file is the **first thing any AI agent or new contributor reads** when working in the MPAPP vault. It is the authoritative source for vault conventions, frontmatter contracts, and the cross-cutting rules every change must obey.

If anything in this file conflicts with another note in the vault, **this file wins** until updated.

---

## What is this vault?

The Obsidian vault at `D:\GitHub\MPAPP\vault\` is the knowledge base for **MPAPP**, a multi-year project to build a C++ cross-platform UI framework analogous to .NET MAUI with stricter compile-time type safety.

The MAUI source at `D:\GitHub\MPAPP\maui\` is the **authoritative specification** for the component surface MPAPP must mirror. The deep-dive analysis is at [[60_Research/dotnet-maui-deep-dive]].

---

## The 12 Rules

These rules apply to every PR, every note, every line of code. Violating them is a blocker.

### Rule 1 — No macros in the public API

Per [[ADR-0002-no-macros-in-public-api]] and [[ADR-0009-public-api-template-wrappers-only]]:

- User-facing surface uses **template wrapper types** (`Observable<T>`, `Computed<...>`, `Command<>`).
- No `MPAPP_*(...)` macros in any header a user includes.
- **Internal preprocessor guards** (`#if MPAPP_ANDROID`, build-time flags) are exempt — they're not public API.

### Rule 2 — Interop parity

Per [[ADR-0006-interop-parity]]:

- Every public MPAPP feature works on **every** supported platform (Windows, Android, Linux, macOS, iOS) with equivalent observable behavior.
- Platform-only features live in `mpapp::platform::<name>::` and are documented in their component's "Platform Notes" section as a divergence.
- Every PR description must list which platforms it affects.

### Rule 3 — No time estimates, ever

- Roadmap milestones, tasks, and RFCs describe **order**, **dependencies**, and **exit criteria** — never weeks, months, or quarters.
- If a date appears in frontmatter, it is a *factual* date (decisionDate, created) — never a target.

### Rule 4 — No editing accepted ADRs

- Once `status: accepted`, an ADR is **immutable**.
- To change a decision, write a new ADR with `supersedes: ADR-NNNN` and set the old ADR's `supersededBy:` field.
- Never rewrite history; supersede it.

### Rule 5 — Component work tracked in the inventory

- Any change to a control or handler updates [[10_Architecture/Controls Inventory]] with the new porting status.
- Update the per-component doc at `10_Architecture/Components/<Name>.md` simultaneously — its frontmatter must reflect reality.

### Rule 6 — Mock before real

Per [[ADR-0008-mock-first-implementation]]:

- A new control lands as a **mock C++ class** first (full public API, dummy handler, mock-based tests).
- Real platform implementations are **separate PRs** per platform.
- The porting-status state machine is `not-started → mock → <platform>-real → parity-complete`.

### Rule 7 — MAUI is the spec

- Behavior questions resolve against `D:\GitHub\MPAPP\maui\src\` first.
- Then [[60_Research/dotnet-maui-deep-dive]].
- Then Microsoft's official MAUI docs.
- Then this vault's component notes.

### Rule 8 — CI budget awareness

- GitHub Actions minutes are finite. Don't add a CI matrix axis without thinking about it.
- Long jobs (Android emulator, macOS) go on **self-hosted runners** where possible.
- Per-PR builds shard aggressively; full matrix runs on tagged releases only.
- See [[10_Architecture/CI Strategy]].

### Rule 9 — License vigilance

Per [[RFC-0001-licensing-and-patent-strategy]]:

- Every new third-party dependency requires a row in [[70_References/Third-Party Dependencies]] with: license, version, linking model, posture.
- **No GPL** runtime dependencies.
- **LGPL** allowed only with dynamic linking and a published rebuild path.
- Patent prior-art is audited before any filing (RFC-0001 §4).

### Rule 10 — Read `Current Focus.md` first

- Start every session by reading [[00_Index/Current Focus]] to orient.
- Then read the active milestone (linked from Current Focus).
- Only then start work.

### Rule 11 — Task closure gate

- A task moves from `in-progress` → `done` only when **all** of these are true:
  1. `coveragePercent: 100` (line + branch coverage of new code).
  2. `hasScreenshots: true` if the change has visible UI effect.
  3. `hasRecordings: true` for hard-to-capture flows.
  4. Test files exist in `tests/` inside the task folder and are linked from the task description.
- On close: `git mv 50_Tasks/T-NNNN-slug/ 50_Tasks/_Archive/T-NNNN-slug/`.

### Rule 12 — Cross-platform tooling

Per [[ADR-0007-cross-platform-tooling]]:

- Every tool MPAPP ships (`mpapp` CLI, `mpapp-xc` XAML compiler, `mpapp-jni-gen`, LSP, hot-reload daemon) runs on **Windows, macOS, and Linux**.
- Cross-compilation: any host produces binaries for any target (with Apple signing requiring macOS — see [[10_Architecture/Build System]]).

---

## Folder map

| Folder | Purpose |
|---|---|
| `00_Index/` | Entry points: [[00_Index/Home\|Home]], [[00_Index/Current Focus\|Current Focus]], MOCs |
| `10_Architecture/` | Per-subsystem technical design notes |
| `10_Architecture/Components/` | One note per MAUI control (full per-component docs) |
| `20_ADRs/` | Architecture Decision Records — numbered, immutable once accepted |
| `30_RFCs/` | Proposals under discussion |
| `40_Roadmap/` | Phases and milestones (no dates — see Rule 3) |
| `50_Tasks/` | Granular work items, each is a folder with screenshots/recordings/logs/tests/notes |
| `50_Tasks/_Archive/` | Closed tasks |
| `60_Research/` | Comparative research on MAUI, Qt, Slint, JUCE, etc. |
| `70_References/` | External-link stubs; third-party dependency tracking |
| `80_Glossary/` | One file per term, heavily wikilinked |
| `90_Logs/` | Weekly notes, decision log |
| `_Assets/` | Images, canvas attachments |
| `_Bases/` | All `.base` files (live filtered tables) |
| `_Canvases/` | All `.canvas` files (visual diagrams) |
| `_Templates/` | Note templates (use via Templater) |

The leading `_` prefix on `_Assets/_Bases/_Canvases/_Templates` is intentional — Obsidian's file explorer sorts symbols last, keeping the working areas on top.

---

## Frontmatter contract

All notes use **camelCase** property names, **ISO-8601** dates (`YYYY-MM-DD`), and `[]` for empty lists (never null or missing). Enum values come from the fixed taxonomy below.

### Required properties per `type`

| `type` | Required fields |
|---|---|
| `adr` | `id, title, status, decisionDate, deciders, supersedes, supersededBy, area` |
| `rfc` | `id, title, status, author, created, area, relatedADRs` |
| `task` | `id, title, status, milestone, owner, area, blockedBy, coveragePercent, hasScreenshots, hasRecordings` |
| `milestone` | `id, title, phase, status, deliverables, exitCriteria` |
| `research` | `subject, framework, created, applicableTo, recommendation` |
| `component` | `mauiHandler, mauiDocUrl, mpappStatus, platformWindows, platformAndroid, platformLinux, platformMacos, platformIos` |
| `moc`, `glossary`, `log` | `type` only |

Property names are **case-sensitive**. Do not invent new property names without an RFC.

---

## Tag taxonomy

Tags mirror frontmatter status/area/phase. They exist for graph view and full-text search; **bases query frontmatter**, not tags.

| Namespace | Values |
|---|---|
| `type/` | `adr, rfc, task, milestone, research, component, moc, glossary, log` |
| `status/` | `draft, proposed, review, accepted, rejected, superseded, withdrawn, todo, in-progress, blocked, done, abandoned, planned, active, shipped, dropped, not-started, mock, windows-real, android-real, linux-real, macos-real, ios-real, parity-complete` |
| `area/` | `type-system, build, handlers, markup, threading, properties, widgets, tooling, docs, legal, process` |
| `phase/` | `p0, p1, p2, p3, p4, p5, p6, p7, p8, p9` |
| `framework/` (research only) | `maui, qt, slint, juce, avalonia, imgui, flutter, sciter, wxwidgets, wpf` |
| `platform/` | `windows, android, linux, macos, ios` |

---

## State machines

```
ADR:        proposed → accepted | rejected → superseded
RFC:        draft → review → accepted | rejected | withdrawn
Task:       todo → in-progress → done | blocked | abandoned
            (done triggers _Archive move per Rule 11)
Milestone:  planned → active → shipped | dropped
Porting:    not-started → mock → <platform>-real → parity-complete
```

---

## Naming conventions

- **Prose notes**: Title Case (`Type System.md`, `Build System.md`).
- **ADRs**: `ADR-NNNN-<kebab-slug>.md` (4-digit pad).
- **RFCs**: `RFC-NNNN-<kebab-slug>.md` (4-digit pad).
- **Tasks**: `T-NNNN-<kebab-slug>/` (folder, 4-digit pad).
- **Milestones**: `M-NN-<kebab-slug>.md` (2-digit pad).
- **Components**: `<MauiName>.md` (matches MAUI handler name verbatim — `Button.md`, `CollectionView.md`).
- **Weekly logs**: `YYYY-WNN-Weekly.md` (ISO week).

IDs are assigned by next-free number. Once assigned, IDs never change. Supersession uses `supersedes:` / `supersededBy:` — never renumber.

---

## Bases are the source of truth

Bases (`_Bases/*.base`) are live, queryable views over the vault's frontmatter. **Prefer reading a base over hand-walking the file tree.**

| Base | What it shows |
|---|---|
| [[_Bases/ADRs.base\|ADRs.base]] | All ADRs, grouped by status |
| [[_Bases/Roadmap.base\|Roadmap.base]] | Milestones grouped by phase |
| [[_Bases/Tasks.base\|Tasks.base]] | Open tasks grouped by area |
| [[_Bases/Blockers.base\|Blockers.base]] | Tasks where `blockedBy` is non-empty |
| [[_Bases/RFCs.base\|RFCs.base]] | Active RFCs grouped by status |
| [[_Bases/Research.base\|Research.base]] | Research notes grouped by framework |
| [[_Bases/Components.base\|Components.base]] | Per-component docs grouped by `mpappStatus` |
| [[_Bases/Archive.base\|Archive.base]] | Closed tasks |

---

## Linking conventions

- **First mention** of any architectural concept in a note is a wikilink (`[[Handler]]`).
- **Glossary terms** are always wikilinked.
- Quote other notes via **embeds** (`![[ADR-0005-ios-macos-separate-interop#Decision]]`) — never copy-paste.
- Markdown links (`[text](url)`) are for **external URLs only**.

---

## When asked to do X, do this

| Request | Action |
|---|---|
| "Record a new decision" | New file in `20_ADRs/` from [[_Templates/ADR\|ADR template]]; `status: proposed`. Link from the originating RFC if any. |
| "Open a proposal" | New file in `30_RFCs/` from [[_Templates/RFC\|RFC template]]; `status: draft`. |
| "Start a task" | New folder `50_Tasks/T-NNNN-slug/` with `T-NNNN-slug.md` from [[_Templates/Task\|Task template]] + empty subfolders `screenshots/ recordings/ logs/ tests/ notes/`. |
| "Add a new control" | (Rule 6) Mock first. Update [[Controls Inventory]] and create/update `10_Architecture/Components/<Name>.md`. |
| "What's current?" | Read [[00_Index/Current Focus]]. |
| "Show all blocked work" | Open [[_Bases/Blockers.base]]. |

---

## Don't do this

- Don't write user-facing macros. (Rule 1)
- Don't estimate time. (Rule 3)
- Don't edit an accepted ADR. (Rule 4)
- Don't merge a task without 100% coverage + screenshots/recordings. (Rule 11)
- Don't add a CI matrix axis without checking the budget. (Rule 8)
- Don't add a third-party dep without a license row. (Rule 9)
- Don't invent frontmatter property names — propose them via RFC.
- Don't put time estimates in milestone descriptions. (Rule 3)

---

## References

- [[README\|Human-facing onboarding]]
- [[00_Index/Home]]
- [[00_Index/Current Focus]]
