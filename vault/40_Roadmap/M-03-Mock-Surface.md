---
type: milestone
id: M-03
title: Full mock surface — every MAUI component as a C++ mock
phase: P2
status: planned
deliverables:
  - Every component in Controls Inventory has a mock C++ class
  - Observable<T> properties wired up
  - Dummy platform handlers that log/no-op
  - Mock-based unit tests with 100% line + branch coverage of the public API
  - mpapp-xc handles a representative XAML subset (Button, Label, Entry, VerticalStackLayout, Grid, ScrollView + {Binding}, {OnPlatform}, {StaticResource})
exitCriteria:
  - "Every row in Controls Inventory is at mpappStatus: mock"
  - "Mock test suite has 100% line + branch coverage"
  - "mpapp-xc XAML round-trip works for representative subset"
tags:
  - type/milestone
  - phase/p2
  - status/planned
---

# M-03 — Full Mock Surface

> [!info] Status
> **planned**. Starts after [[M-02-Infrastructure]] closes.

## Scope

The user's key strategy (per [[ADR-0008-mock-first-implementation]]): stand up the **entire** public API surface as mock C++ classes before any platform interop. This locks the API design end-to-end before any platform pressures it.

## Exit Criteria

- [ ] Every component in [[Controls Inventory]] has:
  - A C++ class with the full public API.
  - `Observable<T>` properties.
  - A dummy `<name>_handler<platform::mock>` that logs / no-ops.
  - Mock-based unit tests covering change notifications, binding paths, command invocation, and layout-measurement contracts.
- [ ] 100% line + branch coverage on the public API.
- [ ] `mpapp-xc` handles a representative XAML subset (see deliverables).
- [ ] No platform-specific code yet — handlers are stubs.

## Risks

> [!warning]
> - Mock tests can pass falsely if they don't pin the contract precisely. Add conformance assertions early.
> - The mock surface is large; sequencing matters. Suggest: simple controls → containers → navigation → complex controls.

## Sequencing (within M-03)

1. **Layout & primitives**: View, Layout, VerticalStackLayout (BindableLayout), Grid, ScrollView, Border, Frame, BoxView.
2. **Simple inputs**: Button, Label, Entry, Editor, Switch, Slider, Stepper, CheckBox, RadioButton.
3. **Pickers & date/time**: Picker, DatePicker, TimePicker, SearchBar.
4. **Images & graphics**: Image, ImageButton, GraphicsView, ShapeView.
5. **Navigation**: Page, ContentPage, NavigationPage, FlyoutPage, TabbedPage, Shell.
6. **Collections**: ListView, TableView, IndicatorView.
7. **Complex**: WebView, HybridWebView, RefreshView, SwipeView, SwipeItemView, SwipeItemMenuItem.
8. **Menus & toolbars**: MenuBar, MenuBarItem, MenuFlyout*, Toolbar, TitleBar.
9. **Activity / progress**: ActivityIndicator, ProgressBar.
10. **App-level**: Application, Window, Element, TemplatedView, ContentView, FlyoutView, TabbedView.

## Tasks

Linked via [[_Bases/Tasks.base]] filtered by `milestone == "M-03"`. Tasks are created on a per-component or per-component-group basis as work begins.

## See in code

- Public surface headers (one per component): [`include/mpapp/`](../../include/mpapp/) — 79 headers covering every MAUI control listed in [[Controls Inventory]].
- Mock handlers (the mock-first deliverable): [`include/mpapp/handlers/mock/`](../../include/mpapp/handlers/mock/) — 66 `<component>_handler<platform::mock>` files. Each records `map_<property>` calls into a `calls()` vector for assertion.
- Tests for the mock surface: [`tests/mock_handlers/`](../../tests/mock_handlers/) — 66 `<component>_test.cpp` files, glob-included so adding a component's tests doesn't need a CMakeLists edit.
- XAML compiler scaffolding (this milestone's representative-subset goal): [`tools/mpapp-xc/`](../../tools/mpapp-xc/).
- Type-system primitives that the mock surface stands on: [`include/mpapp/observable.hpp`](../../include/mpapp/observable.hpp) + [`computed.hpp`](../../include/mpapp/computed.hpp) + [`command.hpp`](../../include/mpapp/command.hpp) + [`signal.hpp`](../../include/mpapp/signal.hpp).

## Related

- [[ADR-0008-mock-first-implementation]]
- [[Controls Inventory]]
- [[Components/README]]
- [[Test Harness]]
