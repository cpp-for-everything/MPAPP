---
type: rfc
id: RFC-0013
title: Essentials — device-API core (preferences, secure storage, connectivity, device info)
status: accepted
author: Alex Tsvetanov
created: 2026-05-29
area: process
relatedADRs:
  - ADR-0008
  - ADR-0009
  - ADR-0011
tags:
  - type/rfc
  - status/accepted
  - area/process
---

# RFC-0013 — Essentials (device-API core)

> [!info] Status
> **accepted** — representative core shipped under [[T-0057-essentials-core]]. Per-platform real backends + the remaining device APIs are follow-ups.

## Problem

MAUI Essentials (geolocation, sensors, preferences, secure storage, connectivity, battery, clipboard, file picker, share, device info, …) is an entire axis MPAPP had **zero** of — no device-API surface at all. It's too large for one RFC; this lands a representative, dependency-light **core** that establishes the pattern (cross-platform interface + in-memory mock + DI-injected per-platform backend), with the rest enumerated as follow-ups.

## Proposal

`include/mpapp/essentials/`, each an abstract interface + an in-memory mock implementation (the default + test double). Per-platform real backends implement the same interface and are injected via the DI container (RFC-0011). No macros.

1. **`preferences`** (`preferences.hpp`) — typed key/value settings (string/long/double/bool, default-on-miss, contains/remove/clear). `in_memory_preferences` default. (MAUI `Preferences`.)
2. **`secure_storage`** (`secure_storage.hpp`) — encrypted key/value (MAUI's is async; mock is sync). `in_memory_secure_storage` default. (MAUI `SecureStorage`.)
3. **`connectivity`** (`connectivity.hpp`) — `network_access` level + `connectivity_changed` signal. `mock_connectivity` (settable). (MAUI `Connectivity`.)
4. **`device_info`** (`device_info.hpp`) — platform/idiom/model/version value type + `current_device_info()` derived from the compile-time platform tag. (MAUI `DeviceInfo`.)

The interface+mock split means "real on Win/Linux/Android" = a per-platform backend implementing the interface (Windows ApplicationData/PasswordVault, Linux GSettings/libsecret/NetworkManager, Android SharedPreferences/Keystore/ConnectivityManager) — tracked as follow-ups, the same shape as the RFC-0004 image-loader stubs.

## Detailed design

Each interface is pure-virtual with a `final` in-memory implementation backed by an `unordered_map` (preferences/secure_storage) or a settable field + the intrusive `signal` (connectivity). `preferences` stores strings and layers typed get/set on top (serialize via `to_string` / `stol` / `stod`); typed `get(key, fallback)` returns the fallback on miss or parse failure. `device_info` is a value type; `current_device_info()` uses `if constexpr` over `platform::current`.

### Tests (mock-first)

`tests/mock_handlers/essentials_test.cpp` — 4 cases / 30 assertions: preferences typed round-trip + default-on-miss + remove/clear; secure_storage set/get/remove/remove_all; connectivity access + change-signal (incl. same-value no-op) + is_online; device_info equality + non-unknown current platform/idiom.

## Alternatives

- **A single static `Essentials` facade** like MAUI. Rejected — interfaces + DI injection are testable + swappable (mock vs platform) and align with RFC-0011; a facade would hide the seam.
- **`std::any`-typed preferences.** Rejected — string-backed + typed accessors matches MAUI's serialization semantics and is trivially platform-portable.

## Open Questions

> [!todo] Open
> - [ ] Per-platform real backends for the four core APIs (Win / Linux / Android), then Apple. Stub tickets to open: preferences, secure_storage, connectivity backends.
> - [ ] Remaining Essentials APIs: geolocation, accelerometer/gyroscope/sensors, battery, clipboard, file_picker, share, app_info, browser, email/sms, flashlight, haptics, screenshot, vibration. Each its own interface + mock + per-platform backend.
> - [ ] Async shape: MAUI's SecureStorage / Geolocation are async — return `task<T>` (ADR-0019) in the real layer.

## Migration / Compatibility

Pure addition; no existing surface modified. Apps resolve an essential from the DI provider (`sp.get_required<preferences>()`), so swapping mock→platform is a registration change.

## References

- [[RFC-0011-dependency-injection]] (how backends are injected), [[ADR-0011]]/app-shell, [[ADR-0019-async-executor-native-dispatcher]] (async backends).
- `references/maui/src/Essentials/src/` — `Preferences`, `SecureStorage`, `Connectivity`, `DeviceInfo`.
