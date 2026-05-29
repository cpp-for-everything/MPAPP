// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0012-fonts.md
//
// `mpapp::font_registry` + `configure_fonts` — register embedded font
// files under aliases, MAUI's `ConfigureFonts(b => b.AddFont("Foo.ttf",
// "Foo"))`. The registry maps an alias to its font file; the
// per-platform font loader (a follow-up) resolves the file to a native
// typeface. Header-only; no macros; platform-neutral.

#ifndef MPAPP_FONTS_FONT_REGISTRY_HPP
#define MPAPP_FONTS_FONT_REGISTRY_HPP

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

namespace mpapp {

class font_registry {
public:
    font_registry() = default;

    // Register `filename` under `alias` (MAUI AddFont). Re-registering an
    // alias overwrites it. Returns *this for fluent chaining.
    font_registry& add_font(std::string filename, std::string alias) {
        aliases_[std::move(alias)] = std::move(filename);
        return *this;
    }

    // The font file backing `alias`, or nullopt if unregistered.
    [[nodiscard]] std::optional<std::string> resolve(const std::string& alias) const {
        auto it = aliases_.find(alias);
        if (it == aliases_.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    [[nodiscard]] bool has_alias(const std::string& alias) const {
        return aliases_.find(alias) != aliases_.end();
    }

    [[nodiscard]] std::size_t count() const noexcept { return aliases_.size(); }

private:
    std::unordered_map<std::string, std::string> aliases_{};  // alias -> filename
};

// `configure_fonts(registry, fn)` — the MAUI ConfigureFonts shape: hand
// the registry to a configuration callable. Sugar over direct add_font.
template <class Configure>
font_registry& configure_fonts(font_registry& registry, Configure&& configure) {
    configure(registry);
    return registry;
}

} // namespace mpapp

#endif // MPAPP_FONTS_FONT_REGISTRY_HPP
