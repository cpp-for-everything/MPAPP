---
type: rfc
id: RFC-0011
title: Dependency injection — service_collection / service_provider + app_builder
status: accepted
author: Alex Tsvetanov
created: 2026-05-29
area: process
relatedADRs:
  - ADR-0008
  - ADR-0009
  - ADR-0012
tags:
  - type/rfc
  - status/accepted
  - area/process
---

# RFC-0011 — Dependency Injection

> [!info] Status
> **accepted** — mock surface shipped under [[T-0055-dependency-injection]]. Full `mpapp::run<App>` wiring (resolving the application + services from the provider) is a follow-up.

## Problem

MAUI apps are bootstrapped through `MauiAppBuilder` + `IServiceCollection` / `IServiceProvider` — register services (singletons / transients), then constructor-inject them into pages + view-models. MPAPP had `mpapp::run<App>` + the `application` base but **no DI container** at all: no service registration, no resolution, no lifetimes.

## Proposal

A minimal container in `include/mpapp/di/`, header-only, no macros (ADR-0009):

1. **`service_collection`** — register services: `add_singleton<T>()` / `add_transient<T>()` (default-constructed), `add_singleton<T>(factory)` / `add_transient<T>(factory)` (the constructor-injection path — the factory pulls deps from the provider), `add_singleton<T>(instance)` (pre-built), and `add_singleton<Iface, Impl>()` (interface→impl). Fluent (returns `*this`).
2. **`service_provider`** — `get<T>()` (nullptr if unregistered), `get_required<T>()` (throws), `contains<T>()`. Singletons cached on first resolve; transients created per call. Resolution recurses through factories (`sp.get_required<Dep>()`).
3. **`app_builder`** — MAUI's `MauiAppBuilder`: owns a `service_collection` (`.services()`), `.build()` → `service_provider`.

C++ has no reflection, so **automatic** constructor injection (scanning ctor params) isn't possible without macros. The factory overload is the idiomatic substitute: `sc.add_singleton<Service>([](auto& sp){ return make_shared<Service>(sp.get_required<Repo>()); })`. This is explicit but type-safe and macro-free.

## Detailed design

Registrations are keyed by `std::type_index`; each holds a lifetime, a type-erased `factory(service_provider&) -> shared_ptr<void>`, and (for singletons) a cached `shared_ptr<void>`. `service_provider::get<T>` looks up the key, runs/caches per lifetime, and `static_pointer_cast<T>`s back. The provider is constructed by `service_collection::build()` (moves the registration map in); `get` is non-const (it's the mutable resolution context that fills the singleton cache).

### Tests (mock-first)

`tests/mock_handlers/di_test.cpp` — 7 cases / 18 assertions: singleton identity + shared state, transient distinctness, interface→impl, factory constructor-injection (+ injected singleton identity), pre-built instance, missing-registration (nullptr / throws / `contains`), and `app_builder` build.

## Alternatives

- **Reflection-based auto-injection** (scan ctor signature). Impossible without macros/codegen (Rule 1); the factory overload is the macro-free path. A future `mpapp-xc`-assisted codegen could emit factories from annotations.
- **Scoped lifetime** (per-request scope). Deferred — singleton + transient cover the app-shell needs; scoped lands if/when navigation scopes need it.

## Open Questions

> [!todo] Open
> - [ ] `mpapp::run<App>` integration — resolve the `application` (and optionally handlers) from the provider; inject the provider into pages/VMs.
> - [ ] Scoped lifetime + a `service_scope` RAII type.
> - [ ] `add_keyed_*` (multiple impls of one interface, selected by key).

## Migration / Compatibility

Pure addition; no existing surface modified; platform-neutral (no per-platform code).

## References

- [[ADR-0012-application-window-handler-abstraction]] (the app-shell `run<App>` this will wire into), [[ADR-0009-public-api-template-wrappers-only]].
- `references/maui/src/Core/src/Hosting/MauiAppBuilder.cs`, `Microsoft.Extensions.DependencyInjection` (ServiceCollection / ServiceProvider).
