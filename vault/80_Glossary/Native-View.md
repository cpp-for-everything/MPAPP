---
type: glossary
term: "Native View"
tags:
  - type/glossary
---

# Native View

The platform-specific widget the handler wraps (e.g. `UIButton` on iOS, `GtkButton` on Linux). Opposite of [[Virtual View]].

## See in code

- [`src/handlers/windows/button_handler.cpp`](../../src/handlers/windows/button_handler.cpp) — `native_` is `muxc::Button` (WinUI 3 XAML).
- [`src/handlers/linux/button_handler.cpp`](../../src/handlers/linux/button_handler.cpp) — `native_` is `GtkButton*` (GTK4).
- [`src/handlers/android/button_handler.cpp`](../../src/handlers/android/button_handler.cpp) — `native_` is a JNI global ref to `android.widget.Button`.
- The handler's `native()` accessor returns the native view so example apps can drop it into their platform's layout container (e.g. a `muxc::StackPanel` on Windows).
