---
type: task
id: T-0047
title: Android real image_loader — BitmapFactory + OkHttp + AssetManager pipeline
status: todo
milestone: M-04
owner: ""
area: handlers
blockedBy: []
coveragePercent: 0
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/todo
  - area/handlers
  - phase/p4
  - platform/android
---

# T-0047 — Android real image_loader

## Goal

Land the Android specialization of `mpapp::image_loader<platform::android>` so a bound `image_source_ref` produces an `android.graphics.Bitmap` (or `android.graphics.drawable.Drawable`) consumed by `android.widget.ImageView`. After this lands, `image_handler<platform::android>::map_source_object` swaps the mock loader for the real one.

## Per-kind wire-up table

| `image_source_kind` | Android pipeline |
|---|---|
| `file` | `BitmapFactory::decodeFile(path)` (JNI bridge via `mpapp-jni-gen`) → `ImageView::setImageBitmap`. |
| `uri` | `okhttp3.OkHttpClient::newCall(Request{uri}).execute()` → response body bytes → `BitmapFactory::decodeStream`. `caching_enabled` + `cache_validity` map onto OkHttp's `Cache` (configured at `application` level with `cache_validity` as the per-request `Cache-Control: max-age`). |
| `stream` | Invoke the factory → wrap result bytes in `ByteArrayInputStream` → `BitmapFactory::decodeStream`. |
| `font` | Use `android.graphics.Canvas::drawText` against a `Bitmap` sized by `Paint::measureText` — `Paint::setTypeface(Typeface::createFromAsset(am, font_family))`, `Paint::setColor(tint.argb())`, `Paint::setTextSize(size)`. |
| `resource` | Resolve `resource_name` via `AssetManager::open(name)` → `BitmapFactory::decodeStream`. Falls back to `R.drawable.<sanitize(name)>` lookup if the asset path isn't found (matches MAUI's `embedded resource` behavior). |

## Acceptance Criteria

- [ ] `include/mpapp/handlers/android/image_loader.hpp` declares the specialization; same `load(image_source_ref)` shape, returns a JNI `jobject` (`android.graphics.Bitmap`) wrapped in a `winrt`-style global-ref RAII handle.
- [ ] `src/handlers/android/image_loader.cpp` implements each `kind()` branch using `mpapp-jni-gen` generated bindings.
- [ ] `image_handler<platform::android>::map_source_object` swaps loader + routes to `ImageView::setImageBitmap`.
- [ ] CI: `android-arm64-cross` job is green for an example app loading each kind.
- [ ] Rule-11 closure: screen-recording of each kind rendering on an Android emulator.

## Links

- RFC: [[RFC-0004-image-source-family]].
- Mock surface: [[T-0042-image-source-family-mock]].
- Sibling per-platform tasks: T-0045 / T-0046 / T-0048 / T-0049.
