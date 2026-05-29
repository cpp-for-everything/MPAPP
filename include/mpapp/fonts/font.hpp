// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0012-fonts.md
//
// `mpapp::font` — a font descriptor (family, size, weight, slant).
// Counterpart to MAUI's `Font` struct + `FontAttributes`. A plain value
// type with builder-style helpers; no macros; platform-neutral. The
// `family` is either a system family name or a registered alias (see
// `font_registry`).

#ifndef MPAPP_FONTS_FONT_HPP
#define MPAPP_FONTS_FONT_HPP

#include <cstdint>
#include <string>
#include <utility>

namespace mpapp {

enum class font_slant : std::uint8_t {
    normal  = 0,
    italic  = 1,
    oblique = 2,
};

// Common numeric weights (CSS / OpenType scale, 100..900). Provided as
// named constants rather than an enum so custom weights are expressible.
namespace font_weight {
inline constexpr int thin       = 100;
inline constexpr int light      = 300;
inline constexpr int regular    = 400;
inline constexpr int medium     = 500;
inline constexpr int semibold   = 600;
inline constexpr int bold       = 700;
inline constexpr int black      = 900;
} // namespace font_weight

struct font {
    std::string family{};
    double      size   = 14.0;
    int         weight = font_weight::regular;
    font_slant  slant  = font_slant::normal;

    bool operator==(const font&) const = default;

    [[nodiscard]] static font of_size(std::string family, double size) {
        return font{ std::move(family), size, font_weight::regular, font_slant::normal };
    }

    [[nodiscard]] font with_weight(int w) const {
        font f = *this;
        f.weight = w;
        return f;
    }

    [[nodiscard]] font with_slant(font_slant s) const {
        font f = *this;
        f.slant = s;
        return f;
    }

    [[nodiscard]] font with_size(double s) const {
        font f = *this;
        f.size = s;
        return f;
    }

    // Convenience predicates matching MAUI's FontAttributes.Bold / Italic.
    [[nodiscard]] bool is_bold() const noexcept { return weight >= font_weight::semibold; }
    [[nodiscard]] bool is_italic() const noexcept { return slant != font_slant::normal; }
};

} // namespace mpapp

#endif // MPAPP_FONTS_FONT_HPP
