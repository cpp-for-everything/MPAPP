// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Tests for the real file-backed preferences backend
// (RFC-0013 Essentials). Verifies persistence across instances + the
// typed accessors + escaping.

#include <filesystem>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/file_preferences.hpp>

namespace {

std::filesystem::path temp_pref_file(const char* tag) {
    auto p = std::filesystem::temp_directory_path() /
             (std::string{"mpapp-prefs-test-"} + tag + ".conf");
    std::error_code ec;
    std::filesystem::remove(p, ec);
    return p;
}

} // namespace

TEST_CASE("file_preferences persists across instances", "[mock][essentials][preferences]") {
    const auto path = temp_pref_file("persist");
    {
        mpapp::file_preferences prefs{path};
        prefs.set("faculty_number", std::string{"201221001"});
        prefs.set("remember", true);
        prefs.set("count", 8L);
    }
    {
        mpapp::file_preferences reloaded{path};
        CHECK(reloaded.contains("faculty_number"));
        CHECK(reloaded.get("faculty_number", std::string{"?"}) == "201221001");
        CHECK(reloaded.get("remember", false) == true);
        CHECK(reloaded.get("count", 0L) == 8L);
        CHECK(reloaded.get("missing", std::string{"def"}) == "def");
    }
    std::error_code ec;
    std::filesystem::remove(path, ec);
}

TEST_CASE("file_preferences round-trips multiline + escaped values",
          "[mock][essentials][preferences]") {
    const auto path = temp_pref_file("escape");
    const std::string tricky = "line1\nline2 with = and \\ backslash";
    {
        mpapp::file_preferences prefs{path};
        prefs.set("note", tricky);
    }
    {
        mpapp::file_preferences reloaded{path};
        CHECK(reloaded.get("note", std::string{}) == tricky);
    }
    std::error_code ec;
    std::filesystem::remove(path, ec);
}

TEST_CASE("file_preferences remove + clear persist", "[mock][essentials][preferences]") {
    const auto path = temp_pref_file("removeclear");
    {
        mpapp::file_preferences prefs{path};
        prefs.set("a", std::string{"1"});
        prefs.set("b", std::string{"2"});
        prefs.remove("a");
    }
    {
        mpapp::file_preferences reloaded{path};
        CHECK_FALSE(reloaded.contains("a"));
        CHECK(reloaded.contains("b"));
        reloaded.clear();
    }
    {
        mpapp::file_preferences reloaded2{path};
        CHECK_FALSE(reloaded2.contains("b"));
    }
    std::error_code ec;
    std::filesystem::remove(path, ec);
}
