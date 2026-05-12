---
type: moc
area: handlers
tags:
  - area/handlers
---

# Interop Parity

This note is the canonical statement of CLAUDE rule 2 — interop parity. It is referenced by [[ADR-0006-interop-parity]] and [[CLAUDE]].

## The rule

> Every member of MPAPP's public API surface — every property, method, event, layout primitive, animation operator, command, and observable signal — **must behave equivalently on all five supported platforms** (Windows, Android, Linux, macOS, iOS).
>
> Behavioral parity means: same observable inputs → same observable outputs, within documented numerical / pixel tolerances. Visual styling (colors, fonts, native chrome) may differ — that's expected for a native-look framework.

## What "equivalent" means

The contract is **observable behavior**, not implementation parity. Specifically:

- Property reads/writes have the same semantics.
- Events fire on the same logical conditions.
- Layout produces the same logical bounds (within ≤1px tolerance for rounding).
- Commands can/cannot execute under the same conditions.
- Async operations complete with the same semantics (even if the underlying event loop differs — IOCP vs io_uring vs kqueue).

What can legitimately differ:

- **Native styling** (button corner radius, font face, accent color) — that's the point of native-look.
- **Default behavior tied to OS conventions** (Return key in dialogs, drag-and-drop file paths). When MPAPP can't normalize, it documents the divergence in the component's "Known Differences" section.
- **Performance characteristics** (startup time, memory) — not a parity concern.

## Platform-only features

When a platform offers something genuinely impossible elsewhere (e.g. Linux `D-Bus` integration, Windows-only `Mica` material, iOS-only Dynamic Island), it lives outside the cross-platform API:

```cpp
namespace mpapp::platform::windows {
    void apply_mica(window& w);  // available only when compiling for Windows
}
```

These functions:

- Live in `mpapp::platform::<name>::` namespaces.
- Have `requires` constraints or `#if` guards.
- Are listed in the relevant component's "Platform Notes" section as a documented divergence.

## How we enforce it

1. **Conformance suite.** A platform-agnostic test suite calls public API and checks observable behavior. Runs on every CI run for every supported platform. Same test pass/fail outcome on all five = parity.
2. **PR description requirement.** Every PR must list which platforms it affects. Reviewers check the conformance suite passes on all of them.
3. **Per-component "Known Differences" table.** Any documented divergence is reviewed; if it can be eliminated, it's a bug.
4. **The component porting state machine.** A control only reaches `parity-complete` after all five platforms pass the same conformance tests.

## Related

- [[ADR-0006-interop-parity]]
- [[Handlers]]
- [[Test Harness]]
- [[Platform-Specific Views]]
- [[CLAUDE]] rule 2
