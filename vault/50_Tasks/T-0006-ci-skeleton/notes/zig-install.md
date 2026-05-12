# Zig install snippet for cross-build jobs

Per RFC-0002, Zig is the chosen cross-compilation toolchain. Both `windows-cross` and `linux-cross` jobs pin to a specific Zig version via the workflow-level `ZIG_VERSION` env var. The toolchain is cached so the download only happens on cache-miss.

## Pinned version

```yaml
env:
  ZIG_VERSION: "0.13.0"
```

Bump this in lock-step with what the `mpapp` CLI ships locally; mismatches between CI and dev machines defeat the point of pinning.

## Windows host (PowerShell)

```yaml
- name: Cache Zig toolchain
  id: zig-cache
  uses: actions/cache@v4
  with:
    path: C:\zig
    key: zig-${{ env.ZIG_VERSION }}-windows

- name: Install Zig (pinned)
  if: steps.zig-cache.outputs.cache-hit != 'true'
  shell: pwsh
  run: |
    $url = "https://ziglang.org/download/$env:ZIG_VERSION/zig-windows-x86_64-$env:ZIG_VERSION.zip"
    Invoke-WebRequest -Uri $url -OutFile zig.zip
    Expand-Archive zig.zip -DestinationPath C:\zig-tmp
    Move-Item "C:\zig-tmp\zig-windows-x86_64-$env:ZIG_VERSION" C:\zig
    Remove-Item zig.zip

- name: Add Zig to PATH
  shell: pwsh
  run: Add-Content $env:GITHUB_PATH "C:\zig"
```

## Linux host (bash)

```yaml
- name: Cache Zig toolchain
  id: zig-cache
  uses: actions/cache@v4
  with:
    path: /opt/zig
    key: zig-${{ env.ZIG_VERSION }}-linux

- name: Install Zig (pinned)
  if: steps.zig-cache.outputs.cache-hit != 'true'
  run: |
    curl -fsSL "https://ziglang.org/download/${ZIG_VERSION}/zig-linux-x86_64-${ZIG_VERSION}.tar.xz" -o zig.tar.xz
    sudo mkdir -p /opt/zig
    sudo tar -xJf zig.tar.xz -C /opt/zig --strip-components=1
    rm zig.tar.xz

- name: Add Zig to PATH
  run: echo "/opt/zig" >> "$GITHUB_PATH"
```

## CMake hand-off

Once Zig is on PATH, the cross-build steps invoke CMake with a toolchain file (placeholders pending T-0009):

```yaml
- run: cmake -S . -B build-linux-x64 -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/linux-x64.cmake
```

Inside those toolchain files, `CMAKE_C_COMPILER` and `CMAKE_CXX_COMPILER` are set to `zig cc` and `zig c++` respectively, with `-target <triple>` baked into the launcher. See RFC-0002 for the rationale.

## Links

- [[RFC-0002]] (cross-compilation toolchain selection)
- [[CI Strategy]]
- [[T-0009-cross-compilation-matrix]]
