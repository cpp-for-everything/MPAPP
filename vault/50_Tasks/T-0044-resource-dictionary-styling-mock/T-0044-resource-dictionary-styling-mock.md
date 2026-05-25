---
type: task
id: T-0044
title: RFC-0005 Resource Dictionary + Styling — mock surface
status: completed
milestone: M-04c
owner: ""
area: markup
blockedBy: []
coveragePercent: 100
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/done
  - area/markup
  - area/widgets
  - area/handlers
  - phase/p2
---

# T-0044 — Resource Dictionary + Styling mock surface

## Goal

Land the mock-first slice of [[RFC-0005-resource-dictionaries-and-styling]]: the keyed `std::any` `resource_dictionary` with merged-dictionary composition + signals, the TargetType-tagged `style` with setter bundles + `based_on` inheritance, the `find_in<T>` hierarchical visual-tree walker, and the mock-only `resource_recorder` for assertions. Wires `view::parent()` so the walker has a chain to follow + `view::resources` so each view can carry a local dictionary.

XAML compilation of `{StaticResource}` and `<Style TargetType=…>` is deferred to mpapp-xc (M-09); per-platform handler integration for "real" style application is a follow-up — the mock surface is fully usable in app code today via explicit `style.apply_to(view)` calls.

## Scope

In:

- `include/mpapp/resources/resource_dictionary.hpp` — `mpapp::resource_dictionary`:
  - `put(std::string, std::any)` / `remove(const std::string&)` / `has(...)`.
  - `try_get_local(const std::string&) const -> const std::any*`.
  - `template<class T> try_get(const std::string&) const -> std::optional<T>` with merged-dictionary fallback in iteration order.
  - `merged_dictionaries` vector + `add_merged_dictionary` / `clear_merged_dictionaries` helpers that raise `composition_changed`.
  - `change` struct (`std::string_view key`, `const std::any* new_value`) + `signal<const change&> changed` + `signal<> composition_changed`.
- `include/mpapp/resources/style.hpp` — `mpapp::style`:
  - `target_type` (string), `setters` (`unordered_map<string, function<void(view&)>>`), `based_on` (shared_ptr<style>).
  - `apply_to(view&)` — runs based_on first, then own setters; swallows setter exceptions per RFC.
- `include/mpapp/resources/static_resource.hpp` — `template<class T> find_in<T>(const view&, const std::string&) -> std::optional<T>` walking `v.resources → v.parent()->resources → ... → root.resources`.
- `include/mpapp/handlers/mock/resource_recorder.hpp` — records `put=<key>`, `remove=<key>`, `composition_changed`, `lookup.hit=<key>`, `lookup.miss=<key>`.
- `include/mpapp/view.hpp` — adds `std::shared_ptr<resource_dictionary> resources{}`, `parent_` private member, `parent()` + `set_parent(view*)` accessors. Forward-declares `class resource_dictionary` to avoid pulling `<any>` into every translation unit.
- `include/mpapp/layout.hpp` — `add` / `insert` / `remove` / `clear` maintain the child's `parent_` so the walker has a consistent up-link.
- `tests/mock_handlers/resource_dictionary_test.cpp` — 12 test cases, 40 assertions covering: typed get/put, type-mismatch handling, override + changed signal, remove semantics, merged-dict resolution + ordering, composition_changed, view-tree walker behaviour including null-resources, nearest-ancestor wins on dup keys, parent-pointer detach on remove/clear, style apply, based_on ordering, exception swallowing, recorder hit/miss reporting.

Out (follow-up):

- XAML lowering for `<ResourceDictionary>` literals + `{StaticResource}` / `{DynamicResource}` / `<Style TargetType>` — captured by mpapp-xc M-09 work in the `RFC §Open` list.
- Implicit-style scanning (TargetType match without explicit key) — Lean v2 per the RFC.
- Dynamic resources re-resolving on dictionary change — needs a Bindings RFC.
- Theme swap recipe (light ↔ dark) — documentation task once the Window surface exposes `application_resources`.

## Acceptance Criteria

- [x] `resource_dictionary` stores typed values via `std::any`, walks `merged_dictionaries` in iteration order, fires per-key `changed` on put/remove + `composition_changed` on merge-vector mutation.
- [x] `style` runs `based_on` setters first, then own setters; setter exceptions don't propagate.
- [x] `find_in<T>(view, key)` returns the nearest-ancestor typed match; type mismatch in one dictionary falls through; missing chain returns `nullopt`.
- [x] `view::parent()` returns nullptr by default; `layout::add` / `insert` set it; `layout::remove` / `clear` detach it.
- [x] `resource_recorder` produces the expected `lookup.hit` / `lookup.miss` / `put` / `remove` / `composition_changed` log entries.
- [x] `tests/mock_handlers/resource_dictionary_test.cpp` covers all of the above. 12 test cases, 40 assertions.
- [x] `ctest` is green: 383 → 395 tests pass on Linux WSL.
- [x] Mock test target still links `libmpapp-core.a` + Catch2 only — adds no platform-handler dependency.

## Build evidence

```
$ ninja -C build-wsl mock_handlers_test
[72/72] Linking CXX executable tests/mock_handlers_test

$ ./build-wsl/tests/mock_handlers_test '[resources]'
All tests passed (40 assertions in 12 test cases)

$ ctest --test-dir build-wsl
100% tests passed, 0 tests failed out of 395
Total Test time (real) =  11.91 sec
```

## Links

- RFC: [[RFC-0005-resource-dictionaries-and-styling]].
- Sibling family-RFC precedent: [[RFC-0004-image-source-family]] + [[T-0042-image-source-family-mock]].
- Pattern: ADR-0008 (mock-first), ADR-0024 (these are NOT wrapper-components — they own no native widget, same call as gestures + image sources).
- Affected components: foundational — every existing control becomes resource-aware via the new `view::resources` member.
- Follow-ups (to be opened):
  - mpapp-xc XAML lowering for `<ResourceDictionary>` / `{StaticResource}` / `<Style TargetType=…>` (will live under the M-09 task tree).
  - Per-platform style application (currently app code calls `style.apply_to(view)` explicitly — eager application via a `styled_view` mixin is deferred).
  - Implicit-style registry + lookup (TargetType match without explicit key).
  - Dynamic resource re-resolution (needs Bindings RFC).
