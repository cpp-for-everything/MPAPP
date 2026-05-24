---
type: glossary
term: "Interop Parity"
tags:
  - type/glossary
---

# Interop Parity

The rule that every public MPAPP feature must behave equivalently on all five supported platforms. See [[Interop Parity]].

## See in code

- Rule 2 in action: the same `mpapp::button` (header [`include/mpapp/button.hpp`](../../include/mpapp/button.hpp)) maps to three real native widgets via
  [`src/handlers/windows/button_handler.cpp`](../../src/handlers/windows/button_handler.cpp) (WinUI 3 `muxc::Button`),
  [`src/handlers/linux/button_handler.cpp`](../../src/handlers/linux/button_handler.cpp) (GTK4 `GtkButton`),
  [`src/handlers/android/button_handler.cpp`](../../src/handlers/android/button_handler.cpp) (JNI to `android.widget.Button`).
  All three observe the same `text` / `clicked` contract.
- Platform-only divergences live under `mpapp::platform::<name>::` and are documented in their component doc as a divergence (per [[ADR-0006-interop-parity]]).
