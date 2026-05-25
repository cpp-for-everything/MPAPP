---
type: task
id: T-0046
title: Windows real image_loader — Microsoft.UI.Xaml.Media.Imaging.BitmapImage pipeline
status: todo
milestone: M-04
owner: ""
area: handlers
blockedBy:
  - T-0032
coveragePercent: 0
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/todo
  - area/handlers
  - phase/p4
  - platform/windows
---

# T-0046 — Windows real image_loader

## Goal

Land the Windows specialization of `mpapp::image_loader<platform::windows>` so a bound `image_source_ref` produces a `Microsoft.UI.Xaml.Media.ImageSource` consumed by the WinUI 3 `Image` control. After this lands, `image_handler<platform::windows>::map_source_object` swaps the mock loader for the real one.

## Per-kind wire-up table

| `image_source_kind` | WinUI 3 pipeline |
|---|---|
| `file` | `Windows::Storage::StorageFile::GetFileFromPathAsync(path)` → `OpenAsync(FileAccessMode::Read)` → `BitmapImage::SetSourceAsync(stream)`. |
| `uri` | `BitmapImage(Uri{uri})` — built-in cache respects HTTP cache headers; `caching_enabled` toggle maps to `BitmapImage::CreateOptions = IgnoreImageCache`. `cache_validity` defers to the system HTTP cache (no per-source override at the WinRT level). |
| `stream` | Invoke the factory → wrap result bytes in an `InMemoryRandomAccessStream` → `BitmapImage::SetSourceAsync(stream)`. |
| `font` | Render glyph via Direct2D + DirectWrite (`IDWriteTextLayout::Draw` onto an `ID2D1RenderTarget` backed by a `WIC` bitmap) → `SoftwareBitmap` → `SoftwareBitmapSource`. |
| `resource` | Resolve `resource_name` via the app's `ResourceMap` (`ms-appx:///Assets/<name>` prefix) — fallback to `MPAPP_RESOURCE_ROOT` env var for dev runs. |

## Acceptance Criteria

- [ ] `include/mpapp/handlers/windows/image_loader.hpp` declares the specialization; same `load(image_source_ref)` shape as the mock surface, returns a `winrt::Microsoft::UI::Xaml::Media::ImageSource`.
- [ ] `src/handlers/windows/image_loader.cpp` implements each `kind()` branch.
- [ ] `image_handler<platform::windows>::map_source_object` swaps loader + routes to `Image::Source` setter.
- [ ] Once T-0032 lands, the `windows-native` CI job is green for an example app loading each kind.
- [ ] Rule-11 closure: screenshots of each kind on a WinUI 3 window.

## Blocker

`mpapp-core` does not build standalone on `windows-latest` without WindowsAppSDK provisioning ([[T-0032-windows-appsdk-ci-provisioning]]) — but local dev on a project lead's Windows host with WindowsAppSDK installed can proceed independently.

## Links

- RFC: [[RFC-0004-image-source-family]].
- Mock surface: [[T-0042-image-source-family-mock]].
- CI blocker: [[T-0032-windows-appsdk-ci-provisioning]].
- Sibling per-platform tasks: T-0045 / T-0047 / T-0048 / T-0049.
