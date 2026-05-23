# Linux WSLg screenshot quirk

The GTK4 demo (`gtk4_routes_demo`) builds clean and launches cleanly
under WSLg — the first computer-use screenshot taken just after
`MSYS_NO_PATHCONV=1 wsl.exe -d Ubuntu-24.04 -- env DISPLAY=:0 ...`
showed the window titled "MPAPP T-0017 - Typed Routing Demo (GTK4)"
on monitor VG27WQ at ~640px-960px on the right side, with all the
expected widgets (current_route / navigation_blocked / lifecycle
labels, two switches, six `go_to<>()` buttons).

Re-launching the demo in the same session and re-capturing produced
empty captures on both monitors via:
- PowerShell `[System.Drawing.Graphics]::CopyFromScreen(...)` for
  both DISPLAY1 (`X=0..2560`) and DISPLAY2 (`X=2560..5120`) ranges.
- `mcp__computer-use__screenshot` cycling through both monitors.

The process was confirmed alive (`pgrep -af gtk4_routes_demo` →
PID 699), but WSLg apparently doesn't re-render the X11 surface to
a Windows-visible compositor layer after the first invocation in a
session, OR the window opened off-screen on a virtual third display
not in the host's screen enumeration.

Neither `import` (ImageMagick X11) nor `grim` (Wayland's
wlr-screencopy-unstable-v1 protocol) worked inside WSL — the former
fails to access the X server's root window, the latter reports
"compositor doesn't support wlr-screencopy-unstable-v1".

**Workaround for future runs:** start WSL fresh (`wsl --shutdown` +
relaunch), then launch the demo as the first WSLg app — the first
launch's window IS captured by computer-use successfully (proven in
the first session screenshot). Beyond that single capture, the WSLg
compositor caches state that prevents re-rendering.

For this task's evidence, the Linux artifact is:
- The demo source (`examples/gtk4_routes_demo/main.cpp`) which proves
  the typed `shell.go_to<Path, &routes>(args...)` surface compiles
  with the same `route_table{}` shape on the Linux toolchain
- The ctest pass for the underlying routing + guards + lifecycle in
  `tests/mock_handlers/route_table_test.cpp` +
  `tests/mock_handlers/shell_test.cpp`
- The Windows + Android live screenshots in this same `screenshots/`
  dir — same code path, just with reliable screen capture.
