// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::file_system` — application file-system paths and package-file
// access. Counterpart to MAUI Essentials `FileSystem`. Abstract interface
// + an in-memory mock implementation whose state is fully settable so
// tests can drive every code path without touching the real file system.
// Real per-platform backends (Windows ApplicationData, Linux XDG dirs,
// Android Context.getCacheDir / getFilesDir, Apple NSSearchPathForDirInDomains)
// implement the same interface and are injected via the DI container
// (RFC-0011). No macros.

#ifndef MPAPP_ESSENTIALS_FILE_SYSTEM_HPP
#define MPAPP_ESSENTIALS_FILE_SYSTEM_HPP

#include <optional>
#include <string>
#include <unordered_map>

namespace mpapp {

// Abstract interface — mirrors MAUI's IFileSystem surface.
class file_system {
public:
    virtual ~file_system() = default;

    // Returns the path to a temporary cache directory the app may write to.
    // The platform may evict contents at any time.
    [[nodiscard]] virtual std::string cache_directory() const = 0;

    // Returns the path to the persistent application-data directory.
    [[nodiscard]] virtual std::string app_data_directory() const = 0;

    // Opens a file bundled inside the application package (e.g. Android
    // assets, Windows embedded resources). Returns the file contents as a
    // string on success, or std::nullopt when the file is not found.
    [[nodiscard]] virtual std::optional<std::string>
        open_app_package_file(const std::string& name) const = 0;
};

// Mock / in-memory implementation.
//
// - cache_directory and app_data_directory return settable values.
// - Package files are registered with register_package_file(name, contents)
//   and looked up by open_app_package_file(name).
// - last_open_request() exposes the most-recently-requested package-file
//   name so tests can assert on call arguments.
class mock_file_system final : public file_system {
public:
    // Construct with explicit directory paths (convenient for test fixtures).
    explicit mock_file_system(std::string cache_dir  = "/tmp/cache",
                              std::string app_data_dir = "/tmp/appdata")
        : cache_dir_{ std::move(cache_dir) }
        , app_data_dir_{ std::move(app_data_dir) }
    {}

    // ---- Settable state -------------------------------------------------

    void set_cache_directory(std::string path) {
        cache_dir_ = std::move(path);
    }

    void set_app_data_directory(std::string path) {
        app_data_dir_ = std::move(path);
    }

    // Register a package file so open_app_package_file can return it.
    void register_package_file(const std::string& name, std::string contents) {
        package_files_[name] = std::move(contents);
    }

    // Remove a previously registered package file.
    void unregister_package_file(const std::string& name) {
        package_files_.erase(name);
    }

    // ---- Call-argument recorder -----------------------------------------

    // Returns the name passed to the most recent open_app_package_file call,
    // or std::nullopt if the method has never been called.
    [[nodiscard]] std::optional<std::string> last_open_request() const {
        return last_open_request_;
    }

    // ---- file_system interface ------------------------------------------

    [[nodiscard]] std::string cache_directory() const override {
        return cache_dir_;
    }

    [[nodiscard]] std::string app_data_directory() const override {
        return app_data_dir_;
    }

    [[nodiscard]] std::optional<std::string>
    open_app_package_file(const std::string& name) const override {
        last_open_request_ = name;
        auto it = package_files_.find(name);
        if (it == package_files_.end()) {
            return std::nullopt;
        }
        return it->second;
    }

private:
    std::string cache_dir_;
    std::string app_data_dir_;
    std::unordered_map<std::string, std::string> package_files_{};
    mutable std::optional<std::string> last_open_request_{};
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_FILE_SYSTEM_HPP
