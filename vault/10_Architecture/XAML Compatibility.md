---
type: moc
area: markup
tags:
  - area/markup
---

# XAML Compatibility

This note is the **authoritative cross-cutting matrix** for every non-control XAML feature MPAPP must support. Per [[ADR-0004-maui-xaml-superset-compat]], the goal is *full* MAUI XAML compatibility plus a platform-superset of MPAPP-only extensions in the `xmlns:mpapp` namespace. Per [[ADR-0003-xaml-only-no-custom-dsl]] there is no parallel DSL — XAML is the only markup surface, and `mpapp-xc` (the [[XAML-Compiler]]) is the only path from `.xaml` to C++.

The compatibility surface enumerated here mirrors `references/maui/src/Controls/src/Xaml/` and its `MarkupExtensions/` subfolder, which is the spec per CLAUDE rule 7. Every markup extension, directive, resource/styling construct, trigger/behavior/template/visual-state element that compiles in MAUI XAML today must compile through `mpapp-xc` with equivalent observable behavior on every MPAPP platform per [[ADR-0006-interop-parity]].

Per-control compat status lives in each component's note under `10_Architecture/Components/<Name>.md` and rolls up into [[_Bases/Components.base]]. Use the [[Controls Inventory]] for the live filtered view. This page is the *non-control* surface.

## Phase target taxonomy

Per CLAUDE rule 3 there are no time estimates. "Phase target" refers to the milestone phase from [[_Bases/Roadmap.base]] at which the construct is first required to compile through `mpapp-xc`:

| Phase target | Meaning |
|---|---|
| `P3` | Initial XAML compiler. Must work on day one of `mpapp-xc` shipping. Covers the irreducible XAML core needed to compile a MAUI sample's `MainPage.xaml`. |
| `P4-P7` | Per-platform-specific extensions. Implemented incrementally as the platform real handlers land, per [[ADR-0008-mock-first-implementation]]. |
| `Future` | Post-v1.0. Rarely used, depends on a subsystem not yet designed, or requires runtime reflection MPAPP avoids per [[Type System]]. |

"MPAPP planned support" uses the porting status vocabulary (`not-started`, `mock`, `<platform>-real`, `parity-complete`).

## Markup extensions

Sourced from `references/maui/src/Controls/src/Xaml/MarkupExtensions/`. Every class implementing `IMarkupExtension` or `IMarkupExtension<T>` is listed here. Parameters that are themselves first-class observables in MPAPP get their own row.

