// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Tests for the real std::filesystem-backed file-system
// backend (RFC-0013 Essentials). Exercises every public method and edge
// case against a temp-isolated app-id so the real user config is untouched.

#include <filesystem>
#include <fstream>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/real_file_system.hpp>

namespace {

// Unique app-id string used across all test cases — avoids polluting
// any real per-user config directory.
constexpr const char* kTestAppId = "mpapp-real-fs-test-tmp";

// Helper: write content to a file under a directory, creating dirs if needed.
void write_file(const std::filesystem::path& dir,
                const std::string& name,
                const std::string& content) {
    namespace fs = std::filesystem;
    std::error_code ec;
    fs::create_directories(dir, ec);
    std::ofstream out{ dir / name };
    out << content;
}

// Helper: recursively remove a directory, ignoring errors.
void remove_dir(const std::filesystem::path& p) {
    std::error_code ec;
    std::filesystem::remove_all(p, ec);
}

} // namespace

// ---------------------------------------------------------------------------
// cache_directory
// ---------------------------------------------------------------------------

TEST_CASE("real_file_system cache_directory is non-empty and exists",
          "[mock][essentials][file_system]") {
    // Arrange
    mpapp::real_file_system fs{ kTestAppId };

    // Act
    const std::string cache = fs.cache_directory();

    // Assert
    CHECK_FALSE(cache.empty());
    CHECK(std::filesystem::exists(cache));
    CHECK(std::filesystem::is_directory(cache));

    // Cleanup
    remove_dir(std::filesystem::path{ cache }.parent_path());
}

TEST_CASE("real_file_system cache_directory is idempotent",
          "[mock][essentials][file_system]") {
    // Arrange
    mpapp::real_file_system fs{ kTestAppId };

    // Act — call twice
    const std::string first  = fs.cache_directory();
    const std::string second = fs.cache_directory();

    // Assert — same path both times, still exists
    CHECK(first == second);
    CHECK(std::filesystem::exists(first));

    // Cleanup
    remove_dir(std::filesystem::path{ first }.parent_path());
}

// ---------------------------------------------------------------------------
// app_data_directory
// ---------------------------------------------------------------------------

TEST_CASE("real_file_system app_data_directory is non-empty and exists",
          "[mock][essentials][file_system]") {
    // Arrange
    mpapp::real_file_system fs{ kTestAppId };

    // Act
    const std::string appdata = fs.app_data_directory();

    // Assert
    CHECK_FALSE(appdata.empty());
    CHECK(std::filesystem::exists(appdata));
    CHECK(std::filesystem::is_directory(appdata));

    // Cleanup
    remove_dir(appdata);
}

TEST_CASE("real_file_system app_data_directory is idempotent",
          "[mock][essentials][file_system]") {
    // Arrange
    mpapp::real_file_system fs{ kTestAppId };

    // Act
    const std::string first  = fs.app_data_directory();
    const std::string second = fs.app_data_directory();

    // Assert
    CHECK(first == second);
    CHECK(std::filesystem::exists(first));

    // Cleanup
    remove_dir(first);
}

// ---------------------------------------------------------------------------
// open_app_package_file — file found
// ---------------------------------------------------------------------------

TEST_CASE("real_file_system open_app_package_file returns contents when file exists",
          "[mock][essentials][file_system]") {
    // Arrange — write a package file into a temp base directory
    const auto base = std::filesystem::temp_directory_path() /
                      "mpapp-real-fs-pkg-test";
    const std::string name    = "hello.txt";
    const std::string content = "Hello, MPAPP!\nSecond line.";
    write_file(base, name, content);

    mpapp::real_file_system fs{ kTestAppId, base };

    // Act
    const auto result = fs.open_app_package_file(name);

    // Assert
    REQUIRE(result.has_value());
    CHECK(*result == content);

    // Cleanup
    remove_dir(base);
}

TEST_CASE("real_file_system open_app_package_file works with empty content",
          "[mock][essentials][file_system]") {
    // Arrange
    const auto base = std::filesystem::temp_directory_path() /
                      "mpapp-real-fs-pkg-empty";
    const std::string name = "empty.txt";
    write_file(base, name, "");

    mpapp::real_file_system fs{ kTestAppId, base };

    // Act
    const auto result = fs.open_app_package_file(name);

    // Assert — file exists but is empty; result should be present with ""
    REQUIRE(result.has_value());
    CHECK(result->empty());

    // Cleanup
    remove_dir(base);
}

