// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP.
//
// Unit tests for the mpapp CLI dispatcher.
//
// The dispatcher writes to caller-provided std::ostream sinks, so tests
// capture output via std::stringstream and assert on its contents without
// touching the real stdout/stderr.

#include "mpapp_cli/dispatch.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <sstream>
#include <string>
#include <vector>

namespace {

// Convert a list of string literals into a mutable argv array suitable for
// passing to dispatch(). The strings are owned by the caller; this helper
// just hands out the pointers.
struct argv_buf {
    std::vector<std::string> storage;
    std::vector<char*> ptrs;

    explicit argv_buf(std::initializer_list<const char*> args) {
        storage.reserve(args.size());
        ptrs.reserve(args.size());
        for (const char* a : args) {
            storage.emplace_back(a);
        }
        for (auto& s : storage) {
            ptrs.push_back(s.data());
        }
    }

    int argc() const { return static_cast<int>(ptrs.size()); }
    char** argv() { return ptrs.data(); }
};

struct dispatch_result {
    int code;
    std::string out;
    std::string err;
};

dispatch_result run(std::initializer_list<const char*> args) {
    argv_buf buf{args};
    std::stringstream out;
    std::stringstream err;
    int code = mpapp::cli::dispatch(buf.argc(), buf.argv(), out, err);
    return {code, out.str(), err.str()};
}

} // namespace

TEST_CASE("no args prints help and exits 0", "[dispatch]") {
    auto r = run({"mpapp"});
    REQUIRE(r.code == 0);
    REQUIRE(r.out.find("usage: mpapp") != std::string::npos);
    REQUIRE(r.out.find("build") != std::string::npos);
    REQUIRE(r.err.empty());
}

TEST_CASE("version flag prints version and exits 0", "[dispatch]") {
    auto r = run({"mpapp", "--version"});
    REQUIRE(r.code == 0);
    REQUIRE(r.out.find("mpapp") != std::string::npos);
    REQUIRE(r.err.empty());
}

TEST_CASE("help flag prints help and exits 0", "[dispatch]") {
    auto r = run({"mpapp", "--help"});
    REQUIRE(r.code == 0);
    REQUIRE(r.out.find("usage: mpapp") != std::string::npos);
}

TEST_CASE("help subcommand prints help and exits 0", "[dispatch]") {
    auto r = run({"mpapp", "help"});
    REQUIRE(r.code == 0);
    REQUIRE(r.out.find("usage: mpapp") != std::string::npos);
}

TEST_CASE("help <sub> prints per-subcommand usage", "[dispatch]") {
    auto r = run({"mpapp", "help", "build"});
    REQUIRE(r.code == 0);
    REQUIRE(r.out.find("mpapp build") != std::string::npos);
    REQUIRE(r.out.find("--target") != std::string::npos);
}

TEST_CASE("unknown subcommand exits 2 with error", "[dispatch]") {
    auto r = run({"mpapp", "foo"});
    REQUIRE(r.code == 2);
    REQUIRE(r.err.find("unknown command: foo") != std::string::npos);
    // Help is also dumped to err so the user sees the available commands.
    REQUIRE(r.err.find("usage: mpapp") != std::string::npos);
}

TEST_CASE("build with no flags reports host target", "[dispatch][build]") {
    auto r = run({"mpapp", "build"});
    REQUIRE(r.code == 0);
    REQUIRE(r.out.find("[mpapp build]") != std::string::npos);
    REQUIRE(r.out.find("not yet implemented") != std::string::npos);
    REQUIRE(r.out.find("target=<host>") != std::string::npos);
}

TEST_CASE("build --target windows-x64 recognized", "[dispatch][build]") {
    auto r = run({"mpapp", "build", "--target", "windows-x64"});
    REQUIRE(r.code == 0);
    REQUIRE(r.out.find("[mpapp build]") != std::string::npos);
    REQUIRE(r.out.find("target=windows-x64") != std::string::npos);
}

TEST_CASE("build --target without value exits 2", "[dispatch][build]") {
    auto r = run({"mpapp", "build", "--target"});
    REQUIRE(r.code == 2);
    REQUIRE(r.err.find("missing value for --target") != std::string::npos);
}

TEST_CASE("build with unknown flag exits 2", "[dispatch][build]") {
    auto r = run({"mpapp", "build", "--bogus"});
    REQUIRE(r.code == 2);
    REQUIRE(r.err.find("unknown argument") != std::string::npos);
}

TEST_CASE("new myapp recognizes name", "[dispatch][new]") {
    auto r = run({"mpapp", "new", "myapp"});
    REQUIRE(r.code == 0);
    REQUIRE(r.out.find("[mpapp new]") != std::string::npos);
    REQUIRE(r.out.find("name=myapp") != std::string::npos);
}

TEST_CASE("new without name exits 2", "[dispatch][new]") {
    auto r = run({"mpapp", "new"});
    REQUIRE(r.code == 2);
    REQUIRE(r.err.find("missing required") != std::string::npos);
    REQUIRE(r.err.find("usage: mpapp new") != std::string::npos);
}

TEST_CASE("new with extra positional exits 2", "[dispatch][new]") {
    auto r = run({"mpapp", "new", "a", "b"});
    REQUIRE(r.code == 2);
    REQUIRE(r.err.find("unexpected argument") != std::string::npos);
}

TEST_CASE("run --target android-arm64 recognized", "[dispatch][run]") {
    auto r = run({"mpapp", "run", "--target", "android-arm64"});
    REQUIRE(r.code == 0);
    REQUIRE(r.out.find("[mpapp run]") != std::string::npos);
    REQUIRE(r.out.find("target=android-arm64") != std::string::npos);
}

TEST_CASE("package with no flags reports host target", "[dispatch][package]") {
    auto r = run({"mpapp", "package"});
    REQUIRE(r.code == 0);
    REQUIRE(r.out.find("[mpapp package]") != std::string::npos);
    REQUIRE(r.out.find("target=<host>") != std::string::npos);
}

TEST_CASE("xaml-compile recognizes file and --out", "[dispatch][xaml]") {
    auto r = run({"mpapp", "xaml-compile", "foo.xaml", "--out", "bar.gen.hpp"});
    REQUIRE(r.code == 0);
    REQUIRE(r.out.find("file=foo.xaml") != std::string::npos);
    REQUIRE(r.out.find("out=bar.gen.hpp") != std::string::npos);
}

TEST_CASE("xaml-compile without file exits 2", "[dispatch][xaml]") {
    auto r = run({"mpapp", "xaml-compile"});
    REQUIRE(r.code == 2);
    REQUIRE(r.err.find("missing required") != std::string::npos);
}

TEST_CASE("xaml-compile with only --out and no file exits 2", "[dispatch][xaml]") {
    auto r = run({"mpapp", "xaml-compile", "--out", "bar.gen.hpp"});
    REQUIRE(r.code == 2);
    REQUIRE(r.err.find("missing required") != std::string::npos);
}

TEST_CASE("xaml-compile --out without value exits 2", "[dispatch][xaml]") {
    auto r = run({"mpapp", "xaml-compile", "foo.xaml", "--out"});
    REQUIRE(r.code == 2);
    REQUIRE(r.err.find("missing value for --out") != std::string::npos);
}