| Markup ext. | MAUI behavior summary | MPAPP planned support | Phase target | Notes / refs |
|---|---|---|---|---|
| `{Binding Path}` | Resolves a property path against `BindingContext`. Default mode `OneWay`, default path `.`. | not-started | P3 | Lowers to compile-time-resolved `Binding<T,U>` per [[Type System]]; see [[Binding-Path]] |
| `{Binding Path=...}` with `Source=` | Override the binding context with an explicit object reference. | not-started | P3 | `Source` may be `{x:Reference}`, `{StaticResource}`, `{RelativeSource}`, or a literal |
| `{Binding ..., Mode=OneWay}` | Source → target only. | not-started | P3 | |
| `{Binding ..., Mode=TwoWay}` | Bidirectional. Requires writable target and `INotifyPropertyChanged`-equivalent source. | not-started | P3 | MPAPP uses `Observable<T>` from [[Observable-Property]] in lieu of `INotifyPropertyChanged` |
| `{Binding ..., Mode=OneTime}` | Snapshot at apply. | not-started | P3 | |
| `{Binding ..., Mode=OneWayToSource}` | Target → source only. | not-started | P3 | |
| `{Binding ..., Mode=Default}` | Falls back to the bindable property's `DefaultBindingMode`. | not-started | P3 | |
| `{Binding ..., Converter=}` | `IValueConverter`-equivalent transform. | not-started | P3 | MPAPP `value_converter<TFrom, TTo>` concept; compile-time-checked |
| `{Binding ..., ConverterParameter=}` | Auxiliary parameter passed to converter. | not-started | P3 | |
| `{Binding ..., StringFormat=}` | `string.Format`-style display formatting. | not-started | P3 | MPAPP uses `std::format` syntax — note divergence; see "Known intentional divergences" |
| `{Binding ..., UpdateSourceEventName=}` | Names the event that flushes target → source on a non-bindable target. | not-started | P3 | Less common; required by Entry/Editor scenarios — see [[Components/Entry]], [[Components/Editor]] |
| `{Binding ..., FallbackValue=}` | Value used when the binding fails to resolve. | not-started | P3 | |
| `{Binding ..., TargetNullValue=}` | Value used when the resolved source value is `null`. | not-started | P3 | |
| `{Binding ..., x:DataType=}` | Compile-time type assertion for the binding source. | not-started | P3 | MPAPP makes this **mandatory** when `BindingContext` cannot be inferred — divergence noted below |
| `{StaticResource Key}` | Resource lookup walking parent objects then `Application.Resources`. Snapshot at parse time. | not-started | P3 | See `StaticResourceExtension.cs` |
| `{DynamicResource Key}` | Resource lookup with change tracking. Re-resolves when the resource dictionary mutates. | not-started | P3 | Required for theming; see `DynamicResourceExtension.cs` |
| `{AppThemeBinding Light=, Dark=, Default=}` | Theme-conditional value that re-evaluates on `RequestedTheme` change. | not-started | P3 | See `AppThemeBindingExtension.cs` |
| `{OnPlatform iOS=, Android=, MacCatalyst=, WinUI=, Tizen=, Default=}` | Platform-conditional value. | not-started | P3 | Lowers to MPAPP `on_platform<T>` — compile-time selection where possible, runtime dispatch where not |
| `{OnPlatform ...}` with `<On Platform="X" Value="Y"/>` syntax | Object-element form of `OnPlatform`. | not-started | P3 | Equivalent to attribute form |
| `{OnIdiom Phone=, Tablet=, Desktop=, TV=, Watch=, Default=}` | Device-idiom-conditional value. | not-started | P3 | See `OnIdiomExtension.cs` |
| `{x:Static Type.Member}` | Reference to a static C++ member (constexpr field, static function returning a value). | not-started | P3 | Type must resolve via `xmlns` |
| `{x:Type T}` | C++ type literal — emits a `type_info` token usable as a value. | not-started | P3 | Required by `DataTemplate.DataType`, `RelativeSource.AncestorType`, `x:Array.Type` |
| `{x:Reference x:Name}` | Reference to a named element in the same XAML scope. | not-started | P3 | See `ReferenceExtension.cs`; resolved at compile time |
| `{x:Null}` | Null literal — required because XAML attribute values can't naturally express null. | not-started | P3 | Lowers to `std::nullopt` or `nullptr` depending on target type |
| `{x:Array Type=..., ...}` | Inline typed array. | not-started | P3 | See `ArrayExtension.cs`; lowers to `std::array` or `std::vector` |
| `{TemplateBinding Path}` | Bind to a property on the templated parent. Used inside `ControlTemplate`. | not-started | P3 | See `TemplateBindingExtension.cs` |
| `{RelativeSource Self}` | Source = the element itself. | not-started | P3 | Common in style triggers |
| `{RelativeSource TemplatedParent}` | Source = the templated parent (equivalent to `TemplateBinding`). | not-started | P3 | |
| `{RelativeSource AncestorType={x:Type T}}` | Source = nearest ancestor of type T. | not-started | P3 | `AncestorLevel` supported for ordinal selection |
| `{RelativeSource AncestorType=..., AncestorLevel=N}` | Source = N-th ancestor of type T. | not-started | P3 | |
| `{RelativeSource FindAncestor, AncestorType=...}` | Explicit `FindAncestor` mode. | not-started | P3 | |
| `{RelativeSource FindAncestorBindingContext, AncestorType=...}` | Source = nearest ancestor whose `BindingContext` is of type T. | not-started | P3 | |
| `{FontImage FontFamily=, Glyph=, Color=, Size=}` | Constructs an `ImageSource` from a glyph in a font. | not-started | P3 | Used heavily by `Button.ImageSource`, `MenuItem.IconImageSource` — see [[Components/Button]] |
| `{NullExtension}` | Verbose form of `{x:Null}`. | not-started | P3 | See `NullExtension.cs`; rarely written but supported |
| `{TypeExtension Type=...}` | Verbose form of `{x:Type}`. | not-started | P3 | See `TypeExtension.cs` |
| `{StaticExtension Member=Type.Field}` | Verbose form of `{x:Static}`. | not-started | P3 | See `StaticExtension.cs` |
| `{DataTemplate Type}` | Constructs a `DataTemplate` whose root is the given type. | not-started | P3 | See `DataTemplateExtension.cs`; common in `ItemsView.ItemTemplate` |
| `{StyleSheet Source=}` / `{StyleSheet}` inline | CSS-style stylesheet attachment to a `VisualElement.Resources`. | not-started | Future | MPAPP intentionally defers CSS-style selectors — see "Known intentional divergences" |
| `{Constraint Property=, Source=, Factor=, Constant=}` | Used inside `RelativeLayout` to express constraints. | not-started | Future | Tied to `RelativeLayout`, which MAUI itself has deprecated — see "Known intentional divergences" |
| `{ConstraintExpression Type=, Property=, ...}` | Verbose constraint form. | not-started | Future | Same fate as `Constraint` |
| `{mpapp:Computed Expression=}` | **MPAPP-only.** Reactive computed expression evaluated against the binding context. | not-started | P3 | Lowers to `Computed<...>`; superset-only, no MAUI equivalent |
| `{mpapp:OnHost Windows=, Android=, Linux=, macOS=, iOS=, Default=}` | **MPAPP-only.** Adds Linux as a first-class platform branch. | not-started | P3 | Superset of MAUI `OnPlatform`; required because MAUI has no Linux branch |
| `{mpapp:OnHandlerVersion ...}` | **MPAPP-only.** Branches on the runtime handler API level (e.g. WinUI 3 vs. legacy Win32 backend). | not-started | Future | Reserved; no consumer yet |

