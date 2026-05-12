---
type: moc
area: handlers
tags:
  - area/handlers
---

# Platform-Specific Views

Two mechanisms for platform-conditional code:

1. **`on_platform<T>` template** — compile-time value selection at the expression level.
2. **`#if MPAPP_*` preprocessor guards** — block-level platform code (exempt from CLAUDE rule 1 since it is internal preprocessor, not public-API macros).

## `on_platform<T>`

```cpp
auto color = mpapp::on_platform<mpapp::color>{
    .windows  = system_accent,
    .android  = material_green,
    .linux    = system_accent,
    .macos    = system_accent,
    .ios      = sky_blue,
    .fallback = neutral_gray,
}();
```

Implementation: `if constexpr (current_platform == platform::windows) return ws.windows;` chains. **All five platforms must be named** (compile error otherwise), with `fallback` as a guard. No runtime branch survives.

## XAML `{OnPlatform}`

```xml
<Label TextColor="{OnPlatform iOS=Blue, Android=Green, Default=Gray}"/>
```

This lowers to the C++ `on_platform<>` template above. The XAML compiler `mpapp-xc` emits the expression.

## `#if MPAPP_*` (block-level)

For *whole code blocks* that only make sense on one platform:

```cpp
void some_internal_function() {
#if MPAPP_PLATFORM_ANDROID
    JNIEnv* env = fbjni::Environment::current();
    // Android-only logic
#elif MPAPP_PLATFORM_WINDOWS
    // Windows-only logic
#endif
}
```

These guards are valid:

- Inside framework internals (`src/`).
- Inside user code when the user explicitly opts into platform-specific blocks.

They are **not** valid:

- In public header files that the user includes (CLAUDE rule 1).

## Interop parity caveat

Per [[Interop Parity]] (CLAUDE rule 2), platform-conditional code in the **public API** is a smell. Use `on_platform` for value differences (colors, fonts, paddings); use platform-namespaced functions (`mpapp::platform::windows::apply_mica`) for genuinely platform-only features.

## Decision matrix

| Pattern | When to use |
|---|---|
| `on_platform<T>{ ... }()` | Single-value differences (color, font, padding, default behavior tweak) |
| `mpapp::platform::<plat>::func()` | Genuinely platform-only features (Mica on Windows, Dynamic Island on iOS) |
| `#if MPAPP_PLATFORM_*` | Internal implementation differences in framework `.cpp` files |
| Different handler specializations | Native widget creation, property mapping |

## See also

- [[Interop Parity]]
- [[Handlers]]
- [[Components/README]]
- [[ADR-0006-interop-parity]]
