// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Catch2 tests for mpapp::file_system / mock_file_system.

#include <optional>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/file_system.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// cache_directory
// ---------------------------------------------------------------------------

TEST_CASE("mock_file_system default cache_directory", "[mock][essentials][file_system]") {
    // Arrange
    mock_file_system fs;

    // Act
    const std::string dir = fs.cache_directory();

    // Assert — default is the constructor default ("/tmp/cache")
    CHECK(dir == "/tmp/cache");
}

TEST_CASE("mock_file_system set_cache_directory is reflected", "[mock][essentials][file_system]") {
    // Arrange
    mock_file_system fs;

    // Act
    fs.set_cache_directory("/var/cache/myapp");

    // Assert
    CHECK(fs.cache_directory() == "/var/cache/myapp");
}

TEST_CASE("mock_file_system cache_directory can be set to empty string",
          "[mock][essentials][file_system]") {
    // Arrange
    mock_file_system fs;

    // Act
    fs.set_cache_directory("");

    // Assert
    CHECK(fs.cache_directory().empty());
}

// ---------------------------------------------------------------------------
// app_data_directory
// ---------------------------------------------------------------------------

TEST_CASE("mock_file_system default app_data_directory", "[mock][essentials][file_system]") {
    // Arrange
    mock_file_system fs;

    // Act
    const std::string dir = fs.app_data_directory();

    // Assert — default is the constructor default ("/tmp/appdata")
    CHECK(dir == "/tmp/appdata");
}

TEST_CASE("mock_file_system set_app_data_directory is reflected",
          "[mock][essentials][file_system]") {
    // Arrange
    mock_file_system fs;

    // Act
    fs.set_app_data_directory("/home/user/.local/share/myapp");

    // Assert
    CHECK(fs.app_data_directory() == "/home/user/.local/share/myapp");
}

TEST_CASE("mock_file_system app_data_directory can be set to empty string",
          "[mock][essentials][file_system]") {
    // Arrange
    mock_file_system fs;

    // Act
    fs.set_app_data_directory("");

    // Assert
    CHECK(fs.app_data_directory().empty());
}

// ---------------------------------------------------------------------------
// open_app_package_file — found path
// ---------------------------------------------------------------------------

TEST_CASE("mock_file_system open_app_package_file returns registered contents",
          "[mock][essentials][file_system]") {
    // Arrange
    mock_file_system fs;
    fs.register_package_file("config.json", R"({"key":"value"})");

    // Act
    const auto result = fs.open_app_package_file("config.json");

    // Assert
    REQUIRE(result.has_value());
    CHECK(*result == R"({"key":"value"})");
}

TEST_CASE("mock_file_system open_app_package_file with binary-like contents",
          "[mock][essentials][file_system]") {
    // Arrange
    mock_file_system fs;
    const std::string data{ "\x00\x01\x02\x03", 4 };
    fs.register_package_file("data.bin", data);

    // Act
    const auto result = fs.open_app_package_file("data.bin");

    // Assert
    REQUIRE(result.has_value());
    CHECK(*result == data);
}

TEST_CASE("mock_file_system multiple package files coexist",
          "[mock][essentials][file_system]") {
    // Arrange
    mock_file_system fs;
    fs.register_package_file("a.txt", "AAA");
    fs.register_package_file("b.txt", "BBB");

    // Act + Assert
    const auto a = fs.open_app_package_file("a.txt");
    const auto b = fs.open_app_package_file("b.txt");
    REQUIRE(a.has_value());
    REQUIRE(b.has_value());
    CHECK(*a == "AAA");
    CHECK(*b == "BBB");
}

// ---------------------------------------------------------------------------
// open_app_package_file — not-found path
// ---------------------------------------------------------------------------

TEST_CASE("mock_file_system open_app_package_file returns nullopt for unknown file",
          "[mock][essentials][file_system]") {
    // Arrange
    mock_file_system fs;

    // Act
    const auto result = fs.open_app_package_file("does_not_exist.png");

    // Assert
    CHECK_FALSE(result.has_value());
}

TEST_CASE("mock_file_system open_app_package_file returns nullopt after unregister",
          "[mock][essentials][file_system]") {
    // Arrange
    mock_file_system fs;
    fs.register_package_file("splash.png", "PNG_DATA");
    REQUIRE(fs.open_app_package_file("splash.png").has_value());

    // Act
    fs.unregister_package_file("splash.png");

    // Assert
    CHECK_FALSE(fs.open_app_package_file("splash.png").has_value());
}