## XAML directives (the `xmlns:x` namespace)

Every directive in the language namespace `http://schemas.microsoft.com/winfx/2009/xaml`. These are processed by the parser before user-defined extension resolution.

| Directive | Purpose | MPAPP planned support | Phase target | Notes / refs |
|---|---|---|---|---|
| `x:Class` | Names the code-behind C++ class generated for this XAML file. | not-started | P3 | Required on every root element of a compiled XAML file |
| `x:ClassModifier` | C# visibility (`public`/`internal`) for the generated class. | not-started | P3 | Maps to `MPAPP_EXPORT` / file-scope visibility |
| `x:Name` | Names an element so it can be referenced via `{x:Reference}` or accessed as a field on the code-behind class. | not-started | P3 | Lowers to a typed field on the code-behind |
| `x:FieldModifier` | C# visibility of the generated field for an `x:Name`d element (`public`, `internal`, `private`). | not-started | P3 | Defaults to `private` in MPAPP regardless of MAUI default to match C++ convention |
| `x:Key` | Resource dictionary key. Required on `ResourceDictionary` entries unless the entry is an implicit `Style`. | not-started | P3 | |
| `x:TypeArguments` | Generic type arguments for the root element. | not-started | P3 | Critical for `ContentPage`/`ContentView` subclasses; lowers to C++ template instantiation |
| `x:Arguments` | Constructor arguments for an element that has no parameterless constructor. | not-started | P3 | Used by `Color`, `Thickness`, custom types |
| `x:FactoryMethod` | Names a static factory used in place of a constructor. | not-started | P3 | Less common but required by some `FontFamily`/`FontImageSource` scenarios |
| `x:DataType` | Compile-time data type for the binding context within this element subtree. | not-started | P3 | MPAPP **requires** this for every binding that does not have an explicit `Source` — divergence from MAUI's optional treatment |
| `x:Uid` | Unique ID for resource-driven localization. | not-started | Future | Tied to `.resw` / `.resx`-style localization; MPAPP's localization story (see "Known intentional divergences") is not yet designed |
| `x:Code` | Inline C# code block inside XAML. | not-started | Never | MPAPP **rejects** inline code by design — see "Known intentional divergences" |
| `xmlns` (default) | MAUI XAML default namespace `http://schemas.microsoft.com/dotnet/2021/maui`. | not-started | P3 | Aliased in MPAPP to its own URI plus a compatibility shim accepting the MAUI URI verbatim |
| `xmlns:x` | XAML language namespace. | not-started | P3 | |
| `xmlns:local="clr-namespace:..."` | User CLR namespace declaration. | not-started | P3 | MPAPP form: `xmlns:local="cpp-namespace:my::app"`; the MAUI `clr-namespace:` form is also accepted via the compat shim |
| `xmlns:local="clr-namespace:...;assembly=..."` | CLR namespace with assembly disambiguation. | not-started | P3 | `;assembly=` is parsed but ignored — MPAPP has no per-assembly types; the symbol must be unambiguous |
| `xmlns:mpapp` | **MPAPP-only.** Superset namespace for MPAPP extensions. | not-started | P3 | URL: `http://schemas.mpapp.dev/2025/xaml` |
| `mc:Ignorable` | Markup-compatibility ignorable namespace prefixes. | not-started | P3 | Required for tooling round-trip with mixed-namespace XAML |
| `d:` design-time namespace | Design-time-only properties (`d:DataContext`, `d:DesignWidth`). | not-started | P4-P7 | Parsed and stripped before lowering; tooling-only |

