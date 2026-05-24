# T-0030 — dual vcpkg roots on the Windows host

The Windows dev machine ended up with two vcpkg installs:

- **`C:/Users/alext/vcpkg`** — original, has `cairo:x64-windows`
  (used for the Cairo backend on Windows since the M-04c Cairo
  rollout). Skia install attempted here but failed to complete.
- **`C:/tools/vcpkg`** — second install, used to install
  `skia:x64-windows` successfully (binary cache survived; build
  artifacts present in `installed/x64-windows/` and
  `packages/skia_x64-windows/`). Also has `cairo:x64-windows`
  installed in this session (instant — same binary cache).

The Skia build invocation uses `C:/tools/vcpkg` because that's where
Skia lives; Cairo is also there for the demo's PNG writer. Both vcpkg
roots are independent — no cross-references.

If consolidating is desired in the future, the cleanest move is to
install both Cairo and Skia into `C:/Users/alext/vcpkg` (the
original) and decommission `C:/tools/vcpkg`. Until then, the
build scripts (`_tmp_skia_win.bat` and the existing
`mpapp_install_windows_app_sdk` helper) work fine with the dual
arrangement.

## Android Skia install failure — root cause investigation

`vcpkg install skia:arm64-android` and `skia:x64-android` were
attempted in `C:/tools/vcpkg`. On re-running with
`ANDROID_NDK_HOME=D:/android-sdk/ndk/26.1.10909125` + `--debug`,
vcpkg progressed past the compiler-detection step (NDK was found,
clang invocation worked) and started building Skia's transitive
dependencies. The build failed on **ICU**:

```text
clang: error: no such file or directory: 'uconvmsguconvmsg_dat.S'
clang: error: no input files
-- return status = 1
Error creating with assembly code. Failed command:
  clang --target=aarch64-none-linux-android28 ...
        -o uconvmsg\uconvmsg_dat.o uconvmsg\uconvmsg_dat.S
Error generating assembly code for data.
make[2]: *** [Makefile:158: uconvmsg/libuconvmsg.a] Error 1
```

The smoking gun is the missing path separator in
`uconvmsguconvmsg_dat.S`: ICU's autotools Makefile emits the source
path as `uconvmsg\uconvmsg_dat.S` (Windows backslash), then when
the make rule runs that through `msys2/bash`, the shell interprets
`\u` as an escape sequence and eats the backslash → `uconvmsguconvmsg_dat.S`.

This is a known **Windows-host cross-compile bug in ICU's autotools
build**. The Makefile assumes Unix path semantics and the Windows
backslash gets mangled inside msys2's bash. The same ICU triplet
builds fine on a Linux host (where the separator is `/` natively).

## Workaround #1 (attempted) — drop ICU via `skia[core,png,jpeg]`

ICU is an opt-out feature in vcpkg's Skia port. Running

```bat
vcpkg install "skia[core,png,jpeg]:arm64-android"
```

with `ANDROID_NDK_HOME=D:/android-sdk/ndk/26.1.10909125` did skip
ICU. Skia's own build then ran for ~3 minutes (792/962 source
files compiled) before hitting a **second** Windows-host
cross-compile bug:

```text
FAILED: [code=1] libskcms.a
cmd.exe /c  "...python.exe" "...gn/rm.py" "libskcms.a"
            && D:/...llvm-ar.exe rcs libskcms.a `cat libskcms.a.rsp`
The filename, directory name, or volume label syntax is incorrect.
```

Skia's GN-generated ninja rule uses POSIX shell backtick command
substitution `` `cat libskcms.a.rsp` `` to expand its argument file,
but the rule runs under `cmd.exe /c` on Windows — `cmd.exe` doesn't
understand backticks and treats them as literal characters,
causing the path lookup to fail.

So cross-compiling Skia to Android from a Windows host hits at
least two upstream tooling assumptions:

| # | Stage | Assumption | What breaks |
|---|---|---|---|
| 1 | ICU autotools (transitive) | Unix `/` path separator | `\u...` escape eats path separator |
| 2 | Skia's GN-generated ninja rules | POSIX shell backticks | cmd.exe can't parse backticks |

Both are upstream-tooling bugs, not MPAPP issues. Patching either
requires a vcpkg port fork.

## Recommended path forward

**Install Skia for Android from a Linux host** (WSL with a Linux
NDK). The POSIX-shell + Unix-path assumptions hold natively on
Linux, so both bugs above disappear. WSL on this machine doesn't
have a Linux NDK installed; that's the one-time setup step before
the retry.

Until then: Android keeps Cairo as its only real graphics backend.
The MPAPP plumbing for Android Skia is already in place — both
`examples/android_hello/app/src/main/cpp/CMakeLists.txt` and
`build.gradle.kts` accept `MPAPP_GRAPHICS_BACKEND=skia` via the
`-PmpappGraphicsBackend=skia` gradle property, falling back to
stub when Skia isn't found at the configured prefix. So once
someone successfully installs `skia:<android-triplet>` (from any
host), the gradle invocation just needs the new property to
swap backends.

