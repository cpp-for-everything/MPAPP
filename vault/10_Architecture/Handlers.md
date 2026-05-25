---
type: moc
area: handlers
tags:
  - area/handlers
---

# Handlers

The handler pattern is MPAPP's analog to MAUI's handler architecture (see [[60_Research/dotnet-maui-deep-dive]] §4). Each cross-platform virtual control has a small per-platform handler that owns the native widget and translates property/event changes between the two.

Since [[ADR-0024-wrapper-component-pattern]] every leaf component is split into **two layers** — a platform-agnostic *surface* (`mpapp::internal::basic_<name>`) and a user-facing *wrapper* (`mpapp::<name>`) that embeds the platform handler by value and auto-binds it in its constructor. The handler itself is unchanged; what changed is how it attaches to the surface.

## Architecture

```
mpapp::button                                ← user-facing wrapper
    │ inherits publicly + embeds by value →
    │
    ├── mpapp::internal::basic_button        ← platform-agnostic surface
    │      │ Observable<std::string> text
    │      │ signal<>                clicked
    │      │ button_handler<P>*      handler_  (POINTER)
    │      │ set_handler() / handler() / has_handler()
    │      ▼
    │   ctor calls: set_handler(embedded_handler_);
    │                embedded_handler_.map_text(*this);
    │                embedded_handler_.map_clicked(*this);
    │
    └── mpapp::internal::button_handler<platform::current>   ← embedded handler
            │ owns →
            ▼
        Native control
            ├─ Windows: winrt::Microsoft::UI::Xaml::Controls::Button
            ├─ Android: fbjni::global_ref<jobject>  (android.widget.Button)
            ├─ Linux:   GtkButton*
            ├─ macOS:   NSButton*
            └─ iOS:     UIButton*
```

App code interacts with the wrapper layer only:

```cpp
mpapp::button b;
b.text = "Save";
b.clicked.subscribe(slot, [&]{ /* ... */ });
```

Mock tests interact with the surface layer directly, so they do **not** link the per-platform handler library:

```cpp
mpapp::internal::basic_button b;
mpapp::button_handler<mpapp::platform::mock> h;
h.map_text(b);
b.text = "hello";
REQUIRE(h.calls_as_strings() == std::vector<std::string>{"text=", "text=hello"});
```

The `mpapp::<name>_handler<Platform>` alias keeps a templatable handler-type name in the public namespace (default `Platform = platform::current`); the actual specialisations live in `mpapp::internal::`.

## Static dispatch via CRTP

The platform-agnostic surface inherits from a CRTP base:

```cpp
template <class Derived>
class control {
    // shared logic, uses static_cast<Derived*>(this) for dispatch
};

namespace mpapp::internal {

class basic_button : public control<basic_button> {
    Observable<std::string> text{""};
    mpapp::signal<>         clicked{};

    button_handler<platform::current>&       handler() noexcept       { return *handler_; }
    bool                                     has_handler() const noexcept { return handler_ != nullptr; }
    void set_handler(button_handler<platform::current>& h) noexcept { handler_ = &h; }
private:
    button_handler<platform::current>*       handler_ = nullptr;
};

}  // namespace mpapp::internal

namespace mpapp {

class button : public internal::basic_button {
public:
    button() {
        set_handler(embedded_handler_);
        embedded_handler_.map_text(*this);
        embedded_handler_.map_clicked(*this);
    }
private:
    internal::button_handler<platform::current> embedded_handler_;
};

template <class Platform = platform::current>
using button_handler = internal::button_handler<Platform>;

}  // namespace mpapp
```

CRTP parameterises on the **surface** type (`control<basic_button>`), not the wrapper, so introspection lives on the platform-agnostic layer where the handler reads it.

