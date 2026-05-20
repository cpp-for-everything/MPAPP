---
type: adr
id: ADR-0013
title: Data-driven widget dispatch via per-platform registries
status: accepted
decisionDate: 2026-05-20
deciders:
  - alex
supersedes: []
supersededBy: []
area: handlers
---

# ADR-0013 — Data-driven widget dispatch via per-platform registries

## Status

**accepted** — Phase 0 of the M-04b bulk port. Lands the per-platform `widget_registry` + dispatch-surface fall-through together so the existing `dynamic_cast` chains keep working untouched while new widgets self-register via a static initializer. Unblocks the parallel worker workflow in [[40_Roadmap/M-04b-handler-bulk-port]].

## Context

Every container handler in MPAPP — `stack_layout`, `window`, `scroll_view`, `border`, `content_view`, and any future composite widget — needs to map a heterogeneous `view*` child to its platform-native handle. The pattern up through commit `3b2cc69` is an explicit `dynamic_cast` chain at every dispatch site:

```cpp
if (auto* b = dynamic_cast<button*>(v); b && b->has_handler()) return b->handler().native();
if (auto* l = dynamic_cast<label*>(v); l && l->has_handler()) return l->handler().native();
// ...one branch per widget type, repeated in 15 dispatch sites across 3 platforms...
```

This was tolerable at five widgets; at twenty-four widgets it's a sustained source of:

1. **Merge conflicts when parallelizing work.** Every new widget batch touches the same 15 dispatch sites (5 dispatch surfaces × 3 runtime platforms). When the [[2026-W21-autonomous-bulk-port|2026-W21 autonomous run]] hit the harness's file-state tracking limits, the dispatch wiring was where every Edit started failing. Workers in isolated worktrees would all conflict at the same anchor points.
2. **Boilerplate scaling linearly with widget count.** Each new widget needs ~9 dispatch case additions (5 surfaces × ~2 cases per surface, on average) plus 3 CMakeLists source-list entries plus 1 test list entry — all just to register the widget's existence to the framework.
3. **A single source of forgetting.** Picker, DatePicker, TimePicker, and Image all hit "include exists but dispatch case missing" or vice-versa failures during the 2026-W21 run because the 15-touch-point coordination is hand-tracked.

The fix is to invert the relationship: widgets register themselves with a per-platform registry, and the dispatch sites query that registry instead of hardcoding each widget type.

## Decision

### Per-platform widget registry

Three new headers (one per runtime platform) at `include/mpapp/handlers/{android,linux,windows}/widget_dispatch.hpp`. Each declares a registry namespace with two functions:

```cpp
// include/mpapp/handlers/android/widget_dispatch.hpp
namespace mpapp::detail::android {
    using dispatcher_fn = jobject (*)(view*);
    void register_dispatcher(dispatcher_fn fn);
    jobject dispatch(view* v);
}
```

The Linux variant returns `GtkWidget*`. The Windows variant returns `winrt::Microsoft::UI::Xaml::UIElement` (the common base of every WinUI control type).

The implementation (`src/handlers/{android,linux,windows}/widget_dispatch.cpp`) is a singleton `std::vector<dispatcher_fn>` populated at static-initialization time and iterated linearly at dispatch time. Linear search is fine — typical dispatch is O(20–60) and runs once per Observable change on the UI thread, not per frame.

### Self-registration via static initializer

Each widget's platform handler `.cpp` adds an anonymous-namespace registrar at the bottom:

```cpp
namespace {
    jobject dispatch_button(view* v) {
        if (auto* b = dynamic_cast<button*>(v); b && b->has_handler()) {
            return b->handler().native();
        }
        return nullptr;
    }
    struct registrar {
        registrar() { mpapp::detail::android::register_dispatcher(dispatch_button); }
    };
    [[maybe_unused]] registrar _reg;
}
```

Static-init order across translation units is unspecified by the C++ standard, but doesn't matter here: every registrar is just a `vector::push_back`, all registrars complete before `main()` runs, and dispatch is only ever called at runtime.

### Fall-through at dispatch sites

The five existing dispatch sites per platform (`stack_layout::add_child`, `window::child_jobject` / `apply_content`, `scroll_view::child_jobject` / `apply_content`, `border::apply_content`, `content_view::apply_content` — when wired) gain a single line at the top:

```cpp
if (auto* native = mpapp::detail::android::dispatch(v); native != nullptr) {
    // use `native` and return
}
// fall through to legacy dynamic_cast chain (gradually emptied over time)
```

The existing legacy `dynamic_cast` chains are untouched in this ADR's landing commit. Migration of existing widgets to self-register is incremental, can happen widget-by-widget, and doesn't block new widget arrivals.

### Glob the example CMakeLists

