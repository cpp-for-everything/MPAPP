---
type: adr
id: ADR-0004
title: XAML compatibility is full MAUI set plus platform supersets
status: accepted
decisionDate: 2026-05-12
deciders:
  - Alex Tsvetanov
supersedes: ""
supersededBy: ""
area: markup
tags:
  - type/adr
  - status/accepted
  - area/markup
---

# ADR-0004 — XAML compatibility is full MAUI set plus platform supersets

> [!success] Status
> **accepted** on 2026-05-12.

## Context

[[ADR-0003-xaml-only-no-custom-dsl]] commits to XAML as the only markup. The remaining question is *scope*: a subset of MAUI XAML, the whole set, or a superset?

The user's directive: the **whole MAUI XAML set** must be supported, and **platform-specific MPAPP additions** are allowed where MAUI lacks a component.

This decision makes the work easy to slice: every handler/control under `D:\GitHub\MPAPP\maui\src\Core\src\Handlers\` and every control type under `D:\GitHub\MPAPP\maui\src\Controls\src\Core\` becomes one MPAPP component.

## Decision

`mpapp-xc` must accept any well-formed MAUI XAML document and produce an MPAPP build artifact with **equivalent observable behavior**. Compatibility scope:

- **Controls** — every MAUI handler and control: ActivityIndicator, Application, Border, Button, CheckBox, ContentView, DatePicker, Editor, Element, Entry, FlyoutView, GraphicsView, HybridWebView, Image, ImageButton, IndicatorView, Label, Layout, MenuBar, MenuBarItem, MenuFlyout*, NavigationPage, Page, Picker, ProgressBar, RadioButton, RefreshView, ScrollView, SearchBar, ShapeView, Slider, Stepper, SwipeItemMenuItem, SwipeItemView, SwipeView, Switch, TabbedView, TimePicker, Toolbar, View, WebView, Window, BindableLayout, BoxView, ContentPage, FlyoutPage, Frame, ListView, Shell, TabbedPage, TableView, TemplatedView, TitleBar. (Tracked in [[Controls Inventory]].)
- **Markup extensions** — `{Binding}`, `{StaticResource}`, `{DynamicResource}`, `{OnPlatform}`, `{OnIdiom}`, `{x:Static}`, `{x:Type}`, `{x:Reference}`, `{x:Null}`, `{TemplateBinding}`.
- **XAML directives** — `x:Class`, `x:Name`, `x:Key`, `x:DataType`, `x:FieldModifier`, `xmlns` and custom namespaces.
- **Constructs** — resource dictionaries, styles (implicit + explicit), triggers (property, data, event), behaviors, data templates, control templates.

**Superset additions** are allowed for platform-specific MPAPP controls or attributes that have no MAUI equivalent. They live in the `mpapp:` XML namespace and never collide with the default MAUI namespace.

Documentation lives **one file per component** in `10_Architecture/Components/<Name>.md`. The cross-cutting matrix in [[XAML Compatibility]] is an index over those files.

## Consequences

### Positive

- Crystal-clear work-slicing: 55 components × 5 platforms.
- Strong migration story for MAUI users: their XAML files work.
- Per-component docs catch divergences before they ship silently.

### Negative

- Large surface to cover (55+ components, all markup extensions). The work is mechanical but not small.
- Some MAUI quirks (legacy properties, deprecated controls) must be preserved for compatibility.

### Neutral

- The C++ API and XAML scope grow together; neither can lead the other.

## Alternatives Considered

- **Subset compatibility.** Rejected — the user explicitly wants the whole set.
- **Custom XAML dialect with breaking changes from MAUI.** Rejected — defeats migration goal.

## References

- [[ADR-0003-xaml-only-no-custom-dsl]]
- [[10_Architecture/XAML Compatibility]]
- [[10_Architecture/Controls Inventory]]
- `D:\GitHub\MPAPP\maui\src\Core\src\Handlers\`
- `D:\GitHub\MPAPP\maui\src\Controls\src\Core\`
