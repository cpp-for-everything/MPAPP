// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::real_file_system` — a REAL, cross-platform backend for the
// `mpapp::file_system` interface, written once and compiled into every
// target (no ifdefs): directories are resolved via std::filesystem and
// the platform's standard environment variables (XDG_CONFIG_HOME / APPDATA
// / HOME), with a temp-dir fallback so sandboxed targets still get a
// writable path. Package files are read from a configurable base directory
// on the real file system. Counterpart to MAUI Essentials' FileSystem.

#ifndef MPAPP_ESSENTIALS_REAL_FILE_SYSTEM_HPP
#define MPAPP_ESSENTIALS_REAL_FILE_SYSTEM_HPP

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>

#include "file_system.hpp"

namespace mpapp {

// Resolve a sensible per-user application-data directory for `app_id`. Order:
//   1. $XDG_CONFIG_HOME/<app_id>      (Linux / freedesktop)
//   2. $APPDATA\<app_id>              (Windows)
//   3. $HOME/.config/<app_id>         (macOS / Linux fallback)
//   4. <temp>/<app_id>-appdata        (sandboxed / unknown)
// No ifdefs — whichever environment variable exists wins.
[[nodiscard]] inline std::filesystem::path
default_app_data_directory(std::string_view app_id) {
    namespace fs = std::filesystem;
    auto env = [](const char* name) -> const char* {
        // std::getenv is the portable read; MSVC flags it C4996 ("unsafe").
        // The guarded pragma silences it on MSVC only (invisible to GCC/Clang,
        // so no -Wunknown-pragmas noise) — an internal build guard, not a
        // public macro (Rule 1 exempt).
#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable : 4996)
#endif
        const char* v = std::getenv(name);
#if defined(_MSC_VER)
#  pragma warning(pop)
#endif
        return (v != nullptr && v[0] != '\0') ? v : nullptr;
    };
    if (const char* xdg = env("XDG_CONFIG_HOME"))
        return fs::path{xdg} / app_id;
    if (const char* appdata = env("APPDATA"))
        return fs::path{appdata} / app_id;
    if (const char* home = env("HOME"))
        return fs::path{home} / ".config" / app_id;
    std::error_code ec;
    return fs::temp_directory_path(ec) / (std::string{app_id} + "-appdata");
}

// Real file-system backend. All three interface methods operate on the real
// file system; no mocking is performed.
//
// - cache_directory()      returns <temp>/<app_id>/cache (created on demand)
// - app_data_directory()   returns the XDG/APPDATA/HOME-resolved path (created)
// - open_app_package_file  reads files relative to the package_base_dir
//   (defaults to app_data_directory() when not explicitly overridden)
//
// Rule of Zero: all members are copyable/movable value types; no explicit
// destructor, copy, or move needed.
class real_file_system final : public file_system {
public:
    // Construct with an explicit app identifier.  An optional
    // `package_base_dir` overrides the directory searched by
    // open_app_package_file; when empty the app_data_directory is used.
    explicit real_file_system(std::string app_id,
                              std::filesystem::path package_base_dir = {})
        : app_id_{ std::move(app_id) }
        , package_base_dir_{ std::move(package_base_dir) }
    {}

    // ---- file_system interface ------------------------------------------

    // Returns a cache directory path and ensures it exists.
    // Path: <temp_directory>/<app_id>/cache
    [[nodiscard]] std::string cache_directory() const override {
        namespace fs = std::filesystem;
        std::error_code ec;
        fs::path dir = fs::temp_directory_path(ec) / app_id_ / "cache";
        fs::create_directories(dir, ec);
        return dir.string();
    }

    // Returns the persistent application-data directory and ensures it exists.
    [[nodiscard]] std::string app_data_directory() const override {
        namespace fs = std::filesystem;
        fs::path dir = default_app_data_directory(app_id_);
        std::error_code ec;
        fs::create_directories(dir, ec);
        return dir.string();
    }

    // Reads a file bundled under the package base directory.
    // Returns the file contents on success, std::nullopt when not found.
    [[nodiscard]] std::optional<std::string>
    open_app_package_file(const std::string& name) const override {
        namespace fs = std::filesystem;
        fs::path base = package_base_dir_.empty()
                            ? fs::path{ app_data_directory() }
                            : package_base_dir_;
        fs::path target = base / name;
        std::ifstream in{ target };
        if (!in)
            return std::nullopt;
        return std::string{ std::istreambuf_iterator<char>{ in },
                            std::istreambuf_iterator<char>{} };
    }

    // ---- Accessors ---------------------------------------------------------

    [[nodiscard]] const std::string& app_id() const noexcept {
        return app_id_;
    }

    [[nodiscard]] const std::filesystem::path& package_base_dir() const noexcept {
        return package_base_dir_;
    }

private:
    std::string           app_id_;
    std::filesystem::path package_base_dir_;
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_REAL_FILE_SYSTEM_HPP
