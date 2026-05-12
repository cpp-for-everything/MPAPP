---
type: moc
area: build
tags:
  - area/build
---

# Build System

MPAPP uses **CMake 3.28+ with C++ modules** as the build substrate, wrapped by an `mpapp` CLI. Cross-compilation is a first-class requirement per [[ADR-0007-cross-platform-tooling]].

## Cross-compilation matrix

A developer must be able to produce binaries for all five targets from any supported host:

| Host \ Target | Windows | Linux | Android | macOS | iOS |
|---|---|---|---|---|---|
| **Windows** | ✅ native (MSVC / Clang) | ✅ Zig (verified†) | ⚠️ NDK required‡ | ⚠️ unsigned only* (Zig static lib verified†) | ⚠️ unsigned only* + SDK required‡ |
| **Linux** | ✅ MinGW / Zig | ✅ native | ⚠️ NDK required‡ | ⚠️ unsigned only* | ⚠️ unsigned only* + SDK required‡ |
| **macOS** | ✅ MinGW / Zig | ✅ Clang/Zig + sysroot | ⚠️ NDK required‡ | ✅ native | ✅ native |

> [!warning] Apple signing requires macOS
> *Apple binaries built on non-Mac hosts (via osxcross) produce *unsigned* artifacts useful for sanity-check compilation only. They cannot run on Simulator / device / users' machines without re-signing on macOS. **This is an Apple SDK / Gatekeeper constraint, not a tooling limit.**

> [!info] † Empirically verified from a Windows host
> Per [[T-0009-cross-compilation-matrix]] (validated 2026-05-12 with Zig 0.13.0), the Windows-host row of this matrix has been exercised end-to-end for Windows-x64 (native), Linux-x64, Linux-arm64, and macOS-arm64. See [[T-0009-cross-compilation-matrix/notes/matrix-status]] for per-target artefacts and SHA-checked archive byte signatures.

> [!warning] ‡ Zig 0.13 alone is insufficient for Android / iOS
> Zig 0.13.0 bundles libc/sysroot only for the triples in `zig targets | jq .libc[]` — that list does **not** include `aarch64-linux-android` or any iOS triple. Cross-compiling to either target therefore still requires:
> - **Android**: the Android NDK on the include/lib search path (`-DCMAKE_SYSROOT=$NDK/.../sysroot`).
> - **iOS**: a macOS host with Xcode, or stand-alone Apple SDK extraction (osxcross). Apple SDKs are not redistributable.
>
> When a future Zig release bundles bionic this caveat can be removed; until then [[ADR-0011-cross-compilation-toolchain]]'s "Consequences" section should be updated to mirror this empirical finding.

## A.6 Toolchain candidates

Per [[ADR-0011-cross-compilation-toolchain]], **Zig (`zig cc`) is locked in** as the primary cross-compilation toolchain. One install of Zig wraps Clang + LLD with bundled libc headers and sysroots for Windows / Linux / Android targets from any host. Apple targets cross-compile to unsigned binaries via osxcross; signed Apple builds still require macOS.

- **Vendoring:** Zig is *not* bundled in the repo. The `mpapp` CLI auto-installs Zig on first cross-compile to `~/.mpapp/toolchains/zig-<version>/`. The version is pinned in `cmake/toolchains/zig.cmake`.
- **Host-native builds:** developers who never cross-compile may keep using MSVC (Windows), system Clang (Linux), or Xcode Clang (macOS). Zig is only consulted when a cross-target toolchain file is selected.
- **Empirical validation:** [[T-0009-cross-compilation-matrix]] (Zig 0.13.0, Windows host, 2026-05-12) — see [[T-0009-cross-compilation-matrix/notes/matrix-status]] for the per-target table. Windows, Linux x64/arm64, and macOS arm64 build cleanly from Zig alone; Android and iOS need external SDKs (NDK / Xcode) because Zig 0.13 does not bundle their libc/headers.
- **Closed RFC:** [[RFC-0002-cross-compilation-toolchain]] evaluated Zig against LLVM/Clang + per-target sysroots; the decision record is [[ADR-0011-cross-compilation-toolchain]].

## CMake structure

