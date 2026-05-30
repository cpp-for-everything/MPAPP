---
type: task
id: T-0067
title: Windows unpackaged WinUI 3 — TextBox first-render crash (missing merged resources.pri)
status: in-progress
milestone: M-05
owner: ""
area: handlers
blockedBy: []
coveragePercent: 0
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/in-progress
  - area/handlers
  - platform/windows
---

# T-0067 — Windows TextBox first-render crash (resources.pri)

## Discovered by
End-to-end run of `examples/uiss` on **Windows native (WinUI 3)** — the first
time the app was actually executed (previously only build/link-verified, see
T-0060). `_build_winui3`-style build; run `build/examples/uiss/uiss.exe`.

## Symptom
- `uiss.exe` exits immediately with `0xC0000409` (fast-fail). WER:
  faulting module **`Microsoft.UI.Xaml.dll`**, exception **`0xc000027b`**
  (stowed exception), stowed hr **`0x80004005`** (E_FAIL).
- The diagnostic launch log (`MPAPP_LOG_LAUNCH=<path>` env var, handled by
  `src/handlers/windows/application_handler.cpp`) shows the **entire UI tree
  builds fine**: `OnLaunched … on_launch returned` — login page, ≡ menu, and
  all 10 section pages (incl. the new CarouselView + picker) construct without
  error. The crash is a **deferred E_FAIL on the first WinUI layout/render
  pass**, after `OnLaunched` returns, on the message loop.

