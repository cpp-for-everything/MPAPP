---
type: rfc
id: RFC-0006
title: Visual State Manager — grouped pseudo-state setters routed through RFC-0005 setters
status: accepted
author: Alex Tsvetanov
created: 2026-05-29
area: markup
relatedADRs:
  - ADR-0008
  - ADR-0024
tags:
  - type/rfc
  - status/accepted
  - area/markup
  - area/widgets
---

# RFC-0006 — Visual State Manager

> [!info] Status
> **accepted** — mock surface shipped under [[T-0044-resource-dictionary-styling-mock|T-0044]]'s sibling task (this RFC's implementation ticket). Per-platform real wire-up (auto-routing system states from native input events) is captured as follow-up tasks per platform.

## Problem

MAUI's `VisualStateManager` is how every interactive control switches its visuals between pseudo-states: **Normal / Pressed / PointerOver / Disabled / Focused / Selected**. A button darkens when pressed; a `Disabled` entry greys its border; a `PointerOver` hyperlink underlines itself — all driven by attached `VisualStateGroup` lists declared once in a style.

Without VSM each control has to bind its own visual to the right input signal (`pressed`, `is_enabled.changed`, `pointer.entered`, …), duplicating the wiring per widget. MPAPP currently does exactly that — every concrete control's per-platform handler hard-codes its own pressed/disabled visuals. That's:

- Not user-customisable. Apps that want a different "Pressed" tint have to reach inside the platform handler.
- Inconsistent. Each handler chooses its own pressed colour / focus ring / disabled opacity.
- Not portable. `Disabled` on WinUI 3 fades opacity, on GTK4 it greys, on UIKit it desaturates — no shared semantic.

## Proposal

Land **`mpapp::visual_state_manager`** as a thin value type that sits beside RFC-0005's `style` and reuses the same `function<void(view&)>` setter shape:

```cpp
namespace mpapp {

class visual_state {
public:
    std::string name;
    std::unordered_map<std::string, std::function<void(view&)>> setters;
};

class visual_state_group {
public:
    std::string                name;
    std::vector<visual_state>  states;
    // The currently-applied state's name. Empty before the first
    // go_to_state call; only one state per group is active at a time.
    std::string                current_state;
};

class visual_state_manager {
public:
    std::vector<visual_state_group> groups;

    // Walk every group, find the first that contains a state named
    // `state_name`, mark it current, run its setters against `v`.
    // Returns the number of groups that transitioned — 0 means the
    // name wasn't found in any group; 1 means a normal single-group
    // transition; >1 means the same state name appears in multiple
    // groups (rare; matches MAUI's "all matching groups transition").
    int go_to_state(view& v, std::string_view state_name);
};

// MAUI parity: a single namespace of canonical state names every
// system-driven recogniser uses. Apps may define custom names freely.
namespace visual_states {
    inline constexpr std::string_view normal        = "Normal";
    inline constexpr std::string_view disabled      = "Disabled";
    inline constexpr std::string_view focused       = "Focused";
    inline constexpr std::string_view selected      = "Selected";
    inline constexpr std::string_view pointer_over  = "PointerOver";
    inline constexpr std::string_view pressed       = "Pressed";
}

} // namespace mpapp
```

Per [[ADR-0008-mock-first-implementation]] this RFC ships the C++ surface + tests; per-platform auto-routing of system states from native input events (Win/Linux/Android handlers should call `vsm.go_to_state(v, "Pressed")` on a real `PointerPressed` and `"Normal"` on release) is a follow-up per platform.

Per [[ADR-0024-wrapper-component-pattern]] VSM is NOT a wrapper-component — same call as gestures, image sources, resource dictionaries: no native widget owned, pure configuration.

## Detailed design

### File layout

```
include/mpapp/resources/
    visual_state_manager.hpp     ← all three types + go_to_state +
                                    visual_states::* string_views
tests/mock_handlers/
    visual_state_manager_test.cpp
```

### Semantics