`examples/{android_hello,gtk4_hello,windows_button_spike}` and `tests/CMakeLists.txt` switch from hand-maintained source lists for the handler dir to `file(GLOB ... CONFIGURE_DEPENDS)`. After this, adding a new widget's `.cpp` doesn't require editing any CMakeLists — it's picked up automatically and CMake re-configures on next build.

CONFIGURE_DEPENDS does have a small per-build cost (CMake checks the glob result against the cached one). On a project this size that cost is negligible. The trade-off is decisively in favor of velocity: no more "I forgot to add the .cpp to four CMakeLists" failures.

### Worker invariant

Once Phase 0 lands, **a worker in an isolated worktree can add a new widget without modifying any pre-existing file** (other than appending one row to `Controls Inventory.md` and writing one `Components/<Name>.md`). The worker writes:

- `include/mpapp/<widget>.hpp` (cross-platform header — new)
- `include/mpapp/handlers/mock/<widget>_handler.hpp` (new)
- `include/mpapp/handlers/{windows,linux,android}/<widget>_handler.hpp` (3 new)
- `src/handlers/{windows,linux,android}/<widget>_handler.cpp` (3 new — each with a self-registering registrar)
- `tests/mock_handlers/<widget>_test.cpp` (new)
- `vault/10_Architecture/Components/<Widget>.md` frontmatter update (one row touched)
- One row added to `vault/10_Architecture/Controls Inventory.md` (one row touched)

That's it. Worker branches don't conflict with each other on anything except the Inventory row addition, which is a near-trivial 3-way merge — git handles it automatically when the rows are at different positions.

## Consequences

### Positive

- **Parallel workers stop colliding.** The 15 dispatch-site touch points per widget go to zero. Multiple workers can land widgets concurrently with no merge conflicts.
- **CMakeLists stop being a serialization point.** Glob picks up new files automatically.
- **Existing dispatch chains keep working.** Phase 0 is purely additive — nothing breaks, nothing regresses.
- **Per-widget code stays cohesive.** A widget's `dynamic_cast → handler().native()` glue lives in the widget's own `.cpp` instead of being scattered across 15 dispatch sites in 9 unrelated files.
- **Build cost is negligible.** Linear dispatch over a typical 30–60 registrars on the UI thread once per Observable change is unmeasurable. The CMake glob cost is unmeasurable.

### Negative

- **Static initialization order is unspecified** across translation units. We're insulated from that here (no init order dependency among registrars) but the pattern requires care in future extensions.
- **Linear dispatch is O(N).** At N=60 widgets, still negligible. At N=500 (Shell + collection items + ...), we'd want a `typeid`-keyed map. Cross that bridge if the inventory ever grows that large.
- **`file(GLOB)` has [the documented downside](https://cmake.org/cmake/help/latest/command/file.html#filesystem) of not detecting *removed* files cleanly across some generators.** `CONFIGURE_DEPENDS` mitigates the *added*-file case; if we ever delete a handler file, a full reconfigure is needed. Cheap.
- **Migration of existing widgets to self-register is unfinished work.** Phase 0 lands the mechanism without converting button/label/entry/etc. Until they're all converted, the legacy `dynamic_cast` chains co-exist alongside the registry. Acceptable but visually messy until completed.

### Neutral

- **macOS / iOS handlers** will need the same registry pattern when an Apple host comes online (per [[ADR-0005-ios-macos-separate-interop]]). The `.mm` files for those platforms already exist in the tree; they just need their own `widget_dispatch.mm` and registrar conversion when activated.

## Worker prompt template

`vault/_Templates/Worker-Prompt.md` carries the canonical agent prompt. The short version:

> Add `mpapp::<widget>` to MPAPP. Read [[ADR-0013-data-driven-widget-dispatch]] for the registry pattern. Write 9 new files (cross-platform header + mock handler + mock test + 3 platform headers + 3 platform cpps; each platform cpp self-registers). Append one row to [[Controls Inventory]]. Update `Components/<Widget>.md` frontmatter to `mpappStatus: android-real`. **Do not edit any pre-existing file beyond those two metadata touch points.** Build all three platforms green. Commit on your branch.

## Migration trigger

A separate later ADR (or pragmatic decision recorded in the session log) converts existing widgets to self-register and removes the legacy `dynamic_cast` chains. That migration is mechanical (the same `dynamic_cast → handler().native()` lambda, just moved into the widget's `.cpp`) and can happen alongside other work — it's not a blocker.

## Related

- [[ADR-0006-interop-parity]] — every public feature works on every platform; this ADR is invisible to the public API, only changes the internal dispatch mechanism.
- [[ADR-0012-application-window-handler-abstraction]] — the original widget-handler pattern this ADR generalizes.
- [[40_Roadmap/M-04b-handler-bulk-port]] — the milestone this unblocks.
- [[2026-W21-autonomous-bulk-port]] — the session log that surfaced the dispatch-chain problem at scale.