## Resource & styling system

The resource and styling system covers `ResourceDictionary` plumbing, `Style` definition, and the `Setter` mechanism.

| Feature | MAUI behavior summary | MPAPP planned support | Phase target | Notes / refs |
|---|---|---|---|---|
| `ResourceDictionary` (inline) | Attached to any `VisualElement.Resources`. Scoped to that element's subtree. | not-started | P3 | |
| `ResourceDictionary` with `Source=` | External XAML file merged in. Source is a relative URI to a `.xaml`. | not-started | P3 | Compile-time-resolved into the same translation unit |
| `ResourceDictionary.MergedDictionaries` | Composes multiple dictionaries. Lookups walk in declaration order. | not-started | P3 | |
| `Application.Resources` | App-global resource dictionary. Last fallback in resource resolution. | not-started | P3 | See [[Components/Application]] |
| `Style` (implicit, `TargetType=...` only, no `x:Key`) | Applies to every instance of `TargetType` in scope. | not-started | P3 | |
| `Style` (explicit, `x:Key=...`) | Applies only when referenced by `Style="{StaticResource Key}"`. | not-started | P3 | |
| `Style.BasedOn={StaticResource Parent}` | Inherits setters and triggers from a parent style. | not-started | P3 | |
| `Style.ApplyToDerivedTypes` | When `true`, an implicit style for `TargetType=Button` also applies to `ImageButton`, etc. | not-started | P3 | See [[Components/Button]], [[Components/ImageButton]] |
| `Style.CanCascade` | Allows the style to cascade into nested `VisualElement`s of the same `TargetType`. | not-started | P4-P7 | Less common; opt-in cascade rules |
| `Setter Property=... Value=...` | Sets a property when the style is applied. | not-started | P3 | |
| `Setter` (sub-element form) for complex values | `<Setter Property="X"><Setter.Value>...</Setter.Value></Setter>`. | not-started | P3 | |
| `Setter Property="VisualStateManager.VisualStateGroups"` | Attached VSM groups carried by the style. | not-started | P3 | See VSM section below |
| `Setter Property="..." Value="{StaticResource ...}"` | Resource-derived setter value. | not-started | P3 | |
| `Setter Property="..." Value="{DynamicResource ...}"` | Setter value that re-resolves on dictionary change. | not-started | P3 | |
| `Style.Resources` | Per-style resource dictionary, scoped only to setters/triggers within the style. | not-started | P4-P7 | |
| Implicit `Style` lookup order | Element-local → parent chain → `Application.Resources`. | not-started | P3 | Identical to MAUI |

## Triggers & behaviors

Triggers and behaviors are first-class XAML constructs in MAUI. MPAPP mirrors all of them.

