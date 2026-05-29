// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0004-image-source-family.md
//
// `mpapp::font_image_source` — counterpart to MAUI's FontImageSource.
// Renders a single glyph (often from an icon font like
// MaterialIcons-Regular) into a tinted bitmap at the requested
// pixel size. The per-platform loader rasterises the glyph via the
// platform's font-rendering pipeline (DirectWrite on Windows,
// Pango/Cairo on Linux, CoreText on Apple, Skia on Android).

#ifndef MPAPP_IMAGE_SOURCES_FONT_IMAGE_SOURCE_HPP
#define MPAPP_IMAGE_SOURCES_FONT_IMAGE_SOURCE_HPP

#include <string>

#include "../internal/basic_box_view.hpp"   // reuses `mpapp::color` for the tint (surface, not the wrapper — keeps image sources SDK-free)
#include "../internal/basic_image_source.hpp"
#include "../observable.hpp"

namespace mpapp {

class font_image_source : public internal::basic_image_source {
public:
    font_image_source() = default;

    // ----- Bindable configuration ----------------------------------------
    // Single-character glyph (UTF-8 string — supports multi-byte
    // codepoints in icon fonts). Example: "\xEE\x9E\xA0" for U+E7A0
    // (Material Icons "home" by default).
    Observable<std::string> glyph{};

    // Font family name. Must match a font installed via the platform's
    // font registry; MPAPP's Resource Dictionary RFC (future) will
    // describe app-side font registration.
    Observable<std::string> font_family{};

    // Pixel size of the rasterised glyph. MAUI's default is 16dp; we
    // use raw pixels at v1 (DPI scaling lands with the unified
    // measure RFC).
    Observable<double> size{16.0};

    // Glyph tint. Re-uses `mpapp::color` from box_view.hpp so existing
    // formatters work without change.
    Observable<color> tint{};

    // ----- Polymorphic identity ------------------------------------------
    [[nodiscard]] internal::image_source_kind kind() const noexcept override {
        return internal::image_source_kind::font;
    }
};

} // namespace mpapp

#endif // MPAPP_IMAGE_SOURCES_FONT_IMAGE_SOURCE_HPP