TEST_CASE("real_file_system open_app_package_file reads binary-safe text",
          "[mock][essentials][file_system]") {
    // Arrange — content with backslashes, equals signs, newlines
    const auto base = std::filesystem::temp_directory_path() /
                      "mpapp-real-fs-pkg-tricky";
    const std::string name    = "tricky.txt";
    const std::string content = "line1\nline2=value\\path";
    write_file(base, name, content);

    mpapp::real_file_system fs{ kTestAppId, base };

    // Act
    const auto result = fs.open_app_package_file(name);

    // Assert
    REQUIRE(result.has_value());
    CHECK(*result == content);

    // Cleanup
    remove_dir(base);
}

// ---------------------------------------------------------------------------
// open_app_package_file — file missing
// ---------------------------------------------------------------------------

TEST_CASE("real_file_system open_app_package_file returns nullopt when missing",
          "[mock][essentials][file_system]") {
    // Arrange — base directory exists but the file does not
    const auto base = std::filesystem::temp_directory_path() /
                      "mpapp-real-fs-pkg-missing";
    std::error_code ec;
    std::filesystem::create_directories(base, ec);

    mpapp::real_file_system fs{ kTestAppId, base };

    // Act
    const auto result = fs.open_app_package_file("does-not-exist.txt");

    // Assert
    CHECK_FALSE(result.has_value());

    // Cleanup
    remove_dir(base);
}

TEST_CASE("real_file_system open_app_package_file returns nullopt when base missing",
          "[mock][essentials][file_system]") {
    // Arrange — point to a non-existent base directory
    const auto base = std::filesystem::temp_directory_path() /
                      "mpapp-real-fs-pkg-no-base-dir-xyz987";
    // Ensure it really doesn't exist
    remove_dir(base);

    mpapp::real_file_system fs{ kTestAppId, base };

    // Act
    const auto result = fs.open_app_package_file("anything.txt");

    // Assert
    CHECK_FALSE(result.has_value());
}

// ---------------------------------------------------------------------------
// open_app_package_file — defaults to app_data_directory when no base given
// ---------------------------------------------------------------------------

TEST_CASE("real_file_system open_app_package_file uses app_data_dir as default base",
          "[mock][essentials][file_system]") {
    // Arrange — construct without explicit package_base_dir
    mpapp::real_file_system fs{ kTestAppId };
    const std::string appdata = fs.app_data_directory();

    const std::string name    = "bundled.txt";
    const std::string content = "bundled content";
    write_file(appdata, name, content);

    // Act
    const auto result = fs.open_app_package_file(name);

    // Assert
    REQUIRE(result.has_value());
    CHECK(*result == content);

    // Cleanup
    remove_dir(appdata);
}

// ---------------------------------------------------------------------------
// Accessors
// ---------------------------------------------------------------------------

TEST_CASE("real_file_system accessors return construction values",
          "[mock][essentials][file_system]") {
    // Arrange
    const std::string           id   = "my-test-app";
    const std::filesystem::path base = "/some/path";

    // Act
    mpapp::real_file_system fs{ id, base };

    // Assert
    CHECK(fs.app_id() == id);
    CHECK(fs.package_base_dir() == base);
}

TEST_CASE("real_file_system default package_base_dir is empty path",
          "[mock][essentials][file_system]") {
    // Arrange + Act
    mpapp::real_file_system fs{ "app" };

    // Assert
    CHECK(fs.package_base_dir().empty());
}

// ---------------------------------------------------------------------------
// Interface polymorphism
// ---------------------------------------------------------------------------

TEST_CASE("real_file_system is usable through file_system pointer",
          "[mock][essentials][file_system]") {
    // Arrange
    const auto base = std::filesystem::temp_directory_path() /
                      "mpapp-real-fs-poly-test";
    const std::string name    = "poly.txt";
    const std::string content = "polymorphic";
    write_file(base, name, content);

    const mpapp::file_system* iface =
        new mpapp::real_file_system{ kTestAppId, base };

    // Act
    const std::string cache   = iface->cache_directory();
    const std::string appdata = iface->app_data_directory();
    const auto        pkg     = iface->open_app_package_file(name);

    // Assert
    CHECK_FALSE(cache.empty());
    CHECK_FALSE(appdata.empty());
    REQUIRE(pkg.has_value());
    CHECK(*pkg == content);

    delete iface;

    // Cleanup
    remove_dir(base);
    remove_dir(std::filesystem::path{ cache }.parent_path());
    remove_dir(appdata);
}
