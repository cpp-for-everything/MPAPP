---
type: research
subject: "Qt Property System"
framework: "qt"
created: 2026-05-12
applicableTo:
  - signal-slot-pattern
  - meta-compiler-category
recommendation: adopt
tags:
  - type/research
  - framework/qt
  - area/properties
  - area/type-system
---

# Qt Property System

> [!info] Status
> Fleshed out during [[M-01-Foundations]] for comparative analysis. The signal/slot **concept** is adopted; the macro-and-moc implementation is rejected.

## Summary

Qt's property and signal system is the canonical macro-plus-meta-compiler design for C++ UI: `Q_OBJECT`, `Q_PROPERTY`, and the `moc` build step produce a meta-object table that drives bindings, introspection, and signal/slot wiring at runtime. MPAPP adopts the underlying signal/slot **pattern** and the broad idea of a meta-compiler, but replaces both with template wrapper types and an out-of-process XAML compiler (see [[ADR-0002-no-macros-in-public-api]], [[ADR-0009-public-api-template-wrappers-only]]).

## What They Do

A Qt class participates in the property system by deriving from `QObject` and declaring `Q_OBJECT` in its body. The `moc` (Meta-Object Compiler) parses the header, extracts every `Q_PROPERTY`, `signals:`, `slots:`, and `Q_INVOKABLE`, and emits a generated `moc_*.cpp` file containing a `QMetaObject` table. The table holds string names, type IDs, and function pointers for each property and signal/slot. At runtime, `QObject::connect` looks up signals and slots by name (or by member-function pointer, since Qt 5) and stores a record on the sender; emitting a signal walks the connection list and calls each slot, marshalling arguments through the meta-object machinery if the connection crosses a thread.

`Q_PROPERTY` ties a getter, setter, and `NOTIFY` signal together so QML and the property browser can observe changes uniformly. Qt 6 added `QProperty<T>` — a non-macro, header-only binding type that captures dependencies via thread-local recording — but the legacy `Q_PROPERTY` plus `moc` model remains the dominant pattern in real codebases and is what the rest of Qt (QML, Designer, Test, D-Bus) is built on. The model gives Qt three things at once: dynamic introspection (used by QML, the property editor, and serialization), thread-safe queued signal delivery (used by `QtConcurrent` and async I/O), and a stable ABI through `QObject` indirection. The cost is the upfront commitment to `QObject`, the `moc` build step, and the macro surface in every public header.

## What Works / What Doesn't

### Strengths

- **Signal/slot is the right mental model.** Decoupling emitter from receiver, supporting many-to-many connections, and integrating queued delivery for cross-thread calls is the pattern every modern UI framework has converged on.
- **`NOTIFY` ties data and change-notification together** at the declaration site — you cannot accidentally declare a "bindable" property without a notifier.
- **Introspection enables tooling.** Qt Designer, QML, `QObject::dumpObjectInfo`, and remote-debug tools all consume the same meta-object table.
- **Queued connections** make GUI thread affinity explicit and ergonomic.
- **Qt 6 `QProperty<T>`** proves that header-only, type-safe dependency tracking is feasible without `moc` for property graphs — a direct precedent for MPAPP's [[Observable Properties|Observable<T>]].

### Weaknesses

- **Macros in the public API** (`Q_OBJECT`, `Q_PROPERTY`, `signals:`, `slots:`) — directly forbidden by [[ADR-0002-no-macros-in-public-api]]. They break IDE features (rename, go-to-definition), confuse static analyzers, and force users to learn a parallel mini-language.
- **`moc` is a non-standard build step.** Every consumer must wire it into CMake/qmake. Cross-compilation, single-header libraries, and modules (C++20 `import`) all interact poorly with `moc`.
- **`QObject` is heap-only.** No value semantics, no `constexpr`, no aggregate initialization. Every observable object pays a vtable, a `d_ptr`, and a parent pointer.
- **String-based connection (legacy `SIGNAL()`/`SLOT()`)** silently fails at runtime if names mismatch. The Qt 5 member-pointer form fixes this but the string form is still widespread.
- **No compile-time binding paths.** QML resolves property paths at runtime; type errors surface as runtime warnings, not compiler diagnostics.
- **Property write loops** are caught by `QProperty<T>`'s dependency tracker only at runtime.

## Applicable to MPAPP

| Item | Verdict | Where it lives in MPAPP |
|---|---|---|
| Signal/slot **concept** | **adopt** | Intrusive `signal<T...>` member on [[Observable-Property\|Observable<T>]]; see [[10_Architecture/Observable Properties]]. |
| Meta-compiler **as a category** | **adopt** | MPAPP's [[XAML-Compiler\|mpapp-xc]] is the analogue — but it consumes XAML, not C++ headers, and emits a `consteval` tree, not a runtime table. |
| `Q_PROPERTY` + `NOTIFY` pairing | **adapt** | `Observable<T>` couples value and change-signal in one type; no macro required (see [[ADR-0009-public-api-template-wrappers-only]]). |
| `QObject` base class | **reject** | MPAPP values are stack-allocatable and `constexpr`-friendly where possible. |
| `Q_OBJECT` macro / `moc` step | **reject** | See [[10_Architecture/No Macros In Public API]] and [[ADR-0002-no-macros-in-public-api]]. |
| String-based `SIGNAL()`/`SLOT()` | **reject** | All MPAPP connections are typed; binding paths resolved at compile time via [[Binding-Path]]. |
| Queued cross-thread delivery | **adopt** | Mirrored by [[Dispatcher]] and [[10_Architecture/Threading and Dispatcher]]. |
| `QProperty<T>` dependency capture | **further-study** | Thread-local recording is one option for [[Observable-Property]] computed dependencies; see [[10_Architecture/Observable Properties]] §dependency-tracking. |

> [!important] Recommendation
> `adopt` — adopt the **signal/slot pattern** and the **meta-compiler-as-a-category** idea. **Reject** the specific implementation: no `Q_OBJECT`, no `moc`, no macros in public API, no `QObject` base class. MPAPP's template-wrapper-plus-XAML-compiler design (see [[ADR-0009-public-api-template-wrappers-only]] and [[XAML-Compiler]]) achieves the same goals — introspection, change notification, cross-thread delivery — without macros or a header-rewriting preprocessor.

## References

- Qt Properties: https://doc.qt.io/qt-6/properties.html
- The Meta-Object System: https://doc.qt.io/qt-6/metaobjects.html
- Signals & Slots: https://doc.qt.io/qt-6/signalsandslots.html
- `moc` reference: https://doc.qt.io/qt-6/moc.html
- `QProperty<T>` (Qt 6 bindable properties): https://doc.qt.io/qt-6/qproperty.html
- Olivier Goffart, "How Qt Signals and Slots Work": https://woboq.com/blog/how-qt-signals-slots-work.html
- KDAB, "QProperty internals": https://www.kdab.com/qt-6-2-the-new-property-system-in-qt-6/
