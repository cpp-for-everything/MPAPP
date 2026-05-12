---
type: moc
---

# Templates

This folder contains the canonical note templates. Use Obsidian's Templater plugin (already enabled) to instantiate new notes from these.

## Templates

- [[ADR]] — Architecture Decision Record
- [[RFC]] — Request For Comments (proposal under discussion)
- [[Task]] — Granular work item (folder, see [[50_Tasks/README]])
- [[Milestone]] — Phase milestone
- [[Research]] — External-framework comparative analysis
- [[Component]] — Per-MAUI-control documentation note

## Templater patterns

Templater expressions used in the templates:

- `<% tp.date.now("YYYY-MM-DD") %>` — today's date, ISO-8601
- `<% tp.file.title %>` — the new note's filename

## ID assignment

IDs are zero-padded numerics, assigned by the next-free number in their respective folder:

| Type | Pattern | Padding |
|---|---|---|
| ADR | `ADR-NNNN-<slug>.md` | 4 digits |
| RFC | `RFC-NNNN-<slug>.md` | 4 digits |
| Task | `T-NNNN-<slug>/` (folder) | 4 digits |
| Milestone | `M-NN-<slug>.md` | 2 digits |

Once assigned, IDs never change. Supersession of ADRs uses `supersedes:` / `supersededBy:` frontmatter, never renumbering.

## Frontmatter contract

All templates use:
- **camelCase** property names
- **ISO-8601** dates (`YYYY-MM-DD`)
- **Empty list `[]`**, never null or missing
- **Fixed enum values** from the canonical tag taxonomy (see [[CLAUDE]])

Bases query the frontmatter; tags exist for graph view and search.
