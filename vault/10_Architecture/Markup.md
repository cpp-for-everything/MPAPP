---
type: moc
area: markup
tags:
  - area/markup
---

# Markup

Per [[ADR-0003-xaml-only-no-custom-dsl]], MPAPP supports **XAML as the only markup language**. The C++ API is canonical; XAML is sugar.

Per [[ADR-0004-maui-xaml-superset-compat]], compatibility scope is the **full MAUI XAML set plus platform supersets**.

## The compiler: `mpapp-xc`

`mpapp-xc` is a C++ tool (cross-platform per [[ADR-0007-cross-platform-tooling]]) that:

1. Parses `.xaml` files (libxml2 + custom MAUI-XAML schema).
2. Resolves markup extensions, x:Static / x:Type references, resource dictionaries, styles, triggers.
3. Emits `consteval` C++ functions that, when called, build the equivalent visual tree using MPAPP's public API.
4. Embeds `#line` directives so any C++ compile error from the generated tree points back to the original XAML location.

```
MainPage.xaml          mpapp-xc           MainPage.gen.hpp           C++ compiler
─────────────       ──────────────►     ─────────────────────►     ──────────────►   .obj
  <VerticalStack    parse + lower       consteval auto Build() {    type-checks      links into
    <Button …/>                            return mpapp::v_stack    against MPAPP    final binary
    <Label …/>                                { mpapp::button{…},   public API
  </VerticalStack>                            mpapp::label{…} };
                                          }
```

## Round-trip equivalence

```xml
<!-- MainPage.xaml -->
<VerticalStackLayout Padding="20" Spacing="10">
    <Label Text="{Binding name}" FontSize="24"/>
    <Button Text="Click" Command="{Binding increment}"/>
</VerticalStackLayout>
```

```cpp
// Hand-written equivalent — same observable behavior.
auto build_main_page(todo_view_model& vm) {
    return mpapp::vertical_stack_layout{
        .padding = 20,
        .spacing = 10,
        .children = std::tuple{
            mpapp::label{ .text = bind(&todo_view_model::name), .font_size = 24 },
            mpapp::button{
                .text    = "Click",
                .command = bind(&todo_view_model::increment),
            },
        },
    };
}
```

## Diagnostics — the key UX commitment

C++ error messages from generated code are notoriously bad. MPAPP treats diagnostic quality as a first-class product concern:

- Every generated line carries `#line N "MainPage.xaml"` so the compiler reports errors at the XAML source location.
- `mpapp-xc` performs its own static checks (does the binding path resolve? does the type match?) before handing off to C++, producing XAML-level errors with full source spans.
- The LSP for `.xaml` files surfaces both `mpapp-xc` errors and downstream C++ errors that map back via source maps.

## Compatibility scope (live tracking)

The compat matrix lives in [[XAML Compatibility]], which is an index over the per-component docs in [[Components/]]. The per-component docs are the authoritative source.

## Hot reload

XAML hot reload is simpler than C++ hot reload (see [[Hot Reload]]). On save:

1. `mpapp-xc` recompiles just the changed `.xaml`.
2. The running app receives the new `consteval`-built visual tree.
3. The visual tree is swapped via a stable handle table.
4. Bound data is preserved.

Works on every dev surface that runs MPAPP — desktop and emulators alike.

## See in code

- [`tools/mpapp-xc/`](../../tools/mpapp-xc/) — the XAML compiler binary. Reads `.xaml`, emits `consteval` C++ that calls into the public component API.
- [`tools/mpapp/`](../../tools/mpapp/) — the `mpapp` developer CLI wrapping the XAML compiler invocation and other developer commands. Cross-platform per Rule 12.
- The emit target — every `mpapp::<component>` type in [`include/mpapp/`](../../include/mpapp/) — is the same public C++ surface a hand-written UI uses; the generated `consteval` tree imports no codegen-private headers.

## See also

- [[ADR-0003-xaml-only-no-custom-dsl]]
- [[ADR-0004-maui-xaml-superset-compat]]
- [[XAML Compatibility]]
- [[Controls Inventory]]
- [[Hot Reload]]
- [[60_Research/dotnet-maui-deep-dive]] §3 (XAML)
