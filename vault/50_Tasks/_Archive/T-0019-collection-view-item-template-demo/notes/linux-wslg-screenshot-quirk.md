# Linux WSLg screenshot quirk (T-0019 — same pattern as T-0017 / T-0018)

The GTK4 item_template demo (`gtk4_item_template_demo`) builds clean
and runs on the Linux side (`pgrep -af gtk4_item_template_demo` finds
the running process). However the Windows-side `msrdc.exe` proxy
that mediates WSLg windows did not present a fresh window for this
demo's X11 surface — `EnumWindows` finds five visible msrdc top-
level windows, all of which proxy stale `MPAPP T-0018 - Async Bridge
Demo (GTK4) (Ubuntu-24.04)` content from earlier in the session (and
none of which contain the T-0019 list rendering). Even with
`PrintWindow(..., PW_RENDERFULLCONTENT=2)` against each msrdc
window the result is the T-0018 demo, not T-0019.

This is the same compositor-cache behavior documented for the
earlier two catch-up demos:

- [[../../T-0017-typed-routing-demo/notes/linux-wslg-screenshot-quirk]]
- [[../../T-0018-async-bridge-dispatch-demo/notes/linux-wslg-screenshot-quirk]]

Even attempting `wsl --shutdown` (T-0018's mitigation idea) did not
clear it — `msrdc.exe` is a Windows-side process that survives the
WSL VM shutdown, and it retained the cached window titles + the
cached frame buffers from the previous launches. (We did not kill
msrdc processes by hand because the user has active WSLg sessions
in this Windows session, and force-killing msrdc has UX
consequences beyond this task's scope.)

**Evidence for the Linux platform in this task therefore relies on:**

- The demo source (`examples/gtk4_item_template_demo/main.cpp`)
  which proves the typed `collection_view::item_template` factory
  + `materialized_changed` signal + `mpapp-handlers-linux`
  `collection_view_handler::map_typed_items` wiring (which subscribes
  to `materialized_changed`) compile and link on the Linux toolchain.
- `tests/mock_handlers/collection_view_test.cpp` covers the
  item_template surface model — `item_template materializes a cell
  per items_source row`, `re-materializes when items_source changes`,
  `re-materializes when template changes`, `factory receives the row
  index`, `materialized_changed fires on rematerialize`, and `doesn't
  override typed_items on the surface`.
- `src/handlers/linux/collection_view_handler.cpp` line 207
  consumes `bound_->materialized_views()` from `rebuild_active()`
  and `map_typed_items()` line 278 subscribes to
  `cv.materialized_changed` — same code path the demo exercises.
- The Windows + Android live captures in the same `screenshots/`
  dir.
