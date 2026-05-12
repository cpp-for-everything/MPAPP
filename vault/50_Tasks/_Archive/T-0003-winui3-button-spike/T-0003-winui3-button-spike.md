---
type: task
id: T-0003
title: WinUI 3 button handler spike
status: done
milestone: M-01
owner: ""
area: handlers
blockedBy:
  - T-0002
coveragePercent: 100
hasScreenshots: true
hasRecordings: false
tags:
  - type/task
  - status/done
  - area/handlers
  - platform/windows
  - phase/p0
---

# T-0003 — WinUI 3 button handler spike

## Goal

Prove the CRTP handler architecture by implementing a real `button_handler<platform::windows>` against C++/WinRT and WinUI 3. Single button, single property (`text`), single event (`clicked`). Click in a desktop window → command fires → observable count increments → label updates.

## Acceptance Criteria

- [x] `src/handlers/windows/button_handler.cpp` implements the handler.
- [x] `examples/windows_button_spike/main.cpp` opens a WinUI 3 window with a `mpapp::button` and a `mpapp::label` driven by a `view_model` containing `Observable<int> count`.
- [x] Click increments `count`, label re-renders with new value (screenshot shows `Count: 18` after a click session).
- [x] Build succeeds on Windows host (MSVC) via `_build_winui3.bat` (Recipe B).
- [x] Screenshots of the working window in `screenshots/`.
- [ ] Screen recording showing click → count increment in `recordings/` *(skipped — the static screenshot pair before/after clicks suffices; promoted to a real recording when the M-03 Button handler shipping is done).*
- [x] 100% coverage of the spike-realised handler surface (`map_text`, `map_clicked`, native widget RAII).

## Implementation

- Handler header: `include/mpapp/handlers/windows/button_handler.hpp`
- Handler implementation: `src/handlers/windows/button_handler.cpp`
- Label handler (mirrors the same pattern): `include/mpapp/handlers/windows/label_handler.hpp`, `src/handlers/windows/label_handler.cpp`
- Cross-platform surface: `include/mpapp/button.hpp`, `include/mpapp/label.hpp`
- Platform tags: `include/mpapp/platform.hpp`
- CRTP base: `include/mpapp/control.hpp`
- Example app: `examples/windows_button_spike/main.cpp` + `app.manifest`
- CMake glue: `cmake/WindowsAppSDK.cmake`, `examples/CMakeLists.txt`, `examples/windows_button_spike/CMakeLists.txt`
- Dependency provisioning notes: [[notes/dependency-resolution]]
- Build-trip notes (the RPC_E_WRONG_THREAD root-cause): [[notes/rpc-e-wrong-thread]]

## Notes

C++/WinRT pitfalls observed during the spike — full write-up in [[notes/dependency-resolution]] and [[notes/rpc-e-wrong-thread]]:

- WinUI 3 native widgets (`Microsoft::UI::Xaml::Controls::Button`) MUST be constructed on the UI thread that `Application::Start` sets up. Constructing them in `wWinMain` (even after `winrt::init_apartment(single_threaded)`) raises `RPC_E_WRONG_THREAD = 0x8001010E`.
- `Microsoft.WindowsAppRuntime.dll` AND `Microsoft.WindowsAppRuntime.Bootstrap.dll` must both ship next to the unpackaged EXE — Bootstrap.dll is the import target for `MddBootstrapInitialize2`; WindowsAppRuntime.dll registers the undocked reg-free WinRT manifest via `WindowsAppRuntime_EnsureIsLoaded`. Skipping the latter also produces `0x8001010E`.
- Including `winrt/Microsoft.UI.Xaml.Controls.h` is not enough to instantiate `IButtonBase::Click` — the compiler also needs `winrt/Microsoft.UI.Xaml.Controls.Primitives.h` for the `consume_*` helper definitions.
- `IXamlMetadataProvider.GetXamlType` takes a `Windows::UI::Xaml::Interop::TypeName` (the old Windows.UI namespace, not Microsoft.UI). For the spike we elide the interface since the UI is built fully programmatically.

## Links

- Milestone: [[M-01-Foundations]]
- Related: [[Handlers]], [[Platform Interop]], [[Components/Button]], [[70_References/CppWinRT]]
- Example app: `examples/windows_button_spike/main.cpp`
- Screenshots:
  - `screenshots/window_initial.png` — fresh window, `Count: 0`
  - `screenshots/window.png` — after several clicks, `Count: 18`

## Closure notes

- **Closed:** 2026-05-12
- **Merged commits:** `0a6fcd6` (initial implementation), `414fc35` (merge into main), `dde5f35` (post-merge build fix-ups keeping WinUI handlers out of `mpapp-core` and propagating the Catch2 path).
- **Delivered:** Working CRTP `button_handler<platform::windows>` + companion `label_handler` against C++/WinRT and WinUI 3 (`include/mpapp/handlers/windows/*.hpp`, `src/handlers/windows/*.cpp`), cross-platform `mpapp::button` / `mpapp::label` surface, platform tag (`include/mpapp/platform.hpp`), CRTP base (`include/mpapp/control.hpp`), and a runnable `examples/windows_button_spike` that maps a `view_model` with `Observable<int> count` to a real WinUI 3 window — click increments the observable and the label re-renders. Screenshots `window_initial.png` (Count: 0) and `window.png` (Count: 18) capture the before/after.
- **Coverage:** `coveragePercent: 100` on the spike-realised handler surface (`map_text`, `map_clicked`, native widget RAII). Screen recording was intentionally skipped — the static screenshot pair captures the visible behaviour and a real recording is deferred to the M-03 Button handler shipping task.
