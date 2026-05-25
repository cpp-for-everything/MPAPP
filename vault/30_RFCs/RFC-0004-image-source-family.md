---
type: rfc
id: RFC-0004
title: ImageSource family — file / uri / stream / font / resource sources + load pipeline
status: draft
author: Alex Tsvetanov
created: 2026-05-25
area: handlers
relatedADRs:
  - ADR-0006
  - ADR-0008
  - ADR-0024
tags:
  - type/rfc
  - status/draft
  - area/handlers
  - area/widgets
---

# RFC-0004 — ImageSource family

> [!info] Status
> **draft** — under discussion.

## Problem

`mpapp::image::source` is currently `Observable<std::string>` (a file path). That covers the trivial "load this PNG from disk" case but matches none of MAUI's `ImageSource` family:

| MAUI type | What it loads |
|---|---|
| `FileImageSource` | local file path |
| `UriImageSource` | remote URL with caching + cache validity |
| `StreamImageSource` | a `Stream` factory (async) |
| `FontImageSource` | a single glyph from a font, tinted |
| `ResourceImageSource` (via `ImageSource.FromResource`) | embedded resource in an assembly |

Without this family MPAPP can't express any of:

- A `ListView` cell that pulls its avatar from a remote URL (UriImageSource is the MAUI idiom).
- An icon button whose glyph is a font character (FontImageSource — every MAUI app uses this for material/lucide icons).
- An image built from in-memory bytes (StreamImageSource — image uploaders, generated content).
- A bundled-resource fallback (ResourceImageSource).

This blocks `Image`, `ImageButton`, `ImageCell`, and any `Button` / `MenuFlyoutItem` that wants an icon — a meaningful subset of every shipping MAUI app.

## Proposal

Introduce a polymorphic `ImageSource` family matching MAUI's surface 1:1, but using value-typed `std::shared_ptr<basic_image_source>` (not C# inheritance) so the same source can be referenced by multiple consumers without ownership ambiguity. Each concrete source carries the configuration the loader needs; the **per-platform loader** is a separate task per platform that turns a source into a native bitmap handle.

Per [[ADR-0008-mock-first-implementation]] this RFC lands ONLY the source-family surface + a mock loader. Real per-platform decoders (GDK-Pixbuf on Linux, BitmapImage on WinUI, BitmapFactory on Android, NSImage on macOS, UIImage on iOS) are subsequent tasks.

The wrapper-pattern from [[ADR-0024]] does NOT apply to sources for the same reason it didn't apply to gesture recognizers ([[RFC-0003-gesture-recognizers]]): sources own no native widget; they're configuration consumed by a loader attached to a view's handler.

## Detailed Design

### Inheritance + namespacing

```
mpapp::internal::basic_image_source            ← abstract base (kind() virtual)
        ▲
        ├── mpapp::file_image_source           ← local file path
        ├── mpapp::uri_image_source            ← remote URL + cache config
        ├── mpapp::stream_image_source         ← byte-stream factory (sync mock; async real)
        ├── mpapp::font_image_source           ← glyph + font + tint + size
        └── mpapp::resource_image_source       ← embedded resource name (looked up
                                                   from a per-app resource registry —
                                                   real impl follows the resource-
                                                   dictionary RFC)
```

The base lives in `mpapp::internal::` (framework-internal polymorphic upcast), concrete sources live in `mpapp::` (user-facing configuration objects).

### File layout

```
include/mpapp/image_sources/                   ← new directory
    file_image_source.hpp
    uri_image_source.hpp
    stream_image_source.hpp
    font_image_source.hpp
    resource_image_source.hpp
include/mpapp/internal/
    basic_image_source.hpp                     ← polymorphic base + image_source_kind enum
```

### Surface — base + canonical (file) source

```cpp
// include/mpapp/internal/basic_image_source.hpp
namespace mpapp::internal {

enum class image_source_kind : std::uint8_t {
    file     = 0,
    uri      = 1,
    stream   = 2,
    font     = 3,
    resource = 4,
};

class basic_image_source {
public:
    virtual ~basic_image_source() = default;

    basic_image_source(const basic_image_source&)            = delete;
    basic_image_source& operator=(const basic_image_source&) = delete;

    [[nodiscard]] virtual image_source_kind kind() const noexcept = 0;

protected:
    basic_image_source() = default;
};

} // namespace mpapp::internal
```

