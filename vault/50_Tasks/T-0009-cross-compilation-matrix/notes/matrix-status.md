# Cross-compilation matrix — empirical status

> [!info] Validation context
> **Host:** Windows 11 Pro N 10.0.26200, x86_64.
> **Toolchain:** Zig 0.13.0 (installed per [[zig-install]]).
> **CMake:** 4.2.3-msvc3 (bundled with Visual Studio 2025 18 Community).
> **Ninja:** 1.12.1.
> **Date:** 2026-05-12.

## Status legend

- ✅ — built and tested cleanly
- ✅ unsigned — built but binary needs signing on macOS to run (Apple targets)
- ⚠️ partial — built but missing some functionality (footnoted)
- ❌ — build failed (footnoted)
- ⏳ — not yet attempted (host outside this validation pass)

## Results — Windows host

This is the empirical Windows-host row of the matrix in
[[Build System]] §Cross-compilation matrix. Each row links to the build
log produced by `cmake --build`.

| Target        | Toolchain file                      | Status         | Artifact                                | Notes |
|---------------|-------------------------------------|----------------|-----------------------------------------|-------|
| windows-x64   | (native, MSVC default)              | ✅             | `build-windows-x64/mpapp-core.lib` (724 KB) + 14/14 tests pass | Baseline. See [[../logs/windows-x64.log]]. |
| linux-x64     | `cmake/toolchains/linux-x64.cmake`  | ✅             | `build-linux-x64/libmpapp-core.a` (13 KB ELF64 x86_64) | Zig 0.13 ships gnu libc. See [[../logs/linux-x64.log]]. |
| linux-arm64   | `cmake/toolchains/linux-arm64.cmake`| ✅             | `build-linux-arm64/libmpapp-core.a` (13 KB ELF64 aarch64) | First configure took ~36 s (Zig caches gnu sysroot per target). See [[../logs/linux-arm64.log]]. |
| android-arm64 | `cmake/toolchains/android-arm64.cmake` | ❌ [^1]      | none — fails during compile of `<cstddef>` (libcxx → `features.h` missing) | Zig 0.13 does **not** bundle bionic libc. See [[../logs/android-arm64.log]]. |
| macos-arm64   | `cmake/toolchains/macos-arm64.cmake`| ✅ unsigned    | `build-macos-arm64/libmpapp-core.a` (10 KB Mach-O 64 arm64) | Static-library archive; signing only matters for executables / dylibs. See [[../logs/macos-arm64.log]]. |
| ios-arm64     | `cmake/toolchains/ios-arm64.cmake`  | ❌ [^2]        | none — libcxx `<math.h>` references `FP_INFINITE` etc. from a missing system header | Zig 0.13 does **not** bundle the iOS SDK. See [[../logs/ios-arm64.log]]. |

[^1]: **Android failure detail.** `zig targets | jq .libc` on 0.13.0 lists
    musl and gnu Linux libc variants only — no `aarch64-linux-android`.
    The compile of any C++ TU that includes `<cstddef>` (which transitively
    pulls in libc++'s `__config` → `<features.h>`) fails because
    `<features.h>` is glibc-specific and the bionic substitute is not on
    the include path. **Resolution path**: either (a) install the Android
    NDK and pass its sysroot via `--sysroot=$NDK/.../sysroot`, or
    (b) wait for a Zig release that bundles bionic. Tracking under
    [[ADR-0011-cross-compilation-toolchain]] — the ADR's claim that "Zig
    bundles libc/headers for Android (bionic)" should be revised when a
    follow-up ADR or RFC reconciles this.

[^2]: **iOS failure detail.** Same root cause as Android — Zig 0.13's
    bundled libc++ assumes a system `<math.h>` that defines `FP_INFINITE`,
    `FP_NORMAL`, etc. On the `aarch64-ios-none` target there is no such
    header on the include path. **Resolution path**: requires a macOS host
    with Xcode installed (or a stand-alone iOS SDK extracted via
    `osxcross`-style tooling) — Apple SDKs are not redistributable.

## Reconciliation with the optimistic matrix

[[Build System]] §Cross-compilation matrix is the *optimistic* matrix —
it lists the *intended* outcome of every host × target combination once the
toolchain story is complete. The Windows-host row reads:

| Host \ Target | Windows | Linux | Android | macOS | iOS |
|---|---|---|---|---|---|
| Windows | ✅ native (MSVC / Clang) | ✅ Clang/Zig + sysroot | ✅ Android NDK / Zig | ⚠️ unsigned only* | ⚠️ unsigned only* |

The empirical results above reconcile with this as follows:

- **Windows-x64, Linux-x64, Linux-arm64**: match the optimistic prediction
  exactly.
- **macOS-arm64**: matches the "unsigned only" caveat — but note that the
  smoke target is a static `.a` so signing is moot at this level.
- **Android-arm64**: the optimistic matrix says "Android NDK / Zig" —
  *with NDK* still works, *Zig alone* on 0.13.0 does **not**. The Build
  System note has been updated to reflect this constraint.
- **iOS-arm64**: same caveat — Zig 0.13 alone is insufficient even for a
  static library because libc++ headers fail to resolve. The signing
  constraint remains as before; the build constraint is new.

## What this means for the project

- For the **Phase 1 milestone** the validated targets (Windows, Linux x64,
  Linux arm64, macOS arm64) are enough to prove the toolchain approach: one
  host, one Zig install, four working targets.
- Android and iOS cross-compilation from Windows is **blocked on a sysroot
  source** until either (a) the Android NDK is added as a side-by-side
  dependency or (b) a future Zig release bundles the missing libc/SDK.
  Neither is in scope for T-0009 — both are filed as follow-up work and
  referenced from [[ADR-0011-cross-compilation-toolchain]]'s "Consequences"
  section once that ADR is amended.

## Reproducing this matrix

```powershell
# Bootstrap toolchain (see zig-install.md)
$env:PATH = "$env:USERPROFILE\.mpapp\toolchains\zig-0.13.0;$env:PATH"

# Path to CMake + Ninja from Visual Studio
$cmakeDir = "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin"
$ninjaDir = "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja"
$env:PATH = "$cmakeDir;$ninjaDir;$env:PATH"

# Cross-target loop
foreach ($t in @("linux-x64","linux-arm64","macos-arm64","android-arm64","ios-arm64")) {
    cmake -S . -B build-$t `
          "-DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/$t.cmake" `
          -DMPAPP_BUILD_TOOLS=OFF -DMPAPP_BUILD_EXAMPLES=OFF `
          -DBUILD_TESTING=OFF -G Ninja
    cmake --build build-$t
}
```

For the Windows native baseline: run `_build.bat` from the repo root.
