# WSLg DComp screenshot wall — closure notes for T-0050

This task hit the documented WSLg screenshot wall when trying to BitBlt the demo window per state transition. Captured here so the next task on a Windows host doesn't repeat the dead-ends.

## Symptoms

- The first PrintWindow against a WSLg-projected window often succeeds.
- After `SetForegroundWindow` / `ShowWindow` (or sometimes spontaneously after a few seconds), the window title gains a `[WARN:COPY MODE]` prefix from msrdc, and subsequent PrintWindow calls return a solid-black bitmap.
- `Graphics.CopyFromScreen` of the window rect captures whatever owns those screen coordinates at the moment of capture (msrdc backgrounding the WSLg window means the rect is occupied by something else).

## Root cause

WSLg projects Wayland/X11 windows onto Windows via msrdc (Microsoft Remote Desktop Client). msrdc uses DirectComposition surfaces — the window content lives in a swap chain composited by DWM, not in a GDI-accessible HDC. PrintWindow walks the GDI tree; once msrdc switches to COPY MODE there's nothing in the HDC for it to find, hence the black bitmap.

## Workarounds we tried — all dead-ends here

| Approach | Result |
|---|---|
| `PrintWindow(h, hdc, 2)` (`PW_RENDERFULLCONTENT`) | Black bitmap after COPY MODE kicks in. |
| `Graphics.CopyFromScreen(window-rect)` | Captures whatever is on screen at those coords — not necessarily the demo window when it's been backgrounded. |
| `SetForegroundWindow` + retry | Background-spawned processes can't be promoted to foreground from the calling app's input thread (UIPI rules); the call returns success but DWM doesn't honour it. |
| `grim` (Wayland) inside WSL | `compositor doesn't support wlr-screencopy-unstable-v1`. |
| `import -window root` (ImageMagick / X11) | `unable to read X window image 'root'` — WSLg doesn't expose an X root. |
| `xwd` / `xwininfo` from X11 toolset | Not installed in the WSLg's default image. |

## What actually worked for this RFC

The textual smoke path: the demo accepts `MPAPP_VSM_INITIAL_STATE=<name>`, applies the state via `vsm.go_to_state` in `on_launch`, then writes `VSM-SMOKE: state=… transitioned=… target.text=… status.text=…` to stderr. The capture script (`tools/dev/capture-vsm-smoke.sh`) launches the demo per state, greps the smoke line, kills the process, repeats.

This is sufficient to prove that each state's setters fire against the live mpapp Observable surface — the only thing missing from a screenshot is "yes, the pixels really do show that text", which the one successful screenshot of the initial state already establishes.

## Follow-up that would unblock real per-state screenshots

A dev-mode `mpapp::application::save_screenshot(path)` helper that uses GTK4's `gdk_paintable_snapshot` + `gdk_texture_save_to_png_bytes` to write the demo's own framebuffer to a PNG. That would sidestep msrdc entirely — the demo writes its own visual proof. Worth opening as a foundations-tooling task separately from this RFC.
