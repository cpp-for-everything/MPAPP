---
type: glossary
term: "Platform Tag"
tags:
  - type/glossary
---

# Platform Tag

An empty type used to select handler specializations: `platform::windows`, `platform::android`, `platform::linux`, `platform::macos`, `platform::ios`, `platform::mock`. See [[Handlers]].

## See in code

- [`include/mpapp/platform.hpp`](../../include/mpapp/platform.hpp) — the six tag types in the `mpapp::platform` namespace.
- Usage at the dispatch site: every handler header partial-specializes on a platform tag, e.g. `button_handler<platform::windows>` in [`include/mpapp/handlers/windows/button_handler.hpp`](../../include/mpapp/handlers/windows/button_handler.hpp).
- Linux tag uses the trailing underscore (`platform::linux_`) because `linux` is a predefined macro on Linux toolchains — see the comments in `include/mpapp/platform.hpp` for the rationale.