## The fix: skip vcpkg's Skia compile entirely

The two bugs above are entirely **inside vcpkg's GN-driven build
machinery**. They have nothing to do with Skia itself — once you
have a working Skia static archive for the target ABI, MPAPP's
backend implementation links + runs unchanged. So rather than
patch the upstream-Skia/vcpkg-port stack, MPAPP now ships
`cmake/MpappFindSkia.cmake` which **auto-downloads a pinned
community prebuilt for every supported platform** by default.

### Default flow (every platform, no prerequisites)

```sh
cmake -S . -B build -DMPAPP_GRAPHICS_BACKEND=skia
# → "MPAPP fetching Skia prebuilt: <plat>-<arch> (m143-da51f0d60e-4)"
# → "MPAPP graphics backend: skia (fetched)"
```

`mpapp_find_skia()` picks the right zip for
`(CMAKE_SYSTEM_NAME, CMAKE_SYSTEM_PROCESSOR / CMAKE_ANDROID_ARCH_ABI)`,
fetches via `FetchContent` with SHA-256 verification, and caches
under `<build>/_deps/mpapp_skia_prebuilt-src/`. The download is
40-70 MB and happens once per build dir. The expanded tree gets
read as if the user had unzipped it themselves and pointed
`MPAPP_SKIA_PREFIX` at it.

| Source | Pinned version | Platforms in MPAPP's table |
|---|---|---|
| HumbleUI/SkiaBuild | `m143-da51f0d60e-4` (April 2026) | android-{arm64,x64}, linux-x64, macos-{arm64,x64}, windows-x64 |
| (alternative — same shape) JetBrains/skia-pack | archived March 2026; m144 last | same matrix; works as a `MPAPP_SKIA_PREFIX` source if a mirror is required |

The shared zip layout MPAPP detects:

```
<prefix>/
├── include/core/SkCanvas.h, ...         ← public headers
├── modules/skshaper/, skottie/, ...     ← module headers
├── out/Release-<arch>/
│   ├── libskia.a  OR  skia.lib          ← 26-50 MB main static
│   ├── libfreetype2.a, libharfbuzz.a,   ← all transitive deps
│   ├── libicu.a, libpng.a, ...            as separate .a/.lib files
│   └── defines.cmake                    ← `add_definitions(-DSK_*)`
│                                          generated from the actual
│                                          ninja invocation
├── src/                                 ← internal-but-shipped headers
└── third_party/externals/               ← transitive dep headers
```

### Override paths (offline / mirror / vcpkg / custom GN)

`mpapp_find_skia()` tries `MPAPP_SKIA_PREFIX` first if set, so any
of these still work:

```sh
# 1. vcpkg layout (CMake config package exporting unofficial::skia::skia)
cmake ... -DMPAPP_SKIA_PREFIX=/path/to/vcpkg/installed/<triplet>
# → "MPAPP graphics backend: skia (vcpkg)"

# 2. Unzipped HumbleUI/JetBrains prebuilt (skip the download)
cmake ... -DMPAPP_SKIA_PREFIX=/path/to/unzipped/Skia-m143-...
# → "MPAPP graphics backend: skia (prebuilt)"
```

For Gradle / android_hello, the equivalent property is
`-PmpappSkiaPrefix=...` — only forwarded to CMake when set;
otherwise the auto-fetch runs.

### Bumping the pinned version

Each platform/arch needs a SHA-256 in the table at the top of
`cmake/MpappFindSkia.cmake`. To bump:

1. Pick a new tag from https://github.com/HumbleUI/SkiaBuild/releases
2. For each row in the table, download the corresponding zip and
   compute `sha256sum`. (Or `(Get-FileHash -A SHA256 file.zip).Hash`
   in PowerShell.)
3. Update `MPAPP_SKIA_PREBUILT_VERSION` + the
   `_MPAPP_SKIA_SHA256_<plat>-<arch>` entries.
4. Confirm the existing skia_backend.cpp compiles against the new
   milestone — Skia's public API is usually stable across milestones,
   but watch for renames in `SkPath` / `SkBitmap` / `SkCanvas`.

### Trade-offs vs. building from source

| | Auto-fetched prebuilt (default) | vcpkg compile (override) |
|---|---|---|
| Setup | nothing — `cmake` does it | install vcpkg + run `vcpkg install skia:<triplet>` |
| Time-to-first-build | ~30 s download + ~10 s extract | ~30 min Skia source compile |
| Version pinning | central row in MpappFindSkia.cmake | vcpkg baseline + manifest |
| Source of trust | community CI maintainer (HumbleUI) | Microsoft (vcpkg maintainers) |
| Customization | take what's built (GPU + Vulkan + SVG + Skottie all on) | full GN args control via vcpkg port features |
| Windows-host → Android cross-compile | **works** | broken — see Workaround #1 above |
| Offline / air-gapped builds | requires one-time `MPAPP_SKIA_PREFIX=` setup | works after initial vcpkg install |
