// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// Linux GIO implementation of `mpapp::linux_launcher` and
// `mpapp::linux_browser`.
// <gio/gio.h> is confined to this translation unit.

#include "mpapp/essentials/linux/launcher_linux.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

#include <string_view>

#include <gio/gio.h>

namespace {

// Returns true when `uri` starts with `scheme_prefix` using ASCII
// case-insensitive comparison up to the length of the prefix.
[[nodiscard]] bool starts_with_scheme(std::string_view uri,
                                      std::string_view scheme_prefix) noexcept
{
    if (uri.size() < scheme_prefix.size()) {
        return false;
    }
    for (std::size_t i = 0; i < scheme_prefix.size(); ++i) {
        const char c = (uri[i] >= 'A' && uri[i] <= 'Z')
                       ? static_cast<char>(uri[i] + ('a' - 'A'))
                       : uri[i];
        if (c != scheme_prefix[i]) {
            return false;
        }
    }
    return true;
}

// Returns true for URI schemes that are reliably handled on Linux.
[[nodiscard]] bool is_known_scheme(std::string_view uri) noexcept
{
    return starts_with_scheme(uri, "http://")   ||
           starts_with_scheme(uri, "https://")  ||
           starts_with_scheme(uri, "mailto:")   ||
           starts_with_scheme(uri, "tel:")      ||
           starts_with_scheme(uri, "file://")   ||
           starts_with_scheme(uri, "file:///");
}

// Extract the scheme portion from a URI (the part before the first ':').
// Returns an empty string if no ':' is found.
[[nodiscard]] std::string extract_scheme(const std::string& uri)
{
    const auto colon = uri.find(':');
    if (colon == std::string::npos || colon == 0) {
        return {};
    }
    std::string scheme = uri.substr(0, colon);
    // Lowercase the scheme for the GIO query.
    for (char& c : scheme) {
        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c + ('a' - 'A'));
        }
    }
    return scheme;
}

// Query GIO for a default application handler for the given URI scheme.
// Returns true when GIO reports at least one registered handler.
[[nodiscard]] bool gio_has_handler_for_scheme(const std::string& scheme)
{
    if (scheme.empty()) {
        return false;
    }
    GAppInfo* info = g_app_info_get_default_for_uri_scheme(scheme.c_str());
    if (info == nullptr) {
        return false;
    }
    g_object_unref(info);
    return true;
}

// Invoke g_app_info_launch_default_for_uri for the given UTF-8 URI.
// Returns true on success (GIO returned TRUE), false otherwise.
// Clears the GError on failure.
[[nodiscard]] bool gio_open(const std::string& uri)
{
    if (uri.empty()) {
        return false;
    }
    GError* error = nullptr;
    const gboolean ok = g_app_info_launch_default_for_uri(
        uri.c_str(), nullptr, &error);
    if (error != nullptr) {
        g_error_free(error);
    }
    return ok == TRUE;
}

// Compile-time checks: both classes must be instantiatable.
static_assert(sizeof(mpapp::linux_launcher) > 0,
              "linux_launcher must be a complete type");
static_assert(sizeof(mpapp::linux_browser) > 0,
              "linux_browser must be a complete type");

} // anonymous namespace

namespace mpapp {

// ---- linux_launcher ----------------------------------------------------------

bool linux_launcher::can_open(const std::string& uri) const
{
    if (uri.empty()) {
        return false;
    }
    // Fast path for common well-known schemes.
    if (is_known_scheme(uri)) {
        return true;
    }
    // For any other scheme, ask GIO whether a handler exists.
    const std::string scheme = extract_scheme(uri);
    return gio_has_handler_for_scheme(scheme);
}

bool linux_launcher::try_open(const std::string& uri)
{
    if (uri.empty()) {
        return false;
    }
    return gio_open(uri);
}

void linux_launcher::open(const std::string& uri)
{
    if (uri.empty()) {
        return;
    }
    (void)gio_open(uri); // return value intentionally discarded
}

// ---- linux_browser -----------------------------------------------------------

bool linux_browser::open(const std::string& uri)
{
    // Linux has no in-app browser; always delegate to GIO.
    return gio_open(uri);
}

bool linux_browser::open(const std::string& uri,
                          browser_launch_mode /*mode*/)
{
    // Launch mode is advisory on Linux; ignore it.
    return gio_open(uri);
}

bool linux_browser::open(const std::string& uri,
                          const browser_launch_options& /*options*/)
{
    // Options (mode, title_mode) are advisory on Linux; ignore them.
    return gio_open(uri);
}

} // namespace mpapp

#endif // defined(__linux__) && !defined(__ANDROID__)
