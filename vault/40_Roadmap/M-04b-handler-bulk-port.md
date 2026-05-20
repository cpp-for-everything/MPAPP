---
type: milestone
id: M-04b
title: Handler bulk port — parallel-worker phase
phase: p3
status: active
deliverables:
  - All remaining MAUI widgets reach `android-real` (Win + Linux + Android compile-verified) status
  - Each landed via an isolated git-worktree worker branch merged into main
  - Heavy widgets (Shell, NavigationPage, TabbedPage, FlyoutPage, ListView, WebView, HybridWebView, CollectionView, Grid-as-real-layout) are deferred to M-04c with ADR gates
exitCriteria:
  - 0 components in `Controls Inventory.md` at `not-started` status, except those explicitly carved out to M-04c
  - All committed handler `.cpp` files self-register via the ADR-0013 registry
  - Mock test suite green on Windows; gtk4_hello + android_hello compile green
tags:
  - type/milestone
  - phase/p3
  - phase/p4
  - phase/p5
---

# M-04b — Handler bulk port (parallel-worker phase)

The intermediate milestone between [[M-04-Windows-Real]] and [[M-05-Android-Real]]. M-04 + M-05 are running concurrently in practice — every batch this milestone ships lands the Win + Linux + Android handler at once.

## Why this is its own milestone

M-04b factors out the **mechanical bulk port** of widgets that have a clear native primitive on each platform. It's distinct from:

- M-03 (mock surface) — already done.
- M-04 (Windows-real) — Application/Window/StackLayout-level work; landed in T-0011.
- M-05 (Android-real) — same scope as M-04 but on Android; running in parallel here.
- M-04c — the **heavy widgets** that need design ADRs before code: Shell, NavigationPage, TabbedPage, FlyoutPage, ListView, TableView, CollectionView, WebView, HybridWebView, Grid (as a real layout engine), ShapeView, GraphicsView.

## Process

Per [[ADR-0013-data-driven-widget-dispatch]]:

1. **Phase 0** (done) — widget registry + glob CMakeLists landed. Workers can add widgets without modifying any pre-existing source file beyond two metadata touches (Inventory row + Component frontmatter).
2. **Phase 1** (active) — parallel workers in isolated git worktrees. Each handles 1 widget (or a small related family). Branches merge into `main` sequentially. The [[_Templates/Worker-Prompt|worker prompt template]] is the canonical agent brief.
3. **Phase 2** — sweep migration: convert existing widgets (button / label / entry / switch / check_box / radio_button / slider / stepper / editor / scroll_view / box_view / border / activity_indicator / progress_bar / search_bar / picker / date_picker / time_picker / image / image_button / content_view) to self-register via the ADR-0013 registry. Delete the legacy `dynamic_cast` chains. Mechanical, can happen alongside new widget work.

## Tracker

The live tracker is at [[Controls Inventory]] (status column). The summary below mirrors it as of the milestone open.

### Pending widgets — bulk-portable

These are the widgets a worker can take. Status reflects current state in `Controls Inventory.md`.

