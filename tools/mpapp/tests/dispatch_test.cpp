// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP.
//
// Unit + integration tests for the mpapp CLI dispatcher.
//
// The dispatcher writes to caller-provided std::ostream sinks, so tests
// capture output via std::stringstream and assert on its contents without
// touching the real stdout/stderr. For the `build` and `xaml-compile`
// subcommands we exercise the dry-run path (`MPAPP_CLI_DRY_RUN=1`) so
// the suite never actually spawns cmake or mpapp-xc.

#include "mpapp_cli/dispatch.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdlib>
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

// RAII scope guard that sets MPAPP_CLI_DRY_RUN=1 for the lifetime of the
// test case and restores the prior value on destruction. The build and
// xaml-compile integration tests use this so they never actually spawn
// cmake or mpapp-xc.
class dry_run_scope {
   public:
    dry_run_scope() {
#if defined(_WIN32)
        char* buf = nullptr;
        std::size_t len = 0;
        if (_dupenv_s(&buf, &len, "MPAPP_CLI_DRY_RUN") == 0 && buf) {
            had_prior_ = true;
            prior_ = buf;
            std::free(buf);
        }
        _putenv_s("MPAPP_CLI_DRY_RUN", "1");
#else
        const char* prior = std::getenv("MPAPP_CLI_DRY_RUN");
        had_prior_ = prior != nullptr;
        if (had_prior_) prior_ = prior;
        ::setenv("MPAPP_CLI_DRY_RUN", "1", 1);
#endif
    }
    ~dry_run_scope() {
#if defined(_WIN32)
        if (had_prior_) {
            _putenv_s("MPAPP_CLI_DRY_RUN", prior_.c_str());
        } else {
            _putenv_s("MPAPP_CLI_DRY_RUN", "");
        }
#else
        if (had_prior_) {
            ::setenv("MPAPP_CLI_DRY_RUN", prior_.c_str(), 1);
        } else {
            ::unsetenv("MPAPP_CLI_DRY_RUN");
        }
#endif
    }
    dry_run_scope(const dry_run_scope&) = delete;
    dry_run_scope& operator=(const dry_run_scope&) = delete;

   private:
    bool had_prior_ = false;
    std::string prior_;
};

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

TEST_CASE("build dry-run with no flags uses host default", "[dispatch][build]") {
    dry_run_scope dry;
    auto r = run({"mpapp", "build"});
    REQUIRE(r.code == 0);
    REQUIRE(r.out.find("cmake -S") != std::string::npos);
    REQUIRE(r.out.find("-B build") != std::string::npos);
    REQUIRE(r.out.find("cmake --build build") != std::string::npos);
    // No --target → no toolchain file flag.
    REQUIRE(r.out.find("CMAKE_TOOLCHAIN_FILE") == std::string::npos);
    // Default config is Debug.
    REQUIRE(r.out.find("-DCMAKE_BUILD_TYPE=Debug") != std::string::npos);
}

TEST_CASE("build dry-run with --target uses toolchain file", "[dispatch][build]") {
    dry_run_scope dry;
    auto r = run({"mpapp", "build", "--target", "windows-x64"});
    REQUIRE(r.code == 0);
    REQUIRE(r.out.find("cmake -S") != std::string::npos);
    REQUIRE(r.out.find("-B build-windows-x64") != std::string::npos);
    REQUIRE(r.out.find("-DCMAKE_TOOLCHAIN_FILE=") != std::string::npos);
    REQUIRE(r.out.find("windows-x64.cmake") != std::string::npos);
    REQUIRE(r.out.find("cmake --build build-windows-x64") != std::string::npos);
}

TEST_CASE("build with missing toolchain file exits 2", "[dispatch][build]") {
    dry_run_scope dry;
    auto r = run({"mpapp", "build", "--target", "nonexistent-triple"});
    REQUIRE(r.code == 2);
    REQUIRE(r.err.find("toolchain file not found") != std::string::npos);
    REQUIRE(r.err.find("nonexistent-triple") != std::string::npos);
}

TEST_CASE("build dry-run with --config Release propagates", "[dispatch][build]") {
    dry_run_scope dry;
    auto r = run({"mpapp", "build", "--config", "Release"});
    REQUIRE(r.code == 0);
    REQUIRE(r.out.find("-DCMAKE_BUILD_TYPE=Release") != std::string::npos);
}

TEST_CASE("build dry-run with --build-dir overrides default", "[dispatch][build]") {
    dry_run_scope dry;
    auto r = run({"mpapp", "build", "--build-dir", "out/x"});
    REQUIRE(r.code == 0);
    REQUIRE(r.out.find("-B out/x") != std::string::npos);
    REQUIRE(r.out.find("cmake --build out/x") != std::string::npos);
}

TEST_CASE("build --target without value exits 2", "[dispatch][build]") {
    auto r = run({"mpapp", "build", "--target"});
    REQUIRE(r.code == 2);
    REQUIRE(r.err.find("missing value for --target") != std::string::npos);
}

TEST_CASE("build --config without value exits 2", "[dispatch][build]") {
    auto r = run({"mpapp", "build", "--config"});
    REQUIRE(r.code == 2);
    REQUIRE(r.err.find("missing value for --config") != std::string::npos);
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

TEST_CASE("xaml-compile dry-run echoes invocation", "[dispatch][xaml]") {
    dry_run_scope dry;
    auto r = run({"mpapp", "xaml-compile", "foo.xaml", "--out", "bar.gen.hpp"});
    REQUIRE(r.code == 0);
    REQUIRE(r.out.find("mpapp-xc foo.xaml --out bar.gen.hpp") !=
            std::string::npos);
}

TEST_CASE("xaml-compile dry-run without --out", "[dispatch][xaml]") {
    dry_run_scope dry;
    auto r = run({"mpapp", "xaml-compile", "foo.xaml"});
    REQUIRE(r.code == 0);
    REQUIRE(r.out.find("mpapp-xc foo.xaml") != std::string::npos);
    REQUIRE(r.out.find("--out") == std::string::npos);
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
