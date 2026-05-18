# WSL blocker on the T-0007 worker environment

## Mode

**Blocked at step 1 (WSL not installed) — graceful-failure path taken
per the unit brief.** Source files for `examples/gtk4_hello/` were
written so they will build the moment a WSL2 Ubuntu guest is available;
only the install + screenshot steps are deferred.

## Host environment

- **OS:** Windows 11 Pro N, 10.0.26200
- **Shell:** PowerShell 5.1, non-elevated
- **Bash on PATH:** `C:\Program Files\Git\usr\bin\bash.exe` (Git for
  Windows — not a WSL bash)

## What was attempted

```powershell
PS> wsl --status
The Windows Subsystem for Linux is not installed. You can install by
running 'wsl.exe --install'.
For more information please visit https://aka.ms/wslinstall

PS> wsl --list --verbose
The Windows Subsystem for Linux is not installed. ...

PS> wsl --install --no-distribution
The Windows Subsystem for Linux is not installed. ...

PS> Get-WindowsOptionalFeature -Online -FeatureName Microsoft-Windows-Subsystem-Linux
Get-WindowsOptionalFeature : The requested operation requires elevation.
```

Two compounding blockers:

1. **WSL is not installed** on the host. `wsl --install` is the right
   command but it needs an elevated session and reboots the box.
2. **The worker session is non-elevated.** Neither
   `Get-WindowsOptionalFeature` nor `wsl --install` can run from here,
   so we cannot enable `Microsoft-Windows-Subsystem-Linux` /
   `VirtualMachinePlatform` either.

Because step 1 fails, every downstream step is unreachable:

- Cannot `apt install libgtk-4-dev` (no WSL guest).
- Cannot run the cross-built ELF (no Linux loader on the host).
- Cannot capture a WSLg screenshot (no compositor bridge yet).

## Recovery path

When a real human (or a privileged automation) is on the box:

1. Open an **elevated** PowerShell and run:
   ```powershell
   wsl --install -d Ubuntu-22.04
   wsl --update
   wsl --set-default-version 2
   ```
   Reboot when prompted.
2. Finish Ubuntu first-run, then follow [[wslg-setup]] from step 2
   (`apt install` + build + run).
3. Capture the GTK4 window to
   `vault/50_Tasks/T-0007-wslg-gtk4-hello/screenshots/wslg-gtk4-window.png`
   and flip `hasScreenshots: true` in the task frontmatter.
4. Bump task `status: todo` → `in-progress` → `done` and archive per
   Rule 11 (see [[CLAUDE]]).

## What is shippable today

- `examples/gtk4_hello/main.c` — GTK4 hello app source.
- `examples/gtk4_hello/CMakeLists.txt` — `pkg_check_modules(GTK4 ...)`
  target, gated on `UNIX AND NOT APPLE`.
- `examples/CMakeLists.txt` — `add_subdirectory(gtk4_hello)` behind the
  same gate.
- [[wslg-setup]] — full install recipe and both build paths.

On the current Windows host the new subdirectory is inert: the
`UNIX AND NOT APPLE` guard excludes it from the WinUI 3 build, so this
change is a no-op for Windows CI / dev.
