---
type: log
tags:
  - type/log
  - area/handlers
  - platform/windows
---

# RPC_E_WRONG_THREAD (0x8001010E) from `Application::Start` — root cause

Most of the spike's iteration time went into chasing this error. Documenting the answer so it never costs us another half day.

## Symptom

The unpackaged WinUI 3 EXE crashes very early with:

```
The application called an interface that was marshalled for a different thread.
HRESULT: 0x8001010E (RPC_E_WRONG_THREAD)
```

The error fires from `winrt::Microsoft::UI::Xaml::Application::Start(...)` — long before our `OnLaunched` is reached. Bootstrapping the Windows App Runtime succeeds (`MddBootstrapInitialize2` returns `S_OK`), `WindowsAppRuntime_EnsureIsLoaded` returns `S_OK`, and the apartment is in STA, yet `Start` still throws.

## Root cause

We were constructing a `winrt::Microsoft::UI::Xaml::Controls::Button` and `winrt::Microsoft::UI::Xaml::Controls::TextBlock` **on the thread that calls `Application::Start`, _before_ Start runs**. The `spike_state` struct held them by value, and we declared `spike_state state{};` inside `wWinMain`.

WinUI 3 XAML types must be activated on the UI thread that `Application::Start` sets up. Activating any XAML type before that thread exists puts the COM proxies in the wrong apartment; the marshaller realises this when `Start` later tries to consume them and throws `RPC_E_WRONG_THREAD`.

## Fix

Move all XAML-type construction into the `Application::Start` callback. Concretely: put `spike_state` (the struct that owns the MPAPP handlers, which in turn own the native widgets) inside the `App` class as a value member. `App` itself is constructed via `winrt::make<App>()` inside the Start callback, so all native widget construction happens on the UI thread.

```cpp
struct App : ApplicationT<App> {
    spike_state state{}; // constructed on the UI thread
    // ...
};

Application::Start([](auto&&) {
    winrt::make<App>();
});
```

## False leads we tried first

- ✗ Skipping `winrt::init_apartment` (no effect).
- ✗ Switching MTA vs STA (no effect).
- ✗ Adding the `WindowsAppRuntimeAutoInitializer.cpp` / `MddBootstrapAutoInitializer.cpp` / `UndockedRegFreeWinRT-AutoInitializer.cpp` as compiled sources. **This is correct and necessary** for reg-free WinRT but it does not by itself solve the wrong-thread error.
- ✗ Adding a side-by-side manifest with `dpiAwareness=PerMonitorV2` (kept for correctness but not the fix).
- ✗ Removing the `IXamlMetadataProvider` implementation from `App` (no effect — XAML metadata provider isn't required when the UI tree is fully programmatic).

## Lessons for future Windows handlers

- **Never construct a native widget inside the handler ctor unless the host can guarantee the call happens on the UI thread.** For T-0003 we're OK because the host (the example app) constructs the handlers inside the `App` instance, which `winrt::make` invokes on the UI thread.
- If MPAPP later wants a "create the handler eagerly, attach to host lazily" pattern, the handler ctor must defer the WinUI activation until `attach()` is called — _not_ in the ctor. Logged as a follow-up item for the M-03 button shipping work.
