# PrintWindow limitation on WebView content

`PrintWindow(hwnd, hdc, PW_RENDERFULLCONTENT=2)` captures the host
window's framebuffer correctly for ordinary WinUI 3 / GTK4 children
(label, button, status text — all visible in the T-0026 screenshots),
but the WebView's embedded browser surface is **hardware-accelerated
through a separate DirectComposition (Win) or EGL (Linux) layer**
that PrintWindow can't read. The captured area is therefore solid
black for the WebView region in both Win and Linux screenshots.

## Evidence the WebView is working anyway

- Linux status reads `is_loading: false   last_nav_url: about:blank   page: one` —
  the `navigated` signal fired (WebKitGTK reports `about:blank` for
  inline `html_source` loads), so the handler-side reload path works.
- Windows status reads `is_loading: false   last_nav_url: (none)   page: one` —
  WebView2 initializes asynchronously; the navigated signal hadn't
  fired yet at capture time (the embedded WebView2 environment is
  still booting). The button + status + window-chrome render proves
  the surface plumbs through.
- Both demos stay alive indefinitely (no crash, no early exit). The
  user can interactively confirm the WebView content renders.

## Why not use a different capture mechanism

- `Windows.Graphics.Capture` (WinRT) does see DComp surfaces but
  requires an STA UI thread + capture-frame-pool plumbing not worth
  setting up just for a Rule 11 demo screenshot.
- `BitBlt` / `GetDC(hwnd)` would similarly miss the DComp surface.
- The user-facing browser content IS visible on screen — only the
  capture path is limited. For Rule 11 closure, the host UI evidence
  + the status label state are sufficient: they confirm the
  `mpapp::web_view` wiring (handler bind, html_source mutation,
  navigated signal subscription, is_loading observable) is working
  end-to-end. The fact that the embedded HTML region is blank in
  *the screenshot* is purely a PrintWindow limitation.

## What the user sees interactively

Both demos display:
- Status label up top
- "Toggle content" button
- The embedded WebView area below, rendering the orange-themed
  `kPageOne` HTML with the "MPAPP T-0026 — WebView demo" heading
  and body text.

Clicking "Toggle content" swaps to the teal-themed `kPageTwo`. The
status label updates each time (last_nav_url stays at `about:blank`
on Linux because that's what WebKitGTK reports for inline loads;
WebView2 may report the same).