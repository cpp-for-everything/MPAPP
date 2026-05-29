---
type: rfc
id: RFC-0012
title: Fonts — font descriptor + alias registry (ConfigureFonts)
status: accepted
author: Alex Tsvetanov
created: 2026-05-29
area: widgets
relatedADRs:
  - ADR-0008
  - ADR-0009
tags:
  - type/rfc
  - status/accepted
  - area/widgets
---

# RFC-0012 — Fonts

> [!info] Status
> **accepted** — mock surface shipped under [[T-0056-fonts]]. Per-platform font-file → native-typeface loading is a follow-up.

## Problem

Controls carried ad-hoc font properties (a family string, a size double) with no shared `Font` type and no font registry. MAUI centralizes this: a `Font` struct (family / size / attributes) + `ConfigureFonts(b => b.AddFont("OpenSans.ttf", "OpenSans"))` registering embedded fonts under aliases.

## Proposal

`include/mpapp/fonts/`, header-only, no macros:

1. **`font`** (`font.hpp`) — value type: `family`, `size`, `weight` (100..900 via `font_weight::*` constants), `slant` (`font_slant`). Builder helpers (`of_size`, `with_weight`/`with_slant`/`with_size`) + `is_bold()` / `is_italic()` predicates (MAUI's FontAttributes). `operator==` defaulted.
2. **`font_registry`** (`font_registry.hpp`) — `add_font(filename, alias)` (fluent), `resolve(alias)` → optional filename, `has_alias`, `count`. + `configure_fonts(registry, fn)` sugar matching MAUI's `ConfigureFonts` callback shape.

`font::family` is either a system family name or a registered alias; the per-platform font loader (follow-up) resolves an alias's file to a native typeface (CoreText / DirectWrite / Pango / Android Typeface).

## Detailed design

`font` is a plain aggregate-ish struct with const builder methods returning modified copies (original immutable). `font_registry` is an `unordered_map<alias, filename>`; re-registering an alias overwrites. `configure_fonts` just invokes the callable with the registry (so app bootstrap reads like MAUI).

### Tests (mock-first)

`tests/mock_handlers/font_test.cpp` — 5 cases / 29 assertions: builder helpers + immutability + bold/italic predicates, equality, registry add/resolve/overwrite/miss, and `configure_fonts`.

## Alternatives

- **`FontAttributes` bit-enum (None/Bold/Italic)** like MAUI. Used a numeric `weight` + `slant` instead — strictly more expressive (medium/semibold/black, oblique) and still answers `is_bold()`/`is_italic()`.

## Open Questions

> [!todo] Open
> - [ ] Per-platform font loader: alias/file → native typeface (DirectWrite / Pango / Android Typeface / CoreText).
> - [ ] Wire `font` into the control text surfaces (replace ad-hoc family/size Observables with a single `Observable<font>`).
> - [ ] `<font>` / FontImageSource glyph cross-ref (RFC-0004 already has `font_image_source`).

## Migration / Compatibility

Pure addition; no existing surface modified. Wiring `font` into controls is a follow-up that will deprecate the ad-hoc font Observables.

## References

- [[RFC-0004-image-source-family]] (`font_image_source` glyph sibling), [[ADR-0009-public-api-template-wrappers-only]].
- `references/maui/src/Graphics/src/Graphics/Font.cs`, `Microsoft.Maui.Hosting.FontCollectionExtensions` (ConfigureFonts/AddFont).
