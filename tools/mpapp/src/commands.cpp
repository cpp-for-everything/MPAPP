// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP.
//
// mpapp CLI subcommand implementations + the registry + the top-level
// dispatcher.
//
// `new`, `run`, and `package` are still stubs that echo their recognized
// arguments. `build` and `xaml-compile` invoke the real underlying tools
// (cmake, mpapp-xc). Both honor `MPAPP_CLI_DRY_RUN=1` in the environment
// to print the would-be invocation and exit 0 without spawning a child —
// the integration tests use this to assert on the constructed command
// without touching the host's real toolchain.

#include "mpapp_cli/commands.hpp"
#include "mpapp_cli/dispatch.hpp"

#include "process.hpp"

#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

#if defined(_WIN32)
#  include <windows.h>
#elif defined(__APPLE__)
#  include <mach-o/dyld.h>
#else
#  include <unistd.h>
#endif

namespace mpapp::cli {

namespace {

constexpr std::string_view kVersion = "mpapp 0.0.1";

// Look up a subcommand by name. Returns nullptr if not found.
const command* find_command(std::string_view name) {
    for (const auto& cmd : registry) {
        if (cmd.name == name) {
            return &cmd;
        }
    }
    return nullptr;
}

} // namespace

int print_version(std::ostream& out) {
    out << kVersion << '\n';
    return 0;
}

int print_help(std::string_view sub, std::ostream& out) {
    if (!sub.empty()) {
        if (const auto* cmd = find_command(sub)) {
            out << "usage: " << cmd->usage << '\n';
            out << "  " << cmd->summary << '\n';
            return 0;
        }
        out << "unknown command: " << sub << '\n';
        // Fall through and print the global help so the user can see what's
        // available.
    }

    out << "usage: mpapp <command> [<args>]\n";
    out << "\nCommands:\n";
    constexpr std::size_t column = 16;
    for (const auto& cmd : registry) {
        const std::size_t pad =
            cmd.name.size() < column ? column - cmd.name.size() : 1;
        out << "  " << cmd.name << std::string(pad, ' ') << cmd.summary << '\n';
    }
    out << "\nFlags:\n";
    out << "  --version       Print the mpapp version.\n";
    out << "  --help          Print this help.\n";
    return 0;
}

int dispatch(int argc, char** argv, std::ostream& out, std::ostream& err) {
    if (argc < 2) {
        return print_help({}, out);
    }

    const std::string_view first = argv[1];

    if (first == "--version" || first == "-V") {
        return print_version(out);
    }

    if (first == "--help" || first == "-h" || first == "help") {
        // `help <sub>` — per-subcommand help.
        std::string_view sub;
        if (argc >= 3) {
            sub = argv[2];
        }
        return print_help(sub, out);
    }

    if (const auto* cmd = find_command(first)) {
        // Hand the rest of argv to the subcommand. argv[0] becomes the
        // subcommand name; argv[1..argc-1] are its arguments.
        args_span sub_args{};
        sub_args.argc = argc - 1;
        sub_args.argv = argv + 1;
        return cmd->run(sub_args, out, err);
    }

    err << "unknown command: " << first << '\n';
    print_help({}, err);
    return 2;
}

namespace commands {

namespace fs = std::filesystem;

namespace {

// Returns true and advances `i` if argv[i] equals `flag`. Reads the value
// from argv[i+1] into `value`; reports the missing-value error to `err` and
// returns false if no value follows.
bool consume_value_flag(args_span args,
                        int& i,
                        std::string_view flag,
                        std::string& value,
                        std::ostream& err) {
    if (std::string_view{args.argv[i]} != flag) {
        return false;
    }
    if (i + 1 >= args.argc) {
        err << "missing value for " << flag << '\n';
        return false;
    }
    value = args.argv[i + 1];
    i += 1; // The outer loop bumps once more.
    return true;
}

bool dry_run_enabled() {
#if defined(_WIN32)
    char* buf = nullptr;
    std::size_t len = 0;
    if (_dupenv_s(&buf, &len, "MPAPP_CLI_DRY_RUN") != 0 || !buf) {
        return false;
    }
    const bool enabled = std::string_view{buf} == "1";
    std::free(buf);
    return enabled;
#else
    const char* v = std::getenv("MPAPP_CLI_DRY_RUN");
    return v != nullptr && std::string_view{v} == "1";
#endif
}

// Returns true iff `cmake_lists` contains a `project(mpapp...)` invocation.
// We scan up to a small fixed prefix of the file — the project() call is
// always within the first few lines of a CMake root file — and match the
// project name case-insensitively. Used by find_repo_root to disambiguate
// sibling repos that might also have a CMakeLists.txt.
bool looks_like_mpapp_root(const fs::path& cmake_lists) {
    std::ifstream in(cmake_lists);
    if (!in) return false;
    std::string line;
    int scanned = 0;
    while (scanned < 50 && std::getline(in, line)) {
        ++scanned;
        std::size_t s = line.find_first_not_of(" \t\r\n");
        if (s == std::string::npos) continue;
        std::string_view trimmed{line};
        trimmed.remove_prefix(s);
        if (trimmed.starts_with('#')) continue;
        static constexpr std::string_view needle = "project(mpapp";
        if (trimmed.size() < needle.size()) continue;
        bool match = true;
        for (std::size_t i = 0; i < needle.size(); ++i) {
            char lhs = trimmed[i];
            if (lhs >= 'A' && lhs <= 'Z') lhs = static_cast<char>(lhs + 32);
            if (lhs != needle[i]) { match = false; break; }
        }
        if (match) return true;
    }
    return false;
}

void echo_command(const std::vector<std::string>& args, std::ostream& out) {
    for (std::size_t i = 0; i < args.size(); ++i) {
        if (i) out << ' ';
        out << args[i];
    }
    out << '\n';
}

} // namespace

fs::path mpapp_exe_dir() {
#if defined(_WIN32)
    wchar_t buf[MAX_PATH + 1] = {};
    const DWORD n = GetModuleFileNameW(nullptr, buf, MAX_PATH);
    if (n == 0 || n == MAX_PATH) return {};
    return fs::path{buf}.parent_path();
#elif defined(__APPLE__)
    char buf[4096];
    uint32_t size = sizeof(buf);
    if (_NSGetExecutablePath(buf, &size) != 0) return {};
    std::error_code ec;
    fs::path p = fs::weakly_canonical(fs::path{buf}, ec);
    if (ec) p = fs::path{buf};
    return p.parent_path();
#else
    std::error_code ec;
    fs::path p = fs::read_symlink("/proc/self/exe", ec);
    if (ec) return {};
    return p.parent_path();
#endif
}

std::optional<fs::path> find_repo_root() {
    fs::path start = mpapp_exe_dir();
    if (start.empty()) {
        std::error_code ec;
        start = fs::current_path(ec);
        if (ec) return std::nullopt;
    }
    std::error_code ec;
    fs::path dir = fs::weakly_canonical(start, ec);
    if (ec) dir = start;

    for (;;) {
        const fs::path candidate = dir / "CMakeLists.txt";
        if (fs::exists(candidate, ec) && looks_like_mpapp_root(candidate)) {
            return dir;
        }
        const fs::path parent = dir.parent_path();
        if (parent.empty() || parent == dir) {
            return std::nullopt;
        }
        dir = parent;
    }
}

int run_new(args_span args, std::ostream& out, std::ostream& err) {
    // Required positional: <name>.
    std::string name;
    for (int i = 1; i < args.argc; ++i) {
        std::string_view a = args.argv[i];
        if (a.starts_with('-')) {
            err << "unknown flag: " << a << '\n';
            return 2;
        }
        if (!name.empty()) {
            err << "unexpected argument: " << a << '\n';
            return 2;
        }
        name = a;
    }
    if (name.empty()) {
        err << "mpapp new: missing required <name>\n";
        err << "usage: mpapp new <name>\n";
        return 2;
    }
    out << "[mpapp new] not yet implemented (got: name=" << name << ")\n";
    return 0;
}

namespace {

// run/package still stub out to a "[mpapp <name>] not yet implemented" line.
int run_target_only_stub(std::string_view name,
                         args_span args,
                         std::ostream& out,
                         std::ostream& err) {
    std::string target;
    for (int i = 1; i < args.argc; ++i) {
        std::string_view a = args.argv[i];
        if (a == "--target") {
            if (!consume_value_flag(args, i, "--target", target, err)) {
                return 2;
            }
            continue;
        }
        err << "unknown argument: " << a << '\n';
        return 2;
    }
    out << "[mpapp " << name << "] not yet implemented (got: target="
        << (target.empty() ? "<host>" : target) << ")\n";
    return 0;
}

} // namespace

int run_build(args_span args, std::ostream& out, std::ostream& err) {
    std::string target;
    std::string config = "Debug";
    std::string build_dir;
    for (int i = 1; i < args.argc; ++i) {
        std::string_view a = args.argv[i];
        if (a == "--target") {
            if (!consume_value_flag(args, i, "--target", target, err)) {
                return 2;
            }
            continue;
        }
        if (a == "--config") {
            if (!consume_value_flag(args, i, "--config", config, err)) {
                return 2;
            }
            continue;
        }
        if (a == "--build-dir") {
            if (!consume_value_flag(args, i, "--build-dir", build_dir, err)) {
                return 2;
            }
            continue;
        }
        err << "unknown argument: " << a << '\n';
        return 2;
    }

    // Default build dir depends on whether a target was specified.
    if (build_dir.empty()) {
        build_dir = target.empty() ? "build" : "build-" + target;
    }

    // Resolve the repo root so we can hand cmake an explicit -S. We need it
    // anyway for the toolchain lookup.
    const auto repo_root = find_repo_root();
    if (!repo_root) {
        err << "mpapp build: could not locate repo root "
               "(no CMakeLists.txt with project(mpapp) up the tree)\n";
        return 2;
    }

    std::string toolchain_file;
    if (!target.empty()) {
        const fs::path tc =
            *repo_root / "cmake" / "toolchains" / (target + ".cmake");
        std::error_code ec;
        if (!fs::exists(tc, ec)) {
            err << "mpapp build: toolchain file not found: " << tc.string()
                << "\n";
            err << "  expected at cmake/toolchains/" << target << ".cmake "
                   "relative to repo root\n";
            return 2;
        }
        toolchain_file = tc.string();
    }

    // Compose the configure command.
    std::vector<std::string> configure = {
        "cmake",
        "-S", repo_root->string(),
        "-B", build_dir,
    };
    if (!toolchain_file.empty()) {
        configure.push_back("-DCMAKE_TOOLCHAIN_FILE=" + toolchain_file);
    }
    configure.push_back("-DCMAKE_BUILD_TYPE=" + config);

    std::vector<std::string> build = {
        "cmake", "--build", build_dir,
    };

    if (dry_run_enabled()) {
        out << "[mpapp build] dry run\n";
        echo_command(configure, out);
        echo_command(build, out);
        return 0;
    }

    if (int rc = process::spawn(configure, nullptr); rc != 0) {
        err << "mpapp build: cmake configure failed (exit " << rc << ")\n";
        return rc < 0 ? 1 : rc;
    }
    if (int rc = process::spawn(build, nullptr); rc != 0) {
        err << "mpapp build: cmake --build failed (exit " << rc << ")\n";
        return rc < 0 ? 1 : rc;
    }
    return 0;
}

int run_run(args_span args, std::ostream& out, std::ostream& err) {
    return run_target_only_stub("run", args, out, err);
}

int run_package(args_span args, std::ostream& out, std::ostream& err) {
    return run_target_only_stub("package", args, out, err);
}

int run_xaml_compile(args_span args, std::ostream& out, std::ostream& err) {
    std::string file;
    std::string out_path;
    for (int i = 1; i < args.argc; ++i) {
        std::string_view a = args.argv[i];
        if (a == "--out") {
            if (!consume_value_flag(args, i, "--out", out_path, err)) {
                return 2;
            }
            continue;
        }
        if (a.starts_with('-')) {
            err << "unknown flag: " << a << '\n';
            return 2;
        }
        if (!file.empty()) {
            err << "unexpected argument: " << a << '\n';
            return 2;
        }
        file = a;
    }
    if (file.empty()) {
        err << "mpapp xaml-compile: missing required <file>\n";
        err << "usage: mpapp xaml-compile <file> [--out <path>]\n";
        return 2;
    }

    // Dry-run echoes the bare tool name so assertions stay portable across
    // build dirs; the real invocation needs the absolute path we resolved.
    if (dry_run_enabled()) {
        std::vector<std::string> argv = {"mpapp-xc", file};
        if (!out_path.empty()) {
            argv.push_back("--out");
            argv.push_back(out_path);
        }
        out << "[mpapp xaml-compile] dry run\n";
        echo_command(argv, out);
        return 0;
    }

    // Locate mpapp-xc: prefer the sibling exe next to the running mpapp,
    // then fall back to PATH.
    std::string xc_path;
    if (const fs::path exe_dir = mpapp_exe_dir(); !exe_dir.empty()) {
#if defined(_WIN32)
        const fs::path sibling = exe_dir / "mpapp-xc.exe";
#else
        const fs::path sibling = exe_dir / "mpapp-xc";
#endif
        std::error_code ec;
        if (fs::exists(sibling, ec) && !fs::is_directory(sibling, ec)) {
            xc_path = sibling.string();
        }
    }
    if (xc_path.empty()) {
        xc_path = process::which("mpapp-xc");
    }
    if (xc_path.empty()) {
        err << "mpapp xaml-compile: could not locate mpapp-xc; "
               "build it first or add to PATH\n";
        return 2;
    }

    std::vector<std::string> argv = {xc_path, file};
    if (!out_path.empty()) {
        argv.push_back("--out");
        argv.push_back(out_path);
    }
    const int rc = process::spawn(argv, nullptr);
    if (rc < 0) {
        err << "mpapp xaml-compile: failed to spawn mpapp-xc\n";
        return 1;
    }
    return rc;
}

int run_help(args_span args, std::ostream& out, std::ostream& /*err*/) {
    std::string_view sub;
    if (args.argc >= 2) {
        sub = args.argv[1];
    }
    return print_help(sub, out);
}

} // namespace commands

// Definition of the registry. The size in the array template argument must
// match the count of entries below; the declaration in dispatch.hpp pins
// the same size at compile time.
const std::array<command, 6> registry = {{
    {"new",
     "Create a new MPAPP app from a template.",
     "mpapp new <name>",
     &commands::run_new},
    {"build",
     "Build for the current host (or --target to cross-compile).",
     "mpapp build [--target <triple>] [--config Debug|Release] "
     "[--build-dir <path>]",
     &commands::run_build},
    {"run",
     "Build + launch on host (or emulator).",
     "mpapp run [--target <triple>]",
     &commands::run_run},
    {"package",
     "Produce a distributable artifact.",
     "mpapp package [--target <triple>]",
     &commands::run_package},
    {"xaml-compile",
     "Run mpapp-xc on a single file.",
     "mpapp xaml-compile <file> [--out <path>]",
     &commands::run_xaml_compile},
    {"help",
     "Print help (or per-subcommand help).",
     "mpapp help [<subcommand>]",
     &commands::run_help},
}};

} // namespace mpapp::cli
