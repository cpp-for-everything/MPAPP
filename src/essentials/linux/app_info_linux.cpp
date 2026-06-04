// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// Linux GLib/GSettings implementation of `mpapp::linux_app_info`.
// GLib and GIO headers are confined to this translation unit.

#include "mpapp/essentials/linux/app_info_linux.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <cstdio>        // std::fgets, std::fopen, std::fclose
#include <cstring>       // std::strlen
#include <dlfcn.h>       // dlopen, dlsym, dlclose
#include <string>

#include <gio/gio.h>     // GLib + GIO (GSettings, g_get_application_name, …)

namespace {

// ---- /proc/self/comm fallback -----------------------------------------------

// Read the process name from /proc/self/comm (max 15 chars, NUL-terminated).
// Returns an empty string on failure.
[[nodiscard]] std::string read_proc_comm()
{
    std::FILE* f = std::fopen("/proc/self/comm", "r");
    if (f == nullptr) {
        return {};
    }
    char buf[64] = {};
    const char* result = std::fgets(buf, static_cast<int>(sizeof(buf)), f);
    std::fclose(f);
    if (result == nullptr) {
        return {};
    }
    // Strip trailing newline if present.
    const std::size_t len = std::strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    }
    return std::string{buf};
}

// ---- GSettings theme probe --------------------------------------------------

// Try to read the org.freedesktop.appearance color-scheme key.
// Returns:
//   app_theme::dark        when color-scheme == 1 (prefer-dark)
//   app_theme::light       when color-scheme == 2 (prefer-light)
//   app_theme::unspecified when color-scheme == 0 (no preference) or
//                          when the schema / key is unavailable.
[[nodiscard]] mpapp::app_theme theme_from_freedesktop_settings() noexcept
{
    static constexpr const char* kSchema = "org.freedesktop.appearance";
    static constexpr const char* kKey    = "color-scheme";

    // Check whether the schema is installed before trying to open it.
    GSettingsSchemaSource* src = g_settings_schema_source_get_default();
    if (src == nullptr) {
        return mpapp::app_theme::unspecified;
    }

    GSettingsSchema* schema =
        g_settings_schema_source_lookup(src, kSchema, TRUE);
    if (schema == nullptr) {
        return mpapp::app_theme::unspecified;
    }
    const gboolean has_key = g_settings_schema_has_key(schema, kKey);
    g_settings_schema_unref(schema);
    if (!has_key) {
        return mpapp::app_theme::unspecified;
    }

    GSettings* settings = g_settings_new(kSchema);
    if (settings == nullptr) {
        return mpapp::app_theme::unspecified;
    }

    const guint32 value =
        static_cast<guint32>(g_settings_get_uint(settings, kKey));
    g_object_unref(settings);

    switch (value) {
        case 1u: return mpapp::app_theme::dark;
        case 2u: return mpapp::app_theme::light;
        default: return mpapp::app_theme::unspecified;
    }
}

// ---- GTK settings probe (dlopen, no link-time GTK dependency) ---------------

// Attempt to read the GtkSettings property "gtk-application-prefer-dark-theme"
// by dynamically loading libgtk-3.so.0 and resolving gtk_settings_get_default.
// Avoids a link-time dependency on GTK so that the translation unit compiles
// with only gio-2.0 on the linker line.
//
// Returns app_theme::dark / ::light / ::unspecified.  Any failure (library not
// present, symbol not found, null settings) returns unspecified.
[[nodiscard]] mpapp::app_theme theme_from_gtk_settings() noexcept
{
    // Try libgtk-3.so.0 then libgtk-4.so.1 (GTK4 exposes the same property).
    static const char* const kLibs[] = {
        "libgtk-3.so.0",
        "libgtk-4.so.1",
        nullptr
    };

    using FnGetDefault = GObject*(*)();

    void*        lib = nullptr;
    FnGetDefault fn  = nullptr;

    for (const char* const* libname = kLibs; *libname != nullptr; ++libname) {
        lib = dlopen(*libname, RTLD_LAZY | RTLD_LOCAL | RTLD_NOLOAD);
        if (lib == nullptr) {
            // Only attempt to load GTK if it is already resident (RTLD_NOLOAD).
            // This avoids initialising GTK as a side-effect.
            continue;
        }
        void* sym = dlsym(lib, "gtk_settings_get_default");
        if (sym != nullptr) {
            // POSIX requires this cast dance for function pointers from dlsym.
            __extension__
            fn = reinterpret_cast<FnGetDefault>(reinterpret_cast<std::uintptr_t>(sym));
            break;
        }
        dlclose(lib);
        lib = nullptr;
    }

    if (fn == nullptr) {
        return mpapp::app_theme::unspecified;
    }

    GObject* settings = fn();
    if (settings == nullptr) {
        dlclose(lib);
        return mpapp::app_theme::unspecified;
    }

    gboolean prefer_dark = FALSE;
    g_object_get(settings,
                 "gtk-application-prefer-dark-theme", &prefer_dark,
                 nullptr);

    dlclose(lib);

    return prefer_dark ? mpapp::app_theme::dark : mpapp::app_theme::light;
}