| Feature | MAUI behavior summary | MPAPP planned support | Phase target | Notes / refs |
|---|---|---|---|---|
| `Trigger Property=... Value=...` (property trigger on the element itself) | Fires setters while the named property has the named value. Reverts on exit. | not-started | P3 | |
| `Trigger.EnterActions` | `TriggerAction`s run when the trigger condition becomes true. | not-started | P3 | |
| `Trigger.ExitActions` | `TriggerAction`s run when the trigger condition becomes false. | not-started | P3 | |
| `Trigger.Setters` | Setters applied while the trigger is active. | not-started | P3 | |
| `DataTrigger Binding="{Binding ...}" Value="..."` | Trigger driven by a binding rather than a property. | not-started | P3 | |
| `DataTrigger` with `Setters`, `EnterActions`, `ExitActions` | Same lifecycle as `Trigger`. | not-started | P3 | |
| `EventTrigger Event="..."` | Fires actions on the named CLR event. No reversion. | not-started | P3 | MPAPP maps to its `Signal<...>` event surface |
| `EventTrigger.Actions` collection | Each action runs in order. | not-started | P3 | |
| `MultiTrigger Conditions` | Trigger that fires when all listed `PropertyCondition`/`BindingCondition`s match. | not-started | P3 | |
| `PropertyCondition Property=... Value=...` | Used inside `MultiTrigger.Conditions`. | not-started | P3 | |
| `BindingCondition Binding=... Value=...` | Used inside `MultiTrigger.Conditions`. | not-started | P3 | |
| `TriggerAction<T>` (user-defined) | Custom action invoked from a trigger. | not-started | P3 | Lowers to a C++ class implementing `trigger_action<T>` |
| `Behavior` (base class, on `VisualElement.Behaviors`) | Reusable attachable behavior. Lifecycle hooks `OnAttachedTo`/`OnDetachingFrom`. | not-started | P3 | |
| `Behavior<T>` (typed) | Strongly typed behavior; `T` must be assignable to the attached element. | not-started | P3 | Native C++ template form, no boxing |
| `Behavior` with bindable properties | Behaviors can declare bindable properties to be configured in XAML. | not-started | P3 | |
| `EventToCommandBehavior` (community-style behavior) | Pattern: forwards an event to an `ICommand`-equivalent. | not-started | P4-P7 | MPAPP provides the equivalent under `mpapp::behaviors::event_to_command<>`; users may write their own |

## Templates

| Feature | MAUI behavior summary | MPAPP planned support | Phase target | Notes / refs |
|---|---|---|---|---|
| `DataTemplate` (inline) | Item-content template. Used by `ItemsView`, `BindableLayout`, `Picker.ItemDisplayBinding`. | not-started | P3 | See [[Components/BindableLayout]], [[Components/Picker]] |
| `DataTemplate` (by `x:Key`, referenced via `{StaticResource}`) | Reusable template stored in a `ResourceDictionary`. | not-started | P3 | |
| `DataTemplate` with `x:DataType` | Compile-time-typed template item. | not-started | P3 | Required by MPAPP's strict binding model; advisory in MAUI |
| `DataTemplateSelector` | User-defined selector returning a `DataTemplate` per item. | not-started | P3 | Lowers to a C++ class implementing `data_template_selector<TItem>` |
| `ControlTemplate` | Replaces a templated control's visual tree. | not-started | P3 | See [[Components/TemplatedView]] |
| `ControlTemplate` with `TemplateBinding` | Inside-template references to the templated parent's properties. | not-started | P3 | |
| `ContentPresenter` | Marker inside a `ControlTemplate` where the templated parent's `Content` is injected. | not-started | P3 | |
| `ItemsView.ItemTemplate` | Item template applied to each item in a collection. | not-started | P3 | Per [[ADR-0009-public-api-template-wrappers-only]], the public surface uses C++ templates for type safety |
| `Shell.ItemTemplate` / `Shell.MenuItemTemplate` | Shell-specific templates. | not-started | P4-P7 | See [[Components/Shell]] |
| `ListView.HeaderTemplate` / `FooterTemplate` / `GroupHeaderTemplate` | Per-region templates for collection controls. | not-started | P4-P7 | See [[Components/ListView]] |
| `RefreshView.RefreshContentTemplate` | Pull-to-refresh content. | not-started | P4-P7 | See [[Components/RefreshView]] |
| `SwipeView.SwipeItems` templates | Templated swipe items per side. | not-started | P4-P7 | See [[Components/SwipeView]], [[Components/SwipeItemView]] |

## Visual State Manager

The `VisualStateManager` (VSM) is its own subsystem in MAUI and gets its own row group.

