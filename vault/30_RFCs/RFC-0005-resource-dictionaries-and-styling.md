---
type: rfc
id: RFC-0005
title: Resource dictionaries + styling — keyed value store + per-target-type property bundles
status: accepted
author: Alex Tsvetanov
created: 2026-05-25
area: markup
relatedADRs:
  - ADR-0004
  - ADR-0008
  - ADR-0024
tags:
  - type/rfc
  - status/accepted
  - area/markup
  - area/widgets
---

# RFC-0005 — Resource Dictionaries + Styling

> [!info] Status
> **accepted** — mock surface shipped under [[T-0044-resource-dictionary-styling-mock]]; XAML lowering of `{StaticResource}` / `<Style TargetType=…>` and implicit-style scanning deferred per the open-questions section.

## Problem

MAUI's `ResourceDictionary` + `Style` are foundational for theming, deduplication, and any non-trivial UI. They unlock:

- `<Color x:Key="BrandPrimary">#264653</Color>` reused everywhere via `{StaticResource BrandPrimary}`.
- `<Style TargetType="Button">` applied implicitly to every Button, or explicitly via `Style="{StaticResource MyButton}"`.
- Merged dictionaries for theme composition (`<MergedDictionaries>...`).
- Resource lookup that walks the visual tree (`Element.Resources` → `Window.Resources` → `App.Resources` → merged) — same algorithm MAUI uses.

MPAPP has none of this. Every example hard-codes colors/strings. Style sharing is impossible. The `resource_image_source` from [[RFC-0004-image-source-family]] hits the same wall — there's nowhere to look the resource name up.

## Proposal

Land two coupled types under MPAPP's existing surface conventions:

1. **`mpapp::resource_dictionary`** — keyed `std::any` value store with merged-dictionary composition, change signals, and a hierarchical lookup walker that traverses the visual tree's `resources` member. Counterpart to MAUI's `ResourceDictionary`.

2. **`mpapp::style`** — a TargetType-tagged bundle of property setters (`std::function<void(view&)>` setters keyed on a property name). Counterpart to MAUI's `Style`. Apps reference styles by key from a resource dictionary.

Per [[ADR-0008-mock-first-implementation]] this RFC ships the C++ surface + a mock that records lookups; XAML compilation of `{StaticResource}` / `<Style TargetType=...>` is a follow-up `mpapp-xc` task.

Resource dictionaries are NOT [[Wrapper-Component]]s — they own no native widget, they're configuration. Same call as gestures + image sources.

## Detailed Design

### File layout

```
include/mpapp/resources/
    resource_dictionary.hpp     ← mpapp::resource_dictionary
    style.hpp                   ← mpapp::style
    static_resource.hpp         ← StaticResource lookup helper (find_in)
include/mpapp/handlers/mock/
    resource_recorder.hpp       ← records lookups for assertion
```

### Surface — resource_dictionary

```cpp
namespace mpapp {

class resource_dictionary {
public:
    resource_dictionary() = default;

    // Type-erased store. `std::any` matches MAUI's
    // IDictionary<string, object>. Callers retrieve typed values via
    // `try_get<T>` or via the `static_resource` helper.
    void put(std::string key, std::any value);
    void remove(const std::string& key);

    [[nodiscard]] bool             has(const std::string& key) const noexcept;
    [[nodiscard]] const std::any*  try_get_local(const std::string& key) const noexcept;

    template <class T>
    [[nodiscard]] std::optional<T> try_get(const std::string& key) const {
        if (const auto* v = try_get_local(key)) {
            if (const T* casted = std::any_cast<T>(v)) return *casted;
        }
        for (const auto& md : merged_dictionaries) {
            if (auto v = md->try_get<T>(key)) return v;
        }
        return std::nullopt;
    }

    // Composition: lookups walk merged dictionaries AFTER the local
    // store (matches MAUI's resolution order). Mutating
    // `merged_dictionaries` does NOT auto-refire `changed` on every
    // key — apps that need to observe theme swaps should listen to
    // `composition_changed`.
    std::vector<std::shared_ptr<resource_dictionary>> merged_dictionaries{};

    // Per-key change notification. Fires on put / remove (with empty
    // any for remove). Subscribers route the value to their property.
    struct change {
        std::string_view key;
        const std::any*  new_value;   // nullptr for remove
    };
    mpapp::signal<const change&> changed{};

    // Composition-level signal: fires when `merged_dictionaries` is
    // mutated. Subscribers re-resolve every active lookup.
    mpapp::signal<>              composition_changed{};
};

} // namespace mpapp
```

### Hierarchical lookup walker

```cpp
// include/mpapp/resources/static_resource.hpp
namespace mpapp {

// Walk: this_view.resources → parent.resources → grandparent.resources
// → ... → app.resources. Returns the first match.
template <class T>
[[nodiscard]] std::optional<T> find_in(const view& v, const std::string& key);

} // namespace mpapp
```

