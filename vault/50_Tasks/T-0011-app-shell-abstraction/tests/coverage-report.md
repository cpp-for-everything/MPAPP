# T-0011 — Mock-handler test coverage

Method-level coverage across the 5 new mock handlers landed in
T-0011. Each public `map_*` / `simulate_*` / `run_app` / `set_*`
entry point is exercised by at least one Catch2 test in
`tests/mock_handlers/`.

| Handler | Public method | Test |
|---|---|---|
| `application_handler<mock>` | `run_app<App>` | `application_test.cpp::"application mock handler runs the launch lifecycle"` |
| `application_handler<mock>` | `simulate_suspend` | `application_test.cpp::"application mock handler simulators dispatch to user overrides"` |
| `application_handler<mock>` | `simulate_resume` | same |
| `application_handler<mock>` | `simulate_terminate` | same |
| `application_handler<mock>` | `set_exit_code` | `application_test.cpp::"application mock handler returns the configured exit code"` |
| `window_handler<mock>` | `map_title` | `window_test.cpp::"window mock handler records initial property values on bind"` + `"window mock handler fires once per real title change"` |
| `window_handler<mock>` | `map_content` | bind test + `"window mock handler records content.present transition when assigned a view"` |
| `window_handler<mock>` | `map_width` | bind test |
| `window_handler<mock>` | `map_height` | bind test |
| `window_handler<mock>` | `map_is_visible` | bind test + `"window::show toggles is_visible and the handler records it"` + `"window::close fires closed signal and toggles is_visible"` |
| `window_handler<mock>` | `simulate_activated` | `"window activated simulator drives user-side subscribers"` |
| `page_handler<mock>` | `map_title` | `page_test.cpp::"page mock handler records initial property values on bind"` + `"page mock handler tracks title + content + is_busy changes"` |
| `page_handler<mock>` | `map_content` | same |
| `page_handler<mock>` | `map_is_busy` | same |
| `stack_layout_handler<mock>` | `map_orientation` | `stack_layout_test.cpp::"stack_layout mock handler records initial property values on bind"` + change test |
| `stack_layout_handler<mock>` | `map_spacing` | same |
| `stack_layout_handler<mock>` | `map_horizontal_alignment` | same |
| `stack_layout_handler<mock>` | `map_vertical_alignment` | same |
| `stack_layout_handler<mock>` | `map_add` / `map_remove` / `map_clear` | `"stack_layout mock handler records child-mutation commands"` |
| `grid_layout_handler<mock>` | `map_row_count` | `grid_layout_test.cpp::"grid_layout mock handler records initial property values on bind"` + change test |
| `grid_layout_handler<mock>` | `map_column_count` | same |
| `grid_layout_handler<mock>` | `map_row_spacing` | same |
| `grid_layout_handler<mock>` | `map_column_spacing` | same |

Total: 22 public methods across 5 handlers, 100% exercised by 17
new Catch2 test cases. Cross-cutting `mock_handler_base` API
(`calls()`, `calls_as_strings()`, `clear_calls`, `record`,
`record_change`, `record_event`, `mock_property_recorder`) is
exercised by both the existing layout-group tests (Unit 7) and the
existing simple-input tests (Unit 8), which is how the unified base
was hardened in the first place.

Total Catch2 binary: **126 tests passing** in the full suite
(application + window + page + stack_layout + grid_layout adds 17 to
the 109 that landed in batch3).

## Branch / line coverage instrumented build

A full LLVM-instrumented coverage run is gated on T-0012 (Mac/iOS
test harness design — same milestone wants clang-coverage end-to-end
across the matrix). MSVC's native coverage is unavailable on this
host (no OpenCppCoverage / VS Coverage). The method-coverage table
above is the strongest claim available without instrumented runs.