| Feature | MAUI behavior summary | MPAPP planned support | Phase target | Notes / refs |
|---|---|---|---|---|
| `VisualStateManager.VisualStateGroups` (attached property) | Container for `VisualStateGroup`s on an element. | not-started | P3 | |
| `VisualStateGroup Name=...` | Group of mutually-exclusive states. | not-started | P3 | |
| `VisualState Name=...` | A single named state with `Setters`. | not-started | P3 | |
| `VisualState.Setters` | Property setters applied while the state is active. | not-started | P3 | |
| `VisualState.StateTriggers` | Conditions that automatically switch the state. | not-started | P3 | Includes `StateTrigger`, `AdaptiveTrigger`, custom triggers |
| `StateTrigger IsActive=...` | Boolean-driven state trigger. | not-started | P3 | |
| `AdaptiveTrigger MinWindowWidth=..., MinWindowHeight=...` | Viewport-driven state trigger. | not-started | P3 | Cross-platform — MPAPP wires this to its host-window metrics |
| Common state names: `Normal`, `Disabled`, `Focused`, `PointerOver`, `Pressed`, `Selected` | Conventional state names recognized by certain handlers. | not-started | P3 | Per-control state vocabulary documented in each component note |
| `VisualStateManager.GoToState(element, stateName)` | Programmatic state change. | not-started | P3 | Exposed as `vsm::go_to_state(el, "Pressed")` |
| `Setter Property="VisualStateManager.VisualStateGroups" Value="..."` | VSM carried by a `Style`. | not-started | P3 | Common in implicit button styles |
| `mpapp:HoverState` extensions | **MPAPP-only.** Per-pointer hover states on desktop platforms with first-class touch+mouse simultaneous input. | not-started | P4-P7 | Superset; see "Custom namespaces & extensibility" |

## Custom namespaces & extensibility

XAML's namespace system is the primary extensibility point. MAUI accepts CLR namespaces; MPAPP accepts C++ namespaces.

| Construct | MAUI behavior summary | MPAPP planned support | Phase target | Notes / refs |
|---|---|---|---|---|
| `xmlns:local="clr-namespace:My.App"` | Imports a CLR namespace. | not-started | P3 | Compatibility shim — `clr-namespace:` URIs are parsed and remapped to the C++ namespace of the same shape (`My::App`). For mixed code-behind projects the original spelling is preserved in diagnostics |
| `xmlns:local="cpp-namespace:my::app"` | **MPAPP-only.** Native form. | not-started | P3 | Preferred in new MPAPP-only XAML files |
| `xmlns:mpapp="http://schemas.mpapp.dev/2025/xaml"` | Superset namespace for MPAPP extensions. | not-started | P3 | Includes `mpapp:Computed`, `mpapp:OnHost`, `mpapp:HoverState`, and future extensions |
| `XmlnsDefinition` assembly attribute | Maps multiple CLR namespaces to a single XAML URI. | not-started | P4-P7 | MPAPP equivalent: a compile-time `xmlns_definition` registry consumed by `mpapp-xc` |
| `XmlnsPrefix` assembly attribute | Suggests a default prefix for an `xmlns` URI. | not-started | P4-P7 | Tooling hint only — no runtime behavior |
| `ContentProperty` on a custom type | Declares which property receives child elements when no property element is used. | not-started | P3 | Mandatory for ergonomic XAML; MPAPP equivalent is the `mpapp::content_property` C++ attribute |
| `TypeConverter` on a custom type | Converts an XAML attribute string into a typed value. | not-started | P3 | MPAPP equivalent: `from_xaml_attribute(...)` free function found via ADL |
| Attached properties on user types | `[AttachedPropertyBrowsableForType(...)]` and the `bool TryGet/Set` pattern. | not-started | P3 | MPAPP equivalent: `attached_property<TOwner, TValue>` template |
| Custom `IMarkupExtension` | User-defined `{Foo}` markup extensions. | not-started | P3 | MPAPP equivalent: a `markup_extension<T>` template specialization the compiler discovers via ADL |
| Custom `IValueConverter` | User-defined binding converters. | not-started | P3 | See `Converter=` row in the bindings table |

## Per-control compatibility

Per-component XAML compat is tracked in each control's note under `10_Architecture/Components/<Name>.md`. The live filtered view is [[_Bases/Components.base]]; the full list is [[Controls Inventory]]; the index is [[Components/README]]. Attached layout properties (`Grid.Row`, `Grid.Column`, `Grid.RowSpan`, `Grid.ColumnSpan`, `AbsoluteLayout.LayoutBounds`, `AbsoluteLayout.LayoutFlags`, `FlexLayout.Basis`, `FlexLayout.Grow`, etc.) lower through `mpapp-xc` to typed accessor calls; per-property compat lives on the host layout's component note, not here.

Selected component anchors for the markup features above:

- [[Components/Application]] — `Application.Resources`, theme switching that drives `AppThemeBinding`.
- [[Components/Button]], [[Components/ImageButton]] — `FontImageSource`, `ApplyToDerivedTypes`.
- [[Components/Entry]], [[Components/Editor]] — `UpdateSourceEventName` end-to-end scenarios.
- [[Components/BindableLayout]] — non-`ItemsView` `DataTemplate` host.
- [[Components/Picker]] — `ItemDisplayBinding`, `DataTemplate`.
- [[Components/ListView]] — header/footer/group templates.
- [[Components/RefreshView]], [[Components/SwipeView]], [[Components/SwipeItemView]] — pull/swipe template hosts.
- [[Components/TemplatedView]] — `ControlTemplate` consumer.
- [[Components/Shell]] — Shell-specific templates.

