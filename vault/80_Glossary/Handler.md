---
type: glossary
term: "Handler"
tags:
  - type/glossary
---

# Handler

The bridge between MPAPP's cross-platform virtual control and the native platform widget. CRTP-based, partial-specialized on platform tag. See [[Handlers]].

## See in code

- [`include/mpapp/handlers/mock/button_handler.hpp`](../../include/mpapp/handlers/mock/button_handler.hpp) — canonical mock handler (`button_handler<platform::mock>`); records calls, no native widget.
- [`include/mpapp/handlers/windows/button_handler.hpp`](../../include/mpapp/handlers/windows/button_handler.hpp) + [`src/handlers/windows/button_handler.cpp`](../../src/handlers/windows/button_handler.cpp) — real Windows handler wrapping `muxc::Button`. Mirrored by `src/handlers/linux/button_handler.cpp` (GTK4) and `src/handlers/android/button_handler.cpp` (JNI).
- Every component's handler set lives at `include/mpapp/handlers/<platform>/<component>_handler.hpp` + `src/handlers/<platform>/<component>_handler.cpp` — see [[Controls Inventory]] for the full matrix.
