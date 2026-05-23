# T-0016 notes

Design notes / gotchas captured during the catch-up. The primary task doc is
`../T-0016-canvas-cairo-render-demo.md`.

## Gotchas hit during the catch-up

### Windows pkg-config split-at-colon

vcpkg ships two pkgconf binaries:
- `downloads/tools/msys2/.../usr/bin/pkgconf.exe` — splits paths at `C:`
  (treats `:` as a Unix PATH separator). PKG_CONFIG_PATH=`C:/...` lookup fails
  with "Package cairo was not found in the pkg-config search path".
- `downloads/tools/msys2/.../mingw64/bin/pkgconf.exe` — handles Windows
  drive-letter paths correctly.

The `_build_full.bat` (gitignored, worker helper) and the README's
documented setup both point at the mingw64 one.

### Android iconv + minSdk 28

`cairo:x64-android` and `cairo:arm64-android` install with the default
features set — `core,fontconfig,freetype`. fontconfig calls
`iconv_open / iconv / iconv_close` (in FcSfntNameTranscode). Android's
bionic libiconv was added in API 28. Apps targeting <28 hit
`undefined symbol: iconv_open` at link time.

Workaround: bump `minSdk = 28` in `examples/android_hello/app/build.gradle.kts`.
Apps that don't need real Cairo can drop the `MPAPP_GRAPHICS_BACKEND=cairo`
cmake arg and re-target lower minSdk on stub backend.

### PowerShell `>` binary-corrupts redirected stdout

`adb exec-out screencap -p > file.png` from PowerShell adds a UTF-16 BOM +
line-ending conversion that corrupts the PNG. Use the on-device file path
instead: `adb shell screencap -p /sdcard/screen.png && adb pull /sdcard/screen.png`.

### Android opacity carryover divergence

The cross-platform `cairo_render_demo.cpp` uses the facade's
`set_opacity` followed by a clip+fill that picks up the residual 0.5 alpha
from the previous step. The Android JNI inline replicates Cairo ops
directly without the facade's state stack, so it doesn't carry that alpha
into the clip-fill step. The Android PNG's bottom-right ellipse is fully
opaque where the Win+Linux versions are half-transparent.

Resolution: documented in the task md as a known divergence. Not worth
chasing — the facade's behavior is the spec; the Android demo inlines the
Cairo path for JNI simplicity rather than reusing the facade through JNI
boundaries.
