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
| `vault/` | Obsidian knowledge base — all design, decisions, roadmap, components, tasks |
| `maui/` | Submodule: [dotnet/maui](https://github.com/dotnet/maui) — the spec MPAPP mirrors |
| `maui-toolkit/` | Submodule: [syncfusion/maui-toolkit](https://github.com/syncfusion/maui-toolkit) |
| `maui-community-toolkit/` | Submodule: [CommunityToolkit/Maui](https://github.com/CommunityToolkit/Maui) |
| `dotnet-community-toolkit/` | Submodule: [CommunityToolkit/dotnet](https://github.com/CommunityToolkit/dotnet) |

No C++ source code yet — that lands in phase P1 ([`vault/40_Roadmap/M-02-Infrastructure.md`](vault/40_Roadmap/M-02-Infrastructure.md)).

## Status

**Phase P0 — Foundations** (active). 9 ADRs accepted on day 1; 2 RFCs open; 56 MAUI components stubbed. See [`vault/00_Index/Current Focus.md`](vault/00_Index/Current%20Focus.md) for live status.

## License

To be decided (see [`vault/30_RFCs/RFC-0001-licensing-and-patent-strategy.md`](vault/30_RFCs/RFC-0001-licensing-and-patent-strategy.md)). Likely **Apache 2.0 + commercial dual license** with a contributor agreement.