// ---- Combined theme probe ---------------------------------------------------

// Returns the best available theme preference.
// Priority: org.freedesktop.appearance > GtkSettings > unspecified.
[[nodiscard]] mpapp::app_theme detect_theme() noexcept
{
    const mpapp::app_theme fd = theme_from_freedesktop_settings();
    if (fd != mpapp::app_theme::unspecified) {
        return fd;
    }
    return theme_from_gtk_settings();
}

// ---- Layout direction from locale -------------------------------------------

// Very lightweight heuristic: inspect the LANG/LC_ALL environment variables
// for known RTL language codes (ar, he, fa, ur, ps, yi, ji, sd, ug, dv).
// Returns layout_direction::right_to_left for those locales,
// left_to_right otherwise.  This is best-effort; a full ICU query is outside
// scope.
[[nodiscard]] mpapp::layout_direction detect_layout_direction() noexcept
{
    static constexpr const char* kRtlPrefixes[] = {
        "ar", "he", "fa", "ur", "ps", "yi", "ji", "sd", "ug", "dv", nullptr
    };

    const char* locale = g_getenv("LC_ALL");
    if (locale == nullptr || locale[0] == '\0') {
        locale = g_getenv("LANG");
    }
    if (locale == nullptr || locale[0] == '\0') {
        return mpapp::layout_direction::left_to_right;
    }

    for (const char* const* p = kRtlPrefixes; *p != nullptr; ++p) {
        // Match "ar", "ar_", "ar." at the start of the locale string.
        const std::size_t plen = std::strlen(*p);
        if (std::strncmp(locale, *p, plen) == 0) {
            const char next = locale[plen];
            if (next == '\0' || next == '_' || next == '.' || next == '@') {
                return mpapp::layout_direction::right_to_left;
            }
        }
    }
    return mpapp::layout_direction::left_to_right;
}

} // anonymous namespace

namespace mpapp {

// ---- Constructor / destructor -----------------------------------------------

linux_app_info::linux_app_info(std::string version_string,
                               std::string build_string)
    : version_string_{std::move(version_string)}
    , build_string_{std::move(build_string)}
{}

linux_app_info::~linux_app_info() = default;

// ---- mpapp::app_info interface ----------------------------------------------

std::string linux_app_info::package_name() const
{
    // g_get_prgname() returns the value set via g_set_prgname() or derived
    // from argv[0] by GLib; it does not allocate — no free needed.
    const gchar* prgname = g_get_prgname();
    if (prgname != nullptr && prgname[0] != '\0') {
        return std::string{prgname};
    }
    // /proc/self/comm as a fallback on Linux.
    std::string comm = read_proc_comm();
    if (!comm.empty()) {
        return comm;
    }
    return {};
}

std::string linux_app_info::name() const
{
    // g_get_application_name() returns a human-readable name set via
    // g_set_application_name(); falls back to g_get_prgname() internally
    // when no name has been set.  No free needed (static/owned by GLib).
    const gchar* appname = g_get_application_name();
    if (appname != nullptr && appname[0] != '\0') {
        return std::string{appname};
    }
    return package_name();
}

std::string linux_app_info::version_string() const
{
    return version_string_;
}

std::string linux_app_info::build_string() const
{
    return build_string_;
}

app_theme linux_app_info::requested_theme() const
{
    return detect_theme();
}

layout_direction linux_app_info::requested_layout_direction() const
{
    return detect_layout_direction();
}

void linux_app_info::show_settings_ui()
{
    // Linux does not expose a per-app system settings page. No-op.
}

} // namespace mpapp

#endif // defined(__linux__) && !defined(__ANDROID__)