// ---------------------------------------------------------------------------
// last_open_request call-argument recorder
// ---------------------------------------------------------------------------

TEST_CASE("mock_file_system last_open_request is nullopt before any call",
          "[mock][essentials][file_system]") {
    // Arrange
    mock_file_system fs;

    // Act + Assert — no call yet
    CHECK_FALSE(fs.last_open_request().has_value());
}

TEST_CASE("mock_file_system last_open_request records successful open",
          "[mock][essentials][file_system]") {
    // Arrange
    mock_file_system fs;
    fs.register_package_file("icon.svg", "<svg/>");

    // Act — discard return; we only need the side-effect on last_open_request_
    static_cast<void>(fs.open_app_package_file("icon.svg"));

    // Assert
    REQUIRE(fs.last_open_request().has_value());
    CHECK(*fs.last_open_request() == "icon.svg");
}

TEST_CASE("mock_file_system last_open_request records failed open",
          "[mock][essentials][file_system]") {
    // Arrange
    mock_file_system fs;

    // Act — discard return; we only need the side-effect on last_open_request_
    static_cast<void>(fs.open_app_package_file("no_such_file.xml"));

    // Assert
    REQUIRE(fs.last_open_request().has_value());
    CHECK(*fs.last_open_request() == "no_such_file.xml");
}

TEST_CASE("mock_file_system last_open_request updates on each call",
          "[mock][essentials][file_system]") {
    // Arrange
    mock_file_system fs;
    fs.register_package_file("first.txt", "1");
    fs.register_package_file("second.txt", "2");

    // Act — discard returns; we only need the side-effect on last_open_request_
    static_cast<void>(fs.open_app_package_file("first.txt"));
    static_cast<void>(fs.open_app_package_file("second.txt"));

    // Assert — only the most recent request is stored
    REQUIRE(fs.last_open_request().has_value());
    CHECK(*fs.last_open_request() == "second.txt");
}

// ---------------------------------------------------------------------------
// Custom constructor paths
// ---------------------------------------------------------------------------

TEST_CASE("mock_file_system explicit constructor paths are used",
          "[mock][essentials][file_system]") {
    // Arrange + Act
    mock_file_system fs{ "/mnt/cache", "/mnt/data" };

    // Assert
    CHECK(fs.cache_directory()    == "/mnt/cache");
    CHECK(fs.app_data_directory() == "/mnt/data");
}

// ---------------------------------------------------------------------------
// Interface polymorphism
// ---------------------------------------------------------------------------

TEST_CASE("file_system interface pointer dispatches to mock impl",
          "[mock][essentials][file_system]") {
    // Arrange
    mock_file_system concrete{ "/c", "/d" };
    concrete.register_package_file("readme.txt", "hello");
    file_system& iface = concrete;

    // Act + Assert — virtual dispatch works correctly
    CHECK(iface.cache_directory()    == "/c");
    CHECK(iface.app_data_directory() == "/d");

    const auto result = iface.open_app_package_file("readme.txt");
    REQUIRE(result.has_value());
    CHECK(*result == "hello");

    CHECK_FALSE(iface.open_app_package_file("missing.txt").has_value());
}

// ---------------------------------------------------------------------------
// Re-registration (overwrite) behaviour
// ---------------------------------------------------------------------------

TEST_CASE("mock_file_system re-registering a file overwrites previous contents",
          "[mock][essentials][file_system]") {
    // Arrange
    mock_file_system fs;
    fs.register_package_file("data.txt", "v1");

    // Act
    fs.register_package_file("data.txt", "v2");
    const auto result = fs.open_app_package_file("data.txt");

    // Assert
    REQUIRE(result.has_value());
    CHECK(*result == "v2");
}

// ---------------------------------------------------------------------------
// const correctness
// ---------------------------------------------------------------------------

TEST_CASE("mock_file_system read methods are callable on const ref",
          "[mock][essentials][file_system]") {
    // Arrange
    mock_file_system fs{ "/cache", "/data" };
    fs.register_package_file("notes.txt", "content");
    const mock_file_system& cfs = fs;

    // Act + Assert — all [[nodiscard]] const methods reachable via const ref
    CHECK(cfs.cache_directory()    == "/cache");
    CHECK(cfs.app_data_directory() == "/data");

    const auto r = cfs.open_app_package_file("notes.txt");
    REQUIRE(r.has_value());
    CHECK(*r == "content");

    CHECK_FALSE(cfs.open_app_package_file("nope").has_value());
    REQUIRE(cfs.last_open_request().has_value());
    CHECK(*cfs.last_open_request() == "nope");
}