- **Unapply is opt-in**: unlike MAUI's specificity-tracked unapply, the mock keeps things simple — only the new state's setters run on transition. Apps make each state's setters self-complete (e.g., `Normal.opacity = 1.0`, `Disabled.opacity = 0.5`) rather than relying on automatic reverts. The framework's per-platform real layer may layer reverts in later; the mock surface deliberately doesn't.
- **No-op same-state transition**: calling `go_to_state(v, "Pressed")` when the group's `current_state` is already `"Pressed"` skips setter invocation (matches MAUI's behaviour) and the per-group transition count for that group does not increment.
- **Per-group transition**: a single `go_to_state` call walks every group; each group independently checks for a matching state name. The return value is the count of groups that transitioned (excluding no-op same-state hits). Most apps have one group per VSM — so the return value is typically 0 (miss) or 1 (hit).
- **Setter exceptions** are swallowed per RFC-0005 §style — same `try/catch` pattern.

### Composition with RFC-0005

A `style` may carry `setters["__vsm__"] = [](view& v) { /* attach VSM */ };` to install a VSM onto every view the style is applied to. The hookup is opt-in — the mock surface ships only the VSM types; styling-side integration is left to the binding layer.

For attaching at view level there is no new view member — apps own the VSM (often inside a view-model or an app-level resource dictionary) and call `vsm.go_to_state(v, …)` directly. A future `view::visual_state_manager` member is intentionally **not** added here because the rich `Specificity` semantics from MAUI's BindableProperty layer don't yet exist; landing one now would commit to a shape we'd have to break when bindings land.

### Tests (mock-first)

```cpp
TEST_CASE("visual_state_manager.go_to_state runs the matching state's setters",
          "[mock][resources][vsm]") {
    mpapp::visual_state_manager vsm;
    vsm.groups.push_back(mpapp::visual_state_group{
        .name = "Common",
        .states = {
            mpapp::visual_state{
                .name = std::string{mpapp::visual_states::normal},
                .setters = { /* opacity = 1.0 */ },
            },
            mpapp::visual_state{
                .name = std::string{mpapp::visual_states::disabled},
                .setters = { /* opacity = 0.5 */ },
            },
        },
    });

    test_view v;
    CHECK(vsm.go_to_state(v, mpapp::visual_states::disabled) == 1);
}
```

## Alternatives

- **A `view::visual_state` Observable<std::string>** that controls bind to. Rejected — every control would have to duplicate the "on state change, fire X setter" wire-up; defeats the point of factoring into a shared type.
- **A static registry mapping `(target_type, state_name)` → setter**. Rejected — XAML compatibility demands the per-instance group-list shape so `<VisualStateManager.VisualStateGroups>` can lower cleanly.
- **Full specificity tracking** like MAUI's `BindableProperty.Specificity`. Deferred — needs a Bindings RFC. The mock's "make each state self-complete" guidance is the practical workaround until then.

## Open Questions

> [!todo] Open
> - [ ] Per-platform handler hook — at what point does e.g. `button_handler<platform::linux_>` call `vsm.go_to_state(b, "Pressed")` on a `GtkGestureClick.pressed` event? Probably a generic `view_handler<P>::route_system_states(view&, vsm&)` helper, like `map_gestures` does for recognizers. Captured per platform.
> - [ ] XAML lowering for `<VisualStateManager.VisualStateGroups>` — should land alongside `<Style>` element parsing in mpapp-xc (M-09).
> - [ ] State triggers (`StateTrigger`, `DataStateTrigger`) that automatically drive `go_to_state` from a property comparison. Tied to bindings.

## Migration / Compatibility

- Pure addition. No existing surface is modified.
- A future per-platform real wire-up will not change the public surface — it slots into platform handlers' bind methods.

## References

- [[ADR-0008-mock-first-implementation]].
- [[ADR-0024-wrapper-component-pattern]] — VSM is configuration, not a wrapper.
- [[RFC-0005-resource-dictionaries-and-styling]] — VSM reuses `function<void(view&)>` setters.
- `references/maui/src/Controls/src/Core/VisualStateManager.cs`.