## Known intentional divergences

Per [[ADR-0004-maui-xaml-superset-compat]] MPAPP aims for a *superset* — XAML that compiles in MAUI compiles in MPAPP — but a small set of MAUI surface is **intentionally** rejected, deferred, or behaves differently. Each item below is a conscious decision recorded so reviewers can spot accidental drift.

1. **`x:Code` blocks are rejected.** MAUI allows embedding C# directly via `<x:Code>`. MPAPP does not parse C++ as a sub-language of XAML; the only code-behind form is a separate `.cpp` linked by `x:Class`. `mpapp-xc` emits a hard error. Consistent with [[ADR-0003-xaml-only-no-custom-dsl]].

2. **`x:DataType` is mandatory where MAUI treats it as advisory.** MAUI degrades to reflection-based bindings if `x:DataType` is absent. MPAPP has no runtime reflection (see [[Type System]]), so every `{Binding}` whose source type cannot be inferred from an explicit `Source=` requires `x:DataType` on the enclosing element. `mpapp-xc` raises an error. Users porting MAUI XAML add `x:DataType` once at the data template root — also the recommended path for AOT MAUI builds.

3. **`StringFormat` uses `std::format` syntax, not `string.Format`.** A compatibility shim rewrites the common MAUI patterns (`{0}`, `{0:N2}`, `{0:C}`, `{0:P}`) into `std::format` equivalents, with a diagnostic for anything else.

4. **`{StyleSheet}` is deferred to `Future`.** Adoption is low, the selector-resolution rules duplicate `Style` semantics, and the implementation cost is non-trivial. Tracked but not shipped before v1.0.

5. **`{Constraint}` / `{ConstraintExpression}` are deferred to `Future`.** Their host `RelativeLayout` is itself deprecated in MAUI. Legacy XAML using them must migrate to `Grid`/`FlexLayout` before compiling under `mpapp-xc`.

6. **`x:Uid` localization is deferred to `Future`.** MPAPP's localization story is not yet designed. The directive parses and is preserved in the AST.

7. **Legacy `OnPlatform` branches (`UWP`, `WPF`, `GTK`).** Parsed and routed to the closest equivalent (`WinUI` for `UWP`/`WPF`, ignored for `GTK`) with a deprecation diagnostic.

8. **Linux is a first-class platform.** MAUI has no Linux branch in `OnPlatform`. MPAPP recognises both the MAUI-shaped form (Linux falls through to `Default`) and the superset `mpapp:OnHost` extension. Per [[ADR-0006-interop-parity]]; additive, not a break.

9. **`x:FieldModifier`-defaulted visibility differs.** MPAPP defaults to `private` but uses a `no_unique_address`-style storage layout when possible — invisible to user code.

10. **`IServiceProvider` is not user-facing.** MPAPP's `markup_extension<T>` concept exposes a typed service set discovered at compile time. Code calling `IServiceProvider.GetService(typeof(...))` does not port directly; extensions that simply hold properties and produce a value port one-for-one.

## Compiler and tooling notes

`mpapp-xc` lowers every construct above to a C++ translation unit, preserving source location for diagnostics. Per [[Hot Reload]] every construct must also have a *runtime apply* path so dev builds can replace trees without recompilation — this is why `x:Code` is rejected (no runtime C++ compiler), but every other construct has a `Future`-or-sooner target.

The matrix is normative: any markup feature in MAUI but missing from this page is a bug in this page. When in doubt, open the file under `references/maui/src/Controls/src/Xaml/` and add a row.

## See also

- [[Markup]] — markup subsystem architecture.
- [[XAML-Compiler]] — `mpapp-xc` compiler design.
- [[ADR-0003-xaml-only-no-custom-dsl]], [[ADR-0004-maui-xaml-superset-compat]], [[ADR-0006-interop-parity]], [[ADR-0009-public-api-template-wrappers-only]].
- [[Controls Inventory]], [[Components/README]], [[_Bases/Components.base]].
- [[60_Research/dotnet-maui-deep-dive]] §3.
