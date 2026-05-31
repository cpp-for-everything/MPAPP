# SPDX-License-Identifier: Apache-2.0
# T-0067: package the built УИСС WinUI 3 app as a loose MSIX layout and register it
# (dev mode, unsigned). Package IDENTITY lets the app run as MSIX and makes it
# targetable by computer-use (pixel-based UI tools). NOTE: identity does NOT fix the
# WinUI content-island UI-Automation tree (FlaUI/UIA still see E_UNEXPECTED — a WinUI 3
# framework defect); computer-use works because it drives pixels, not the UIA tree.
#
#   powershell -ExecutionPolicy Bypass -File pack-and-register.ps1 [-Debug <Debug-dir>]
# Then launch:  Start-Process "shell:AppsFolder\uiss_kgcp43ee8ctb2!App"
param(
  [string]$Debug = "$PSScriptRoot\..\..\..\..\build-vs143\examples\uiss\Debug",
  [string]$Pkg   = "$PSScriptRoot\..\..\..\..\build-vs143\package"
)
$ErrorActionPreference = "Stop"
$Debug = (Resolve-Path $Debug).Path
if (-not (Test-Path "$Debug\uiss.exe")) { throw "uiss.exe not found in $Debug — build the MPAPP_UISS_WINUI_SHELL_WIP target first" }

Remove-Item -Recurse -Force $Pkg -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force -Path "$Pkg\Assets" | Out-Null

# Assets: solid TU-blue placeholder logos (manifest requires Square44/150 + StoreLogo).
Add-Type -AssemblyName System.Drawing
function New-SolidPng([string]$path,[int]$w,[int]$h) {
  $b = New-Object System.Drawing.Bitmap($w,$h); $g = [System.Drawing.Graphics]::FromImage($b)
  $g.Clear([System.Drawing.Color]::FromArgb(26,43,87)); $g.Dispose()
  $b.Save($path,[System.Drawing.Imaging.ImageFormat]::Png); $b.Dispose()
}
New-SolidPng "$Pkg\Assets\StoreLogo.png" 50 50
New-SolidPng "$Pkg\Assets\Square150x150Logo.png" 150 150
New-SolidPng "$Pkg\Assets\Square44x44Logo.png" 44 44

# Payload: exe + compiled XAML + resources.pri + app-local DLLs (the WinUI framework
# itself comes from the Microsoft.WindowsAppRuntime.1.8 package dependency).
Copy-Item "$Debug\uiss.exe","$Debug\App.xbf","$Debug\resources.pri" $Pkg -Force
Copy-Item "$Debug\Microsoft.WindowsAppRuntime.Bootstrap.dll","$Debug\Microsoft.WindowsAppRuntime.dll",`
          "$Debug\Microsoft.Web.WebView2.Core.dll","$Debug\WebView2Loader.dll" $Pkg -Force -ErrorAction SilentlyContinue
Copy-Item "$PSScriptRoot\AppxManifest.xml" "$Pkg\AppxManifest.xml" -Force

# Register the loose layout (dev mode = no signing required).
Get-AppxPackage -Name "uiss" -ErrorAction SilentlyContinue | Remove-AppxPackage -ErrorAction SilentlyContinue
Add-AppxPackage -Register "$Pkg\AppxManifest.xml"
$p = Get-AppxPackage -Name "uiss"
Write-Output ("REGISTERED " + $p.PackageFullName + "  (AUMID: " + $p.PackageFamilyName + "!App)")
