// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::app_theme_binding<T>` — a value holder that selects between a
// `light` and `dark` value based on the active `mpapp::app_theme`, mirroring
// MAUI's `AppThemeBinding` markup extension.  When the OS theme is
// `unspecified`, `resolve()` falls back to `default_value` if one was
// provided, otherwise to the `light` value.
//
// This is a plain template value type (Rule of Zero, no virtuals, no macros),
// in the spirit of `mpapp::Observable<T>`.  Conforms to ADR-0002.

#ifndef MPAPP_THEME_APP_THEME_BINDING_HPP
#define MPAPP_THEME_APP_THEME_BINDING_HPP

#include <optional>

#include "../essentials/app_info.hpp"

namespace mpapp {

// Holds one value per theme.  `default_value` is consulted only for the
// `unspecified` theme; when it is absent the `light` value is used as the
// final fallback.
template <class T>
struct app_theme_binding {
    T               light{};
    T               dark{};
    std::optional<T> default_value{};

    // Returns the value appropriate for `theme`.  `dark` and `light` map
    // directly; any other value (notably `app_theme::unspecified`) resolves
    // to `default_value` when present, otherwise to `light`.
    [[nodiscard]] T resolve(mpapp::app_theme theme) const {
        switch (theme) {
            case app_theme::dark:
                return dark;
            case app_theme::light:
                return light;
            default:
                return default_value ? *default_value : light;
        }
    }
};

// Free-function convenience wrapper around `app_theme_binding<T>::resolve`.
template <class T>
[[nodiscard]] T app_theme_value(const app_theme_binding<T>& b, app_theme t) {
    return b.resolve(t);
}

} // namespace mpapp

#endif // MPAPP_THEME_APP_THEME_BINDING_HPP
