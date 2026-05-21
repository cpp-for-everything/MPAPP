---
type: adr
id: ADR-0014
title: "Page navigation stack semantics"
status: proposed
decisionDate: 2026-05-21
deciders: []
supersedes: ""
supersededBy: ""
area: handlers
tags:
  - type/adr
  - status/proposed
  - area/handlers
---

# ADR-0014 — Page navigation stack semantics

> [!info] Status
> **proposed** — awaiting review. Unblocks [[40_Roadmap/M-04c-handler-heavy-port|M-04c]] for `NavigationPage`, `TabbedPage`, `FlyoutPage`, `Shell`.

## Context

[[40_Roadmap/M-04b-handler-bulk-port|M-04b]] landed every bulk-portable widget. What remains is a small set of **page-level** containers that share one big design problem: they manage a **stack of pages** with platform-specific back-handling, lifecycle notifications, and bar chrome.

The MAUI surface is huge:

- `NavigationPage` — push/pop stack with title bar
- `TabbedPage` — tab-host swapping the visible page
- `FlyoutPage` — drawer + detail page
- `Shell` — composes all of the above plus URI routing + flyout menu

Each has its own per-platform native primitive (`mux::Frame`, `GtkStack`, `Fragment` stack, `UINavigationController`, `NSPageController`) with different stack-management semantics. Without a unified contract, every page-level widget reinvents:

- Whether push/pop is sync or async
- How attached properties (like `HasBackButton`, `TitleView`) on the **child** page propagate to the **host's** chrome
- When `OnAppearing` / `OnDisappearing` lifecycle hooks fire
- How the platform back-button intercepts pop
- Whether transitions are animated and how the animation is owned

## Decision

We will adopt a single `mpapp::detail::page_stack` host-agnostic stack engine that every page-level widget (`navigation_page`, `tabbed_view`/`tabbed_page`, `flyout_page`, `shell`) builds on top of. The engine owns:

1. The current page stack (`std::vector<view*>`, root-first, top last).
2. A `signal<view*> page_will_appear` + `signal<view*> page_did_appear` pair (and disappear-equivalents) that the host wires into its native container.
3. Synchronous `push`/`pop`/`insert_before`/`remove` mutators. **Async variants are a thin wrapper that schedules the sync mutator on the UI executor and returns a `task<void>`.** Sync is the primitive; async is sugar.
4. A `set_attached_property<T>(view&, key, T)` / `get_attached_property<T>(view&, key)` mechanism for the "child sets, host reads" pattern (title, has back button, bar icon, etc.). Attached props are stored in a `std::unordered_map<view*, std::any>` owned by the page_stack instance; cleared when the page leaves the stack.

Each page-level widget owns a `page_stack` instance and exposes its own user-facing API (push_async / pop_async / current_page Observable). The handlers translate stack events to native container ops via existing ADR-0013 dispatch — the children are just `view*` resolved via `dispatch(v)`.

**Transitions are not animated in v1.** Each push/pop replaces the host's content slot in one frame. Animated transitions are an opt-in future RFC; we prefer correctness over polish for the M-04c landing.

**Platform back-button binding:** the engine exposes `signal<> hardware_back_requested`. Each host's handler subscribes to its native equivalent (Android `KeyEvent.KEYCODE_BACK`, Windows `mux::SystemNavigationManager.BackRequested`, Linux GTK does not intercept — manual button) and translates it into a `pop()` call when the stack depth is > 1.

## Consequences

### Positive

- One implementation of stack semantics, reused by 4+ page-level widgets.
- ADR-0013 dispatch composes — the same registry that resolves leaf widgets also resolves whatever page-level container nests another page-level container (e.g. a `NavigationPage` inside a `FlyoutPage`'s detail slot).
- Attached-property storage is centralized; the "child sets, host reads" pattern is uniform.
- Lifecycle events fire from one place — no per-widget reinvention.
- Sync-as-primitive makes the engine trivially testable in mock handlers (no executor needed).

### Negative

- `std::any`-backed attached property store is dynamically typed; the type-safety bar is lower than the Observable wrapper convention. Mitigation: define a `attached_prop_key<T>` strong-typed key wrapper so misuse is at least a compile-time mismatch at the call site.
- Async navigation is built on top of sync, which means the C# `await PushAsync(page)` idiom lands as `co_await nav.push_async(p)` and the engine has to be careful that the executor schedule doesn't return control to user code before the sync mutator finishes. Documented as a known behavioral difference.
- No animated transitions in v1 will be visually jarring next to MAUI's defaults. Documented in each per-component note's "Known Differences" table.

### Neutral

- Shell's URI routing layer is a separate concern; this ADR doesn't constrain it. The routing layer translates URIs to push/pop sequences on a `page_stack`; that mapping can be designed independently.

## Alternatives Considered

- **Per-widget stack implementation**: each of NavigationPage/TabbedPage/FlyoutPage/Shell owns its own private stack. Rejected — duplicates lifecycle, attached-property, and back-handling logic 4×, with the inevitable per-platform drift.
- **Async-as-primitive**: make `push_async` the only mutator and have sync be a "wait for the task" wrapper. Rejected — forces the mock handler to host an executor, and the test harness becomes async-aware for what is fundamentally a synchronous data-structure mutation.
- **Native stack types directly** (e.g., let each widget map directly onto `UINavigationController` and skip the engine). Rejected — the engine is the only place lifecycle + attached props + back-handling can be uniform across platforms.

## References

- [[ADR-0006-interop-parity]] — every public feature on every platform.
- [[ADR-0008-mock-first-implementation]] — mock first; the page_stack engine must be testable without a native host.
- [[ADR-0013-data-driven-widget-dispatch]] — child resolution uses the existing registry.
- [[Components/NavigationPage]] · [[Components/TabbedPage]] · [[Components/FlyoutPage]] · [[Components/Shell]]
- [[40_Roadmap/M-04c-handler-heavy-port]]
