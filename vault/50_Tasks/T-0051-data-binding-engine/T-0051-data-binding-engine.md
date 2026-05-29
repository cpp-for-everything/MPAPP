---
type: task
id: T-0051
title: RFC-0007 Data Binding engine — mock surface + cross-platform verify
status: completed
milestone: M-09
owner: ""
area: properties
blockedBy: []
coveragePercent: 100
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/done
  - area/properties
  - area/markup
  - phase/p2
---

# T-0051 — Data Binding engine

## Goal

Land the keystone [[RFC-0007-data-binding]] mock surface: `binding<S,T>` (4 modes + converters), `multi_binding`, `binding_context` (type-erased, on `view`, inherited down the tree), and `relative_source` / `find_ancestor`. Unblocks triggers (RFC-0008), converter-driven UI, and the XAML `{Binding}` lowering.

## Scope

In:

- `include/mpapp/binding/binding.hpp` — `binding_mode` enum, `value_converter<S,T>` base, `binding<S,T>` (one_way/two_way/one_time/one_way_to_source, optional `to_target`/`to_source` converters, re-entrancy guard).
- `include/mpapp/binding/multi_binding.hpp` — `multi_binding<T, Ss…>` (N sources → 1 target combiner; recompute on any change).
- `include/mpapp/binding/binding_context.hpp` — type-erased `binding_context` (`shared_ptr<void>` + `type_info`, exact-type `get<C>()`).
- `include/mpapp/binding/relative_source.hpp` — `effective_binding_context(view)`, `find_ancestor<C>(view, include_self)`, `relative_source_mode` + `resolve_relative_source`.
- `include/mpapp/view.hpp` — `binding_context binding_ctx_` member + `local_binding_context()` / `set_binding_context<C>()`.
- `tests/mock_handlers/binding_test.cpp` — 13 cases / 39 assertions.

Out (follow-up): DataTemplate/DataTemplateSelector generalization; `{Binding}` XAML lowering (mpapp-xc M-09); `RelativeSource TemplatedParent`; `StringFormat`.

## Per-platform verification

Binding is **platform-neutral infrastructure** — no per-platform handler code. It drives `Observable::set`, which fires the same mapper a real handler installed, so a bound update flows through the existing `Observable → handler → native widget` pipeline already real on Win/Linux/Android. "Real on all 3" is by construction; proven by the handler-integration test.

| Platform | Verification |
|---|---|
| Linux (WSL/GCC) | ✅ `ctest` 403 → **416** green; `[binding]` 13 cases / 39 assertions. |
| Windows (MSVC, Release, no WindowsAppSDK) | ✅ `[binding]` 13 cases / 39 assertions in `build-winci`. |
| Android (NDK r26, aarch64 + x86_64) | ✅ headers compile clean (`build/android-binding-smoke-{aarch64,x86_64}.o`; source in `tests/`). **+ live emulator e2e**: android_hello's label rerouted through `mpapp::binding<std::string>`; 3 taps drive "Count: 3" on the `coroute_test` AVD — `adb screencap` evidence at `vault/_Assets/screenshots/android-e2e/binding-after-3taps.png`. |
| Apple | ❌ no host — pure C++23/STL, expected to compile under AppleClang when a host lands. |

## Acceptance Criteria

- [x] All four binding modes behave (source↔target per mode; one_time snapshot; one_way_to_source).
- [x] One-way + two-way converters (`int ↔ string` round-trip).
- [x] `multi_binding` recomputes on any source change; heterogeneous source types.
- [x] `binding_context` exact-type get/clear; `effective_binding_context` inheritance + shadowing + empty.
- [x] `find_ancestor<C>` nearest-typed / self-inclusion / miss; `RelativeSource Self`.
- [x] Binding drives a bound property through the mock handler (proves the real-platform pipeline).
- [x] No macros; composes with Observable/signal/Computed (ADR-0009).
- [x] Green on Linux + Windows; compiles on Android NDK both ABIs.

## Build evidence

```
$ ./build-wsl/tests/mock_handlers_test '[binding]'
All tests passed (39 assertions in 13 test cases)
$ ctest --test-dir build-wsl        ->  100% passed, 416/416
$ build-winci/tests/mock_handlers_test.exe "[binding]"   (MSVC Release)
All tests passed (39 assertions in 13 test cases)
$ aarch64/x86_64-linux-android28-clang++ -std=c++2b -c android-binding-smoke.cpp  -> ok
```

## Links

- RFC: [[RFC-0007-data-binding]].
- Reuses the `view::parent()` link from [[RFC-0005-resource-dictionaries-and-styling]].
- Unblocks: RFC-0008 Triggers, value-converter UI, mpapp-xc `{Binding}` lowering.
