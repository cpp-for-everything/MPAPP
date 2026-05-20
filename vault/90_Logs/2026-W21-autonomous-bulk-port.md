---
type: log
date: 2026-05-20
tags:
  - type/log
  - phase/p3
  - phase/p4
  - phase/p5
---

# 2026-W21 — Autonomous Bulk Port (user napping)

User directive: implement all remaining components autonomously while they sleep, documenting in the vault.

Starting state: **19 components `android-real`** (Application, Window, StackLayout, Button, Label, Entry, Switch, CheckBox, RadioButton, Slider, Stepper, Editor, ScrollView, BoxView, Border, ActivityIndicator, ProgressBar, SearchBar — plus this session's already-shipped batches).

Target: cover as many of the remaining ~36 components as feasible in a single autonomous run. Stop with a clean handoff when context runs low.

## Strategy

- **Sequential rapid-fire** for simple widgets — write handlers (mock + 3 platforms), wire dispatch on 9 files, update CMakeLists on 3 files + tests/CMakeLists, build all 3 platforms, update Components/<Name>.md frontmatter + Controls Inventory row, commit.
- **Live-verify on Android** is selective: only when the widget has a meaningful visible state in the spike (most don't — adding 30 widgets to the spike would be absurd). Compile-verify across Win/Linux/Android is the proof-of-correctness bar for the rest.
- **Mock-first per Rule 6**: every not-started widget lands as `mpapp::<widget>` + `handlers/mock/<widget>_handler.hpp` + `tests/mock_handlers/<widget>_test.cpp` first, then real handlers in the same commit.
- **Skip live-verify but compile-verify**: marked `android-real (compile-verified)` in the inventory and component docs, distinguished from `android-real (live-verified)` for the ones I actually drove on the emulator earlier in the session.

## Progress

Each row is committed as a separate batch.

| # | Widget | Status | Commit | Notes |
|---|--------|--------|--------|-------|
| 1 | ScrollView (mock → real) | shipped | `07a31bc` | `bind_content(scroll_view&, view&)` helper for non-owning content; Android spike wraps full layout. Live-verified on emulator-5554. |
| 2 | BoxView (mock → real) | shipped | `365c1e3` | Win Border + SolidColorBrush + CornerRadius; Linux GtkBox + per-instance CSS provider; Android View + GradientDrawable. **Live-verified on emulator**: corner radius tied to slider, drag-to-5.0 visibly rounded from ~4px to ~20px. |
| 3 | Border (mock → real) | shipped | `4eeb87a` | Same color-parsing + RoundRectangle(N) descriptor shape parser shared in spirit across Win/Linux/Android. Stroke + corner-radius. Dash/cap/join surface preserved as Observable but not yet wired through real handlers — deferred. |
| 4 | ActivityIndicator (not-started → real) | shipped | `a9ab92d` | Win ProgressRing; Linux GtkSpinner (start/stop + CSS color); Android indeterminate ProgressBar (setIndeterminateTintList via ColorStateList). |
| 5 | ProgressBar (not-started → real) | shipped | `ef2b359` | Determinate; Win mux::ProgressBar (Value 0..1); Linux GtkProgressBar (gtk_progress_bar_set_fraction + CSS for `trough > progress`); Android determinate ProgressBar (Max=10000, setProgressTintList + setProgressBackgroundTintList). |
| 6 | SearchBar (not-started → real) | shipped | `1487e16` | Win AutoSuggestBox with Find QueryIcon; Linux GtkSearchEntry; Android SearchView (setIconified(false) + setQuery + setQueryHint). |
| 7 | Picker (not-started → real) | shipped | (pending build) | Win ComboBox + Items.Append; Linux GtkDropDown + GtkStringList with splice; Android Spinner + freshly-built ArrayAdapter<String> per items update (android.R.layout.simple_spinner_item = 0x01090008). Items recorded as count in mock (`items.count`) — std::format doesn't ship a formatter for `vector<string>` and the concept fallback didn't route cleanly on GCC 14. Title slot deferred on Linux/Android. |

## Caveats

- **Heavy widgets** (Shell, NavigationPage, ListView, WebView, HybridWebView, CollectionView if added) require lifecycle / navigation design that can't be rushed in a bulk port. If I run out of budget before them, they're flagged in the handoff section.
- **Abstract bases** (View, Layout, Element) don't need "real" handlers — they're type hierarchy. They stay at `mock` (which is the correct terminal state for them) and the inventory's snapshot description gets a note clarifying that.
- **Deprecated widgets** (Frame, deprecated in MAUI 9) get a thin handler for compat + a note that new code should use Border.

## Handoff

(Written at the end of the run with everything still pending.)
