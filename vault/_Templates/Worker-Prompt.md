# Worker Prompt Template — MPAPP widget port

This is the canonical agent brief for an isolated git-worktree worker porting a single widget per [[ADR-0013-data-driven-widget-dispatch]] + [[40_Roadmap/M-04b-handler-bulk-port]].

Pass this verbatim to `Agent` with `isolation: "worktree"` and `run_in_background: true` (or foreground if the wave is small). Fill in `{widget_name}`, `{display_name}`, and `{native_widgets}` per the table at the bottom of this template before sending.

---

## Prompt to paste into Agent.prompt

> **Task:** Add `mpapp::{widget_name}` to MPAPP, with real handlers on Windows (WinUI 3), Linux (GTK4), and Android (JNI). Mock handler + tests too. Don't touch any pre-existing source file beyond the two metadata files called out at the bottom.
>
> **Read these first**, in order:
> - `D:\GitHub\MPAPP\vault\CLAUDE.md` — the 12 rules.
> - `D:\GitHub\MPAPP\vault\20_ADRs\ADR-0013-data-driven-widget-dispatch.md` — the dispatch registry pattern your platform handler `.cpp` files must follow.
> - `D:\GitHub\MPAPP\include\mpapp\handlers\windows\widget_dispatch.hpp`, `linux/widget_dispatch.hpp`, `android/widget_dispatch.hpp` — the registry headers you'll register against.
> - **One reference widget** to copy patterns from. Pick one that already self-registers — if none does yet, use the existing `slider` handlers as the closest shape match and translate the registrar idiom from ADR-0013 yourself.
> - `D:\GitHub\MPAPP\vault\10_Architecture\Components\{display_name}.md` — your widget's component doc (already exists as a stub).
>
> **Files to write (all new, no edits to existing files):**
>
> 1. `include/mpapp/{widget_name}.hpp` — cross-platform header. Declares `mpapp::{widget_name} : public view` with Observable properties for the surface MAUI exposes. If you need a `<format>` formatter for any custom struct types, guard it behind `#if __has_include(<format>) && !defined(__ANDROID__)` — Android's NDK ships without `<format>`.
> 2. `include/mpapp/handlers/mock/{widget_name}_handler.hpp` — mock handler. Inherits `mock_handler_base`. One `map_<property>(widget&)` per Observable.
> 3. `tests/mock_handlers/{widget_name}_test.cpp` — 2–3 Catch2 tests of `bind initial values` + `change tracking`. Use existing `tests/mock_handlers/slider_test.cpp` or `box_view_test.cpp` as a template.
> 4. `include/mpapp/handlers/windows/{widget_name}_handler.hpp` + `src/handlers/windows/{widget_name}_handler.cpp` — wraps `{native_windows}`. End the `.cpp` with a self-registering registrar per ADR-0013.
> 5. `include/mpapp/handlers/linux/{widget_name}_handler.hpp` + `src/handlers/linux/{widget_name}_handler.cpp` — wraps `{native_linux}`. Same registrar pattern.
> 6. `include/mpapp/handlers/android/{widget_name}_handler.hpp` + `src/handlers/android/{widget_name}_handler.cpp` — wraps `{native_android}`. Same registrar pattern. Every JNI helper must start with `if (env->ExceptionCheck()) env->ExceptionClear();`. Hold the native object as a global ref; release in the destructor.
>
> **Two metadata edits allowed (and required):**
>
> - `vault/10_Architecture/Components/{display_name}.md` — change frontmatter: `mpappStatus: android-real`, set `platformWindows`/`platformAndroid`/`platformLinux` to `true`, change the `tags` from `status/not-started` (or `status/mock`) to `status/android-real`.
> - `vault/10_Architecture/Controls Inventory.md` — find the row for your widget and replace `not-started` (or `mock`) with `android-real (Win {native_windows} + Linux {native_linux} + Android {native_android}; <one-sentence behavior note>)`.
>
> **Do NOT edit any of these — they are coordinated centrally:**
>
> - `src/handlers/{android,linux,windows}/stack_layout_handler.cpp`
> - `src/handlers/{android,linux,windows}/window_handler.cpp`
> - `src/handlers/{android,linux,windows}/scroll_view_handler.cpp`
> - `src/handlers/{android,linux,windows}/border_handler.cpp`
> - `src/handlers/{android,linux,windows}/content_view_handler.cpp`
> - Any `CMakeLists.txt` — the example CMakeLists glob `src/handlers/<platform>/*.cpp` and `tests/mock_handlers/*_test.cpp` automatically per ADR-0013.
> - `examples/android_hello/app/src/main/cpp/native_main.cpp`
>
> **Registrar pattern** (Android example, adapt the type for Linux + Windows):
>
> ```cpp
> // At the bottom of src/handlers/android/{widget_name}_handler.cpp
> #include "mpapp/handlers/android/widget_dispatch.hpp"
>
> namespace {
>     jobject dispatch_{widget_name}(::mpapp::view* v) {
>         if (auto* w = dynamic_cast<::mpapp::{widget_name}*>(v); w && w->has_handler()) {
>             return w->handler().native();
>         }
>         return nullptr;
>     }
>     struct registrar {
>         registrar() {
>             ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_{widget_name});
>         }
>     };
>     [[maybe_unused]] registrar _reg;
> } // namespace
> ```
>
> Linux version uses `GtkWidget*` + `::mpapp::detail::linux_dispatch::register_dispatcher`. Windows version uses `::winrt::Microsoft::UI::Xaml::UIElement` + `::mpapp::detail::windows_dispatch::register_dispatcher`.
>
> **macOS / iOS** — write the .mm files only if a Mac host is currently in play; otherwise skip. The expected naming is `src/handlers/macos/{widget_name}_handler.mm` + `src/handlers/ios/{widget_name}_handler.mm`. They follow the same registrar pattern with `NSView*` / `UIView*` return types.
>
> **Verification:**
>
> - Windows: `D:\GitHub\MPAPP\_build_full.bat` from a Developer Command Prompt. Must reach `=== SUCCESS ===` with no test regressions.
> - Linux: `wsl -d Ubuntu-24.04 -- bash -lc 'cd /mnt/d/GitHub/MPAPP && cmake --build build-linux'`. Must reach `Linking CXX executable examples/gtk4_hello/gtk4_hello`.
> - Android: `D:\GitHub\MPAPP\_build_android.bat`. Must reach `BUILD SUCCESSFUL`.
> - All three must be green before you commit.
>
> **Commit:**
>
> - One commit on a worktree branch named `bulk/widget/{widget_name}` with message: `feat(M-04b): {widget_name} handlers (3-of-5 platforms real) via ADR-0013 registry`.
> - Don't push. The coordinator merges branches.
>
> **End your turn with a single status line** like: `READY: {widget_name} on branch bulk/widget/{widget_name}; Win 143/143, Linux green, Android green.`
>
> If you hit a non-trivial blocker (e.g. native widget doesn't exist on one platform, or the surface requires a design ADR), end with `BLOCKED: {widget_name} — <one-line reason>` instead of committing partial work.

---

## Fill-ins by widget

For each pending widget, the table below maps `{widget_name}` → `{native_windows}` / `{native_linux}` / `{native_android}` so an orchestrator can dispatch waves without re-researching each native primitive.

| `{widget_name}` | `{display_name}` | `{native_windows}` | `{native_linux}` | `{native_android}` |
|---|---|---|---|---|
| `title_bar` | TitleBar | `mux::Controls::TitleBar` (WinUI 3 1.5+) | `GtkHeaderBar` | `androidx.appcompat.widget.Toolbar` (as titlebar) |
| `toolbar` | Toolbar | `mux::Controls::CommandBar` | `GtkActionBar` | `androidx.appcompat.widget.Toolbar` |
| `indicator_view` | IndicatorView | hand-rolled dots (StackPanel + Ellipses) | GtkBox + dot widgets | RadioGroup styled as dots |
| `refresh_view` | RefreshView | `mux::Controls::RefreshContainer` | manual gesture overlay | `androidx.swiperefreshlayout.SwipeRefreshLayout` |
| `flyout_view` | FlyoutView | `mux::Controls::NavigationView` | `GtkPaned` (drawer) | `androidx.drawerlayout.widget.DrawerLayout` |
| `tabbed_view` | TabbedView | `mux::Controls::TabView` | `GtkNotebook` | `TabLayout` + `ViewPager2` |
| `content_page` | ContentPage | (subclass of Page) | (subclass of Page) | (subclass of Page) |
| `templated_view` | TemplatedView | `mux::Controls::ContentControl` with ControlTemplate | `GtkBin`-style wrapper | `FrameLayout` |
| `menu_bar` | MenuBar | `mux::Controls::MenuBar` | `GtkPopoverMenuBar` + `GMenuModel` | `androidx.appcompat.widget.Toolbar` (as menu host) |
| `menu_bar_item` | MenuBarItem | `mux::Controls::MenuBarItem` | (child of GtkPopoverMenuBar) | (child of menu host) |
| `menu_flyout` | MenuFlyout | `mux::Controls::MenuFlyout` | `GtkPopoverMenu` | `androidx.appcompat.widget.PopupMenu` |
| `menu_flyout_item` | MenuFlyoutItem | `mux::Controls::MenuFlyoutItem` | (child of GtkPopoverMenu) | (child of PopupMenu) |
| `menu_flyout_separator` | MenuFlyoutSeparator | `mux::Controls::MenuFlyoutSeparator` | (child) | (child) |
| `menu_flyout_sub_item` | MenuFlyoutSubItem | `mux::Controls::MenuFlyoutSubItem` | (child) | (child) |
| `swipe_view` | SwipeView | `mux::Controls::SwipeControl` | gesture-based composition over `GtkBox` | `androidx.viewpager2`-wrapped FrameLayout |
| `swipe_item_view` | SwipeItemView | (child of SwipeControl) | (child) | (child) |
| `swipe_item_menu_item` | SwipeItemMenuItem | (child of SwipeControl) | (child) | (child) |
| `page` | Page | `mux::Controls::Page` | `GtkBox` (single-child) | `FrameLayout` |
| `bindable_layout` | BindableLayout | (attached property → child generator) | (attached property → child generator) | (attached property → child generator) |
| `frame` | Frame | `mux::Controls::Border` (deprecated alias) | `GtkBox` + CSS | `FrameLayout` + GradientDrawable |

(Add rows as M-04b absorbs more widgets. The M-04c table in [[M-04b-handler-bulk-port]] lists what's deferred.)

## Tracker integration

When a worker reports `READY:`, append a row to the tracker in [[40_Roadmap/M-04b-handler-bulk-port|the M-04b milestone]]:

| Widget | Branch | Worker status | Merge status |
|---|---|---|---|
| {widget_name} | `bulk/widget/{widget_name}` | done | pending merge |

After the coordinator runs `git merge --no-ff bulk/widget/{widget_name}` from main and the build is verified green, flip merge status to `merged`.
