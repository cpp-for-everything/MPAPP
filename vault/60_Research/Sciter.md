---
type: research
subject: "Sciter"
framework: "sciter"
created: 2026-05-12
applicableTo: []
recommendation: reject
tags:
  - type/research
  - framework/sciter
---

# Sciter

> [!info] Status
> Rejected. Wrong audience and wrong rendering model for MPAPP.

## Summary

Sciter is an embeddable UI engine for desktop applications. A native C/C++ host application embeds the Sciter runtime and authors its UI in HTML, CSS, and a JavaScript-like scripting language called TIScript (or, in newer versions, a subset of standard JS). Sciter is studied here because it occupies the "embedded HTML engine" slot in the design space — a slot MPAPP explicitly rejects in favor of [[XAML Compatibility|XAML]] and [[Native-View|native views]].

## What They Do

Sciter ships a small (≈ 6–8 MB) self-contained runtime that renders an HTML/CSS document model into a window. The host C++ code creates a Sciter window, loads HTML, and exposes native functions to the scripting layer. Sciter draws everything itself with its own renderer (`Skia` in recent versions); it does not host system-native controls. Layout is CSS flexbox/grid; styling is CSS. Bindings between native data and DOM are achieved through script callbacks and a "behaviour" system that attaches C++ logic to CSS selectors.

## Strengths

- **Small footprint.** Single DLL/`.so`/`.dylib`; substantially smaller than embedding a full Chromium (CEF, WebView2, Electron).
- **Familiar markup.** Web developers can be productive immediately. HTML/CSS skills transfer.
- **Mature.** Around since the late 2000s; used by some commercial products (notably some antivirus, music, and trading UIs).
- **Decoupled from the OS browser.** Unlike WebView2 or WKWebView, Sciter ships its own renderer — no surprise updates from the OS.
- **Reasonable theming flexibility.** CSS variables, custom properties, and scripting allow per-app skins.

## Weaknesses

- **Wrong audience for MPAPP.** MPAPP's target user is a C++ developer who wants to write **[[XAML Compatibility|XAML]] + C++**, mirroring the [[ADR-0004-maui-xaml-superset-compat|MAUI surface]]. Sciter's audience writes HTML/CSS/JS. The two communities barely overlap.
- **Not native look-and-feel.** Sciter draws every widget itself. A Sciter "button" is not an `android.widget.Button`, not a Win32 `BUTTON`, not an `NSButton`, not a `UIButton`. Achieving platform-correct visuals (Material You, Fluent, AppKit vibrancy, iOS dynamic type) would require painstaking re-implementation in CSS — and would always be one OS release behind. This violates the spirit of [[ADR-0006-interop-parity]] and the [[Handlers|handler model]].
- **Accessibility is a perpetual gap.** Custom-rendered DOM does not map cleanly to UIA / AT-SPI / NSAccessibility / TalkBack. Sciter has improved here but cannot match what a real native widget tree gives for free.
- **License posture.** Sciter is **commercial / dual-licensed** (Sciter.JS has an MIT-licensed core, but the production-grade Sciter SDK has historically required per-developer or per-distribution fees). Per [[RFC-0001-licensing-and-patent-strategy]] and Rule 9, this is a non-starter for MPAPP's runtime dependency posture.
- **Closed ecosystem.** Limited third-party tooling. No XAML interop, no MAUI-handler equivalence, no path to [[Hot-Reload|hot reload]] of `.xaml` files.
- **Scripting language drift.** Long history of changing the embedded scripting language (TIScript → JS) creates migration risk.
- **No path to mobile parity.** While Sciter has experimental mobile support, the story on iOS and Android is far thinner than its desktop story — MPAPP requires first-class parity per [[Interop Parity]].

## Applicable to MPAPP

- **Explicitly avoid the embedded-HTML pattern.** MPAPP authors UI in XAML compiled by the [[XAML-Compiler|XAML compiler]] into C++ — not in HTML rendered by an embedded engine.
- **Explicitly avoid custom-drawn widget trees** as the primary rendering strategy. MPAPP delegates to platform [[Native-View|native views]] via [[Handlers|handlers]] for behavior, look, and accessibility.
- **Take note of the license trap.** Sciter is a cautionary example of a UI framework whose commercial license would block adoption regardless of technical fit.
- **No reusable patterns identified.** CSS-as-style and JS-as-glue are both deliberately outside MPAPP's design space.

> [!important] Recommendation
> `reject`. Wrong audience (HTML/CSS authors, not C++/XAML), wrong rendering model (custom-drawn, not native), and incompatible license posture per Rule 9.

## References

- Official: https://sciter.com/
- [[ADR-0004-maui-xaml-superset-compat]]
- [[ADR-0006-interop-parity]]
- [[RFC-0001-licensing-and-patent-strategy]]
- [[XAML Compatibility]]
