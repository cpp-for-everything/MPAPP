# SPDX-License-Identifier: Apache-2.0
# Capture one screenshot of the gtk4_vsm_demo window per visual state.
# Pattern:
#   1. Tell WSL to kill any stale demo + launch a fresh one with
#      MPAPP_VSM_INITIAL_STATE=<state>. The demo's on_launch path
#      applies the state immediately via vsm.go_to_state, so the
#      first frame already shows the post-transition text.
#   2. Wait for msrdc to project the GTK4 window onto the host
#      desktop with the demo's title.
#   3. PrintWindow BitBlt that HWND and save to PNG.
#
# Why env var? GTK4's GApplication consumes leftover argv as
# file-open paths; passing the state via argv crashes the demo
# before on_launch runs. Env-var routing sidesteps that.

param(
  [string]$OutDir = 'D:\GitHub\MPAPP\vault\_Assets\screenshots\RFC-0006-vsm',
  [string]$Demo   = '/mnt/d/GitHub/MPAPP/build-wsl/examples/gtk4_vsm_demo/gtk4_vsm_demo',
  [string[]]$States = @('Normal', 'Pressed', 'Disabled')
)

Add-Type -AssemblyName System.Windows.Forms, System.Drawing
$src = @'
using System;
using System.Runtime.InteropServices;
public class WinCap {
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr hWnd, out RECT rect);
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr hWnd, IntPtr hdcBlt, uint nFlags);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
  [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left, Top, Right, Bottom; }
}
'@
Add-Type -TypeDefinition $src -ReferencedAssemblies System.Drawing,System.Windows.Forms -ErrorAction SilentlyContinue

if (-not (Test-Path $OutDir)) {
  New-Item -ItemType Directory -Path $OutDir -Force | Out-Null
}

function Find-DemoWindow {
  return Get-Process | Where-Object {
    $_.MainWindowTitle -like '*Visual State Manager*'
  } | Select-Object -First 1
}

function Kill-DemoInsideWsl {
  & wsl.exe -d Ubuntu-24.04 -- bash -lc "pkill -f gtk4_vsm_demo || true" *>$null
  Start-Sleep -Milliseconds 600
}

foreach ($state in $States) {
  Kill-DemoInsideWsl

  Write-Host "Launching demo for state '$state'..."
  # `setsid` detaches the demo from the wsl.exe process so we can
  # exit immediately without killing the GUI process.
  $cmd = "MPAPP_VSM_INITIAL_STATE=$state setsid $Demo </dev/null >/tmp/vsm-demo.log 2>&1 &"
  & wsl.exe -d Ubuntu-24.04 -- bash -lc "$cmd; sleep 0.2" *>$null

  # Poll up to 20s for the WSLg-projected window to appear with
  # its title populated. msrdc cold-starts can take a few seconds.
  $deadline = (Get-Date).AddSeconds(20)
  $win = $null
  while ((Get-Date) -lt $deadline) {
    Start-Sleep -Milliseconds 500
    $win = Find-DemoWindow
    if ($win) { break }
  }
  if (-not $win) {
    Write-Warning "Demo window for state '$state' did not appear within 20s."
    continue
  }

  # Foreground + render the demo's repaint.
  [void][WinCap]::ShowWindow($win.MainWindowHandle, 5)
  [void][WinCap]::SetForegroundWindow($win.MainWindowHandle)
  Start-Sleep -Milliseconds 1200

  $r = New-Object 'WinCap+RECT'
  [void][WinCap]::GetWindowRect($win.MainWindowHandle, [ref]$r)
  $w  = $r.Right - $r.Left
  $hh = $r.Bottom - $r.Top
  Write-Host "  Window: ${w}x${hh} at ($($r.Left),$($r.Top))"

  $bmp = New-Object System.Drawing.Bitmap $w, $hh
  $g   = [System.Drawing.Graphics]::FromImage($bmp)
  $hdc = $g.GetHdc()
  $ok  = [WinCap]::PrintWindow($win.MainWindowHandle, $hdc, 2)
  $g.ReleaseHdc($hdc)
  $g.Dispose()

  $path = Join-Path $OutDir ("linux-" + $state.ToLowerInvariant() + ".png")
  $bmp.Save($path, [System.Drawing.Imaging.ImageFormat]::Png)
  $bmp.Dispose()
  Write-Host "  Saved: $path  (PrintWindow=$ok)"
}

Kill-DemoInsideWsl
Write-Host "Done."
