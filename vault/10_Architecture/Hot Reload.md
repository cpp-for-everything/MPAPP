---
type: moc
area: tooling
tags:
  - area/tooling
---

# Hot Reload

MPAPP supports hot reload on **every dev surface**, not just desktop. The toolchain is **LLVM/Clang + LLD** — the same compiler stack that builds production binaries.

There are two flavors:

1. **XAML hot reload** — recompile `.xaml`, swap visual tree. Easy. Works everywhere.
2. **C++ hot reload** — recompile changed `.cpp`, swap dynamic library. Harder. Works on most dev surfaces.

## Feasibility matrix

| Surface | C++ hot reload mechanism | Feasibility |
|---|---|---|
| Windows desktop | `LoadLibrary` / `FreeLibrary` + Clang/LLD `.dll` rebuild | ✅ |
| Linux desktop | `dlopen` / `dlclose` + Clang/LLD `.so` rebuild | ✅ |
| macOS desktop | `dlopen` / `dlclose` + Clang/LLD `.dylib` rebuild | ✅ |
| Android emulator | `dlopen` / `dlclose` from app's lib dir | ✅ (Android allows runtime `dlopen` of bundled libs) |
| iOS Simulator | `dlopen` of `.dylib` on Simulator | ✅ (Apple permits dynamic loading on Simulator) |
| **Android real device (dev build)** | Same as emulator | ✅ for dev signing |
| **iOS real device** | Apple restricts dynamic code loading on signed apps | ❌ for production; ⚠️ research dev-entitlement workaround |

XAML hot reload works on **every** surface (no dynamic-loading restrictions apply — XAML hot reload is just data swap).

## C++ hot reload design

Inspired by [Live++](https://liveplusplus.tech/) (commercial) and the open-source [cr](https://github.com/fungos/cr) library, but implemented in-house to avoid licensing entanglement (per [[RFC-0001-licensing-and-patent-strategy]]).

### Architecture

```
User edits MyPage.cpp
        │
        ▼
Watcher detects change           ◄─── filesystem watch
        │
        ▼
mpapp-reload daemon
   • Invokes Clang/LLD on changed .cpp + dependencies
   • Produces hot-reload.dll / .so / .dylib
        │
        ▼
Hot-reload runtime in user app
   • Loads new library via LoadLibrary / dlopen
   • Walks "hot-reloadable" objects (registered via Hot<T>)
   • Re-routes function pointers to new code
   • Preserves Observable<T> state across the swap
        │
        ▼
App continues running with new code
```

### Constraints

- **Function-pointer-stable.** Only function bodies and member functions can change; type layout cannot. Adding a field or virtual function is a full rebuild.
- **No `Observable<T>` schema changes.** The property type stays; the implementation can change.
- **No reload across ABI boundaries.** Handlers and the native interop layer are stable; only application logic hot-reloads.

### `Hot<T>` registration

The user marks classes that should survive hot reload by inheriting (no macro):

```cpp
class MyViewModel : public mpapp::view_model, public mpapp::Hot<MyViewModel> {
    Observable<int> count{0};
    void increment(Command<> = {}) { count.set(count + 1); }
};
```

`Hot<T>` registers the type with the hot-reload runtime and ensures state is preserved.

## XAML hot reload

Much simpler:

1. User saves `MainPage.xaml`.
2. `mpapp-xc` recompiles to `MainPage.gen.hpp`.
3. Hot-reload runtime regenerates the `consteval` tree.
4. Visual tree is swapped via a stable handle table; bound `Observable<T>` state is preserved.

No native code rebuilds, no dynamic loading. Just data manipulation.

## Real-device caveat: iOS

iOS real-device hot reload is the lone unresolved area. Options to investigate (P5+):

- **JIT-relaxation entitlements** (`com.apple.developer.cs.allow-jit`) — allowed for some categories of apps.
- **JIT in dev signing only** — likely possible.
- **Slot-based hot patching** without `dlopen` — a research direction.

For now, the iOS production app contract is "production builds are statically compiled; hot reload is a dev tool on Simulator only."

## See also

- [[Build System]]
- [[Markup]]
- [[70_References/LLVM]]
- [[RFC-0001-licensing-and-patent-strategy]]
