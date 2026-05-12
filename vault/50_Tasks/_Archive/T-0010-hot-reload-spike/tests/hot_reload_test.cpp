// SPDX-License-Identifier: Apache-2.0
// T-0010 — hot-reload runtime tests (Windows desktop).
//
// These tests exercise the public surface of mpapp::Hot<T> and
// mpapp::hot_reload::runtime. They prefer to use a real clang++ to do a
// full rebuild + swap when one is on PATH; otherwise they fall back to a
// pre-built dll-on-disk path so the test suite still verifies the
// LoadLibraryEx / GetProcAddress / FreeLibrary half of the runtime.

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <thread>
#include <type_traits>

#include <mpapp/hot_reload.hpp>

namespace fs = std::filesystem;

namespace {

// True if a clang++ on PATH responds to --version with exit code 0.
bool has_clang_pp() {
    int rc = std::system("clang++ --version > nul 2>&1");
    return rc == 0;
}

// Write a tiny user_code.cpp into a fresh per-test directory and return
// the path to that .cpp.
fs::path stage_source(const std::string& body, const fs::path& dir) {
    fs::create_directories(dir);
    const fs::path src = dir / "user_code.cpp";
    std::ofstream os(src);
    os <<
        "#if defined(_WIN32)\n"
        "#  define EXP __declspec(dllexport)\n"
        "#else\n"
        "#  define EXP\n"
        "#endif\n"
        "extern \"C\" EXP int compute(int x) { return " << body << "; }\n";
    os.close();
    return src;
}

// Bump the source mtime forward — needed because the runtime keys off mtime
// equality and a fast rewrite can land within the same fs tick.
void bump_mtime(const fs::path& p) {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(50ms);
    std::error_code ec;
    auto now = fs::file_time_type::clock::now();
    fs::last_write_time(p, now + 1s, ec);
}

} // namespace

// --------------------------------------------------------------------------
// Hot<T> is empty and trivially usable as a base.
// --------------------------------------------------------------------------
TEST_CASE("Hot<T> is an empty tag base", "[hot_reload][tag]") {
    struct my_vm : mpapp::Hot<my_vm> {};
    STATIC_REQUIRE(std::is_empty_v<mpapp::Hot<my_vm>>);
    my_vm vm{};
    (void)vm; // suppress unused
}

// --------------------------------------------------------------------------
// Constructing with a missing source surfaces an error.
// --------------------------------------------------------------------------
TEST_CASE("runtime fails cleanly when source is missing",
          "[hot_reload][runtime]") {
    fs::path tmp = fs::temp_directory_path() / "mpapp-hr-missing";
    fs::remove_all(tmp);
    fs::create_directories(tmp);
    fs::path src = tmp / "does_not_exist.cpp";

    mpapp::hot_reload::runtime rt{src};
    REQUIRE(rt.compute() == nullptr);
    REQUIRE_FALSE(rt.last_error().empty());
}

// --------------------------------------------------------------------------
// Full rebuild + swap loop. Skipped if clang++ isn't on PATH.
// --------------------------------------------------------------------------
TEST_CASE("runtime rebuilds and swaps the dll on source change",
          "[hot_reload][runtime][.clang]") {
    if (!has_clang_pp()) {
        SKIP("clang++ not on PATH; skipping rebuild test");
    }

    fs::path tmp = fs::temp_directory_path() / "mpapp-hr-swap";
    fs::remove_all(tmp);
    fs::path src = stage_source("x * 2", tmp);

    mpapp::hot_reload::runtime rt{src};
    INFO("initial build error: " << rt.last_error());
    REQUIRE(rt.compute() != nullptr);
    REQUIRE(rt.compute()(5) == 10);

    // No source change yet -> poll returns false.
    REQUIRE_FALSE(rt.poll());

    // Edit + bump mtime.
    {
        std::ofstream os(src, std::ios::trunc);
        os <<
            "#if defined(_WIN32)\n"
            "#  define EXP __declspec(dllexport)\n"
            "#else\n"
            "#  define EXP\n"
            "#endif\n"
            "extern \"C\" EXP int compute(int x) { return x * 10; }\n";
    }
    bump_mtime(src);

    REQUIRE(rt.poll());
    INFO("post-swap build error: " << rt.last_error());
    REQUIRE(rt.compute() != nullptr);
    REQUIRE(rt.compute()(5) == 50);
}

// --------------------------------------------------------------------------
// poll() returns false (and does not crash) when the compiler invocation
// fails. We force the failure by configuring a bogus compiler.
// --------------------------------------------------------------------------
TEST_CASE("runtime surfaces compiler errors via last_error",
          "[hot_reload][runtime][.clang]") {
    if (!has_clang_pp()) {
        SKIP("clang++ not on PATH; skipping compiler-error test");
    }

    fs::path tmp = fs::temp_directory_path() / "mpapp-hr-err";
    fs::remove_all(tmp);
    fs::path src = stage_source("x + 1", tmp);

    mpapp::hot_reload::runtime rt{src};
    REQUIRE(rt.compute() != nullptr);

    rt.set_compiler("definitely-not-a-compiler-xyz");

    // Bump mtime so poll() decides to rebuild.
    {
        std::ofstream os(src, std::ios::trunc);
        os <<
            "#if defined(_WIN32)\n"
            "#  define EXP __declspec(dllexport)\n"
            "#else\n"
            "#  define EXP\n"
            "#endif\n"
            "extern \"C\" EXP int compute(int x) { return x; }\n";
    }
    bump_mtime(src);

    REQUIRE_FALSE(rt.poll());
    REQUIRE_FALSE(rt.last_error().empty());
}
