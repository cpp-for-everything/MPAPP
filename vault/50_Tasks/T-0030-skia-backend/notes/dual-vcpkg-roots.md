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