Mock layer: every `view` gains a `std::shared_ptr<resource_dictionary> resources` member. `find_in` walks `v.resources → v.parent()->resources → ...`. The parent-pointer infrastructure already exists for layout walks.

### Surface — style

```cpp
namespace mpapp {

// A bundle of property setters scoped to a TargetType. When applied
// to a view, every setter runs in turn. Setters are stored by
// property name so they can be looked up + overridden in derived
// styles (MAUI's `BasedOn`).
class style {
public:
    // `target_type_name` is a static class-name string used for
    // implicit-style matching (MAUI's TargetType match). Concrete
    // controls publish their name via a static method `class_name()`.
    explicit style(std::string target_type_name)
        : target_type{std::move(target_type_name)} {}

    std::string                                          target_type;

    // Setter is type-erased so a style can carry setters for any
    // observable property: text, color, padding, ...
    std::unordered_map<std::string,
                       std::function<void(view&)>>       setters{};

    // Style inheritance — applies `based_on`'s setters first, then this
    // style's, so derived setters override.
    std::shared_ptr<style>                               based_on{};

    // Apply every setter (after based_on) to `v`. Runs the based_on
    // chain depth-first. Setters that throw are caught + logged so a
    // bad style doesn't take down the whole app.
    void apply_to(view& v) const;
};

} // namespace mpapp
```

### Mock recorder

```cpp
// include/mpapp/handlers/mock/resource_recorder.hpp
namespace mpapp {

// Wraps a resource_dictionary + records every lookup result so
// tests can assert exactly which keys were resolved + in what
// order. Used in mock tests that exercise resource-driven UI without
// a real binding stack.
class resource_recorder : public mock_handler_base {
public:
    explicit resource_recorder(resource_dictionary& d) : dict_{&d} {}

    template <class T>
    std::optional<T> try_get(const std::string& key) {
        auto result = dict_->try_get<T>(key);
        record_change(result.has_value() ? "lookup.hit" : "lookup.miss", key);
        return result;
    }
private:
    resource_dictionary* dict_;
};

} // namespace mpapp
```

### `view::resources`

Add `std::shared_ptr<resource_dictionary> resources` to `mpapp::view`. Optional — null means "no local resources, walk parent".

### Tests (mock-first)

```cpp
TEST_CASE("resource_dictionary stores + retrieves typed values",
          "[mock][resources]") {
    mpapp::resource_dictionary d;
    d.put("BrandPrimary", std::string{"#264653"});
    d.put("DefaultPadding", 16.0);

    CHECK(d.try_get<std::string>("BrandPrimary") == "#264653");
    CHECK(d.try_get<double>("DefaultPadding")    == 16.0);
    CHECK(!d.try_get<double>("BrandPrimary").has_value());   // wrong type
    CHECK(!d.try_get<std::string>("Missing").has_value());
}
```

## Alternatives

- **`std::variant<color, double, std::string, ...>`** for the value type. Rejected — closed-set limits user types; MAUI's `object` matches `std::any` semantically.
- **Strong typing per key** via templated dictionaries (`typed_resource_dictionary<key, T>`). Rejected — XAML compatibility demands the `<key, object>` shape so the compiler can lower `{StaticResource Anything}` uniformly.
- **Visual-tree-free lookup** (one global registry). Rejected — MAUI's hierarchical lookup is what enables per-page theme overrides.
- **Eager style application** in the wrapper ctor. Deferred — apps may want to opt out per instance. Style application is explicit via `style.apply_to(view)` at v1.

## Open Questions

> [!todo] Open
> - [ ] StaticResource extension XAML lowering — should land alongside `<Style>` element parsing in mpapp-xc (M-09).
> - [ ] Implicit styles (TargetType match without explicit key) — needs a registry on the resource_dictionary. Lean v2.
> - [ ] Dynamic resources (re-resolve on dictionary change) — adds binding-layer complexity; defer to a Bindings RFC.
> - [ ] Theme swap (light ↔ dark) — modelled as swapping a merged dictionary. Document the recipe once the Window surface exposes `application_resources`.

## Migration / Compatibility

- New `view::resources` member is `std::shared_ptr<resource_dictionary>` defaulting to nullptr — every existing concrete component inherits it without code change.
- No existing surface is modified.

## References

- [[ADR-0004-maui-xaml-superset-compat]] — XAML must compile 1:1, so the resource-dictionary keys + lookup semantics must match MAUI.
- [[ADR-0008-mock-first-implementation]].
- [[RFC-0004-image-source-family]] — `resource_image_source` relies on this RFC's resource lookup.
- `references/maui/src/Controls/src/Core/ResourceDictionary.cs`, `Style.cs`.
