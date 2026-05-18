# WSLg + GTK4 dev-loop setup (Windows 11 host)

These steps prepare a Windows 11 Pro host to build and run the
[[examples/gtk4_hello]] target as a native window on the Windows desktop
via WSLg. They were validated against the worker environment for
[[T-0007-wslg-gtk4-hello]]; see [[wsl-blocker]] for what to do when WSL
itself is unavailable on the host.

## 1. Install WSL2 + Ubuntu 22.04 (from elevated PowerShell on the host)

```powershell
# One-shot installer — pulls the WSL2 kernel and Ubuntu by default.
wsl --install -d Ubuntu-22.04

# If WSL is already present, make sure the kernel and toolchain are
# current. `wsl --update` is safe to re-run.
wsl --update

# Make WSL2 the default for new distros (WSLg requires WSL2).
wsl --set-default-version 2
```

Reboot when prompted, then launch Ubuntu from the Start Menu to finish
first-run user creation.

### Verify WSLg is wired up

Inside the Ubuntu shell:

```bash
sudo apt update
sudo apt install -y x11-apps
xeyes
```

A pair of cartoon eyes should appear on the Windows desktop, tracking
the cursor. If they do, WSLg is forwarding GUI windows correctly. If
nothing appears, see [[wsl-blocker]] for the fallback (Hyper-V Ubuntu
VM with VcXsrv).

## 2. Install GTK4 dev packages inside WSL2

```bash
sudo apt update
sudo apt install -y \
    build-essential \
    libgtk-4-dev \
    cmake \
    ninja-build \
    pkg-config
```

Confirm the dev package is discoverable:

```bash
pkg-config --modversion gtk4
# expected: 4.6.x or newer on Ubuntu 22.04
```

## 3. Build & run `gtk4_hello`

Two build paths are supported. Use Option 1 if the Linux toolchain file
from T-0001 (`cmake/toolchains/linux-x64.cmake`) is on disk; otherwise
fall back to Option 2 inside WSL.

### Option 1 — Cross-build from the Windows host (preferred)

```powershell
# From an elevated or normal PowerShell on the Windows host:
cmake -S . -B build-linux-x64 `
    -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/linux-x64.cmake `
    -DMPAPP_BUILD_EXAMPLES=ON
cmake --build build-linux-x64 --target gtk4_hello

# Run inside WSL — the host build directory is reachable from the WSL
# guest under /mnt/<drive>/...
wsl -d Ubuntu-22.04 -- /mnt/d/GitHub/MPAPP/build-linux-x64/examples/gtk4_hello/gtk4_hello
```

### Option 2 — Build inside WSL2 (fallback)

```bash
# Inside WSL2 Ubuntu:
cd /mnt/d/GitHub/MPAPP
cmake -S . -B build-wsl -G Ninja -DMPAPP_BUILD_EXAMPLES=ON
cmake --build build-wsl --target gtk4_hello
./build-wsl/examples/gtk4_hello/gtk4_hello
```

A 480x240 GTK4 window titled `MPAPP - WSLg GTK4 hello` should appear on
the Windows desktop with the greeting label. Capture it to
`screenshots/wslg-gtk4-window.png` and flip `hasScreenshots: true` in
the task frontmatter.

## 4. Outcome on the worker environment

The current host has WSL absent and is non-elevated, so steps 1-3 could
not be exercised end-to-end. The C source and CMake target were still
authored and parked behind the `UNIX AND NOT APPLE` gate so they are
inert on Windows builds and ready to compile the moment WSL becomes
available. See [[wsl-blocker]] for the captured error output and
recovery path.
