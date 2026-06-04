# CMake Toolchain Files

This directory holds the cross-compilation toolchain files MPAPP uses to
produce binaries for every supported target from any supported host. The
choice of Zig as the underlying cross-compiler is recorded in
[ADR-0011](../../vault/20_ADRs/ADR-0011-cross-compilation-toolchain.md);
the broader build-system context lives in
[Build System.md](../../vault/10_Architecture/Build%20System.md).

## File map

| File | Target triple | CMake `SYSTEM_NAME` |
|---|---|---|
| `zig.cmake` | (helper, not a toolchain) | — |
| `windows-x64.cmake` | `x86_64-windows-gnu` | `Windows` |
| `windows-arm64.cmake` | `aarch64-windows-gnu` | `Windows` |
| `linux-x64.cmake` | `x86_64-linux-gnu` | `Linux` |
| `linux-arm64.cmake` | `aarch64-linux-gnu` | `Linux` |
| `android-arm64.cmake` | `aarch64-linux-android.24` | `Android` |
| `macos-arm64.cmake` | `aarch64-macos-none` | `Darwin` |
| `ios-arm64.cmake` | `aarch64-ios-none` | `iOS` |

`zig.cmake` is the shared helper: it pins `MPAPP_ZIG_VERSION`, locates the
`zig` executable, and defines `mpapp_use_zig_target(<triple>)`. Each
per-platform file does only three things — set `CMAKE_SYSTEM_NAME` and
`CMAKE_SYSTEM_PROCESSOR`, include `zig.cmake`, and call the helper. To
support a new target, copy an existing file and change the triple.

## Usage

Invoke CMake with `-DCMAKE_TOOLCHAIN_FILE` pointing at the file you want:

```bash
cmake -S . -B build-linux-x64 \
      -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/linux-x64.cmake
cmake --build build-linux-x64
```

The `mpapp build --target <name>` CLI wrapper does this for you and is the
preferred user-facing entry point. The raw `cmake` invocation above is the
fallback when you need to inspect or override configure flags by hand.

## Where Zig comes from

`zig.cmake` searches for `zig` in two places, in this order:

1. **`MPAPP_ZIG_HOME`** — the per-user MPAPP toolchain cache. The defaults
   are:
   - Windows: `%USERPROFILE%\.mpapp\toolchains\zig-<version>\zig.exe`
   - POSIX:   `$HOME/.mpapp/toolchains/zig-<version>/zig`
2. **`PATH`** — any `zig` resolvable by `find_program` falls through here.

If neither yields a match, configuration fails with a `FATAL_ERROR` telling
the user exactly what version is expected and where to put it. The `mpapp`
CLI's auto-install logic (Unit-6, separate batch) populates the
`MPAPP_ZIG_HOME` path on first cross-compile; this toolchain layer only
*finds* Zig, it does not download it.

Override the search path by exporting `MPAPP_ZIG_HOME` or passing
`-DMPAPP_ZIG_HOME=<dir>` to `cmake`.

## Pinned version

The Zig version is pinned in **one place only**: the `MPAPP_ZIG_VERSION`
cache variable at the top of `zig.cmake`. Rolling Zig means editing that
single string and propagating the change to the CI workflow files that
pre-install Zig on hosted runners (see `.github/workflows/` from T-0006).

## How the helper wires Zig into CMake

CMake's compiler-detection step runs `${CMAKE_C_COMPILER}` with no easy way
to inject extra flags. The robust pattern (used by the upstream Zig
project) is to generate a tiny wrapper script per build directory that
exec's `zig cc --target=<triple>` and forwards `"$@"`. `mpapp_use_zig_target`
writes that wrapper with `file(WRITE ...)`, makes it executable on POSIX,
and sets `CMAKE_C_COMPILER` / `CMAKE_CXX_COMPILER` to the wrapper paths.

The wrapper lives in `${CMAKE_BINARY_DIR}/mpapp-zig-wrappers/`, so it is
per-build-tree and cleaned by `cmake --build ... --target clean` semantics.
On Windows the wrappers are `.cmd` shims; elsewhere they are POSIX shell
scripts.

## Apple targets — unsigned-binary caveat

Building `macos-arm64.cmake` or `ios-arm64.cmake` from a non-Mac host
produces an **unsigned** binary. macOS Gatekeeper and the iOS code-signing
pipeline refuse to run these without a separate signing pass on a Mac.
The toolchain files emit a `WARNING` when configured from a non-Apple
host as a reminder.

This is an Apple SDK constraint, not a Zig limitation. See
[ADR-0005-ios-macos-separate-interop](../../vault/20_ADRs/ADR-0005-ios-macos-separate-interop.md)
and [Build System.md § Cross-compilation matrix](../../vault/10_Architecture/Build%20System.md#cross-compilation-matrix).

## Adding a new target

1. Pick the Zig target triple. `zig targets` lists everything supported.
2. Copy the closest existing toolchain file.
3. Update `CMAKE_SYSTEM_NAME`, `CMAKE_SYSTEM_PROCESSOR`, and the
   `mpapp_use_zig_target(...)` call.
4. Add a row to the table above.
5. If the target is Apple-flavoured, keep the `message(WARNING ...)`
   block so the signing constraint stays visible.
6. Wire it into `mpapp build --target` in `tools/mpapp/`.

## See also

- [ADR-0011 — Cross-compilation toolchain is Zig (zig cc)](../../vault/20_ADRs/ADR-0011-cross-compilation-toolchain.md)
- [Build System architecture note](../../vault/10_Architecture/Build%20System.md)
- [Zig wiki — Building Zig using CMake](https://github.com/ziglang/zig/wiki/Building-Zig-using-CMake)
- [Zig as a C compiler](https://ziglang.org/learn/overview/#zig-is-also-a-c-compiler)
