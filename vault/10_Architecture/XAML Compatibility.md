---
type: moc
area: markup
tags:
  - area/markup
---

# XAML Compatibility

This note is the **index** over per-component XAML compat documentation. Per [[ADR-0004-maui-xaml-superset-compat]], the goal is **full MAUI XAML compatibility plus platform-superset additions**.

The authoritative compat status for each control is in its per-component note at `10_Architecture/Components/<Name>.md`.

## How to read this page

- The **components table** is sourced from [[Controls Inventory]]. Open the [[_Bases/Components.base]] for the live filtered view.
- The **markup extensions table** below is the cross-cutting reference for non-control XAML features.

## Markup extensions

| Markup extension | MAUI behavior | MPAPP status | Notes |
|---|---|---|---|
| `{Binding path}` | Resolves a property path against `BindingContext` | not-started | Compile-time-resolved per [[Type System]] |
| `{Binding path, Mode=TwoWay}` | Bidirectional | not-started | |
| `{Binding path, Converter=...}` | Value transform | not-started | |
| `{Binding path, StringFormat=...}` | Display formatting | not-started | |
| `{StaticResource Key}` | Resource lookup at parse time | not-started | |
| `{DynamicResource Key}` | Resource lookup with change tracking | not-started | |
| `{OnPlatform iOS=..., Android=..., Default=...}` | Platform-conditional value | not-started | Lowers to MPAPP `on_platform<T>` |
| `{OnIdiom Phone=..., Tablet=..., Desktop=...}` | Device-idiom-conditional | not-started | |
| `{x:Static Type.Member}` | Reference to static C++ member | not-started | |
| `{x:Type T}` | C++ type literal | not-started | |
| `{x:Reference x:Name}` | Reference to named element | not-started | |
| `{x:Null}` | Null literal | not-started | |
| `{TemplateBinding path}` | Bind to template's parent property | not-started | |

## XAML directives

| Directive | Purpose | MPAPP status |
|---|---|---|
| `x:Class` | Code-behind class binding | not-started |
| `x:Name` | Named element reference | not-started |
| `x:Key` | Resource dictionary key | not-started |
| `x:DataType` | Compile-time binding type | not-started |
| `x:FieldModifier` | Field visibility | not-started |
| `xmlns` (default) | MAUI XAML namespace | not-started |
| `xmlns:x` | XAML language namespace | not-started |
| `xmlns:local` | User code namespace | not-started |
| `xmlns:mpapp` | MPAPP-only superset namespace | not-started |

## Resource & styling system

| Feature | MPAPP status |
|---|---|
| `ResourceDictionary` | not-started |
| `Style` (implicit) | not-started |
| `Style` (explicit, x:Key) | not-started |
| `Style.BasedOn` (inheritance) | not-started |
| `Setter` | not-started |
| `Trigger` (property) | not-started |
| `DataTrigger` | not-started |
| `EventTrigger` | not-started |
| `Behavior` | not-started |
| `DataTemplate` | not-started |
| `ControlTemplate` | not-started |

## Per-component compatibility

See [[_Bases/Components.base]] for the live filtered table. The 55 components MPAPP must support are listed in [[Controls Inventory]]; each has its own note under [[Components/]] tracking per-platform porting status.

## See also

- [[Markup]]
- [[ADR-0003-xaml-only-no-custom-dsl]]
- [[ADR-0004-maui-xaml-superset-compat]]
- [[Controls Inventory]]
- [[Components/README]]
- [[60_Research/dotnet-maui-deep-dive]] §3
