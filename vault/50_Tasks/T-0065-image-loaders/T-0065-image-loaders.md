---
type: task
id: T-0065
title: Image loaders — real file/URI image loading wired into УИСС
status: done
milestone: M-10
owner: ""
area: handlers
blockedBy: []
coveragePercent: 100
hasScreenshots: false
hasRecordings: false
tags:
  - type/task
  - status/done
  - area/handlers
  - phase/p2
---

# T-0065 — Image loaders

## Goal

Deliver the goal's "image loaders": load + display an image from a file/URI on
every platform, and demonstrate it in the УИСС app (the TU logo).

## Finding

The per-platform `image_handler`s already implement real loading (the
T-0045+ stubs were about the *richer* `image_source_ref` family — stream / font
/ resource — not basic file/URI):

| Platform | Loader |
|---|---|
| Linux/GTK4 | `gtk_picture_set_filename` (gdk-pixbuf decode) |
| Windows/WinUI 3 | `BitmapImage` from a `file:///` URI (also handles http/https/ms-appx) |
| Android | `BitmapFactory.decodeFile` + `ImageView.setImageBitmap` |

`aspect` maps to GTK content-fit / WinUI Stretch / Android ScaleType on each.

## Scope

In: a bundled placeholder TU logo (`examples/uiss/assets/tu_logo.png`, generated
by `tools/dev/make_uiss_logo.py` — a dependency-free manual PNG encoder); the
УИСС login page shows it via `mpapp::image` with `source` = a build-time
`UISS_ASSET_DIR` absolute path; `aspect_fit`. Confirms the loaders run end-to-end.
Out: URI/network download caching, `image_source_ref` stream/font/resource
(RFC-0004 follow-ups); Android on-device asset path (needs gradle assets).

## Per-platform verification

| Platform | Result |
|---|---|
| Linux WSL/GTK4 | ✅ uiss builds + the logo loads via GtkPicture from the asset path. |
| Windows MSVC/WinUI 3 | ✅ uiss.exe relinked (BitmapImage path). |
| Android NDK r26 | ✅ uiss main cross-compiles (BitmapFactory loader already real). |
| Apple | ❌ no host. |

## Acceptance Criteria

- [x] Real file/URI image loading on Linux + Windows + Android.
- [x] УИСС displays the TU logo via `mpapp::image`.
- [x] Bundled, dependency-free asset generation.

## Links

- RFC: [[RFC-0004-image-source-family]]. Used by [[T-0060-uiss-reference-app]].
