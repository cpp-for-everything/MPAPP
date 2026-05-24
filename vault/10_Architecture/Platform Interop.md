---
type: moc
area: handlers
tags:
  - area/handlers
---

# Platform Interop

MPAPP has **five independent interop backends**, one per supported platform. Each backend translates MPAPP's cross-platform handler API into native API calls.

Per [[ADR-0005-ios-macos-separate-interop]], iOS and macOS are separate backends — no Mac Catalyst.

## Per-platform strategy

| Platform | Native API | Interop technology | License | Notes |
|---|---|---|---|---|
| Windows | WinUI 3 | [C++/WinRT](https://github.com/microsoft/cppwinrt) | MIT | Header-only. Unpackaged app first. |
| Android | Android Views (Java) | [fbjni](https://github.com/facebookincubator/fbjni) + custom codegen | Apache 2.0 | Typed JNI wrappers. Never expose raw `jobject`. |
| Linux | GTK4 | C ABI | LGPL | Dynamic linking only ([[RFC-0001-licensing-and-patent-strategy]]). |
| macOS | AppKit | Objective-C++ `.mm` | system | Direct `objc_msgSend`. RAII `ns_retain_ptr<T>`. |
| iOS | UIKit | Objective-C++ `.mm` | system | Separate from macOS — different handlers per control. |

## Why these choices

### Windows: C++/WinRT (not WIL, not Win32 directly)

- Native WinUI 3 access without `IUnknown` boilerplate.
- Header-only — no extra runtime dependency.
- Microsoft-supported; long-term stability.
- Refcounting via `winrt::com_ptr<T>` — RAII.

### Android: fbjni (not raw JNI, not the Android NDK directly)

- Raw JNI is a graveyard of memory bugs (forgot to delete local refs, wrong refcount, classloader nightmares).
- fbjni provides `jni::local_ref<T>`, `jni::global_ref<T>`, exception bridging, and `HybridClass` for C++ ↔ Java.
- Custom codegen tool `mpapp-jni-gen` emits typed wrappers for the specific Android API surface we use.

### Linux: GTK4 (not Qt, not raw X11/Wayland)

- LGPL — compatible with our dual-license model with dynamic linking ([[RFC-0001-licensing-and-patent-strategy]]).
- Clean C ABI — interops well with C++ without binding overhead.
- GNOME default — most distros have it pre-installed.
- Qt was considered and rejected: its QObject model conflicts with our `Observable<T>` system, and Qt commercial licensing complicates our story.

### macOS: AppKit (not Catalyst)

- Native AppKit produces apps that feel native on macOS — proper menu bars, sidebars, NSWindow integration.
- Catalyst (which MAUI uses) produces iOS-flavored apps on macOS. Per [[ADR-0005-ios-macos-separate-interop]], we reject this.

### iOS: UIKit (not SwiftUI)

- UIKit is the established Apple cross-platform UI API.
- SwiftUI bindings from C++ are messy and SwiftUI itself has stability concerns on older iOS versions.

## Common patterns across backends

### RAII handles

Every native pointer is wrapped in an RAII handle that owns its lifecycle:

| Platform | Handle type |
|---|---|
| Windows | `winrt::com_ptr<T>` |
| Android | `fbjni::global_ref<T>` / `local_ref<T>` |
| Linux | `g_object_ptr<T>` (custom thin wrapper around `g_object_ref/unref`) |
| macOS / iOS | `ns_retain_ptr<T>` (custom thin wrapper around ARC) |

### Threading

All native UI calls happen on the platform's main thread. See [[Async Executor and Event Loops]] for how MPAPP marshals onto the right loop on each platform.

### Exception safety

Native callbacks (event handlers, signal callbacks) **must not** let C++ exceptions escape into native runtime stacks. Every interop boundary uses a `try/catch` adapter that routes exceptions to `mpapp::error_reporter` and converts them to platform-appropriate error signals.

## See in code

- **Windows** — [`src/handlers/windows/`](../../src/handlers/windows/) — 62 `.cpp` handlers using C++/WinRT (`winrt::com_ptr<T>` for RAII; types like `muxc::Button`). Canonical example: [`button_handler.cpp`](../../src/handlers/windows/button_handler.cpp). WinUI 3 runtime DLLs deployed via [`cmake/WindowsAppSDK.cmake`](../../cmake/WindowsAppSDK.cmake)'s `mpapp_add_winappsdk_runtime()` helper.
- **Linux** — [`src/handlers/linux/`](../../src/handlers/linux/) — 62 `.cpp` handlers against GTK4's C ABI. Canonical example: [`button_handler.cpp`](../../src/handlers/linux/button_handler.cpp). LGPL dynamic linkage (Rule 9).
- **Android** — [`src/handlers/android/`](../../src/handlers/android/) — 69 `.cpp` handlers using JNI. Canonical example: [`button_handler.cpp`](../../src/handlers/android/button_handler.cpp); plus the kind-discriminated event routers ([`item_click_router.cpp`](../../src/handlers/android/item_click_router.cpp), [`widget_dispatch.cpp`](../../src/handlers/android/widget_dispatch.cpp), etc.) per [[ADR-0022-android-kind-discriminated-routers]]. Java glue lives at [`examples/android_hello/app/src/main/java/io/mpapp/`](../../examples/android_hello/app/src/main/java/io/mpapp/) — `MppClickRouter.java`, `MppCollectionAdapter.java`, etc.
- **macOS** — [`src/handlers/macos/`](../../src/handlers/macos/) — Objective-C++ `.mm` files. App-shell seed set today (`application`, `button`, `label`, `window`); fill-in pending an Apple host (M-07).
- **iOS** — [`src/handlers/ios/`](../../src/handlers/ios/) — same seed shape as macOS but UIKit-backed; fill-in pending M-08.

## See also

- [[Handlers]] — the handler pattern itself
- [[Interop Parity]] — what "native handlers" must guarantee
- [[Async Executor and Event Loops]] — main-thread marshaling
- [[ADR-0005-ios-macos-separate-interop]]
- [[70_References/CppWinRT]]
- [[70_References/fbjni]]
- [[70_References/GTK4]]
- [[70_References/Objective-C++]]
