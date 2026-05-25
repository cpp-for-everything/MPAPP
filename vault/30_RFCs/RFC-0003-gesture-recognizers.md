---
type: rfc
id: RFC-0003
title: Gesture recognizers — view-attached input event family
status: draft
author: Alex Tsvetanov
created: 2026-05-25
area: handlers
relatedADRs:
  - ADR-0006
  - ADR-0008
  - ADR-0024
tags:
  - type/rfc
  - status/draft
  - area/handlers
  - area/widgets
---

# RFC-0003 — Gesture recognizers

> [!info] Status
> **draft** — under discussion.

## Problem

MAUI exposes a family of gesture recognizers (`TapGestureRecognizer`, `PanGestureRecognizer`, `PinchGestureRecognizer`, `SwipeGestureRecognizer`, `PointerGestureRecognizer`, plus the DnD pair) that attach to any `View` via `View.GestureRecognizers`. Each recognizer carries configuration as bindable properties (`NumberOfTapsRequired`, `TouchPoints`, `Direction`, …) and raises events (`Tapped`, `PanUpdated`, `PinchUpdated`, `Swiped`, `PointerEntered`/`Moved`/`Pressed`/`Released`).

MPAPP has *none* of this today. The only input event modeled is the ad-hoc `mpapp::button::clicked` signal — a stand-in baked directly into one widget. That covers the trivial "user pressed this button" case, but it does not:

