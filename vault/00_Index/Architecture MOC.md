---
type: moc
tags:
  - type/moc
---

# Architecture MOC

Map of Content for everything under `10_Architecture/`. Each note is the canonical source for its subsystem.

## Type system & properties

- [[Type System]] — overall design, template wrapper types
- [[Observable Properties]] — `Observable<T>` implementation
- [[No Macros In Public API]] — Rule 1 canonical statement

## Handlers & platform interop

- [[Handlers]] — CRTP architecture
- [[Platform Interop]] — five backends
- [[Interop Parity]] — Rule 2 canonical statement
- [[Platform-Specific Views]] — `on_platform<>` template

## Markup

- [[Markup]] — XAML compiler, `mpapp-xc`
- [[XAML Compatibility]] — compat matrix index

## Components

- [[Controls Inventory]] — 56-component porting matrix
- [[Components/README]] — per-component doc index

## Build & tooling

- [[Build System]] — CMake, cross-compilation, `mpapp` CLI
- [[CI Strategy]] — Action-minute budget, sharding, self-hosted runners
- [[Hot Reload]] — LLVM-based, all dev surfaces

## Async & threading

- [[Async Executor and Event Loops]] — IOCP / io_uring / kqueue / epoll
- [[Threading and Dispatcher]] (cross-ref)

## Quality

- [[Test Harness]] — mock-first; human-free Apple UI tests

## Canvases

- [[_Canvases/Architecture-Overview.canvas]]
- [[_Canvases/Build-Dependency-Graph.canvas]]
- [[_Canvases/Phase-Roadmap.canvas]]
- [[_Canvases/Interop-Parity-Matrix.canvas]]
- [[_Canvases/Cross-Compilation-Matrix.canvas]]