## Root cause (confirmed by bisection)
Bisected the login page element-by-element (temporarily pointing
`window_.content` at `login_.page`, then trimming `root`):
- Plain labels, styled labels (font/size/weight/**color**), the subtitle
  **tap gesture**, and the **button** all render fine.
- Adding an **`mpapp::entry` (WinUI `TextBox`)** reproduces the crash. `Button`
  works, `TextBox` does not.

Forcing the theme load (`Resources().MergedDictionaries().Append(
XamlControlsResources{})`) changed the error to the **smoking gun**:
`0x802B000A — Cannot locate resource from
'ms-appx:///Microsoft.UI.Xaml/Themes/themeresources.xaml'`.

So: the **unpackaged** app has **no merged `resources.pri`**, so MRT can't
resolve the WinUI framework theme resources via `ms-appx://`. `TextBox` hard-
requires `themeresources.xaml` at render; `Button` degrades without it (which
is why the earlier VSM/button Windows demos "ran"). **This is a build/deploy
infra gap, not app logic** — `uiss` app code is correct and platform-neutral.

## What did NOT work (ruled out)
1. `XamlControlsResources{}` merge in the App **ctor** — throws (Resources()
   not ready) — caught.
2. Same merge in **OnLaunched** — `XamlControlsResources{}` construction
   itself throws (it tries to load the missing themeresources).
3. **Copying** the WinUI framework pri
   (`build/packages/Microsoft.WindowsAppSDK.WinUI/runtimes-framework/win-x64/native/Microsoft.UI.Xaml.Controls.pri`)
   next to the exe as `resources.pri` — still crashes (app's default resource
   map name must be the app, with framework maps merged in).
4. `makepri merge /if <framework pris…>` — exit 87 (invalid params).
5. `makepri new` minimal app pri (root map `uiss`, no framework refs) — still
   crashes (no framework resource maps in it).

## The fix (the MrtCore flow the WindowsAppSDK MSBuild `.targets` run)
Generate the app `resources.pri` so it **merges the framework package PRIs**.
`makepri new` auto-merges `.pri` files found under `/pr`. Recipe to wire into
the Windows `uiss`/example CMake (and ideally a reusable
`cmake/WindowsAppSDK.cmake` helper, since it affects *every* mpapp Windows app
with a text control):

1. `makepri.exe` lives at
   `C:\Program Files (x86)\Windows Kits\10\bin\<sdkver>\x64\makepri.exe`.
2. Stage a dir containing the **framework win-x64 PRIs** from `build/packages`:
   `Microsoft.UI.Xaml.Controls.pri`, `Microsoft.UI.pri`,
   `Microsoft.WindowsAppRuntime.pri`, `Microsoft.Windows.Workloads.pri`.
3. `makepri createconfig /cf priconfig.xml /dq en-US /o`
4. `makepri new /pr <stage> /cf priconfig.xml /of <outdir>/resources.pri
   /in <AppName> /o`  — indexes the stage, **merging** the framework PRIs into
   the app index whose root map is `<AppName>`.
5. Deploy `resources.pri` next to the exe (alongside the WindowsAppRuntime DLL
   copy step `mpapp_add_winappsdk_runtime`).

NOTE: step 4 merging behavior must be verified — the merged pri's framework
file entries (`ms-resource://Microsoft.UI.Xaml/Files/...themeresources.xbf`)
must still resolve at runtime from the loaded framework package. If `makepri
new` doesn't merge as hoped, fall back to invoking the WindowsAppSDK MrtCore
`.targets` via a one-shot `msbuild` of a tiny stub project, or replicate its
`makepri new … /mn <manifest>` invocation with the framework PRIs referenced
in the priconfig `<index>`.

## Verification plan (resume here)
- [ ] Wire pri generation into the Windows build; rebuild `uiss`.
- [ ] `uiss.exe` launches to a **rendered window** (login page visible).
- [ ] computer-use screenshot of login; type fac# `201221001` + any ЕГН;
      click **Вход**; screenshot the ≡ nav; open **Информация** → screenshot
      the announcements **CarouselView**; click ▶ to page it; open **Плащания**
      → screenshot the picker. Save shots to this task's `screenshots/`.
- [ ] Confirm the same fix lets `windows_button_spike` etc. host a TextBox.

## Update — pri generation works; the gap is pri *loading*
Generated a correct merged app pri via `makepri new /pr <stage-with-the-4
framework win-x64 PRIs> /cf priconfig.xml /of resources.pri /in uiss /o`
(861 KB). `makepri dump` confirms it embeds the theme resources as
**EmbeddedData** under `ms-resource://uiss/Files/Microsoft.UI.Xaml/Themes/
themeresources.xbf` — exactly the URI ms-appx resolves to for the unpackaged
"uiss" package. **But deploying it next to the exe does NOT change the crash:
minimal-pri, merged-pri, and no-pri all produce the identical generic E_FAIL.**
⇒ Decisive conclusion: **unpackaged WinUI 3 is not loading `<exedir>\
resources.pri`** in this hand-rolled CMake setup. So the fix is not just
*making* the right pri — it's getting MRT/WinUI to *load* it.

Next directions to try (in order):
1. Generate the pri via the **WindowsAppSDK MSBuild MrtCore targets** (build a
   tiny stub `.vcxproj` that `<Import>`s the SDK props/targets so it emits a
   correctly-registered `resources.pri` + whatever load wiring it adds), then
   mirror that wiring in CMake. This is the canonical, supported path.
2. Investigate whether MRT auto-load needs the pri's primary map name to equal
   the app's runtime identity (try `/in` = exe module name / AUMID), or whether
   an explicit `ResourceManager`/`ResourceContext` init is required at startup
   for unpackaged.
3. Consider giving `uiss` (and the Windows examples) a lightweight **MSIX
   packaging** path for the demo, where pri + framework resources resolve by
   construction.

## Update 2 — decisive: the gap is pri *loading*, and root namespace
- **Garbage-pri test:** replacing `resources.pri` with non-pri bytes yields the
  **identical** crash as no-pri and as the correct merged pri ⇒ the app is
  **not consuming `<exedir>\resources.pri` at all**. Generating the perfect pri
  is necessary but **not sufficient** — the *loading*/registration the
  WindowsAppSDK MSBuild MrtCore targets do at build time is the missing half.
- **Read the SDK targets** (`build/packages/Microsoft.Windows.SDK.BuildTools.MSIX/build/...MrtCore.PriGen.targets`):
  - `ProjectPriIndexName` defaults to `$(TargetName)` (the exe name).
  - For **unpackaged ("Centennial") apps the resources go in the ROOT
    namespace** — `PriIndexName` is set **empty** and `PrependPriInitialPath`
    is false, so resources resolve as `ms-resource:///Files/...` (root), not
    under an app-named map. My `/in uiss` filed them under `uiss/...`.
  - Tried `makepri new … /in ""` to force root namespace → **exit 4** (CLI
    rejects empty `/in`). Root namespace must be set via the **priconfig**
    (`<packaging>` / index `root`/`initialPath`), not the `/in` flag — OR by
    letting the MrtCore targets run.

## Recommended implementation (next session, fresh context)
Stop hand-rolling makepri. Replicate the SDK flow the supported way:
1. Author a tiny **MSBuild stub** `.vcxproj` (or `.proj`) that imports
   `Microsoft.WindowsAppSDK` + `Microsoft.Windows.SDK.BuildTools.MSIX` props/
   targets with `<WindowsPackageType>None</WindowsPackageType>` and the same
   package refs the CMake build resolves (under `MPAPP_PACKAGES_DIR`).
2. `msbuild` it once to emit a **known-good unpackaged `resources.pri`**;
   inspect its `makepri dump` (index name, root namespace) to learn the exact
   structure + the exact `makepri` command line the target logged.
3. Mirror that one `makepri` invocation as a CMake `add_custom_command` that
   deploys `resources.pri` next to every WinUI exe (extend
   `mpapp_add_winappsdk_runtime` in `cmake/WindowsAppSDK.cmake`).
4. Re-run `uiss.exe`; expect a rendered login window. Then drive
   login→nav→CarouselView and screenshot (a **Windows-MCP** server with
   Screenshot/Click/Type is now available in-session — use it, or computer-use).

## Update 3 — proved the platform works; isolated the gap to host resource-init
Built a minimal **.NET unpackaged WinUI 3 probe** (`<WindowsPackageType>None`,
a Window + TextBox + Button) via `dotnet build`:
- **It renders a TextBox fine** ⇒ unpackaged WinUI 3 + this WindowsAppRuntime
  (1.8 / 8000.859.21.0) works on this machine.
- Its app pri (`winuiprobe.pri`, 664 B) is **tiny / app-only — no framework
  themeresources**. So framework theme resources resolve at runtime from the
  framework package (via the bootstrap), NOT from the app pri.
- **Remove that pri → the probe crashes with the identical `0xC0000409`** as
  uiss ⇒ a (tiny, app-named) pri is *required*; its job is to initialise MRT.

Deploying the equivalent tiny pri (map `uiss`) next to `uiss.exe`, as both
`uiss.pri` and `resources.pri`, **still crashes**. So the difference between
the working .NET probe and `uiss.exe` is **not the pri** — it's how the
**C++/CMake host wires resource loading** vs the .NET WinUI host (which auto-
registers the app pri at startup). The VS C++ WinUI *project system* does this
at build time; our hand-rolled CMake build does not.

**Packaged attempt** (loose-registered MSIX, `AppxManifest.xml` w/
`Microsoft.WindowsAppRuntime.1.8` + `Microsoft.VCLibs` `<PackageDependency>`,
`runFullTrust`): activation works, identity detected, on_launch completes —
but themeresources **still** fails (tried tiny pri map=`MPAPP.UISS` and the
861 KB merged pri map=`MPAPP.UISS`). So packaging alone doesn't fix it either.

### Landed this session
- **`fix(windows)` (commit `3c6f7b0`):** `run_app_impl` now detects package
  identity (`GetCurrentPackageFullName`) and only runs the unpackaged
  `MddBootstrap*` when there is none — required for any future packaged
  deployment; unpackaged path unchanged (verified still reaches
  `Application::Start`).

### The remaining gap (for next session)
Replicate the **.NET / VS-C++ WinUI host's resource-context registration** in
the C++ app so `uiss.exe` actually loads its `resources.pri`. Concretely:
- Decompile/inspect what the .NET WinUI generated `Main` (or
  `Microsoft.WindowsAppSDK`'s C# host) does at startup to register the app pri
  — likely a `Microsoft.Windows.ApplicationModel.Resources.ResourceManager`
  init or an MRT `MrtResourceManager`/`ResourceContext` call — and call the
  C++/WinRT equivalent from `run_app_impl` *before* `Application::Start`.
- Then a **tiny app `resources.pri`** (map = exe identity, app-only, from the
  trivial makepri flow) deployed next to the exe should suffice (no framework
  merge needed — that was a wrong turn).
- Repro reference: `build/pristub/` (the working .NET probe) — diff its startup
  + deployed layout against uiss.

## OUTCOME — desktop + mobile demonstrated working end-to-end
Per the "working desktop and mobile app" goal, both platforms were driven live
and screenshotted (artifacts under `build/`):
- **Desktop — Linux/GTK4 (the actual УИСС app):** launches, renders the login
  screen (TU logo via real image loader, navy bold typography, Cyrillic), the
  **Вход button click logs in** (validation + navigation fire), the **10-section
  flyout nav** shows, the **Информация** page renders the full student record,
  and the **CarouselView pages** between announcements via the ◀ ▶ buttons
  (`scroll_to` → native GtkStack page switch). The GTK window surfaces on the
  Windows desktop via WSLg and was captured with `Graphics.CopyFromScreen`
  (bypasses the DComp/PrintWindow wall). WSLg blocks *synthetic keyboard* +
  *popup-menu clicks* (host limitation, not the app) — the faculty number was
  supplied via the app's real prefs feature; the demo's ЕГН prefill was a
  local, reverted tweak.
- **Mobile — Android emulator (`coroute_test`):** `io.mpapp.example` installed +
  launched; renders native widgets (BoxView, ShapeView ellipse, data-bound
  label, EditText, Switch, CheckBox, SeekBar, Button, expanders) — framework
  driving real `android.*` views on-device, captured via `adb screencap`.

**Windows (WinUI 3) remains the one platform not rendering** — root cause fully
established above (the Ninja/CMake build doesn't run the WindowsAppSDK MSBuild
MrtCore resource integration that the VS generator / .NET host do; a correct
pri is necessary but the C++ host never *loads* it). Fix path: switch the
Windows build to the CMake **Visual Studio generator** with WinUI vcxproj
integration (per fredemmott/cmake-cpp-winrt-winui3 + OpenKneeboard), OR
replicate the host's resource-context registration in C++. Tracked here.

The conditional-bootstrap fix (`3c6f7b0`) stands as a correct prerequisite.

## Status snapshot at handoff
- All exploratory edits **reverted**; `git status` clean (only obsidian/
  Home/README CRLF noise). The committed tree is unaffected by this debugging.
- The crash is **reproducible** from a clean build; root cause is **certain**.
- The fix is **infra** (resources.pri generation) — not yet implemented.

## Links
- [[T-0060-uiss-reference-app]] (the app), [[00_Index/Current Focus]].
- Diagnostic switch: `MPAPP_LOG_LAUNCH=<file>` in
  `src/handlers/windows/application_handler.cpp`.