```cpp
// include/mpapp/image_sources/file_image_source.hpp
namespace mpapp {

class file_image_source : public internal::basic_image_source {
public:
    file_image_source() = default;
    explicit file_image_source(std::string file_path) : file{std::move(file_path)} {}

    Observable<std::string> file{};

    [[nodiscard]] internal::image_source_kind kind() const noexcept override {
        return internal::image_source_kind::file;
    }
};

} // namespace mpapp
```

The other four concrete sources follow the same template:

```cpp
class uri_image_source : public internal::basic_image_source {
public:
    Observable<std::string>   uri{};
    Observable<std::chrono::milliseconds>
                              cache_validity{std::chrono::hours{24}};   // MAUI default
    Observable<bool>          caching_enabled{true};
    // kind() returns image_source_kind::uri
};

class stream_image_source : public internal::basic_image_source {
public:
    // The factory is invoked by the per-platform loader (or the mock
    // recorder) when the source is bound to an image control. Returns
    // an owning bytes buffer (the real impl uses std::span<const std::byte>
    // off the wire; the mock keeps it simple as std::vector<std::byte>).
    std::function<std::vector<std::byte>()> factory{};
    // kind() returns image_source_kind::stream
};

class font_image_source : public internal::basic_image_source {
public:
    Observable<std::string>  glyph{};
    Observable<std::string>  font_family{};
    Observable<double>       size{16.0};
    Observable<color>        tint{};      // re-uses box_view's `color` type
    // kind() returns image_source_kind::font
};

class resource_image_source : public internal::basic_image_source {
public:
    Observable<std::string>  resource_name{};
    // kind() returns image_source_kind::resource
};
```

### Convenience: `image_source_ref`

Most consumers want "a thing to load" not "a typed concrete source". Adopt a type alias for the shared_ptr — same convention as `std::shared_ptr<basic_gesture_recognizer>`:

```cpp
namespace mpapp {
using image_source_ref = std::shared_ptr<internal::basic_image_source>;
}
```

### `mpapp::image::source` migration

Today: `Observable<std::string> source{}` on `internal::basic_image`. The string is a file path.

Migration path (NOT in this RFC's scope — captured under follow-up):

1. Add `Observable<image_source_ref> source_object{}` member alongside the existing string `source`.
2. Per-platform `image_handler<P>::map_source_object` is added next to `map_source`.
3. The wrapper's `mpapp::image` ctor maps both; users can use either.
4. When all per-platform real loaders support every source kind, deprecate the string-only `source` (no removal — keep backward compat for one release).

This staged migration avoids breaking any caller that currently sets `img.source = "icon.png";`.

### Mock loader

```cpp
// include/mpapp/handlers/mock/image_loader.hpp
namespace mpapp::internal {

class image_loader<platform::mock> : public mock_handler_base {
public:
    // Records `load(<kind>:<repr>)` per bound source. The mock doesn't
    // actually decode anything — it captures the request so tests can
    // assert what would have been loaded.
    void load(const image_source_ref& src) {
        if (!src) { record_event("load(null)"); return; }
        switch (src->kind()) {
            case image_source_kind::file: {
                const auto& s = static_cast<const file_image_source&>(*src);
                record_change("load(file)", s.file.get());
                break;
            }
            case image_source_kind::uri: {
                const auto& s = static_cast<const uri_image_source&>(*src);
                record_change("load(uri)", s.uri.get());
                break;
            }
            // ... stream / font / resource ...
        }
    }
};

} // namespace mpapp::internal
```

### Tests

```cpp
// tests/mock_handlers/image_source_test.cpp
TEST_CASE("file_image_source carries a path + reports kind",
          "[mock][image_source][file]") {
    mpapp::file_image_source f{"avatar.png"};
    CHECK(f.kind()    == mpapp::internal::image_source_kind::file);
    CHECK(f.file.get() == "avatar.png");
}

TEST_CASE("mock image_loader records the source kind + key",
          "[mock][image_source]") {
    mpapp::image_loader<mpapp::platform::mock> loader;
    loader.load(std::make_shared<mpapp::file_image_source>("a.png"));
    loader.load(std::make_shared<mpapp::uri_image_source>(/* uri = */"https://e/b.png"));
    REQUIRE(loader.calls_as_strings() == std::vector<std::string>{
        "load(file)=a.png",
        "load(uri)=https://e/b.png",
    });
}
```

### XAML compatibility

```xml
<!-- MAUI (must compile 1:1 per ADR-0004) -->
<Image>
    <Image.Source>
        <UriImageSource Uri="https://example.com/icon.png"
                        CacheValidity="01:00:00"
                        CachingEnabled="True"/>
    </Image.Source>
</Image>

<Image>
    <Image.Source>
        <FontImageSource Glyph="&#xE7A0;"
                         FontFamily="MaterialIcons-Regular"
                         Color="DodgerBlue"
                         Size="24"/>
    </Image.Source>
</Image>
```

The XAML compiler (`mpapp-xc`) lowers each `<XxxImageSource>` element to a `std::make_shared<mpapp::xxx_image_source>(...)`. The implicit-string converter MAUI offers (`<Image Source="icon.png"/>` shorthand) lowers to `std::make_shared<mpapp::file_image_source>("icon.png")`. Tracked under M-09.

## Alternatives

- **Stay with `Observable<std::string>` everywhere.** Rejected — no way to express remote URIs or font glyphs.

- **`std::variant<file_image_source, uri_image_source, stream_image_source, font_image_source, resource_image_source>`.** Considered. Rejected — variant is value-typed (5 source kinds, the largest being `stream_image_source` with a `std::function`), so the variant size = max(sizes). Shared ownership is awkward; binding layers want a stable ref.

- **Hand-rolled enum + union.** Rejected — same shape as variant with worse ergonomics.

- **Open polymorphism (no closed `kind()` enum).** Rejected — the family is bounded by MAUI's surface (5 kinds plus DnD's drag-payload sources which we model separately). A closed `kind()` keeps the per-platform loader's dispatch a clean switch, mirroring the gesture-recognizer choice in [[RFC-0003-gesture-recognizers]].