The handler is a partial template specialisation on a `platform_tag`, declared as a primary template inside `mpapp::internal::` (the wrapper header's umbrella include pulls the right one):

```cpp
namespace mpapp::internal {

template <class Platform>
class button_handler;

template <>
class button_handler<platform::windows> {
    winrt::Microsoft::UI::Xaml::Controls::Button native_;
    void map_text(basic_button& b);
    void map_clicked(basic_button& b);
};

template <>
class button_handler<platform::android> {
    jobject native_;
    void map_text(basic_button& b);
    void map_clicked(basic_button& b);
};

}  // namespace mpapp::internal
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
| `ButtonHandler` | `mpapp::internal::button_handler<P>` | `references/maui/src/Core/src/Handlers/Button/ButtonHandler.cs` |
| `IButtonHandler` interface | implicit from CRTP `control<basic_button>` | `references/maui/src/Core/src/Handlers/Button/IButton.cs` |
| Property mapper (Dictionary) | `constexpr std::array<property_mapper_entry, N>` | `references/maui/src/Core/src/Handlers/Button/ButtonHandler.Standard.cs` |
| `VirtualView` / `PlatformView` | `basic_button&` / `native_t*` (typed) | — |
| Bound `IElement.Handler` setter | `set_handler()` on the surface, auto-called by the wrapper ctor | — |

## Handler lifecycle

1. **App construction.** User declares `mpapp::button b;`. The wrapper's default constructor runs, which:
   - Constructs the embedded `internal::button_handler<platform::current>` member by value (this allocates the native widget — see "Lifecycle note" in the wrapper header's comment block; the UI subsystem must be initialised, which `mpapp::run<App>` guarantees before constructing the app instance).
   - Sets `basic_button::handler_` to point at the embedded handler.
   - Calls every `embedded_handler_.map_<property>(*this)` so the initial Observable values land on the native widget and the change-signal subscriptions are wired.
2. **Property mutation.** User writes `b.text = "Save"`. The Observable's `set` short-circuits on `==` and (if changed) fires its `changed` signal. The mapper-subscribed slot in the handler runs `apply_text(...)` which mutates the native widget.
3. **Event back-propagation.** Native events (`UIButton` touch-up, `GtkButton::clicked`, WinUI `Click`) call back into `b.clicked.emit()`, which fans out to app subscribers.
4. **Destruction.** RAII releases the native ref (`global_ref` on Android, `__bridge_retained` on Apple, smart pointers on Windows/Linux). The wrapper ctor's order — embedded handler member declared **after** the surface inheritance — guarantees the handler is destroyed before the surface, so a late native signal cannot fire into a freed surface pointer.

## Mock-test link isolation

[[ADR-0008-mock-first-implementation]] requires `mock_handlers_test` to build + link against the mock library only, with no per-platform handler library on its link line. The two-layer split preserves this contract:

- The surface holds the handler by **pointer**, so the surface's translation unit has no ODR-use of platform handler symbols.
- Per-platform handler headers `#include "../../internal/basic_<name>.hpp"` (the surface) instead of the wrapper, so they too have no dependency on the wrapper's by-value handler member.
- Mock tests instantiate `mpapp::internal::basic_<name>` directly. The wrapper class is *defined* (the wrapper header is reachable transitively) but never *instantiated* in test code, so the linker has no reason to demand the platform handler's constructor/destructor symbols.

The result: `nm` on `mock_handlers_test.o` shows no `mpapp::internal::<name>_handler<platform::linux_>::...` symbols even on a Linux host.

## See in code

The handler pattern in three concrete views, all keyed on `mpapp::button`:

- **Surface** (cross-platform, zero handler awareness, pointer-typed handler):
  [`include/mpapp/internal/basic_button.hpp`](../../include/mpapp/internal/basic_button.hpp)
  — `Observable<std::string> text{}`, `signal<> clicked`, `set_handler(...)`.
- **Wrapper** (user-facing, embeds the platform-current handler by value):
  [`include/mpapp/button.hpp`](../../include/mpapp/button.hpp)
  — `class button : public internal::basic_button` with a default ctor that auto-binds, plus the `template <class P = platform::current> using button_handler` alias.
- **Mock handler** (`internal::button_handler<platform::mock>`):
  [`include/mpapp/handlers/mock/button_handler.hpp`](../../include/mpapp/handlers/mock/button_handler.hpp)
  — records `map_text(text)` invocations and `clicked` emissions into a vector for assertion. The shape every component's mock follows. Takes `basic_button&` (the surface) so it does not pull in the wrapper.
- **Real handlers** (`internal::button_handler<platform::windows>` etc.):
  [`src/handlers/windows/button_handler.cpp`](../../src/handlers/windows/button_handler.cpp) (WinUI 3 `muxc::Button`)
  · [`src/handlers/linux/button_handler.cpp`](../../src/handlers/linux/button_handler.cpp) (GTK4 `GtkButton`)
  · [`src/handlers/android/button_handler.cpp`](../../src/handlers/android/button_handler.cpp) (JNI to `android.widget.Button`).
  Same `map_text` / `map_clicked` signatures as the mock — body differs. The `dispatch_button` per-platform function casts `view*` to `::mpapp::internal::basic_button*` for the [[ADR-0013-data-driven-widget-dispatch]] widget-dispatch registry.

Platform-tag dispatch lives in [`include/mpapp/platform.hpp`](../../include/mpapp/platform.hpp). Every migrated component repeats this trio (surface + wrapper + per-platform handler set); the full matrix is in [[Controls Inventory]].

## See also

- [[ADR-0024-wrapper-component-pattern]] — the wrapper + surface split.
- [[ADR-0008-mock-first-implementation]] — the link-isolation contract.
- [[ADR-0013-data-driven-widget-dispatch]] — the dispatch registry that casts to `basic_<name>*`.
- [[Type System]] — the CRTP machinery.
- [[Platform Interop]] — per-platform native APIs.
- [[Interop Parity]] — handler contract.
- [[Controls Inventory]] — list of all controls to port.
- [[Components/Button]] — example component.
- `D:\GitHub\MPAPP\references\maui\src\Core\src\Handlers\` — reference implementation.
