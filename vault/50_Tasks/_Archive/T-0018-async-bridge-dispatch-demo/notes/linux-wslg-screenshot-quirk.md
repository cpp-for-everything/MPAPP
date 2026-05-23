# Linux WSLg screenshot quirk (T-0018 — same pattern as T-0017)

The GTK4 async-bridge demo (`gtk4_async_bridge_demo`) builds and
launches cleanly under WSLg, and the `msrdc` window with title
"MPAPP T-0018 - Async Bridge Demo (GTK4) (Ubuntu-24.04)" appears in
the Windows process list with a non-zero rectangle (`L=1889 T=683
W=648 H=389` in this session). The X11 surface itself, however, is
not picked up by Windows-side screen capture — the captured pixels
in the msrdc window region come from whatever is *behind* it on the
host compositor (in this session, a fullscreen game running on the
secondary display).

This reproduces the same compositor-cache behavior documented for
T-0017's `gtk4_routes_demo`. The full diagnosis is in
[[../../T-0017-typed-routing-demo/notes/linux-wslg-screenshot-quirk]].
A summary of what we tried for this task:

1. Cold-launched the demo as the first WSLg app after `wsl --shutdown`
   — the WSL VM did stop, but the Windows-side `msrdc.exe` host
   process (PID 26616 in this session) survived the shutdown and
   re-attached when the demo connected to `:0`. The host-side
   compositor surface stayed in the same not-rendering-to-Windows
   state as before the shutdown.
2. PowerShell `[System.Drawing.Graphics]::CopyFromScreen(...)` over
   both monitors — same empty / showthrough result.
3. `mcp__computer-use__screenshot` cycling through both monitors —
   same result.

**Evidence for the Linux platform in this task therefore relies on:**

- The demo source (`examples/gtk4_async_bridge_demo/main.cpp`) which
  proves the typed `register_method` / `register_async_method<T>` +
  `dispatch_async` surface compiles and links on the Linux toolchain
  against `mpapp-core` + `mpapp-handlers-linux`.
- The same unit-tested behavior covered by
  `tests/mock_handlers/hybrid_bridge_test.cpp` (16 cases including
  the 7 Phase-F-specific `dispatch_async` cases), which the Linux
  ctest run exercises against the same `mpapp::hybrid_bridge`
  header-only code path the demo uses.
- The Windows + Android live captures in the same `screenshots/`
  dir — same code path, just with reliable screen capture surfaces.