- generalise across views (a panel can't observe a tap),
- compose (you can't add a long-press recognizer + a swipe recognizer to the same view),
- expose the position / button mask the gesture happened with,
- support multi-finger / multi-touch (`PinchGestureRecognizer`, `PanGestureRecognizer.TouchPoints`),
- support hover-style pointer events on desktop (the `PointerGestureRecognizer` family).

Without this subsystem, **every** non-trivial UI flow that MAUI gets for free — drag-to-reorder, swipe-to-delete, double-click, pinch-zoom, hover-driven tooltips — must be hand-rolled per component. This RFC fixes that.

## Proposal

Land a gesture-recognizer subsystem matching MAUI's surface 1:1, with the same five concrete recognizers (Tap / Pan / Pinch / Swipe / Pointer; DnD deferred), attached to every `mpapp::view` via a collection member. Each recognizer is its own concrete class with `Observable<T>` config + `mpapp::signal<...>` events. The per-platform `view_handler` is the wire-up point: it owns the native gesture listeners and forwards events to the attached recognizers.

Recognizers themselves are NOT [[Wrapper-Component|wrapper-pattern]] components. They have no native widget of their own; they only attach to one. The wrapper-pattern split (`internal::basic_<name>` + `mpapp::<name>` wrapper) is for visible widgets that own a native handle. A recognizer is a configuration + signal — a value-typed attachment. The closest precedent in the existing codebase is the `gesture_recognizers` member on [[Components/View|view]] (today: not yet present) holding `std::shared_ptr<internal::basic_gesture_recognizer>` polymorphically.

Mock-first per [[ADR-0008-mock-first-implementation]]: every recognizer ships with a recording handler-side path + a per-platform simulator that lets tests drive synthetic events without a real platform event loop.

## Detailed Design

### Inheritance + namespacing

```
mpapp::internal::basic_gesture_recognizer        ← abstract base (polymorphic, gesture_kind() virtual)
        ▲
        ├── mpapp::tap_gesture_recognizer        ← config + signal, in mpapp::
        ├── mpapp::pan_gesture_recognizer
        ├── mpapp::pinch_gesture_recognizer
        ├── mpapp::swipe_gesture_recognizer
        └── mpapp::pointer_gesture_recognizer
```

Concrete recognizers live in `mpapp::` (not `mpapp::internal::`) because they are user-facing configuration objects, not platform-agnostic surfaces of a widget. The base lives in `internal::` because the polymorphic upcast is a framework-internal concern (the `view::gesture_recognizers` collection stores `shared_ptr<internal::basic_gesture_recognizer>`).

### File layout

```
include/mpapp/gestures/                         ← new directory
    gesture_kind.hpp                            ← gesture_kind enum
    tap_gesture_recognizer.hpp                  ← TapGestureRecognizer surface
    pan_gesture_recognizer.hpp                  ← PanGestureRecognizer surface
    pinch_gesture_recognizer.hpp                ← PinchGestureRecognizer surface
    swipe_gesture_recognizer.hpp                ← SwipeGestureRecognizer surface
    pointer_gesture_recognizer.hpp              ← PointerGestureRecognizer surface
include/mpapp/internal/
    basic_gesture_recognizer.hpp                ← polymorphic base
```

The `gestures/` directory groups them so adding the 5 headers doesn't crowd the top of `include/mpapp/`. The `internal/basic_gesture_recognizer.hpp` follows the existing convention for cross-component base types.

### Surface — TapGestureRecognizer (canonical)

```cpp
// include/mpapp/internal/basic_gesture_recognizer.hpp
namespace mpapp::internal {

enum class gesture_kind : std::uint8_t {
    tap     = 0,
    pan     = 1,
    pinch   = 2,
    swipe   = 3,
    pointer = 4,
};

class basic_gesture_recognizer {
public:
    virtual ~basic_gesture_recognizer() = default;

    basic_gesture_recognizer(const basic_gesture_recognizer&)            = delete;
    basic_gesture_recognizer& operator=(const basic_gesture_recognizer&) = delete;

    virtual gesture_kind kind() const noexcept = 0;

protected:
    basic_gesture_recognizer() = default;
};

} // namespace mpapp::internal
```

```cpp
// include/mpapp/gestures/tap_gesture_recognizer.hpp
namespace mpapp {

enum class button_mask : std::uint8_t {
    none      = 0,
    primary   = 1 << 0,   // left mouse / touch / pen tip
    secondary = 1 << 1,   // right mouse / pen barrel
};

constexpr button_mask operator|(button_mask a, button_mask b) noexcept {
    return static_cast<button_mask>(
        static_cast<std::uint8_t>(a) | static_cast<std::uint8_t>(b));
}
constexpr bool any(button_mask a, button_mask b) noexcept {
    return (static_cast<std::uint8_t>(a) & static_cast<std::uint8_t>(b)) != 0;
}

struct tapped_event_args {
    double      x = 0.0;          // position relative to attached view (px)
    double      y = 0.0;
    button_mask buttons = button_mask::primary;
};

class tap_gesture_recognizer : public internal::basic_gesture_recognizer {
public:
    tap_gesture_recognizer() = default;

    // ----- Bindable configuration ----------------------------------------
    Observable<int>         number_of_taps_required{1};
    Observable<button_mask> buttons{button_mask::primary};

    // ----- Event ---------------------------------------------------------
    mpapp::signal<const tapped_event_args&> tapped{};

    internal::gesture_kind kind() const noexcept override {
        return internal::gesture_kind::tap;
    }
};

} // namespace mpapp
```

### Attachment surface on `view`

```cpp
// include/mpapp/view.hpp — additions (NOT a wrapper-pattern split — view is the
// inheritance root; gesture recognizers are config attached to it).
namespace mpapp {

class view {
    // ... existing members ...

    // Polymorphic collection of attached recognizers. shared_ptr because:
    //   (a) the view does not own the recognizer exclusively — a binding
    //       layer can hold a ref to mutate config from a view-model;
    //   (b) MAUI's IList<IGestureRecognizer> allows duplicates / shared refs;
    //   (c) the platform handler reads through the collection on every
    //       map_gestures(view&) call, so the lifetime needs to be stable
    //       beyond a single function-call scope.
    std::vector<std::shared_ptr<internal::basic_gesture_recognizer>>
        gesture_recognizers{};

    // Convenience emplace-and-return. `T` must derive from
    // `internal::basic_gesture_recognizer`. Returns a reference for fluent
    // setup (`btn.add_gesture<tap_gesture_recognizer>().number_of_taps_required = 2`).
    template <class T, class... Args>
    T& add_gesture(Args&&... args) {
        auto p = std::make_shared<T>(std::forward<Args>(args)...);
        T& ref = *p;
        gesture_recognizers.push_back(std::move(p));
        return ref;
    }
};

} // namespace mpapp
```

### Per-platform handler wire-up

Each platform's `view_handler<P>` gains a `map_gestures(basic_<view-type>&)` method (parallel to the existing `map_text` family on the subclasses). The handler walks `view.gesture_recognizers`, dispatches on `kind()`, and installs the appropriate native listener:

| Platform | Tap | Pan | Pinch | Swipe | Pointer |
|---|---|---|---|---|---|
| Windows (WinUI 3) | `UIElement.Tapped` (RoutedEvent) | `ManipulationDelta` | `ManipulationDelta` (scale) | `ManipulationCompleted` (direction inferred) | `PointerEntered/Exited/Moved/Pressed/Released` |
| Linux (GTK4) | `GtkGestureClick` (`pressed`/`released`) | `GtkGestureDrag` | `GtkGestureZoom` | `GtkGestureSwipe` | `GtkEventControllerMotion` + `GtkGestureClick` |
| Android | `GestureDetector.OnGestureListener` (single/double tap) | `MotionEvent` ACTION_MOVE | `ScaleGestureDetector` | `GestureDetector.onFling` | `View.OnHoverListener` + `MotionEvent` |
| macOS (AppKit) | `NSClickGestureRecognizer` | `NSPanGestureRecognizer` | `NSMagnificationGestureRecognizer` | `NSPanGestureRecognizer` w/ direction filter | `mouseEntered:` / `mouseMoved:` / mouseDown/Up |
| iOS (UIKit) | `UITapGestureRecognizer` | `UIPanGestureRecognizer` | `UIPinchGestureRecognizer` | `UISwipeGestureRecognizer` | `UIHoverGestureRecognizer` + touch events |

Real handlers land in M-04b (Windows + Linux + Android first; macOS/iOS in M-07/M-08). This RFC commits the **mock** path + the contract; per-platform real implementations are subsequent tasks.

### Mock handler

The mock recording pattern from [[ADR-0008-mock-first-implementation]] applies unchanged:

```cpp
// include/mpapp/handlers/mock/view_handler.hpp — additions
namespace mpapp::internal {

class view_handler<platform::mock> : public mock_handler_base {
public:
    // ... existing map_* methods ...

    void map_gestures(view& v) {
        for (const auto& r : v.gesture_recognizers) {
            switch (r->kind()) {
                case gesture_kind::tap:
                    record_event("gesture.tap_attached"); break;
                case gesture_kind::pan:
                    record_event("gesture.pan_attached"); break;
                // ...
            }
        }
    }

    // Test helper — simulate a native tap dispatching to recognizers.
    void simulate_tap(view& v, double x = 0.0, double y = 0.0,
                      button_mask b = button_mask::primary) {
        for (const auto& r : v.gesture_recognizers) {
            if (r->kind() == gesture_kind::tap) {
                static_cast<tap_gesture_recognizer&>(*r).tapped.emit(
                    tapped_event_args{x, y, b});
            }
        }
        record_event("gesture.tap_simulated");
    }

    // simulate_pan / simulate_pinch / simulate_swipe / simulate_pointer_*
    // follow the same shape.
};

} // namespace mpapp::internal
```

### Tests (mock-first)

```cpp
// tests/mock_handlers/gesture_tap_test.cpp
TEST_CASE("tap recognizer attaches and fires on simulated tap",
          "[mock][gesture][tap]") {
    mpapp::internal::basic_button b;
    auto& tap = b.add_gesture<mpapp::tap_gesture_recognizer>();

    int hits = 0;
    mpapp::tapped_event_args last_args{};
    mpapp::signal_slot<const mpapp::tapped_event_args&> slot;
    auto cb = [&](const mpapp::tapped_event_args& a) {
        ++hits;
        last_args = a;
    };
    tap.tapped.subscribe(slot, cb);

    mpapp::view_handler<mpapp::platform::mock> h;
    h.map_gestures(b);
    h.simulate_tap(b, /*x=*/12.5, /*y=*/7.0);

    REQUIRE(hits == 1);
    CHECK(last_args.x == 12.5);
    CHECK(last_args.y == 7.0);
    CHECK(last_args.buttons == mpapp::button_mask::primary);
}
```

### XAML compatibility

```xml
<!-- MAUI (must compile 1:1 per ADR-0004) -->
<Button Text="Tap me">
    <Button.GestureRecognizers>
        <TapGestureRecognizer
            Tapped="OnTapped"
            NumberOfTapsRequired="2"
            Buttons="Primary,Secondary"/>
    </Button.GestureRecognizers>
</Button>
```

The XAML compiler (`mpapp-xc`) lowers `<Button.GestureRecognizers><TapGestureRecognizer .../></Button.GestureRecognizers>` to:

```cpp
auto& _g0 = _btn.add_gesture<mpapp::tap_gesture_recognizer>();
_g0.number_of_taps_required = 2;
_g0.buttons                 = mpapp::button_mask::primary | mpapp::button_mask::secondary;
_g0.tapped.subscribe(_g0_slot, _g0_cb);  // _g0_cb wraps OnTapped
```

This is a separate task once the C++ surface is stable — captured under M-09 tooling.

## Alternatives

- **Stay with widget-specific signals (status quo).** Rejected — every new gesture needs a new signal on every widget, no composition, no XAML-1:1 parity.

- **Make recognizers wrapper-pattern components.** Rejected — recognizers don't own a native widget. The wrapper layer's job (auto-bind an embedded handler) makes no sense here. The closest analog is "a recognizer attaches to the view's handler", which we model directly via `view::map_gestures()`.

- **Visitor instead of `gesture_kind` enum + `static_cast`.** Considered. Rejected for now — adds template / virtual machinery for a 5-member closed set. `gesture_kind` + cast is fine for the first version; the visitor approach is a follow-up if the recognizer family grows beyond 7-8 members.

- **`std::variant<tap_gesture_recognizer, pan_gesture_recognizer, …>` instead of `shared_ptr<basic>`.** Considered. Rejected — `variant` is value-typed, so the binding-layer "share a ref to the same recognizer" use case becomes awkward, and the variant size scales with the largest member.

- **`Command<>` parameter instead of `mpapp::signal<>`.** Rejected — every existing event in MPAPP (clicked, activated, closed, message_received, …) uses `mpapp::signal`. Gesture events get the same treatment for surface consistency.

## Open Questions

> [!todo] Open
> - [ ] Should `DragGestureRecognizer` + `DropGestureRecognizer` land in this RFC or a follow-up? Lean follow-up — DnD has additional payload-type concerns that warrant their own design pass.
> - [ ] Position relative to attached view vs. relative to a `RelativeToContainer` per MAUI? Lean view-relative for v1; add an overload later if needed.
> - [ ] `TapGestureRecognizer.Command` / `CommandParameter` — MPAPP uses `Command<>` per [[ADR-0009-public-api-template-wrappers-only]]. Tap raises `signal<const tapped_event_args&>`. Map XAML `Command="..."` to a `Command<>`-tagged method on the view-model; tap recognizer subscribes to its own `tapped` signal and invokes the command via the XAML compiler's binding pass. Confirm with [[Markup]] before locking the XAML lowering.
> - [ ] PointerGestureRecognizer events on touch-only platforms (Android, iOS) — fall back to emitting on touch-press / touch-release? Or no-op? MAUI's choice is "behaves like UIPress / TouchListener if available, no-op otherwise". Confirm via the per-platform RFC follow-up.

## Migration / Compatibility

- `mpapp::button::clicked` (the ad-hoc signal on button only) stays in place — it's wired to a real platform event today and downstream code subscribes to it. Once gesture recognizers ship + per-platform real handlers exist, the suggested migration is "use `tap_gesture_recognizer` for any new interactive code; keep `button::clicked` for the trivial button case as a sugar layer that internally adds a tap recognizer." A follow-up RFC can decide whether to deprecate `button::clicked` or keep it as a shorthand.
- The `view` base class gains a new member (`gesture_recognizers` collection). This is a layout-affecting change to a base class — every existing surface inherits it. No source-compatibility break (no existing field is moved or removed); only `sizeof(view)` and every derived class's footprint grows by one `std::vector`. The shipped `vault/10_Architecture/Components/View.md` will be updated alongside the implementation PR.
- The wrapper-pattern migration ([[ADR-0024-wrapper-component-pattern]]) is unaffected: the wrapper's auto-bind still wires the embedded handler; the recognizer collection is attached separately via `add_gesture<T>()` after construction.

## References

- [[ADR-0008-mock-first-implementation]] — the mock-first contract this RFC honors.
- [[ADR-0024-wrapper-component-pattern]] — why recognizers DON'T use the wrapper layer.
- [[Handlers]] — the broader handler architecture this RFC extends.
- [[Components/View]] — the inheritance root that grows the `gesture_recognizers` collection.
- `references/maui/src/Controls/src/Core/TapGestureRecognizer.cs` — MAUI source-of-truth surface (per [[CLAUDE]] Rule 7).
- `references/maui/src/Controls/src/Core/Platform/GestureManager` — the per-platform wire-up reference for the upcoming real handlers.
