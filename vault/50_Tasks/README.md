---
type: moc
area: process
tags:
  - area/process
---

# Tasks

Each task is a **folder** containing the task description plus all related artifacts.

## Layout

```
50_Tasks/
├── README.md                       (this file)
├── T-NNNN-<slug>/
│   ├── T-NNNN-<slug>.md            (task description with frontmatter)
│   ├── screenshots/                (visual proof of working implementation)
│   ├── recordings/                 (screen recordings / demos)
│   ├── logs/                       (build logs, test output)
│   ├── tests/                      (unit test files linked from the description)
│   └── notes/                      (working notes, dead-ends, decisions)
└── _Archive/
    └── T-NNNN-<slug>/               (closed tasks; same internal structure)
```

The leading `_` on `_Archive/` keeps it sorted last in Obsidian's file tree.

## Closure gate (CLAUDE rule 11)

A task only moves from `in-progress` → `done` when all of these are true:

1. **`coveragePercent: 100`** — line + branch coverage of all new code.
2. **`hasScreenshots: true`** if the change has any visible UI effect.
3. **`hasRecordings: true`** for flows hard to capture in a static image.
4. **Test files exist in `tests/`** and are linked from the task description.

On close:

```bash
git mv 50_Tasks/T-NNNN-slug/ 50_Tasks/_Archive/T-NNNN-slug/
```

The active task list ([[_Bases/Tasks.base]]) stays focused on open work. The archive ([[_Bases/Archive.base]]) is read-only history.

## Bases

- [[_Bases/Tasks.base]] — open tasks grouped by area, with `coveragePercent` and `hasScreenshots` columns.
- [[_Bases/Blockers.base]] — tasks where `blockedBy.length() > 0`.
- [[_Bases/Archive.base]] — closed tasks.

## Conventions

- ID is **next-free integer**, 4-digit zero-pad.
- Slug is **kebab-case**, short, descriptive (`cmake-skeleton`, `template-type-spike`).
- Task folder name and inner `.md` file share the same `T-NNNN-<slug>` stem.
- One task per PR (or one PR per task). Don't bundle.
- A task belongs to **exactly one milestone**.
- A task has **at most one owner**; team work is split into multiple tasks.
