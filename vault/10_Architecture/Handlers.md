---
type: moc
area: handlers
tags:
  - area/handlers
---

# Handlers

The handler pattern is MPAPP's analog to MAUI's handler architecture (see [[60_Research/dotnet-maui-deep-dive]] §4). Each cross-platform virtual control has a small per-platform handler that owns the native widget and translates property/event changes between the two.

## Architecture

```
mpapp::Button (virtual control)
    │ owns →
    │
    ▼
mpapp::button_handler<platform_tag> (CRTP specialization)
    │ owns →
    │
    ▼
Native control
    ├─ Windows: winrt::Microsoft::UI::Xaml::Controls::Button
    ├─ Android: fbjni::global_ref<jobject>  (android.widget.Button)
    ├─ Linux:   GtkButton*
    ├─ macOS:   NSButton*
    └─ iOS:     UIButton*
```

## Static dispatch via CRTP

The cross-platform control inherits from a CRTP base:

```cpp
template <class Derived>
class control {
    // shared logic, uses static_cast<Derived*>(this) for dispatch
};

class button : public control<button> {
    Observable<std::string> text{""};
    void clicked(Command<> = {});
    // …
};
```

The handler is a partial template specialization on a `platform_tag`:

```cpp
template <class Platform>
struct button_handler;

template <>
struct button_handler<platform::windows> {
    winrt::Microsoft::UI::Xaml::Controls::Button native;
    void map_text(button& b);
    void map_clicked(button& b);
};

template <>
struct button_handler<platform::android> {
    fbjni::global_ref<jobject> native;
    void map_text(button& b);
    void map_clicked(button& b);
};
```

`if constexpr (std::same_as<Platform, platform::windows>)` selects at compile time. No v-tables for property setters.

## Property mappers as `constexpr` tables

Each handler exposes its property→setter mapping as a `constexpr std::array`:

```cpp
struct button_handler<platform::windows> {
    static constexpr std::array mappers = {
        property_mapper_entry{"text",    &button_handler::map_text},
        property_mapper_entry{"clicked", &button_handler::map_clicked},
    };
};
```

Lookup is a flat-array linear scan over ~10–30 entries — faster than `Dictionary<string, Action>` and branch-predictable.

**Customization** = inherit and override the table. No mutating a global dictionary at startup, no ordering bugs.

## Mapping table to MAUI

| MAUI artifact | MPAPP equivalent | Location in MAUI source |
|---|---|---|
| `ButtonHandler` | `mpapp::button_handler<P>` | `references/maui/src/Core/src/Handlers/Button/ButtonHandler.cs` |
| `IButtonHandler` interface | implicit from CRTP `control<button>` | `references/maui/src/Core/src/Handlers/Button/IButton.cs` |
| Property mapper (Dictionary) | `constexpr std::array<property_mapper_entry, N>` | `references/maui/src/Core/src/Handlers/Button/ButtonHandler.Standard.cs` |
| `VirtualView` / `PlatformView` | `button&` / `native_t*` (typed) | — |

## Handler lifecycle

1. **Construction.** The cross-platform control creates the handler for the active platform.
2. **Native view creation.** Handler's `create_native()` constructs the native widget.
3. **Property mapping.** For each set property, the mapper is invoked once to push the value to native.
4. **Bidirectional binding.** Native events (e.g. `UIButton` touch up) call back into `button`'s observable, which propagates to bound view-models.
5. **Destruction.** RAII releases the native ref (`global_ref` on Android, `ns_retain_ptr` on Apple, smart pointers on Windows/Linux).

## See also

- [[Type System]] — the CRTP machinery
- [[Platform Interop]] — per-platform native APIs
- [[Interop Parity]] — handler contract
- [[Controls Inventory]] — list of all controls to port
- [[Components/Button]] — example component
- `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\` — reference implementation
