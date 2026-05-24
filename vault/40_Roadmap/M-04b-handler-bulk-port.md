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
| title_bar | `bulk/widget/title_bar` | done | merged (d07d526) |
| toolbar | `bulk/widget/toolbar` | done | merged |
| indicator_view | `bulk/widget/indicator_view` | done | merged (62553c7) |
| refresh_view | `bulk/widget/refresh_view` | done | merged (bdffd1c) |
| content_page | `bulk/widget/content_page` | done | merged (0fca76b) |
| page | `bulk/widget/page-recovery` | done | merged (9ea6663) — wave-2 recovery |
| flyout_view | `bulk/widget/flyout_view` | done | merged (9eef0fb) — wave-2 recovery |
| templated_view | `bulk/widget/templated_view` | done | merged (bfcfd35) — wave-2 recovery |
| bindable_layout | `worktree-agent-aa35c156d70a971f8` | done | merged (dd76a88) — wave-2 recovery |
| tabbed_view | `bulk/widget/tabbed_view-recovery` | done | merged — wave-3 recovery |
| frame | `bulk/widget/frame-recovery` | done | merged — wave-3 recovery (deprecated alias for Border) |
| menu_bar + menu_bar_item | `bulk/widget/menu_bar_family-recovery` | done | merged — wave-3 recovery |
| menu_flyout family (4) | `bulk/widget/menu_flyout_family-recovery` | done | merged — wave-3 recovery |
| swipe family (3) | `bulk/widget/swipe_family-recovery` | done | merged — wave-3 recovery |

**Wave-2 / wave-3 recovery note (2026-05-21):** Of the 5 wave-2 agents, 4 wrote complete file sets but failed to commit them — the worker prompt template needed explicit "you MUST commit" + "use relative paths only" instructions. Files were recovered by manually staging from worktrees and committing. Same pattern repeated for all 5 wave-3 agents — same fix, same recovery procedure. The worker template now warns about both pitfalls.

**Phase 2 sweep (2026-05-21):** 18 pre-existing widgets (button, label, entry, switch, check_box, radio_button, slider, stepper, editor, box_view, activity_indicator, progress_bar, search_bar, picker, date_picker, time_picker, image, image_button) gained self-registering ADR-0013 registrars via `_add_registrars.py`. Non-destructive — legacy `dynamic_cast` chains in stack_layout/window/scroll_view/border/content_view stay as fallback. 54 files (18 widgets × 3 platforms). Commit `0308d67`.

**Build state after wave-3 recovery + Phase 2 sweep:** Windows 183/183 tests pass; Linux green; Android `BUILD SUCCESSFUL`. All 3 platforms verified.

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

## See in code

- The bulk-portable handler set landed across three platforms:
  [`src/handlers/windows/`](../../src/handlers/windows/) (62 files)
  · [`src/handlers/linux/`](../../src/handlers/linux/) (62 files)
  · [`src/handlers/android/`](../../src/handlers/android/) (69 files).
- The dispatch foundation that made parallel workers possible:
  per-platform dispatch headers at [`include/mpapp/handlers/{windows,linux,android}/widget_dispatch.hpp`](../../include/mpapp/handlers/android/widget_dispatch.hpp) + the per-component `<name>_handler.cpp` self-registration pattern (static initializer that registers a `view*` → native-handle function in the per-platform registry) per [[ADR-0013-data-driven-widget-dispatch]].
- Android kind-discriminated event routers (one shared listener class per event family, per [[ADR-0022-android-kind-discriminated-routers]]): [`src/handlers/android/item_click_router.cpp`](../../src/handlers/android/item_click_router.cpp), [`widget_dispatch.cpp`](../../src/handlers/android/widget_dispatch.cpp), [`text_watcher_dispatch.cpp`](../../src/handlers/android/text_watcher_dispatch.cpp), etc.
- Java glue under [`examples/android_hello/app/src/main/java/io/mpapp/`](../../examples/android_hello/app/src/main/java/io/mpapp/) — `MppClickRouter.java`, `MppCheckedChangeListener.java`, `MppTextWatcher.java`, `MppItemClickRouter.java` (kind-discriminator routers).

## See also

- [[ADR-0013-data-driven-widget-dispatch]] — the foundation that makes parallel workers possible.
- [[ADR-0008-mock-first-implementation]] — mock first, real after.
- [[ADR-0006-interop-parity]] — every public feature on every platform.
- [[2026-W21-autonomous-bulk-port]] — session log from the manual bulk-port attempt that surfaced the need for this milestone.