```
MPAPP/
├── CMakeLists.txt                  # Top-level: include modules, define targets
├── cmake/
│   ├── toolchains/
│   │   ├── windows-x64.cmake
│   │   ├── windows-arm64.cmake
│   │   ├── linux-x64.cmake
│   │   ├── linux-arm64.cmake
│   │   ├── android-arm64.cmake
│   │   ├── android-x86_64.cmake    # Emulator
│   │   ├── macos-arm64.cmake
│   │   ├── macos-x64.cmake
│   │   ├── ios-arm64.cmake
│   │   └── ios-simulator.cmake
│   ├── presets/
│   │   └── CMakePresets.json       # User-facing build presets
│   └── modules/
│       ├── FindFbJni.cmake
│       ├── FindGTK4.cmake
│       └── …
├── include/mpapp/                  # Public headers (no macros — Rule 1)
├── src/
│   ├── core/                       # Cross-platform core
│   ├── handlers/
│   │   ├── windows/                # WinUI 3 handlers
│   │   ├── android/                # fbjni handlers
│   │   ├── linux/                  # GTK4 handlers
│   │   ├── macos/                  # AppKit handlers
│   │   └── ios/                    # UIKit handlers
│   └── executor/
│       ├── windows_iocp.cpp
│       ├── linux_iouring.cpp
│       ├── linux_epoll.cpp
│       ├── apple_kqueue.cpp
│       └── android_alooper.cpp
└── tools/
    ├── mpapp/                      # The CLI
    ├── mpapp-xc/                   # XAML compiler
    └── mpapp-jni-gen/              # Android codegen
```

## Zero-cost link-time abstraction

Per the executor design (see [[Async Executor and Event Loops]]), the same public API exposes different implementations selected by CMake:

```cmake
if(MPAPP_TARGET_WINDOWS)
    target_sources(mpapp-core PRIVATE src/executor/windows_iocp.cpp)
elseif(MPAPP_TARGET_LINUX)
    target_sources(mpapp-core PRIVATE src/executor/linux_iouring.cpp)
elseif(MPAPP_TARGET_APPLE)
    target_sources(mpapp-core PRIVATE src/executor/apple_kqueue.cpp)
elseif(MPAPP_TARGET_ANDROID)
    target_sources(mpapp-core PRIVATE src/executor/android_alooper.cpp)
endif()
```

Same pattern for the handler implementations — only one platform's `.cpp` links into the final binary. No virtual dispatch, no runtime branch, no v-tables in the hot path.

## The `mpapp` CLI

User-facing wrapper:

```
mpapp new <name>                    Create a new MPAPP app from a template
mpapp build                         Build for the host's native target
mpapp build --target ios-arm64      Cross-build for a specific target
mpapp build --all                   Build every target the host supports
mpapp run                           Build + run on host target (or emulator if specified)
mpapp package                       Produce a distributable artifact (.exe, .app, .apk, .ipa, AppImage)
mpapp xaml-compile <file>           Run mpapp-xc on a single file (rare; usually done by build)
```

`mpapp` is C++ itself — eats its own dogfood. Runs on Windows, macOS, Linux per [[ADR-0007-cross-platform-tooling]].

## Linux GUI development from Windows host: WSLg

Windows 11 ships [WSLg](https://github.com/microsoft/wslg) (Windows Subsystem for Linux GUI) natively. Linux GUI apps run as native windows via Wayland/X11 proxying.

```powershell
wsl --install -d Ubuntu-22.04
# Inside WSL:
sudo apt install build-essential libgtk-4-dev clang cmake
mpapp build --target linux-x64
./build/linux-x64/examples/hello       # Runs as a native window on Windows 11
```

Hyper-V Linux VM is the fallback when WSLg has GTK4 quirks.

## CI strategy summary

See [[CI Strategy]] for the full design. Headlines:

- `windows-latest` (GitHub Actions) runs Windows-native, Linux-cross, Android-cross every PR.
- `ubuntu-latest` runs Linux-native, Android-cross every PR.
- `macos-latest` runs Apple targets on tagged releases until the self-hosted macOS runner is online.
- Self-hosted runner (user's Windows machine) handles long Android emulator runs.

## See also

- [[ADR-0007-cross-platform-tooling]]
- [[ADR-0011-cross-compilation-toolchain]]
- [[RFC-0002-cross-compilation-toolchain]]
- [[Async Executor and Event Loops]]
- [[CI Strategy]]
- [[Hot Reload]]
- [[70_References/WSLg]]
- [[70_References/Zig]]
- [[70_References/osxcross]]