| Widget | Win primitive | Linux primitive | Android primitive | Notes |
|---|---|---|---|---|
| Frame | mux::Controls::Border | GtkBox + CSS | FrameLayout + GradientDrawable | `[[deprecated]]` — needs pragma-suppression at dispatch sites; consider skipping. |
| Page | mux::Controls::Page or Frame | GtkBox | FrameLayout | Single content slot; child binding. |
| BindableLayout | (attached property) | (attached property) | (attached property) | Mock exists. Real handlers are about generating + recycling children from `items_source`. |
| Layout | abstract base | abstract base | abstract base | No real handler needed — terminal `mock` status is correct. |
| View | abstract base | abstract base | abstract base | Same. |
| TitleBar | mux::Controls::TitleBar | GtkHeaderBar | androidx.appcompat.widget.Toolbar | Custom window chrome. |
| Toolbar | mux::Controls::CommandBar | GtkActionBar | androidx.appcompat.widget.Toolbar | Action surface. |
| TabbedView | mux::Controls::TabView | GtkNotebook | TabLayout + ViewPager2 | Tab-host control (not the same as TabbedPage). |
| FlyoutView | mux::Controls::NavigationView | GtkPaned drawer | DrawerLayout | Drawer pattern at view level. |
| ContentPage | (subclass of Page) | (subclass of Page) | (subclass of Page) | Trivial alongside Page. |
| TemplatedView | (control template host) | (control template host) | (control template host) | Needs the control-template surface. |
| IndicatorView | hand-rolled dots | hand-rolled dots | RadioGroup styled as dots | Companion to CarouselView (which is M-04c). Can land standalone with no actual carousel coupling. |
| RefreshView | mux::Controls::RefreshContainer | manual gesture + overlay | androidx.swiperefreshlayout.SwipeRefreshLayout | Pull-to-refresh. |
| ShapeView | (deferred — graphics surface) | (deferred) | (deferred) | Actually fits in M-04c — needs graphics ADR. Move there. |
| GraphicsView | (deferred) | (deferred) | (deferred) | M-04c. |
| Element | abstract base | abstract base | abstract base | Mock-only terminal. |
| MenuBar | mux::Controls::MenuBar | GtkPopoverMenuBar | androidx.appcompat.widget.Toolbar menu | Family with MenuBarItem etc. — single worker per family. |
| MenuBarItem | (child of MenuBar) | (child of MenuBar) | (child of MenuBar) | Family member. |
| MenuFlyout | mux::Controls::MenuFlyout | GtkPopoverMenu | androidx.appcompat.widget.PopupMenu | Right-click / long-press menu. |
| MenuFlyoutItem | (child) | (child) | (child) | Family member. |
| MenuFlyoutSeparator | (child) | (child) | (child) | Family member. |
| MenuFlyoutSubItem | (child) | (child) | (child) | Family member. |
| SwipeView | mux::Controls::SwipeControl | gesture-based | androidx.viewpager2 wrapper | Family. |
| SwipeItemView | (child of SwipeView) | (child) | (child) | Family. |
| SwipeItemMenuItem | (child) | (child) | (child) | Family. |
| Image (already done) | — | — | — | Shipped in `ece3fec`. Listed here for completeness. |
| ContentView (already scaffolded) | mux::ContentControl | GtkBox | FrameLayout | Scaffolded in `3b2cc69` — needs registrar conversion + dispatch on the registry side. Trivial follow-up. |

### M-04c (deferred, ADR-gated)

| Widget | Why deferred |
|---|---|
| Shell | Full app shell — combines NavigationPage + TabbedPage + FlyoutPage + URL routing + lifecycle. Single most complex MAUI surface. |
| NavigationPage | Push/pop page stack with platform back-handling. |
| TabbedPage | Tab-host + page swap; couples to NavigationPage stack. |
| FlyoutPage | Drawer pattern at page level. |
| ListView | Virtualized list with item recycling. |
| TableView | Static section/row list — simpler than ListView, could be M-04b if pragmatic. |
| CollectionView | Modern MAUI list (replaces ListView). **Not in the current Inventory** — add a row before starting. |
| WebView | Native browser embed. WebKitGTK on Linux is LGPL — see [[RFC-0001-licensing-and-patent-strategy]]. |
| HybridWebView | WebView + C++ ↔ JS interop bridge. Builds on WebView. |
| ShapeView | 2D drawing primitives. Needs a graphics-backend ADR. |
| GraphicsView | Skia-style canvas. Same. |
| Grid (as real layout engine) | Mock exists; turning track definitions + per-child placement into a real layout engine is non-trivial. |

### Branch / merge tracker

Each Phase 1 worker creates a branch `bulk/widget/<name>`. After local verification, merge into `main` with `git merge --no-ff`. The tracker below is updated as branches land.

| Widget | Branch | Worker status | Merge status |
|---|---|---|---|

(Updated as workers report in. Future sessions: this table is the single source of truth for what's in-flight.)

## How a follow-up session picks this up

1. Read this milestone + [[ADR-0013-data-driven-widget-dispatch]] + [[_Templates/Worker-Prompt|the worker prompt template]].
2. Open the tracker table above + the `Controls Inventory.md` status column.
3. Pick N widgets from the bulk-portable list that aren't already merged.
4. For each, spawn an `Agent` with `isolation: "worktree"` and the worker prompt template, with `{widget_name}` and `{native_widgets}` filled in.
5. Wait for the agents to complete (notifications fire when background agents finish).
6. For each completed worker, run `git merge --no-ff bulk/widget/<name>` from `main`. Resolve the (usually trivial) Inventory-row 3-way merge if any.
7. Build all 3 platforms once at the end. Commit any glob-pickup fixups.
8. Update the tracker.

## See also

- [[ADR-0013-data-driven-widget-dispatch]] — the foundation that makes parallel workers possible.
- [[ADR-0008-mock-first-implementation]] — mock first, real after.
- [[ADR-0006-interop-parity]] — every public feature on every platform.
- [[2026-W21-autonomous-bulk-port]] — session log from the manual bulk-port attempt that surfaced the need for this milestone.
