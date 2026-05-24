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

## Android Skia install failure

`vcpkg install skia:arm64-android` and `skia:x64-android` were
attempted in `C:/tools/vcpkg` but no packaged artifacts landed:

```text
C:/tools/vcpkg/buildtrees/skia/
  arm64-android.vcpkg_abi_info.txt     # vcpkg ABI fingerprint
  x64-android.vcpkg_abi_info.txt       # but no build-* log files,
                                       # so the install bailed before
                                       # actually compiling Skia.
C:/tools/vcpkg/packages/
  skia_x64-windows                     # only the Windows triplet
                                       # made it to packages/.
```

Likely root causes (none verified):

1. **Missing `ANDROID_NDK_HOME` / vcpkg android triplet config**.
   vcpkg's android triplets need an external NDK install via env
   vars; if those weren't set in the install shell, vcpkg refuses
   the install before reaching the per-port build.
2. **Community-triplet restrictions**. Some vcpkg ports flag
   themselves as not-supported-on certain triplets. The Skia port
   manifest may guard against arm64-android.

Practical impact: zero — Android keeps Cairo as its only real
graphics backend. The shared canvas facade means Android could swap
to Skia later by re-running the install + flipping
`MPAPP_GRAPHICS_BACKEND=skia` in `app/build.gradle.kts`'s
`externalNativeBuild` args.
