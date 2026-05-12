# Zig installation for T-0009

This is the exact procedure used to install Zig 0.13.0 on the Windows host
that ran the T-0009 cross-compilation validation on 2026-05-12. Reproduce it
step-for-step to re-run the matrix.

## Why 0.13.0?

Per [[ADR-0011-cross-compilation-toolchain]] §"Vendoring strategy" the Zig
version is *pinned* — `cmake/toolchains/zig.cmake` looks for the
binary under `~/.mpapp/toolchains/zig-0.13.0/`. 0.13.0 is the latest stable
release at the time of validation (released 2024-06-07 upstream).

## Install procedure (Windows host, PowerShell)

```powershell
$dest = "$env:USERPROFILE\.mpapp\toolchains"
New-Item -ItemType Directory -Path $dest -Force | Out-Null

# Download the official Windows x86_64 archive
Invoke-WebRequest `
    -Uri "https://ziglang.org/download/0.13.0/zig-windows-x86_64-0.13.0.zip" `
    -OutFile "$dest\zig-0.13.0.zip" -UseBasicParsing

# Extract and rename to the version-only folder layout expected by the
# toolchain files
Expand-Archive -Path "$dest\zig-0.13.0.zip" -DestinationPath $dest -Force
Rename-Item -Path "$dest\zig-windows-x86_64-0.13.0" -NewName "zig-0.13.0"

# Add to PATH for the current shell. The mpapp CLI will eventually handle
# this automatically (ADR-0011); for the bring-up phase the toolchain files
# also probe $USERPROFILE\.mpapp\toolchains\zig-0.13.0\zig.exe directly.
$env:PATH = "$env:USERPROFILE\.mpapp\toolchains\zig-0.13.0;$env:PATH"

zig version  # → 0.13.0
```

## Equivalent for Linux / macOS hosts

For Linux x86_64:

```bash
mkdir -p ~/.mpapp/toolchains
curl -fL -o /tmp/zig.tar.xz \
    https://ziglang.org/download/0.13.0/zig-linux-x86_64-0.13.0.tar.xz
tar -xJf /tmp/zig.tar.xz -C ~/.mpapp/toolchains/
mv ~/.mpapp/toolchains/zig-linux-x86_64-0.13.0 ~/.mpapp/toolchains/zig-0.13.0
export PATH="$HOME/.mpapp/toolchains/zig-0.13.0:$PATH"
zig version  # → 0.13.0
```

For macOS arm64 the archive is `zig-macos-aarch64-0.13.0.tar.xz`; for
macOS Intel use `zig-macos-x86_64-0.13.0.tar.xz`.

## Verification

```
$ zig version
0.13.0

$ zig targets | jq -r .arch[] | head -3
arm
armeb
aarch64
```

Download size: 80 MB (Windows zip). Extracted size: ~870 MB (the bulk is
bundled libc headers and the Clang/LLVM runtime).

## SHA-256 checksum (Windows archive)

The official `https://ziglang.org/download/index.json` lists the SHA-256 for
each archive. For 0.13.0 Windows x86_64:

```
d859994725ef9402381e557c60bb57497215682e355204d754ee3df75ee3c158
```

To verify locally:

```powershell
(Get-FileHash "$env:USERPROFILE\.mpapp\toolchains\zig-0.13.0.zip" -Algorithm SHA256).Hash.ToLower()
```

## Notes / caveats discovered during T-0009

- Zig 0.13.0 ships libc *headers* and importable sysroots only for the
  triples enumerated under `zig targets | jq -r .libc[]`. Android (bionic)
  and iOS are **not in that list** — see `matrix-status.md` for the
  empirical fallout.
- `zig cc` / `zig c++` work fine, but `zig ar` and `zig ranlib` cannot be
  passed to CMake as multi-token executables (CMake re-parses the cache
  list separator and breaks). `cmake/toolchains/zig.cmake` works
  around this by generating per-target single-token wrapper scripts in
  `$TEMP/mpapp-zig-<triple>/`.
- C++23 module dependency scanning (clang-scan-deps) is **not** bundled in
  Zig 0.13. `zig.cmake` therefore sets
  `CMAKE_CXX_SCAN_FOR_MODULES=OFF` for every cross target. This is safe
  because MPAPP does not yet use C++ named modules in its public surface.
