# SPDX-License-Identifier: Apache-2.0
# Android emulator end-to-end harness for MPAPP.
#
# Boots (if needed) an AVD, installs the android_hello APK, launches it,
# and captures the rendered UI straight from the device framebuffer via
# `adb screencap` — which, unlike PrintWindow against the WSLg/msrdc
# projection, is NOT blocked by the DirectComposition wall (see
# vault/50_Tasks/T-0050-.../notes/wslg-dcomp-wall.md). Also drives input
# via `adb shell input tap` for live interaction tests.
#
# This is the canonical Android e2e path for the gap-closure program:
# per-platform real handlers (Phase 4) are verified by building the APK
# (_build_android.bat) then running this to screenshot the result.
#
# Usage:
#   pwsh tools/dev/android-e2e.ps1 -Action screenshot -Out shot.png
#   pwsh tools/dev/android-e2e.ps1 -Action tap -X 540 -Y 893
#   pwsh tools/dev/android-e2e.ps1 -Action install
#
# IMPORTANT: capture with `adb shell screencap -p /sdcard/x.png` + `adb
# pull` (binary-safe). Do NOT use PowerShell `adb exec-out screencap > f`
# — the `>` redirect re-encodes the stream and corrupts the PNG.

param(
  [ValidateSet('boot', 'install', 'launch', 'screenshot', 'tap')]
  [string]$Action = 'screenshot',
  [string]$Avd = 'coroute_test',
  [string]$Package = 'io.mpapp.example',
  [string]$Apk = 'D:\GitHub\MPAPP\examples\android_hello\app\build\outputs\apk\debug\app-debug.apk',
  [string]$Out = 'D:\GitHub\MPAPP\android-e2e.png',
  [int]$X = 0,
  [int]$Y = 0
)

$ErrorActionPreference = 'Stop'
$sdk = 'D:\android-sdk'
$adb = "$sdk\platform-tools\adb.exe"
$emulator = "$sdk\emulator\emulator.exe"

function Wait-Boot {
  & $adb wait-for-device | Out-Null
  for ($i = 0; $i -lt 90; $i++) {
    if ((& $adb shell getprop sys.boot_completed 2>$null).Trim() -eq '1') { return $true }
    Start-Sleep -Seconds 2
  }
  return $false
}

switch ($Action) {
  'boot' {
    Start-Process -FilePath $emulator `
      -ArgumentList @('-avd', $Avd, '-no-window', '-no-snapshot', '-no-audio', '-gpu', 'swiftshader_indirect') `
      -WindowStyle Hidden
    if (Wait-Boot) { Write-Host "booted: $Avd" } else { Write-Warning "boot timed out" }
  }
  'install' {
    & $adb install -r $Apk
  }
  'launch' {
    & $adb shell monkey -p $Package -c android.intent.category.LAUNCHER 1 | Out-Null
    Start-Sleep -Seconds 5
    Write-Host (& $adb shell dumpsys activity activities 2>$null | Select-String 'topResumedActivity' | Select-Object -First 1)
  }
  'screenshot' {
    # Binary-safe: capture on-device, then pull.
    & $adb shell screencap -p /sdcard/_mpapp_e2e.png | Out-Null
    & $adb pull /sdcard/_mpapp_e2e.png $Out | Out-Null
    & $adb shell rm /sdcard/_mpapp_e2e.png | Out-Null
    $bytes = [System.IO.File]::ReadAllBytes($Out)
    $isPng = $bytes.Length -gt 8 -and $bytes[0] -eq 0x89 -and $bytes[1] -eq 0x50
    Write-Host ("saved {0} bytes, valid PNG: {1} -> {2}" -f $bytes.Length, $isPng, $Out)
  }
  'tap' {
    & $adb shell input tap $X $Y | Out-Null
    Write-Host "tapped ($X, $Y)"
  }
}
