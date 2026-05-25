---
type: task
id: T-0033
title: Gesture recognizers — RFC-0003 first slice (tap recognizer + view attachment + mock contract)
status: in-progress
milestone: M-04b
owner: ""
area: widgets
blockedBy: []
coveragePercent: 100
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/in-progress
  - area/widgets
  - area/handlers
  - phase/p2
---

# T-0033 — Gesture recognizers, RFC-0003 first slice

## Goal

Land the smallest meaningful slice of [[RFC-0003-gesture-recognizers]]: the polymorphic base, the `TapGestureRecognizer` surface, the `view::gesture_recognizers` collection + `add_gesture<T>` helper, and the mock `view_handler<platform::mock>` recording contract (`map_gestures`, `simulate_tap`). Per-platform real handlers + the other four recognizers (Pan / Pinch / Swipe / Pointer) are subsequent tasks.

## Scope

In:

- New base header `include/mpapp/internal/basic_gesture_recognizer.hpp` with the polymorphic abstract base + `gesture_kind` discriminator enum.
- New surface header `include/mpapp/gestures/tap_gesture_recognizer.hpp` with `tap_gesture_recognizer` + `tapped_event_args` + `button_mask` (with `|`, `&`, `any()` helpers).
- `view.hpp` extended with `std::vector<std::shared_ptr<internal::basic_gesture_recognizer>> gesture_recognizers` member + `template <class T, class... Args> T& add_gesture(Args&&...)` convenience emplace-and-return.
- `view_handler<platform::mock>` (in `include/mpapp/handlers/mock/view_handler.hpp`) extended with:
  - `map_gestures(view&)` — records one `gesture.<kind>_attached` entry per attached recognizer.
  - `simulate_tap(view&, double x, double y, button_mask)` — fans the synthetic event out to every attached tap recognizer's `tapped` signal; records `gesture.tap_simulated`.
- New test file `tests/mock_handlers/gesture_tap_test.cpp` — 5 test cases covering attach / map / single-fire / multi-recognizer fan-out / Observable change-emit semantics.

Out (explicit follow-ups):

- Pan / Pinch / Swipe / Pointer recognizer surfaces (one task per recognizer or a single bulk task — TBD when prioritising).
- Per-platform real handlers (`view_handler<platform::linux_>::map_gestures` wiring `GtkGestureClick` etc.). Tracked under [[M-04b-handler-bulk-port]].
- XAML lowering for `<View.GestureRecognizers>` in `mpapp-xc`. Tracked under [[M-09-Tooling-DX]].
- `DragGestureRecognizer` + `DropGestureRecognizer` — deferred per RFC-0003 § Open Questions.

## Acceptance Criteria

- [x] `include/mpapp/internal/basic_gesture_recognizer.hpp` exists with `basic_gesture_recognizer` (virtual dtor, deleted copy/move, protected default ctor, `kind()` pure-virtual) + `gesture_kind` enum.
- [x] `include/mpapp/gestures/tap_gesture_recognizer.hpp` exists; `tap_gesture_recognizer` derives `basic_gesture_recognizer`, ships `number_of_taps_required` (`Observable<int>` default 1), `buttons` (`Observable<button_mask>` default `primary`), `tapped` (`signal<const tapped_event_args&>`), and `kind() -> gesture_kind::tap`.
- [x] `view::gesture_recognizers` + `view::add_gesture<T>` template helper land; `static_assert` enforces `T` derives the base.
- [x] `view_handler<platform::mock>` gains `map_gestures` + `simulate_tap`; the new bodies are in the same file so adding them does not require touching `tests/CMakeLists.txt`.
- [x] `tests/mock_handlers/gesture_tap_test.cpp` lands with at least 5 test cases — picked up automatically by the glob in `tests/CMakeLists.txt`.
- [x] `ctest --test-dir build-wsl` is green; total assertion count moves up by the 5 new gesture tests (was 351 → now 356).
- [x] `vault/10_Architecture/Components/View.md` cross-references the new `gesture_recognizers` member + links [[RFC-0003-gesture-recognizers]].
- [ ] `vault/10_Architecture/Components/View.md` reaches `coveragePercent: 100` on the gesture surface paths once the full RFC is shipped (deferred — covers the rest of the family).

## Notes

The wrapper-pattern question came up briefly during design: recognizers are **not** [[Wrapper-Component]]s. They own no native widget; the platform wire-up lives on the *attached view's* handler. The closest precedent in the codebase is `view::gesture_recognizers` itself, modelled as a polymorphic `shared_ptr` collection that the per-platform `view_handler::map_gestures(view&)` walks at bind time.

The mock recording shape (`gesture.tap_attached` / `gesture.tap_simulated`) mirrors how the existing simulation helpers (`window_handler::simulate_activated`, `button_handler::simulate_click`) report their synthetic events — same `mock_handler_base` API, no new infrastructure.

## Build evidence

- `mock_handlers_test` re-link succeeds; 356/356 ctest assertions pass on Linux WSL (gcc 14.2, GTK4 4.14.5).
- The new gesture tests cover: attach via `add_gesture`, default values match MAUI, `map_gestures` records per-recognizer, `simulate_tap` fans out, multi-recognizer fan-out works, Observable's no-emit-on-same-value contract holds for `button_mask`.

## Links

- Source-of-truth design: [[RFC-0003-gesture-recognizers]].
- Code:
  - [`include/mpapp/internal/basic_gesture_recognizer.hpp`](../../../include/mpapp/internal/basic_gesture_recognizer.hpp)
  - [`include/mpapp/gestures/tap_gesture_recognizer.hpp`](../../../include/mpapp/gestures/tap_gesture_recognizer.hpp)
  - [`include/mpapp/view.hpp`](../../../include/mpapp/view.hpp) — `gesture_recognizers` + `add_gesture<T>`
  - [`include/mpapp/handlers/mock/view_handler.hpp`](../../../include/mpapp/handlers/mock/view_handler.hpp) — `map_gestures` + `simulate_tap`
  - [`tests/mock_handlers/gesture_tap_test.cpp`](../../../tests/mock_handlers/gesture_tap_test.cpp)
- Follow-up tasks: T-0034..T-0037 (one per remaining recognizer family) — to be opened.
- Related ADRs: [[ADR-0008-mock-first-implementation]], [[ADR-0024-wrapper-component-pattern]].
- Component doc: [[Components/View]] — updated to mention `gesture_recognizers`.
