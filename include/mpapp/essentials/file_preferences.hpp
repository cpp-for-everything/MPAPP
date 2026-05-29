// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::file_preferences` — a REAL, persistent backend for the
// `mpapp::preferences` interface, written once and compiled into every
// target (no ifdefs): values are persisted to a `key=value` text file via
// std::filesystem + std::fstream. The config location is resolved from the
// platform's standard environment (XDG_CONFIG_HOME / APPDATA / HOME), with
// a temp-dir fallback so sandboxed targets (Android) still get a writable
// path. Counterpart to MAUI Essentials' platform-backed Preferences, but
// the single-file approach keeps it a pure cross-platform implementation.

#ifndef MPAPP_ESSENTIALS_FILE_PREFERENCES_HPP
#define MPAPP_ESSENTIALS_FILE_PREFERENCES_HPP

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <unordered_map>

#include "preferences.hpp"

namespace mpapp {

// Resolve a sensible per-user config-file path for `app_id`. Order:
//   1. $XDG_CONFIG_HOME/<app_id>/preferences.conf   (Linux / freedesktop)
//   2. $APPDATA\<app_id>\preferences.conf           (Windows)
//   3. $HOME/.config/<app_id>/preferences.conf      (macOS / Linux fallback)
//   4. <temp>/<app_id>-preferences.conf             (sandboxed / unknown)
// No ifdefs — whichever environment variable exists wins.
[[nodiscard]] inline std::filesystem::path
default_preferences_path(std::string_view app_id) {
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
        return fs::path{xdg} / app_id / "preferences.conf";
    if (const char* appdata = env("APPDATA"))
        return fs::path{appdata} / app_id / "preferences.conf";
    if (const char* home = env("HOME"))
        return fs::path{home} / ".config" / app_id / "preferences.conf";
    std::error_code ec;
    return fs::temp_directory_path(ec) /
           (std::string{app_id} + "-preferences.conf");
}

// File-backed preferences. Loads the file on construction; every mutation
// write-through persists the whole map. Process-safe enough for an app's
// own settings (no cross-process locking — matches MAUI's model).
class file_preferences final : public preferences {
public:
    explicit file_preferences(std::filesystem::path file)
        : path_(std::move(file)) {
        load_();
    }

    void set_string(const std::string& key, const std::string& value) override {
        store_[key] = value;
        save_();
    }
    [[nodiscard]] std::optional<std::string>
    get_string(const std::string& key) const override {
        auto it = store_.find(key);
        return it == store_.end() ? std::nullopt
                                  : std::optional<std::string>{ it->second };
    }
    void remove(const std::string& key) override { store_.erase(key); save_(); }
    void clear() override { store_.clear(); save_(); }
    [[nodiscard]] bool contains(const std::string& key) const override {
        return store_.find(key) != store_.end();
    }

    [[nodiscard]] const std::filesystem::path& file_path() const noexcept {
        return path_;
    }

private:
    static std::string escape_(std::string_view s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            if (c == '\\')      out += "\\\\";
            else if (c == '\n') out += "\\n";
            else                out += c;
        }
        return out;
    }
    static std::string unescape_(std::string_view s) {
        std::string out;
        out.reserve(s.size());
        for (std::size_t i = 0; i < s.size(); ++i) {
            if (s[i] == '\\' && i + 1 < s.size()) {
                char n = s[++i];
                out += (n == 'n') ? '\n' : n;
            } else {
                out += s[i];
            }
        }
        return out;
    }

    void load_() {
        std::ifstream in(path_);
        if (!in) return;
        std::string line;
        while (std::getline(in, line)) {
            if (line.empty()) continue;
            auto eq = line.find('=');
            if (eq == std::string::npos) continue;
            store_[line.substr(0, eq)] = unescape_(std::string_view{line}.substr(eq + 1));
        }
    }

    void save_() const {
        std::error_code ec;
        if (path_.has_parent_path())
            std::filesystem::create_directories(path_.parent_path(), ec);
        std::ofstream out(path_, std::ios::trunc);
        if (!out) return;
        for (const auto& [k, v] : store_)
            out << k << '=' << escape_(v) << '\n';
    }

    std::filesystem::path                        path_;
    std::unordered_map<std::string, std::string> store_{};
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_FILE_PREFERENCES_HPP