- **Eagerly migrate `image::source` from string to `image_source_ref` in this RFC.** Considered. Deferred — the migration touches every per-platform `image_handler` + every example that sets `img.source = "..."`. Staging it across two RFCs (this one + a "migrate image::source" follow-up) reduces single-PR risk.

## Open Questions

> [!todo] Open
> - [ ] `Stream` factory shape — sync `std::vector<std::byte>()` (mock) vs `std::future<std::vector<std::byte>>()` (real). Lean: sync in this RFC, async lands with the per-platform loader. The factory signature is the same on both sides; the async-vs-sync semantics live in the loader.
> - [ ] `font_image_source::tint` reuses `box_view::color` from `include/mpapp/box_view.hpp`. Once the dedicated `color` header lands ([[RFC-0005|future RFC]]), update the include path. For v1 the box_view reuse is fine.
> - [ ] `cache_validity` units — `std::chrono::milliseconds` for type safety, even though MAUI's TimeSpan is wider. The XAML lowering converts "01:00:00" → 3600000ms.
> - [ ] `resource_image_source` resolution — needs the Resource Dictionary subsystem (a future RFC). For v1 the resource lookup is a stub; setting one in mock just records the resource_name string.
> - [ ] `Image.Aspect` is on the `image` control, not the source — confirmed matches MAUI. No change required.

## Migration / Compatibility

- `Observable<std::string> image::source` stays for backward compatibility. New `Observable<image_source_ref> image::source_object` lands alongside.
- The per-platform `image_handler<P>::map_source` continues to honour the string. A new `map_source_object` is added next to it.
- The wrapper's `mpapp::image` ctor maps both.
- Future: a follow-up RFC deprecates the string-only `source` once the per-platform real loaders ship.

## References

- [[ADR-0008-mock-first-implementation]] — the mock-first contract.
- [[ADR-0024-wrapper-component-pattern]] — why image sources DON'T use the wrapper layer (same reasoning as RFC-0003 gestures).
- [[RFC-0003-gesture-recognizers]] — sibling design for a polymorphic-collection-of-config family.
- [[Components/Image]] — the canonical consumer.
- `references/maui/src/Controls/src/Core/{File,Uri,Stream,Font}ImageSource.cs` — MAUI source-of-truth surfaces per [[CLAUDE]] Rule 7.
