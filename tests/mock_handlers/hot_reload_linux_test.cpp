// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Linux hot-reload runtime tests (src/hot_reload/linux.cpp).
//
// Exercises the dlopen-based runtime end-to-end: an initial build + load of
// a tiny user_code.cpp, then an edit + poll() that recompiles and swaps the
// .so in-process, with the freshly-loaded `compute` returning new behavior.
// The whole file is a no-op outside Linux desktop (the runtime impl is
// platform-specific).

#if defined(__linux__) && !defined(__ANDROID__)

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <thread>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/hot_reload.hpp>

namespace fs = std::filesystem;

namespace {

// First C++ compiler on PATH that answers --version, or "" if none.
std::string pick_compiler() {
    for (const char* c : {"c++", "g++", "clang++"}) {
        std::string probe = std::string{c} + " --version >/dev/null 2>&1";
        if (std::system(probe.c_str()) == 0) return c;
    }
    return {};
}

fs::path stage_source(const std::string& body, const fs::path& dir) {
    std::error_code ec;
    fs::create_directories(dir, ec);
    const fs::path src = dir / "user_code.cpp";
    std::ofstream os(src, std::ios::trunc);
    os << "extern \"C\" int compute(int x) { return " << body << "; }\n";
    return src;
}

void bump_mtime(const fs::path& p) {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(50ms);
    std::error_code ec;
    fs::last_write_time(p, fs::file_time_type::clock::now() + 1s, ec);
}

} // namespace

TEST_CASE("linux runtime fails cleanly when source is missing",
          "[hot_reload][runtime]") {
    fs::path tmp = fs::temp_directory_path() / "mpapp-hr-missing";
    std::error_code ec;
    fs::remove_all(tmp, ec);
    fs::create_directories(tmp, ec);

    mpapp::hot_reload::runtime rt{tmp / "does_not_exist.cpp"};
    REQUIRE(rt.compute() == nullptr);
    REQUIRE_FALSE(rt.last_error().empty());
}

TEST_CASE("linux runtime rebuilds and swaps the .so on source change",
          "[hot_reload][runtime]") {
    const std::string cc = pick_compiler();
    if (cc.empty()) {
        SKIP("no C++ compiler on PATH; skipping rebuild test");
    }

    fs::path tmp = fs::temp_directory_path() / "mpapp-hr-swap";
    std::error_code ec;
    fs::remove_all(tmp, ec);
    fs::path src = stage_source("x * 2", tmp);

    mpapp::hot_reload::runtime rt{src};
    rt.set_compiler(cc);
    // The ctor already built with the default compiler; if that default
    // wasn't present the load failed - rebuild once with the picked cc.
    if (rt.compute() == nullptr) {
        bump_mtime(src);
        rt.poll();
    }
    INFO("initial build error: " << rt.last_error());
    REQUIRE(rt.compute() != nullptr);
    REQUIRE(rt.compute()(5) == 10);

    // No change yet -> poll() is a no-op.
    REQUIRE_FALSE(rt.poll());

    // Edit the source + bump mtime, then poll to recompile + swap.
    { std::ofstream os(src, std::ios::trunc);
      os << "extern \"C\" int compute(int x) { return x * 10; }\n"; }
    bump_mtime(src);

    REQUIRE(rt.poll());
    INFO("post-swap build error: " << rt.last_error());
    REQUIRE(rt.compute() != nullptr);
    REQUIRE(rt.compute()(5) == 50);

    fs::remove_all(tmp, ec);
}

#endif // __linux__ && !__ANDROID__
